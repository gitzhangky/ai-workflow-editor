#include "qtnodes/QtNodesEditorWidget.hpp"

#include "app/NodeLibraryListWidget.hpp"
#include "qtnodes/EdgeAlignedNodeGeometry.hpp"
#include "qtnodes/StaticNodeDelegateModel.hpp"
#include "qtnodes/StyledNodePainter.hpp"

#include <QtNodes/internal/ConnectionGraphicsObject.hpp>
#include <QtNodes/internal/locateNode.hpp>
#include <QtNodes/internal/NodeConnectionInteraction.hpp>
#include <QtNodes/ConnectionStyle>
#include <QtNodes/DataFlowGraphModel>
#include <QtNodes/DataFlowGraphicsScene>
#include <QtNodes/GraphicsView>
#include <QtNodes/GraphicsViewStyle>
#include <QtNodes/NodeDelegateModelRegistry>
#include <QtNodes/NodeStyle>
#include <QtNodes/StyleCollection>

#include <QAction>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QEvent>
#include <QFile>
#include <QFrame>
#include <QGraphicsItem>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>
#include <QPointF>
#include <QShortcut>
#include <QTimer>
#include <QUndoCommand>
#include <QUndoStack>
#include <QVBoxLayout>

#include <algorithm>
#include <unordered_set>

class NodeCreateCommand : public QUndoCommand
{
public:
    NodeCreateCommand(QtNodesEditorWidget *editor, QString typeKey, QPointF scenePosition)
        : _editor(editor)
        , _typeKey(std::move(typeKey))
        , _scenePosition(scenePosition)
    {}

    QtNodes::NodeId nodeId() const
    {
        return _snapshot.has_value() ? _snapshot->nodeId : QtNodes::InvalidNodeId;
    }

    void undo() override
    {
        if (_editor == nullptr || !_snapshot.has_value())
            return;

        _snapshot = _editor->snapshotNode(_snapshot->nodeId);
        _editor->deleteNodeInternal(_snapshot->nodeId);
        _editor->refreshSelectedNodeState();
        Q_EMIT _editor->workflowModified();
    }

    void redo() override
    {
        if (_editor == nullptr)
            return;

        if (_snapshot.has_value())
            _snapshot->nodeId = _editor->restoreNodeInternal(*_snapshot);
        else {
            const auto nodeId = _editor->createNodeInternal(_typeKey, _scenePosition);
            _snapshot = _editor->snapshotNode(nodeId);
        }

        _editor->selectNode(_snapshot->nodeId);
        Q_EMIT _editor->workflowModified();
    }

private:
    QtNodesEditorWidget *_editor;
    QString _typeKey;
    QPointF _scenePosition;
    std::optional<QtNodesEditorWidget::NodeSnapshot> _snapshot;
};

class NodeDeleteSelectionCommand : public QUndoCommand
{
public:
    explicit NodeDeleteSelectionCommand(QtNodesEditorWidget *editor, bool includeSelectedNodes)
        : _editor(editor)
    {
        if (_editor == nullptr)
            return;

        if (includeSelectedNodes) {
            for (auto const nodeId : _editor->selectedNodeIds()) {
                _nodeSnapshots.push_back(_editor->snapshotNode(nodeId));
                for (auto const &connectionId : _editor->_graphModel->allConnectionIds(nodeId))
                    _connectionIds.insert(connectionId);
            }
        }

        for (auto *item : _editor->_scene->selectedItems()) {
            auto *connectionObject = dynamic_cast<QtNodes::ConnectionGraphicsObject *>(item);
            if (connectionObject != nullptr)
                _connectionIds.insert(connectionObject->connectionId());
        }

        if (_nodeSnapshots.empty() && _connectionIds.empty())
            setObsolete(true);
    }

    void undo() override
    {
        if (_editor == nullptr)
            return;

        for (auto const &snapshot : _nodeSnapshots)
            _editor->restoreNodeInternal(snapshot);

        for (auto const &connectionId : _connectionIds) {
            if (!_editor->_graphModel->connectionExists(connectionId)
                && _editor->_graphModel->connectionPossible(connectionId)) {
                _editor->_graphModel->addConnection(connectionId);
            }
        }

        for (auto const &connectionId : _connectionIds)
            _editor->refreshValidationForConnectionEndpoints(connectionId);

        if (!_nodeSnapshots.empty())
            _editor->selectNode(_nodeSnapshots.front().nodeId);
        else
            _editor->refreshSelectedNodeState();

        Q_EMIT _editor->workflowModified();
    }

    void redo() override
    {
        if (_editor == nullptr)
            return;

        for (auto const &connectionId : _connectionIds) {
            if (_editor->_graphModel->connectionExists(connectionId))
                _editor->_graphModel->deleteConnection(connectionId);
        }

        for (auto const &snapshot : _nodeSnapshots) {
            if (_editor->_graphModel->nodeExists(snapshot.nodeId))
                _editor->deleteNodeInternal(snapshot.nodeId);
        }

        for (auto const &connectionId : _connectionIds)
            _editor->refreshValidationForConnectionEndpoints(connectionId);

        _editor->refreshSelectedNodeState();
        Q_EMIT _editor->workflowModified();
    }

private:
    QtNodesEditorWidget *_editor;
    std::vector<QtNodesEditorWidget::NodeSnapshot> _nodeSnapshots;
    std::unordered_set<QtNodes::ConnectionId> _connectionIds;
};

class ConnectionCreateCommand : public QUndoCommand
{
public:
    ConnectionCreateCommand(QtNodesEditorWidget *editor, QtNodes::ConnectionId connectionId)
        : _editor(editor)
        , _connectionId(connectionId)
    {}

    void undo() override
    {
        if (_editor == nullptr || !_editor->_graphModel->connectionExists(_connectionId))
            return;

        _editor->_graphModel->deleteConnection(_connectionId);
        _editor->refreshValidationForConnectionEndpoints(_connectionId);
        Q_EMIT _editor->workflowModified();
    }

    void redo() override
    {
        if (_editor == nullptr || _editor->_graphModel->connectionExists(_connectionId)
            || !_editor->_graphModel->connectionPossible(_connectionId)) {
            return;
        }

        _editor->_graphModel->addConnection(_connectionId);
        _editor->refreshValidationForConnectionEndpoints(_connectionId);
        Q_EMIT _editor->workflowModified();
    }

private:
    QtNodesEditorWidget *_editor;
    QtNodes::ConnectionId _connectionId;
};

class NodeDisplayNameEditCommand : public QUndoCommand
{
public:
    NodeDisplayNameEditCommand(QtNodesEditorWidget *editor,
                               QtNodes::NodeId nodeId,
                               QString oldValue,
                               QString newValue)
        : _editor(editor)
        , _nodeId(nodeId)
        , _oldValue(std::move(oldValue))
        , _newValue(std::move(newValue))
    {}

    int id() const override { return 1001; }

    bool mergeWith(QUndoCommand const *other) override
    {
        auto const *command = dynamic_cast<NodeDisplayNameEditCommand const *>(other);
        if (command == nullptr || command->_nodeId != _nodeId)
            return false;

        _newValue = command->_newValue;
        return true;
    }

    void undo() override
    {
        if (_editor != nullptr)
            _editor->setNodeDisplayNameInternal(_nodeId, _oldValue);
    }

    void redo() override
    {
        if (_editor != nullptr)
            _editor->setNodeDisplayNameInternal(_nodeId, _newValue);
    }

private:
    QtNodesEditorWidget *_editor;
    QtNodes::NodeId _nodeId;
    QString _oldValue;
    QString _newValue;
};

class NodeDescriptionEditCommand : public QUndoCommand
{
public:
    NodeDescriptionEditCommand(QtNodesEditorWidget *editor,
                               QtNodes::NodeId nodeId,
                               QString oldValue,
                               QString newValue)
        : _editor(editor)
        , _nodeId(nodeId)
        , _oldValue(std::move(oldValue))
        , _newValue(std::move(newValue))
    {}

    int id() const override { return 1002; }

    bool mergeWith(QUndoCommand const *other) override
    {
        auto const *command = dynamic_cast<NodeDescriptionEditCommand const *>(other);
        if (command == nullptr || command->_nodeId != _nodeId)
            return false;

        _newValue = command->_newValue;
        return true;
    }

    void undo() override
    {
        if (_editor != nullptr)
            _editor->setNodeDescriptionInternal(_nodeId, _oldValue);
    }

    void redo() override
    {
        if (_editor != nullptr)
            _editor->setNodeDescriptionInternal(_nodeId, _newValue);
    }

private:
    QtNodesEditorWidget *_editor;
    QtNodes::NodeId _nodeId;
    QString _oldValue;
    QString _newValue;
};

class NodePropertyEditCommand : public QUndoCommand
{
public:
    NodePropertyEditCommand(QtNodesEditorWidget *editor,
                            QtNodes::NodeId nodeId,
                            QString propertyKey,
                            QVariant oldValue,
                            QVariant newValue)
        : _editor(editor)
        , _nodeId(nodeId)
        , _propertyKey(std::move(propertyKey))
        , _oldValue(std::move(oldValue))
        , _newValue(std::move(newValue))
    {}

    int id() const override { return 1003; }

    bool mergeWith(QUndoCommand const *other) override
    {
        auto const *command = dynamic_cast<NodePropertyEditCommand const *>(other);
        if (command == nullptr || command->_nodeId != _nodeId || command->_propertyKey != _propertyKey)
            return false;

        _newValue = command->_newValue;
        return true;
    }

    void undo() override
    {
        if (_editor != nullptr)
            _editor->setNodePropertyInternal(_nodeId, _propertyKey, _oldValue);
    }

    void redo() override
    {
        if (_editor != nullptr)
            _editor->setNodePropertyInternal(_nodeId, _propertyKey, _newValue);
    }

private:
    QtNodesEditorWidget *_editor;
    QtNodes::NodeId _nodeId;
    QString _propertyKey;
    QVariant _oldValue;
    QVariant _newValue;
};

QtNodesEditorWidget::QtNodesEditorWidget(QWidget *parent)
    : QWidget(parent)
    , _registry(buildRegistry())
    , _graphModel(std::make_unique<QtNodes::DataFlowGraphModel>(_registry))
    , _scene(new QtNodes::DataFlowGraphicsScene(*_graphModel, this))
    , _view(new QtNodes::GraphicsView(_scene))
    , _dropPreview(new QFrame(_view->viewport()))
    , _selectedNodeId(QtNodes::InvalidNodeId)
    , _connectionFeedbackActive(false)
{
    setObjectName("workflowCanvas");
    applyWorkbenchStyles();
    setAcceptDrops(true);
    _view->setAcceptDrops(true);
    _view->viewport()->setAcceptDrops(true);
    _view->viewport()->installEventFilter(this);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(_view);

    _scene->setNodeGeometry(std::make_unique<EdgeAlignedNodeGeometry>(*_graphModel));
    _scene->setNodePainter(std::make_unique<StyledNodePainter>());

    _dropPreview->setObjectName("dropPreviewIndicator");
    _dropPreview->setAttribute(Qt::WA_TransparentForMouseEvents);
    _dropPreview->resize(180, 84);
    _dropPreview->hide();

    connect(_scene, &QtNodes::BasicGraphicsScene::nodeSelected, this, [this](QtNodes::NodeId nodeId) {
        handleNodeSelected(nodeId);
    });
    connect(&_scene->undoStack(), &QUndoStack::cleanChanged, this, &QtNodesEditorWidget::cleanStateChanged);
    connect(&_scene->undoStack(), &QUndoStack::canUndoChanged, this, [this](bool) { Q_EMIT interactionStateChanged(); });
    connect(&_scene->undoStack(), &QUndoStack::canRedoChanged, this, [this](bool) { Q_EMIT interactionStateChanged(); });
    connect(_scene, &QGraphicsScene::selectionChanged, this, [this]() {
        const auto nodeIds = selectedNodeIds();
        if (!nodeIds.empty()) {
            if (_selectedNodeId != nodeIds.front())
                handleNodeSelected(nodeIds.front());
        } else if (_selectedNodeId != QtNodes::InvalidNodeId) {
            _selectedNodeId = QtNodes::InvalidNodeId;
            Q_EMIT selectionCleared();
        }

        Q_EMIT interactionStateChanged();
    });

    auto *deleteShortcut = new QShortcut(QKeySequence::Delete, this);
    connect(deleteShortcut, &QShortcut::activated, this, &QtNodesEditorWidget::deleteSelection);

    auto *backspaceShortcut = new QShortcut(QKeySequence(Qt::Key_Backspace), this);
    connect(backspaceShortcut, &QShortcut::activated, this, &QtNodesEditorWidget::deleteSelection);

    _view->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(_view, &QWidget::customContextMenuRequested, this, [this](QPoint const &pos) {
        showCanvasContextMenu(_view->mapToGlobal(pos));
    });
}

QtNodesEditorWidget::~QtNodesEditorWidget() = default;

void QtNodesEditorWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateBuiltInContent();

    QWidget::changeEvent(event);
}

bool QtNodesEditorWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == _view->viewport()) {
        switch (event->type()) {
        case QEvent::DragEnter: {
            auto *dragEnterEvent = static_cast<QDragEnterEvent *>(event);
            dragEnterEvent->acceptProposedAction();
            updateDropPreview(QString::fromUtf8(dragEnterEvent->mimeData()->data(NodeLibraryListWidget::MimeType)),
                              dragEnterEvent->pos(),
                              true);
            return event->isAccepted();
        }
        case QEvent::DragMove: {
            auto *dragMoveEvent = static_cast<QDragMoveEvent *>(event);
            dragMoveEvent->acceptProposedAction();
            updateDropPreview(QString::fromUtf8(dragMoveEvent->mimeData()->data(NodeLibraryListWidget::MimeType)),
                              dragMoveEvent->pos(),
                              true);
            return event->isAccepted();
        }
        case QEvent::Drop: {
            auto *dropEvent = static_cast<QDropEvent *>(event);
            const bool accepted = acceptNodeDrag(dropEvent->mimeData(), dropEvent->pos(), true);
            clearDropPreview();
            return accepted;
        }
        case QEvent::DragLeave:
            clearDropPreview();
            return false;
        case QEvent::MouseMove:
            updateConnectionFeedback(static_cast<QMouseEvent *>(event)->pos());
            return false;
        case QEvent::MouseButtonRelease:
            if (_connectionFeedbackActive) {
                QTimer::singleShot(0, this, [this]() { clearConnectionFeedback(); });
            }
            return false;
        default:
            break;
        }
    }

    return QWidget::eventFilter(watched, event);
}

QtNodes::NodeId QtNodesEditorWidget::createNode(QString const &typeKey, QPointF const &scenePosition)
{
    auto *command = new NodeCreateCommand(this, typeKey, scenePosition);
    _scene->undoStack().push(command);
    return command->nodeId();
}

QtNodes::NodeId QtNodesEditorWidget::createNodeInternal(QString const &typeKey, QPointF const &scenePosition)
{
    auto const *definition = _builtInNodeRegistry.find(typeKey);
    if (definition == nullptr)
        return QtNodes::InvalidNodeId;

    const auto nodeId = _graphModel->addNode(typeKey);
    if (nodeId == QtNodes::InvalidNodeId)
        return nodeId;

    _nodeStates.insert(
        nodeId,
        NodeState{definition->typeKey,
                  definition->displayName,
                  definition->description,
                  definition->defaultProperties,
                  false,
                  false});
    _graphModel->setNodeData(nodeId,
                             QtNodes::NodeRole::Position,
                             scenePosition.isNull() ? defaultScenePosition() : scenePosition);

    if (auto *model = _graphModel->delegateModel<StaticNodeDelegateModel>(nodeId); model != nullptr) {
        model->setDisplayName(definition->displayName);
    }
    applyNodeStyle(nodeId, definition->typeKey);

    return nodeId;
}

QtNodes::NodeId QtNodesEditorWidget::restoreNodeInternal(NodeSnapshot const &snapshot)
{
    _graphModel->loadNode(snapshot.nodeJson);
    _nodeStates.insert(snapshot.nodeId, snapshot.state);

    if (auto *model = _graphModel->delegateModel<StaticNodeDelegateModel>(snapshot.nodeId); model != nullptr)
        model->setDisplayName(snapshot.state.displayName);

    applyNodeStyle(snapshot.nodeId, snapshot.state.typeKey);
    return snapshot.nodeId;
}

bool QtNodesEditorWidget::deleteNodeInternal(QtNodes::NodeId nodeId)
{
    if (!_graphModel->nodeExists(nodeId))
        return false;

    _nodeStates.remove(nodeId);
    _graphModel->deleteNode(nodeId);
    if (_selectedNodeId == nodeId)
        _selectedNodeId = QtNodes::InvalidNodeId;
    return true;
}

bool QtNodesEditorWidget::connectNodes(QtNodes::NodeId outNodeId,
                                       QtNodes::PortIndex outPortIndex,
                                       QtNodes::NodeId inNodeId,
                                       QtNodes::PortIndex inPortIndex)
{
    QtNodes::ConnectionId connectionId{outNodeId, outPortIndex, inNodeId, inPortIndex};

    if (!_graphModel->connectionPossible(connectionId))
        return false;

    _scene->undoStack().push(new ConnectionCreateCommand(this, connectionId));
    return true;
}

void QtNodesEditorWidget::selectNode(QtNodes::NodeId nodeId)
{
    if (!_graphModel->nodeExists(nodeId))
        return;

    _scene->clearSelection();

    if (auto *nodeGraphicsObject = _scene->nodeGraphicsObject(nodeId); nodeGraphicsObject != nullptr) {
        nodeGraphicsObject->setSelected(true);
    }

    handleNodeSelected(nodeId);
}

void QtNodesEditorWidget::clearWorkflow()
{
    _scene->undoStack().clear();

    const auto nodeIds = sortedNodeIds();
    for (auto it = nodeIds.crbegin(); it != nodeIds.crend(); ++it) {
        _graphModel->deleteNode(*it);
    }

    _nodeStates.clear();
    _selectedNodeId = QtNodes::InvalidNodeId;
    _scene->clearSelection();
    clearDropPreview();
    Q_EMIT selectionCleared();
}

void QtNodesEditorWidget::deleteSelectedNodes()
{
    auto *command = new NodeDeleteSelectionCommand(this, true);
    if (command->isObsolete()) {
        delete command;
        return;
    }
    _scene->undoStack().push(command);
}

void QtNodesEditorWidget::deleteSelectedConnections()
{
    auto *command = new NodeDeleteSelectionCommand(this, false);
    if (command->isObsolete()) {
        delete command;
        return;
    }
    _scene->undoStack().push(command);
}

void QtNodesEditorWidget::deleteSelection()
{
    const auto nodeIds = selectedNodeIds();
    if (!nodeIds.empty()) {
        deleteSelectedNodes();
        return;
    }

    deleteSelectedConnections();
}

void QtNodesEditorWidget::selectAllNodes()
{
    _scene->clearSelection();
    for (auto const nodeId : sortedNodeIds()) {
        if (auto *obj = _scene->nodeGraphicsObject(nodeId); obj != nullptr)
            obj->setSelected(true);
    }
}

void QtNodesEditorWidget::centerSelection()
{
    if (_scene->selectedItems().isEmpty())
        _view->zoomFitAll();
    else
        _view->zoomFitSelected();
}

void QtNodesEditorWidget::undo()
{
    _scene->undoStack().undo();
}

void QtNodesEditorWidget::redo()
{
    _scene->undoStack().redo();
}

std::vector<QtNodes::NodeId> QtNodesEditorWidget::selectedNodeIds() const
{
    return _scene->selectedNodes();
}

int QtNodesEditorWidget::nodeCount() const
{
    return static_cast<int>(_nodeStates.size());
}

int QtNodesEditorWidget::connectionCount() const
{
    int count = 0;
    for (auto const nodeId : sortedNodeIds()) {
        for (auto const &connectionId : _graphModel->allConnectionIds(nodeId)) {
            if (connectionId.outNodeId == nodeId)
                ++count;
        }
    }

    return count;
}

bool QtNodesEditorWidget::dropPreviewVisible() const
{
    return _dropPreview->isVisible();
}

bool QtNodesEditorWidget::usesStyledNodePainter() const
{
    return dynamic_cast<StyledNodePainter *>(&(_scene->nodePainter())) != nullptr;
}

qreal QtNodesEditorWidget::styledNodeCornerRadius() const
{
    if (auto *painter = dynamic_cast<StyledNodePainter *>(&(_scene->nodePainter())); painter != nullptr)
        return painter->cornerRadius();

    return 0.0;
}

QSize QtNodesEditorWidget::dropPreviewSize() const
{
    return _dropPreview->size();
}

QSize QtNodesEditorWidget::previewNodeSize(QString const &typeKey) const
{
    const auto cached = _previewSizeCache.constFind(typeKey);
    if (cached != _previewSizeCache.cend())
        return *cached;

    const QSize computed = computePreviewNodeSize(typeKey);
    if (computed.isValid())
        _previewSizeCache.insert(typeKey, computed);

    return computed;
}

bool QtNodesEditorWidget::nodeHasGraphicsEffect(QtNodes::NodeId nodeId) const
{
    if (auto *nodeGraphicsObject = _scene->nodeGraphicsObject(nodeId); nodeGraphicsObject != nullptr)
        return nodeGraphicsObject->graphicsEffect() != nullptr;

    return false;
}

QPointF QtNodesEditorWidget::portPosition(QtNodes::NodeId nodeId,
                                          QtNodes::PortType portType,
                                          QtNodes::PortIndex portIndex) const
{
    if (!_graphModel->nodeExists(nodeId))
        return {};

    return _scene->nodeGeometry().portPosition(nodeId, portType, portIndex);
}

QSize QtNodesEditorWidget::nodeSize(QtNodes::NodeId nodeId) const
{
    if (!_graphModel->nodeExists(nodeId))
        return {};

    return _scene->nodeGeometry().size(nodeId);
}

QVariantMap QtNodesEditorWidget::nodeStyle(QtNodes::NodeId nodeId) const
{
    if (!_graphModel->nodeExists(nodeId))
        return {};

    return _graphModel->nodeData(nodeId, QtNodes::NodeRole::Style).toMap();
}

QString QtNodesEditorWidget::nodeValidationState(QtNodes::NodeId nodeId) const
{
    if (!_graphModel->nodeExists(nodeId))
        return {};

    const auto state =
        _graphModel->nodeData(nodeId, QtNodes::NodeRole::ValidationState).value<QtNodes::NodeValidationState>();

    switch (state._state) {
    case QtNodes::NodeValidationState::State::Valid:
        return QStringLiteral("valid");
    case QtNodes::NodeValidationState::State::Warning:
        return QStringLiteral("warning");
    case QtNodes::NodeValidationState::State::Error:
        return QStringLiteral("error");
    }

    return {};
}

QString QtNodesEditorWidget::nodeValidationMessage(QtNodes::NodeId nodeId) const
{
    if (!_graphModel->nodeExists(nodeId))
        return {};

    return _graphModel->nodeData(nodeId, QtNodes::NodeRole::ValidationState)
        .value<QtNodes::NodeValidationState>()
        ._stateMessage;
}

QString QtNodesEditorWidget::selectedNodeDisplayName() const
{
    auto state = selectedState();
    return state ? state->displayName : QString();
}

QString QtNodesEditorWidget::selectedNodeDescription() const
{
    auto state = selectedState();
    return state ? state->description : QString();
}

QVariant QtNodesEditorWidget::selectedNodeProperty(QString const &propertyKey) const
{
    auto state = selectedState();
    return state ? state->properties.value(propertyKey) : QVariant();
}

QStringList QtNodesEditorWidget::workflowDisplayNames() const
{
    QStringList names;
    for (auto const nodeId : sortedNodeIds()) {
        auto const stateIt = _nodeStates.constFind(nodeId);
        if (stateIt != _nodeStates.cend())
            names.push_back(stateIt->displayName);
    }
    return names;
}

QString QtNodesEditorWidget::descriptionForDisplayName(QString const &displayName) const
{
    for (auto const &nodeState : _nodeStates) {
        if (nodeState.displayName == displayName)
            return nodeState.description;
    }

    return {};
}

QtNodes::NodeId QtNodesEditorWidget::findNodeIdByDisplayName(QString const &displayName) const
{
    for (auto const nodeId : sortedNodeIds()) {
        auto const stateIt = _nodeStates.constFind(nodeId);
        if (stateIt != _nodeStates.cend() && stateIt->displayName == displayName)
            return nodeId;
    }

    return QtNodes::InvalidNodeId;
}

QPointF QtNodesEditorWidget::portScenePosition(QtNodes::NodeId nodeId,
                                               QtNodes::PortType portType,
                                               QtNodes::PortIndex portIndex) const
{
    if (!_graphModel->nodeExists(nodeId))
        return {};

    auto *nodeGraphicsObject = _scene->nodeGraphicsObject(nodeId);
    if (nodeGraphicsObject == nullptr)
        return {};

    return _scene->nodeGeometry().portScenePosition(nodeId, portType, portIndex, nodeGraphicsObject->sceneTransform());
}

bool QtNodesEditorWidget::canUndo() const
{
    return _scene->undoStack().canUndo();
}

bool QtNodesEditorWidget::hasSelection() const
{
    return !_scene->selectedItems().isEmpty();
}

bool QtNodesEditorWidget::hasNodes() const
{
    return !_nodeStates.isEmpty();
}

bool QtNodesEditorWidget::canRedo() const
{
    return _scene->undoStack().canRedo();
}

bool QtNodesEditorWidget::isClean() const
{
    return _scene->undoStack().isClean();
}

void QtNodesEditorWidget::markCurrentStateClean()
{
    _scene->undoStack().setClean();
}

void QtNodesEditorWidget::markCurrentStateDirty()
{
    _scene->undoStack().resetClean();
}

void QtNodesEditorWidget::setNodeDisplayNameInternal(QtNodes::NodeId nodeId, QString const &displayName)
{
    auto nodeStateIt = _nodeStates.find(nodeId);
    if (nodeStateIt == _nodeStates.end())
        return;

    auto const *definition = _builtInNodeRegistry.find(nodeStateIt->typeKey);
    nodeStateIt->displayName = displayName;
    nodeStateIt->displayNameCustomized = definition != nullptr && displayName != definition->displayName;

    if (auto *model = _graphModel->delegateModel<StaticNodeDelegateModel>(nodeId); model != nullptr)
        model->setDisplayName(displayName);

    if (_selectedNodeId == nodeId) {
        Q_EMIT selectedNodeChanged(nodeStateIt->typeKey,
                                   nodeStateIt->displayName,
                                   nodeStateIt->description,
                                   nodeStateIt->properties);
        emitSelectedNodeValidation();
    }

    Q_EMIT workflowModified();
}

void QtNodesEditorWidget::setNodeDescriptionInternal(QtNodes::NodeId nodeId, QString const &description)
{
    auto nodeStateIt = _nodeStates.find(nodeId);
    if (nodeStateIt == _nodeStates.end())
        return;

    auto const *definition = _builtInNodeRegistry.find(nodeStateIt->typeKey);
    nodeStateIt->description = description;
    nodeStateIt->descriptionCustomized = definition != nullptr && description != definition->description;

    if (_selectedNodeId == nodeId) {
        Q_EMIT selectedNodeChanged(nodeStateIt->typeKey,
                                   nodeStateIt->displayName,
                                   nodeStateIt->description,
                                   nodeStateIt->properties);
        emitSelectedNodeValidation();
    }

    Q_EMIT workflowModified();
}

void QtNodesEditorWidget::setNodePropertyInternal(QtNodes::NodeId nodeId,
                                                  QString const &propertyKey,
                                                  QVariant const &value)
{
    auto nodeStateIt = _nodeStates.find(nodeId);
    if (nodeStateIt == _nodeStates.end())
        return;

    nodeStateIt->properties.insert(propertyKey, value);
    applyNodeStyle(nodeId, nodeStateIt->typeKey);

    if (_selectedNodeId == nodeId) {
        Q_EMIT selectedNodeChanged(nodeStateIt->typeKey,
                                   nodeStateIt->displayName,
                                   nodeStateIt->description,
                                   nodeStateIt->properties);
        emitSelectedNodeValidation();
    }

    Q_EMIT workflowModified();
}

QtNodesEditorWidget::NodeSnapshot QtNodesEditorWidget::snapshotNode(QtNodes::NodeId nodeId) const
{
    return NodeSnapshot{nodeId, _graphModel->saveNode(nodeId), _nodeStates.value(nodeId)};
}

void QtNodesEditorWidget::refreshSelectedNodeState()
{
    if (_selectedNodeId != QtNodes::InvalidNodeId && _graphModel->nodeExists(_selectedNodeId)) {
        auto nodeStateIt = _nodeStates.find(_selectedNodeId);
        if (nodeStateIt != _nodeStates.end()) {
            Q_EMIT selectedNodeChanged(nodeStateIt->typeKey,
                                       nodeStateIt->displayName,
                                       nodeStateIt->description,
                                       nodeStateIt->properties);
            emitSelectedNodeValidation();
            return;
        }
    }

    _selectedNodeId = QtNodes::InvalidNodeId;
    Q_EMIT selectionCleared();
}

void QtNodesEditorWidget::refreshValidationForNode(QtNodes::NodeId nodeId)
{
    auto const nodeStateIt = _nodeStates.constFind(nodeId);
    if (nodeStateIt == _nodeStates.cend() || !_graphModel->nodeExists(nodeId))
        return;

    applyNodeStyle(nodeId, nodeStateIt->typeKey);

    if (_selectedNodeId == nodeId)
        emitSelectedNodeValidation();
}

void QtNodesEditorWidget::refreshValidationForConnectionEndpoints(QtNodes::ConnectionId const &connectionId)
{
    refreshValidationForNode(connectionId.outNodeId);
    refreshValidationForNode(connectionId.inNodeId);
}

void QtNodesEditorWidget::setSelectedNodeDisplayName(QString const &displayName)
{
    auto nodeStateIt = _nodeStates.find(_selectedNodeId);
    if (nodeStateIt == _nodeStates.end())
        return;

    if (nodeStateIt->displayName == displayName)
        return;

    _scene->undoStack().push(
        new NodeDisplayNameEditCommand(this, _selectedNodeId, nodeStateIt->displayName, displayName));
}

void QtNodesEditorWidget::setSelectedNodeDescription(QString const &description)
{
    auto nodeStateIt = _nodeStates.find(_selectedNodeId);
    if (nodeStateIt == _nodeStates.end())
        return;

    if (nodeStateIt->description == description)
        return;

    _scene->undoStack().push(
        new NodeDescriptionEditCommand(this, _selectedNodeId, nodeStateIt->description, description));
}

void QtNodesEditorWidget::setSelectedNodeProperty(QString const &propertyKey, QVariant const &value)
{
    auto nodeStateIt = _nodeStates.find(_selectedNodeId);
    if (nodeStateIt == _nodeStates.end())
        return;

    if (nodeStateIt->properties.value(propertyKey) == value)
        return;

    _scene->undoStack().push(
        new NodePropertyEditCommand(this, _selectedNodeId, propertyKey, nodeStateIt->properties.value(propertyKey), value));
}

bool QtNodesEditorWidget::saveWorkflow(QString const &filePath) const
{
    QJsonObject root;
    root["version"] = 1;

    QJsonArray nodes;
    for (auto const nodeId : sortedNodeIds()) {
        auto const stateIt = _nodeStates.constFind(nodeId);
        if (stateIt == _nodeStates.cend())
            continue;

        QJsonObject node;
        node["id"] = static_cast<int>(nodeId);
        node["type"] = stateIt->typeKey;
        node["displayName"] = stateIt->displayName;
        node["description"] = stateIt->description;
        node["properties"] = QJsonObject::fromVariantMap(stateIt->properties);

        const auto position = _graphModel->nodeData(nodeId, QtNodes::NodeRole::Position).toPointF();
        QJsonObject positionObject;
        positionObject["x"] = position.x();
        positionObject["y"] = position.y();
        node["position"] = positionObject;
        nodes.push_back(node);
    }
    root["nodes"] = nodes;

    QJsonArray connections;
    for (auto const nodeId : sortedNodeIds()) {
        for (auto const &connectionId : _graphModel->allConnectionIds(nodeId)) {
            if (connectionId.outNodeId != nodeId)
                continue;

            QJsonObject connection;
            connection["outNodeId"] = static_cast<int>(connectionId.outNodeId);
            connection["outPortIndex"] = static_cast<int>(connectionId.outPortIndex);
            connection["inNodeId"] = static_cast<int>(connectionId.inNodeId);
            connection["inPortIndex"] = static_cast<int>(connectionId.inPortIndex);
            connections.push_back(connection);
        }
    }
    root["connections"] = connections;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;

    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return true;
}

bool QtNodesEditorWidget::loadWorkflow(QString const &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    const auto document = QJsonDocument::fromJson(file.readAll());
    if (!document.isObject())
        return false;

    const auto root = document.object();
    const auto nodes = root["nodes"].toArray();
    const auto connections = root["connections"].toArray();

    struct ParsedNode
    {
        int serializedId = 0;
        QString typeKey;
        QString displayName;
        QString description;
        QVariantMap properties;
        QPointF position;
        bool displayNameCustomized = false;
        bool descriptionCustomized = false;
    };

    struct ParsedConnection
    {
        int outSerializedNodeId = 0;
        QtNodes::PortIndex outPortIndex = 0;
        int inSerializedNodeId = 0;
        QtNodes::PortIndex inPortIndex = 0;
    };

    std::vector<ParsedNode> parsedNodes;
    parsedNodes.reserve(static_cast<std::size_t>(nodes.size()));
    std::vector<ParsedConnection> parsedConnections;
    parsedConnections.reserve(static_cast<std::size_t>(connections.size()));
    QHash<int, QtNodes::NodeId> previewNodeIdMap;
    QtNodes::DataFlowGraphModel previewModel(_registry);

    for (auto const &nodeValue : nodes) {
        const auto nodeObject = nodeValue.toObject();
        const auto positionObject = nodeObject["position"].toObject();
        const QString typeKey = nodeObject["type"].toString();
        auto const *definition = _builtInNodeRegistry.find(typeKey);
        if (definition == nullptr)
            return false;

        const int serializedId = nodeObject["id"].toInt(std::numeric_limits<int>::min());
        if (serializedId == std::numeric_limits<int>::min() || previewNodeIdMap.contains(serializedId))
            return false;

        const auto previewNodeId = previewModel.addNode(typeKey);
        if (previewNodeId == QtNodes::InvalidNodeId)
            return false;

        QVariantMap properties = nodeObject["properties"].toObject().toVariantMap();
        for (auto it = definition->defaultProperties.cbegin(); it != definition->defaultProperties.cend(); ++it) {
            if (!properties.contains(it.key()))
                properties.insert(it.key(), it.value());
        }

        ParsedNode parsedNode;
        parsedNode.serializedId = serializedId;
        parsedNode.typeKey = typeKey;
        parsedNode.displayName = nodeObject["displayName"].toString(definition->displayName);
        parsedNode.description = nodeObject["description"].toString(definition->description);
        parsedNode.properties = properties;
        parsedNode.position = QPointF(positionObject["x"].toDouble(), positionObject["y"].toDouble());
        parsedNode.displayNameCustomized = parsedNode.displayName != definition->displayName;
        parsedNode.descriptionCustomized = parsedNode.description != definition->description;

        previewNodeIdMap.insert(serializedId, previewNodeId);
        parsedNodes.push_back(std::move(parsedNode));
    }

    for (auto const &connectionValue : connections) {
        const auto connectionObject = connectionValue.toObject();
        ParsedConnection parsedConnection;
        parsedConnection.outSerializedNodeId = connectionObject["outNodeId"].toInt(std::numeric_limits<int>::min());
        parsedConnection.outPortIndex =
            static_cast<QtNodes::PortIndex>(connectionObject["outPortIndex"].toInt(std::numeric_limits<int>::min()));
        parsedConnection.inSerializedNodeId = connectionObject["inNodeId"].toInt(std::numeric_limits<int>::min());
        parsedConnection.inPortIndex =
            static_cast<QtNodes::PortIndex>(connectionObject["inPortIndex"].toInt(std::numeric_limits<int>::min()));

        const auto previewOutNodeId = previewNodeIdMap.value(parsedConnection.outSerializedNodeId, QtNodes::InvalidNodeId);
        const auto previewInNodeId = previewNodeIdMap.value(parsedConnection.inSerializedNodeId, QtNodes::InvalidNodeId);
        if (previewOutNodeId == QtNodes::InvalidNodeId || previewInNodeId == QtNodes::InvalidNodeId)
            return false;

        QtNodes::ConnectionId previewConnectionId{
            previewOutNodeId, parsedConnection.outPortIndex, previewInNodeId, parsedConnection.inPortIndex};
        if (!previewModel.connectionPossible(previewConnectionId))
            return false;

        previewModel.addConnection(previewConnectionId);
        parsedConnections.push_back(parsedConnection);
    }

    clearWorkflow();

    QHash<int, QtNodes::NodeId> nodeIdMap;
    for (auto const &parsedNode : parsedNodes) {
        const auto nodeId = createNodeInternal(parsedNode.typeKey, parsedNode.position);
        if (nodeId == QtNodes::InvalidNodeId)
            return false;

        nodeIdMap.insert(parsedNode.serializedId, nodeId);
        _nodeStates[nodeId].displayName = parsedNode.displayName;
        _nodeStates[nodeId].description = parsedNode.description;
        _nodeStates[nodeId].properties = parsedNode.properties;
        _nodeStates[nodeId].displayNameCustomized = parsedNode.displayNameCustomized;
        _nodeStates[nodeId].descriptionCustomized = parsedNode.descriptionCustomized;

        if (auto *model = _graphModel->delegateModel<StaticNodeDelegateModel>(nodeId); model != nullptr) {
            model->setDisplayName(_nodeStates[nodeId].displayName);
        }
        applyNodeStyle(nodeId, _nodeStates[nodeId].typeKey);
    }

    for (auto const &parsedConnection : parsedConnections) {
        const auto outNodeId = nodeIdMap.value(parsedConnection.outSerializedNodeId, QtNodes::InvalidNodeId);
        const auto inNodeId = nodeIdMap.value(parsedConnection.inSerializedNodeId, QtNodes::InvalidNodeId);
        if (outNodeId == QtNodes::InvalidNodeId || inNodeId == QtNodes::InvalidNodeId)
            return false;

        QtNodes::ConnectionId connectionId{
            outNodeId, parsedConnection.outPortIndex, inNodeId, parsedConnection.inPortIndex};
        if (!_graphModel->connectionPossible(connectionId)) {
            return false;
        }

        _graphModel->addConnection(connectionId);
        refreshValidationForConnectionEndpoints(connectionId);
    }

    if (!_nodeStates.isEmpty())
        selectNode(sortedNodeIds().front());
    else
        Q_EMIT selectionCleared();

    return true;
}

void QtNodesEditorWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat(NodeLibraryListWidget::MimeType)) {
        event->acceptProposedAction();
        updateDropPreview(QString::fromUtf8(event->mimeData()->data(NodeLibraryListWidget::MimeType)),
                          event->pos(),
                          false);
    }
}

void QtNodesEditorWidget::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat(NodeLibraryListWidget::MimeType)) {
        event->acceptProposedAction();
        updateDropPreview(QString::fromUtf8(event->mimeData()->data(NodeLibraryListWidget::MimeType)),
                          event->pos(),
                          false);
    }
}

void QtNodesEditorWidget::dropEvent(QDropEvent *event)
{
    if (acceptNodeDrag(event->mimeData(), event->pos(), false))
        event->acceptProposedAction();

    clearDropPreview();
}

std::shared_ptr<QtNodes::NodeDelegateModelRegistry> QtNodesEditorWidget::buildRegistry() const
{
    auto registry = std::make_shared<QtNodes::NodeDelegateModelRegistry>();

    for (auto const &definition : _builtInNodeRegistry.definitions()) {
        registry->registerModel(
            [definition]() {
                return std::make_unique<StaticNodeDelegateModel>(definition);
            },
            definition.categoryDisplayName);
    }

    return registry;
}

void QtNodesEditorWidget::applyWorkbenchStyles()
{
    QtNodes::GraphicsViewStyle::setStyle(R"({
      "GraphicsViewStyle": {
        "BackgroundColor": [245, 240, 231],
        "FineGridColor": [231, 223, 211],
        "CoarseGridColor": [214, 204, 190]
      }
    })");

    QtNodes::ConnectionStyle::setConnectionStyle(R"({
      "ConnectionStyle": {
        "ConstructionColor": [173, 160, 145],
        "NormalColor": [146, 132, 118],
        "SelectedColor": [111, 128, 159],
        "SelectedHaloColor": [206, 216, 233],
        "HoveredColor": [94, 109, 139],
        "LineWidth": 3.0,
        "ConstructionLineWidth": 2.5,
        "PointDiameter": 10.0,
        "UseDataDefinedColors": false
      }
    })");

    if (_view != nullptr) {
        _view->setBackgroundBrush(QtNodes::StyleCollection::flowViewStyle().BackgroundColor);
        _view->resetCachedContent();
        _view->viewport()->update();
    }

    if (_scene != nullptr)
        _scene->update();
}

void QtNodesEditorWidget::applyNodeStyle(QtNodes::NodeId nodeId, QString const &typeKey)
{
    auto *model = _graphModel->delegateModel<StaticNodeDelegateModel>(nodeId);
    if (model == nullptr)
        return;

    const auto nodeStateIt = _nodeStates.constFind(nodeId);
    const QVariantMap properties = nodeStateIt != _nodeStates.cend() ? nodeStateIt->properties : QVariantMap();
    const QVariantMap styleMap = nodeStyleForType(typeKey, properties);
    const auto validationState = validationStateFor(nodeId, typeKey, properties);

    QtNodes::NodeStyle style;
    style.loadJson(QJsonObject::fromVariantMap(styleMap));
    model->setNodeStyle(style);
    _graphModel->setNodeData(nodeId, QtNodes::NodeRole::ValidationState, QVariant::fromValue(validationState));
    if (auto *nodeGraphicsObject = _scene->nodeGraphicsObject(nodeId); nodeGraphicsObject != nullptr) {
        if (nodeGraphicsObject->graphicsEffect() != nullptr)
            nodeGraphicsObject->setGraphicsEffect(nullptr);
    }
    Q_EMIT model->requestNodeUpdate();
}

QVariantMap QtNodesEditorWidget::nodeStyleForType(QString const &typeKey, QVariantMap const &properties) const
{
    QtNodes::NodeStyle style;
    style.NormalBoundaryColor = QColor(QStringLiteral("#C8BBAB"));
    style.SelectedBoundaryColor = QColor(QStringLiteral("#7C8CAA"));
    style.GradientColor0 = QColor(QStringLiteral("#FFFDF9"));
    style.GradientColor1 = QColor(QStringLiteral("#FCF7EF"));
    style.GradientColor2 = QColor(QStringLiteral("#F8F1E6"));
    style.GradientColor3 = QColor(QStringLiteral("#F2E8DA"));
    style.ShadowColor = QColor(205, 194, 180);
    style.ShadowEnabled = true;
    style.FontColor = QColor(QStringLiteral("#3F352D"));
    style.FontColorFaded = QColor(QStringLiteral("#7A6B5E"));
    style.ConnectionPointColor = QColor(QStringLiteral("#FFFFFF"));
    style.FilledConnectionPointColor = QColor(QStringLiteral("#CFC1AF"));
    style.WarningColor = QColor(QStringLiteral("#D48C2F"));
    style.ErrorColor = QColor(QStringLiteral("#C15757"));
    style.PenWidth = 1.7F;
    style.HoveredPenWidth = 2.2F;
    style.ConnectionPointDiameter = 11.0F;
    style.Opacity = 1.0F;

    if (typeKey == QStringLiteral("start") || typeKey == QStringLiteral("condition")
        || typeKey == QStringLiteral("output")) {
        style.NormalBoundaryColor = QColor(QStringLiteral("#9CAECE"));
        style.SelectedBoundaryColor = QColor(QStringLiteral("#6B7F9F"));
        style.GradientColor0 = QColor(QStringLiteral("#F8FAFD"));
        style.GradientColor1 = QColor(QStringLiteral("#EEF3F9"));
        style.GradientColor2 = QColor(QStringLiteral("#E3EBF6"));
        style.GradientColor3 = QColor(QStringLiteral("#D8E3F1"));
        style.FilledConnectionPointColor = QColor(QStringLiteral("#8A9FBE"));
    } else if (typeKey == QStringLiteral("prompt")) {
        style.NormalBoundaryColor = QColor(QStringLiteral("#D0AE7C"));
        style.SelectedBoundaryColor = QColor(QStringLiteral("#A77B3E"));
        style.GradientColor0 = QColor(QStringLiteral("#FFF9F1"));
        style.GradientColor1 = QColor(QStringLiteral("#FBF1E1"));
        style.GradientColor2 = QColor(QStringLiteral("#F6E8D0"));
        style.GradientColor3 = QColor(QStringLiteral("#EFDDBB"));
        style.FilledConnectionPointColor = QColor(QStringLiteral("#C6985B"));
    } else if (typeKey == QStringLiteral("llm")) {
        style.NormalBoundaryColor = QColor(QStringLiteral("#C9A2AD"));
        style.SelectedBoundaryColor = QColor(QStringLiteral("#9C6B79"));
        style.GradientColor0 = QColor(QStringLiteral("#FFF8F8"));
        style.GradientColor1 = QColor(QStringLiteral("#FBEDEF"));
        style.GradientColor2 = QColor(QStringLiteral("#F4E0E5"));
        style.GradientColor3 = QColor(QStringLiteral("#EBCFD7"));
        style.FilledConnectionPointColor = QColor(QStringLiteral("#B88693"));
    } else if (typeKey == QStringLiteral("tool")) {
        style.NormalBoundaryColor = QColor(QStringLiteral("#8EB2AA"));
        style.SelectedBoundaryColor = QColor(QStringLiteral("#5E857D"));
        style.GradientColor0 = QColor(QStringLiteral("#F7FCFA"));
        style.GradientColor1 = QColor(QStringLiteral("#EDF7F2"));
        style.GradientColor2 = QColor(QStringLiteral("#E0EFE8"));
        style.GradientColor3 = QColor(QStringLiteral("#D2E6DE"));
        style.FilledConnectionPointColor = QColor(QStringLiteral("#78A197"));
    }

    return style.toJson().toVariantMap();
}

QtNodesEditorWidget::ValidationResult QtNodesEditorWidget::validationResultFor(QtNodes::NodeId nodeId,
                                                                               QString const &typeKey,
                                                                               QVariantMap const &properties) const
{
    auto const trimmed = [&](QString const &key) { return properties.value(key).toString().trimmed(); };
    ValidationResult result;

    if (_graphModel->nodeExists(nodeId)) {
        auto const hasIncoming = [&](QtNodes::PortIndex portIndex) {
            return !_graphModel->connections(nodeId, QtNodes::PortType::In, portIndex).empty();
        };
        auto const hasOutgoing = [&](QtNodes::PortIndex portIndex) {
            return !_graphModel->connections(nodeId, QtNodes::PortType::Out, portIndex).empty();
        };

        if (typeKey == QStringLiteral("start") && !hasOutgoing(0)) {
            result.state = QtNodes::NodeValidationState::State::Warning;
            result.message = tr("Start node must connect to the next step.");
            return result;
        }

        if (typeKey == QStringLiteral("condition")) {
            if (!hasIncoming(0)) {
                result.state = QtNodes::NodeValidationState::State::Warning;
                result.message = tr("Condition node requires an input connection.");
                return result;
            }

            if (!hasOutgoing(0) || !hasOutgoing(1)) {
                result.state = QtNodes::NodeValidationState::State::Warning;
                result.message = tr("Condition node must connect both True and False branches.");
                return result;
            }
        }

        if (typeKey == QStringLiteral("output") && !hasIncoming(0)) {
            result.state = QtNodes::NodeValidationState::State::Warning;
            result.message = tr("Output node requires an incoming connection.");
            return result;
        }
    }

    if (typeKey == QStringLiteral("prompt")) {
        if (trimmed(QStringLiteral("userPromptTemplate")).isEmpty()) {
            result.state = QtNodes::NodeValidationState::State::Warning;
            result.message = tr("Prompt template is empty.");
            result.propertyKey = QStringLiteral("userPromptTemplate");
        }
        return result;
    }

    if (typeKey == QStringLiteral("llm")) {
        if (trimmed(QStringLiteral("modelName")).isEmpty()) {
            result.state = QtNodes::NodeValidationState::State::Warning;
            result.message = tr("Model name is required.");
            result.propertyKey = QStringLiteral("modelName");
        }
        return result;
    }

    if (typeKey == QStringLiteral("tool")) {
        if (trimmed(QStringLiteral("toolName")).isEmpty()) {
            result.state = QtNodes::NodeValidationState::State::Warning;
            result.message = tr("Tool name is required.");
            result.propertyKey = QStringLiteral("toolName");
            return result;
        }

        const QString inputMapping = trimmed(QStringLiteral("inputMapping"));
        if (!inputMapping.isEmpty()) {
            QJsonParseError parseError;
            const auto document = QJsonDocument::fromJson(inputMapping.toUtf8(), &parseError);
            if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
                result.state = QtNodes::NodeValidationState::State::Error;
                result.message = tr("Input mapping must be a valid JSON object.");
                result.propertyKey = QStringLiteral("inputMapping");
                return result;
            }
        }
    }

    return result;
}

QtNodes::NodeValidationState QtNodesEditorWidget::validationStateFor(QtNodes::NodeId nodeId,
                                                                     QString const &typeKey,
                                                                     QVariantMap const &properties) const
{
    const ValidationResult result = validationResultFor(nodeId, typeKey, properties);
    QtNodes::NodeValidationState state;
    state._state = result.state;
    state._stateMessage = result.message;
    return state;
}

void QtNodesEditorWidget::updateDropPreview(QString const &typeKey,
                                            QPoint const &position,
                                            bool viewportCoordinates)
{
    auto const *definition = _builtInNodeRegistry.find(typeKey);
    if (definition == nullptr) {
        clearDropPreview();
        return;
    }

    const QPoint viewportPoint = viewportCoordinates
                                     ? position
                                     : _view->viewport()->mapFromGlobal(mapToGlobal(position));
    const QSize previewSize = previewNodeSize(typeKey);
    if (previewSize.isValid())
        _dropPreview->resize(previewSize);
    _dropPreview->move(viewportPoint.x() - (_dropPreview->width() / 2),
                       viewportPoint.y() - (_dropPreview->height() / 2));
    _dropPreview->show();
    _dropPreview->raise();

    Q_EMIT dropPreviewMessageChanged(tr("Release to create %1").arg(definition->displayName));
}

void QtNodesEditorWidget::clearDropPreview()
{
    if (_dropPreview->isVisible())
        _dropPreview->hide();

    Q_EMIT dropPreviewMessageChanged(tr("Ready"));
}

void QtNodesEditorWidget::updateConnectionFeedback(QPoint const &viewportPoint)
{
    auto *draftConnection = draftConnectionGraphicsObject();
    if (draftConnection == nullptr) {
        clearConnectionFeedback();
        return;
    }

    const QPointF scenePoint = _view->mapToScene(viewportPoint);
    auto *nodeGraphicsObject = QtNodes::locateNodeAt(scenePoint, *_scene, _view->transform());
    if (nodeGraphicsObject == nullptr) {
        _connectionFeedbackActive = true;
        Q_EMIT dropPreviewMessageChanged(tr("Drag to a compatible port."));
        return;
    }

    QtNodes::NodeConnectionInteraction interaction(*nodeGraphicsObject, *draftConnection, *_scene);
    QtNodes::PortIndex portIndex = QtNodes::InvalidPortIndex;
    if (interaction.canConnect(&portIndex)) {
        _connectionFeedbackActive = true;
        Q_EMIT dropPreviewMessageChanged(
            tr("Release to connect to %1")
                .arg(_graphModel->nodeData(nodeGraphicsObject->nodeId(), QtNodes::NodeRole::Caption).toString()));
        return;
    }

    _connectionFeedbackActive = true;
    Q_EMIT dropPreviewMessageChanged(tr("This port cannot accept the connection."));
}

void QtNodesEditorWidget::clearConnectionFeedback()
{
    if (!_connectionFeedbackActive)
        return;

    _connectionFeedbackActive = false;
    Q_EMIT dropPreviewMessageChanged(tr("Ready"));
}

void QtNodesEditorWidget::retranslateBuiltInContent()
{
    _builtInNodeRegistry.retranslate();
    _previewSizeCache.clear();

    for (auto nodeStateIt = _nodeStates.begin(); nodeStateIt != _nodeStates.end(); ++nodeStateIt) {
        auto const *definition = _builtInNodeRegistry.find(nodeStateIt->typeKey);
        if (definition == nullptr)
            continue;

        if (!nodeStateIt->displayNameCustomized)
            nodeStateIt->displayName = definition->displayName;

        if (!nodeStateIt->descriptionCustomized)
            nodeStateIt->description = definition->description;

        if (auto *model = _graphModel->delegateModel<StaticNodeDelegateModel>(nodeStateIt.key()); model != nullptr)
            model->setDisplayName(nodeStateIt->displayName);
    }

    if (_selectedNodeId != QtNodes::InvalidNodeId) {
        auto state = selectedState();
        if (state) {
            Q_EMIT selectedNodeChanged(state->typeKey, state->displayName, state->description, state->properties);
            emitSelectedNodeValidation();
        }
    }
}

QSize QtNodesEditorWidget::computePreviewNodeSize(QString const &typeKey) const
{
    if (_builtInNodeRegistry.find(typeKey) == nullptr)
        return {};

    QtNodes::DataFlowGraphModel previewModel(_registry);
    EdgeAlignedNodeGeometry previewGeometry(previewModel);
    const auto nodeId = previewModel.addNode(typeKey);
    if (nodeId == QtNodes::InvalidNodeId)
        return {};

    previewGeometry.recomputeSize(nodeId);
    return previewGeometry.size(nodeId);
}

void QtNodesEditorWidget::handleNodeSelected(QtNodes::NodeId nodeId)
{
    auto nodeStateIt = _nodeStates.find(nodeId);
    if (nodeStateIt == _nodeStates.end()) {
        _selectedNodeId = QtNodes::InvalidNodeId;
        Q_EMIT selectionCleared();
        return;
    }

    _selectedNodeId = nodeId;
    Q_EMIT selectedNodeChanged(nodeStateIt->typeKey,
                               nodeStateIt->displayName,
                               nodeStateIt->description,
                               nodeStateIt->properties);
    emitSelectedNodeValidation();
}

void QtNodesEditorWidget::emitSelectedNodeValidation()
{
    if (_selectedNodeId == QtNodes::InvalidNodeId) {
        Q_EMIT selectedNodeValidationChanged(QString(), QString(), QString());
        return;
    }

    const auto nodeStateIt = _nodeStates.constFind(_selectedNodeId);
    if (nodeStateIt == _nodeStates.cend()) {
        Q_EMIT selectedNodeValidationChanged(QString(), QString(), QString());
        return;
    }

    const ValidationResult result = validationResultFor(_selectedNodeId, nodeStateIt->typeKey, nodeStateIt->properties);
    Q_EMIT selectedNodeValidationChanged(nodeValidationState(_selectedNodeId), nodeValidationMessage(_selectedNodeId), result.propertyKey);
}

bool QtNodesEditorWidget::acceptNodeDrag(QMimeData const *mimeData,
                                         QPoint const &position,
                                         bool viewportCoordinates)
{
    if (!mimeData->hasFormat(NodeLibraryListWidget::MimeType))
        return false;

    const auto typeKey = QString::fromUtf8(mimeData->data(NodeLibraryListWidget::MimeType));
    const auto scenePosition = viewportCoordinates ? _view->mapToScene(position)
                                                   : scenePositionForWidgetPoint(position);
    return createNode(typeKey, scenePosition) != QtNodes::InvalidNodeId;
}

QPointF QtNodesEditorWidget::scenePositionForWidgetPoint(QPoint const &widgetPoint) const
{
    const auto globalPoint = mapToGlobal(widgetPoint);
    const auto viewportPoint = _view->viewport()->mapFromGlobal(globalPoint);
    return _view->mapToScene(viewportPoint);
}

QPointF QtNodesEditorWidget::defaultScenePosition() const
{
    return _view->mapToScene(_view->viewport()->rect().center());
}

std::optional<QtNodesEditorWidget::NodeState> QtNodesEditorWidget::selectedState() const
{
    auto nodeStateIt = _nodeStates.constFind(_selectedNodeId);
    if (nodeStateIt == _nodeStates.cend())
        return std::nullopt;

    return *nodeStateIt;
}

QtNodes::ConnectionGraphicsObject *QtNodesEditorWidget::draftConnectionGraphicsObject() const
{
    const auto items = _scene->items();
    for (auto *item : items) {
        auto *connectionObject = dynamic_cast<QtNodes::ConnectionGraphicsObject *>(item);
        if (connectionObject != nullptr && connectionObject->connectionState().requiresPort())
            return connectionObject;
    }

    return nullptr;
}

void QtNodesEditorWidget::showCanvasContextMenu(QPoint const &globalPos)
{
    QMenu menu;
    const auto nodeIds = selectedNodeIds();
    const bool hasSelectedNodes = !nodeIds.empty();

    bool hasSelectedConnections = false;
    for (auto *item : _scene->selectedItems()) {
        if (dynamic_cast<QtNodes::ConnectionGraphicsObject *>(item) != nullptr) {
            hasSelectedConnections = true;
            break;
        }
    }

    if (hasSelectedNodes) {
        QAction *deleteNodeAction = menu.addAction(tr("Delete Node"));
        connect(deleteNodeAction, &QAction::triggered, this, &QtNodesEditorWidget::deleteSelectedNodes);
    }

    if (hasSelectedConnections) {
        QAction *deleteConnectionAction = menu.addAction(tr("Delete Connection"));
        connect(deleteConnectionAction, &QAction::triggered, this, &QtNodesEditorWidget::deleteSelectedConnections);
    }

    if (hasSelectedNodes || hasSelectedConnections)
        menu.addSeparator();

    QAction *selectAllAction = menu.addAction(tr("Select All"));
    selectAllAction->setEnabled(hasNodes());
    connect(selectAllAction, &QAction::triggered, this, &QtNodesEditorWidget::selectAllNodes);

    menu.exec(globalPos);
}

QList<QtNodes::NodeId> QtNodesEditorWidget::sortedNodeIds() const
{
    QList<QtNodes::NodeId> nodeIds;
    nodeIds.reserve(_nodeStates.size());
    for (auto it = _nodeStates.cbegin(); it != _nodeStates.cend(); ++it) {
        nodeIds.push_back(it.key());
    }

    std::sort(nodeIds.begin(), nodeIds.end());
    return nodeIds;
}

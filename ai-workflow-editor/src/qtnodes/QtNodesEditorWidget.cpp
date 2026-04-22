#include "qtnodes/QtNodesEditorWidget.hpp"

#include "app/NodeLibraryListWidget.hpp"
#include "qtnodes/EdgeAlignedNodeGeometry.hpp"
#include "qtnodes/StaticNodeDelegateModel.hpp"
#include "qtnodes/StyledNodePainter.hpp"

#include <QtNodes/internal/ConnectionGraphicsObject.hpp>
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
#include <QPointF>
#include <QShortcut>
#include <QVBoxLayout>

#include <algorithm>

QtNodesEditorWidget::QtNodesEditorWidget(QWidget *parent)
    : QWidget(parent)
    , _registry(buildRegistry())
    , _graphModel(std::make_unique<QtNodes::DataFlowGraphModel>(_registry))
    , _scene(new QtNodes::DataFlowGraphicsScene(*_graphModel, this))
    , _view(new QtNodes::GraphicsView(_scene))
    , _dropPreview(new QFrame(_view->viewport()))
    , _selectedNodeId(QtNodes::InvalidNodeId)
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
        default:
            break;
        }
    }

    return QWidget::eventFilter(watched, event);
}

QtNodes::NodeId QtNodesEditorWidget::createNode(QString const &typeKey, QPointF const &scenePosition)
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

    selectNode(nodeId);
    Q_EMIT workflowModified();
    return nodeId;
}

bool QtNodesEditorWidget::connectNodes(QtNodes::NodeId outNodeId,
                                       QtNodes::PortIndex outPortIndex,
                                       QtNodes::NodeId inNodeId,
                                       QtNodes::PortIndex inPortIndex)
{
    QtNodes::ConnectionId connectionId{outNodeId, outPortIndex, inNodeId, inPortIndex};

    if (!_graphModel->connectionPossible(connectionId))
        return false;

    _graphModel->addConnection(connectionId);
    Q_EMIT workflowModified();
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
    const auto ids = selectedNodeIds();
    if (ids.empty())
        return;

    for (auto const nodeId : ids) {
        _nodeStates.remove(nodeId);
        _graphModel->deleteNode(nodeId);
    }

    _selectedNodeId = QtNodes::InvalidNodeId;
    Q_EMIT selectionCleared();
    Q_EMIT workflowModified();
}

void QtNodesEditorWidget::deleteSelectedConnections()
{
    const auto items = _scene->selectedItems();
    bool deleted = false;

    for (auto *item : items) {
        auto *connectionObject = dynamic_cast<QtNodes::ConnectionGraphicsObject *>(item);
        if (connectionObject == nullptr)
            continue;

        _graphModel->deleteConnection(connectionObject->connectionId());
        deleted = true;
    }

    if (deleted)
        Q_EMIT workflowModified();
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

void QtNodesEditorWidget::setSelectedNodeDisplayName(QString const &displayName)
{
    auto nodeStateIt = _nodeStates.find(_selectedNodeId);
    if (nodeStateIt == _nodeStates.end())
        return;

    auto const *definition = _builtInNodeRegistry.find(nodeStateIt->typeKey);
    nodeStateIt->displayName = displayName;
    nodeStateIt->displayNameCustomized = definition != nullptr && displayName != definition->displayName;

    if (auto *model = _graphModel->delegateModel<StaticNodeDelegateModel>(_selectedNodeId); model != nullptr) {
        model->setDisplayName(displayName);
    }

    Q_EMIT selectedNodeChanged(nodeStateIt->typeKey,
                               nodeStateIt->displayName,
                               nodeStateIt->description,
                               nodeStateIt->properties);
    Q_EMIT workflowModified();
}

void QtNodesEditorWidget::setSelectedNodeDescription(QString const &description)
{
    auto nodeStateIt = _nodeStates.find(_selectedNodeId);
    if (nodeStateIt == _nodeStates.end())
        return;

    auto const *definition = _builtInNodeRegistry.find(nodeStateIt->typeKey);
    nodeStateIt->description = description;
    nodeStateIt->descriptionCustomized = definition != nullptr && description != definition->description;
    Q_EMIT selectedNodeChanged(nodeStateIt->typeKey,
                               nodeStateIt->displayName,
                               nodeStateIt->description,
                               nodeStateIt->properties);
    Q_EMIT workflowModified();
}

void QtNodesEditorWidget::setSelectedNodeProperty(QString const &propertyKey, QVariant const &value)
{
    auto nodeStateIt = _nodeStates.find(_selectedNodeId);
    if (nodeStateIt == _nodeStates.end())
        return;

    nodeStateIt->properties.insert(propertyKey, value);
    Q_EMIT selectedNodeChanged(nodeStateIt->typeKey,
                               nodeStateIt->displayName,
                               nodeStateIt->description,
                               nodeStateIt->properties);
    Q_EMIT workflowModified();
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

    clearWorkflow();

    const auto root = document.object();
    const auto nodes = root["nodes"].toArray();
    QHash<int, QtNodes::NodeId> nodeIdMap;

    for (auto const &nodeValue : nodes) {
        const auto nodeObject = nodeValue.toObject();
        const auto positionObject = nodeObject["position"].toObject();
        const auto nodeId = createNode(nodeObject["type"].toString(),
                                       QPointF(positionObject["x"].toDouble(),
                                               positionObject["y"].toDouble()));
        if (nodeId == QtNodes::InvalidNodeId)
            return false;

        nodeIdMap.insert(nodeObject["id"].toInt(), nodeId);
        auto const *definition = _builtInNodeRegistry.find(nodeObject["type"].toString());
        _nodeStates[nodeId].displayName = nodeObject["displayName"].toString();
        _nodeStates[nodeId].description = nodeObject["description"].toString();
        _nodeStates[nodeId].properties = nodeObject["properties"].toObject().toVariantMap();
        if (definition != nullptr) {
            for (auto it = definition->defaultProperties.cbegin(); it != definition->defaultProperties.cend(); ++it) {
                if (!_nodeStates[nodeId].properties.contains(it.key()))
                    _nodeStates[nodeId].properties.insert(it.key(), it.value());
            }
        }
        _nodeStates[nodeId].displayNameCustomized =
            definition != nullptr && _nodeStates[nodeId].displayName != definition->displayName;
        _nodeStates[nodeId].descriptionCustomized =
            definition != nullptr && _nodeStates[nodeId].description != definition->description;

        if (auto *model = _graphModel->delegateModel<StaticNodeDelegateModel>(nodeId); model != nullptr) {
            model->setDisplayName(_nodeStates[nodeId].displayName);
        }
    }

    const auto connections = root["connections"].toArray();
    for (auto const &connectionValue : connections) {
        const auto connectionObject = connectionValue.toObject();
        const auto outNodeId = nodeIdMap.value(connectionObject["outNodeId"].toInt(), QtNodes::InvalidNodeId);
        const auto inNodeId = nodeIdMap.value(connectionObject["inNodeId"].toInt(), QtNodes::InvalidNodeId);

        if (outNodeId == QtNodes::InvalidNodeId || inNodeId == QtNodes::InvalidNodeId)
            return false;

        if (!connectNodes(outNodeId,
                          static_cast<QtNodes::PortIndex>(connectionObject["outPortIndex"].toInt()),
                          inNodeId,
                          static_cast<QtNodes::PortIndex>(connectionObject["inPortIndex"].toInt()))) {
            return false;
        }
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

    QtNodes::NodeStyle style;
    style.loadJson(QJsonObject::fromVariantMap(nodeStyleForType(typeKey)));
    model->setNodeStyle(style);
    if (auto *nodeGraphicsObject = _scene->nodeGraphicsObject(nodeId); nodeGraphicsObject != nullptr) {
        if (nodeGraphicsObject->graphicsEffect() != nullptr)
            nodeGraphicsObject->setGraphicsEffect(nullptr);
    }
    Q_EMIT model->requestNodeUpdate();
}

QVariantMap QtNodesEditorWidget::nodeStyleForType(QString const &typeKey) const
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
        if (state)
            Q_EMIT selectedNodeChanged(state->typeKey, state->displayName, state->description, state->properties);
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
    connect(selectAllAction, &QAction::triggered, this, [this]() {
        for (auto const nodeId : sortedNodeIds()) {
            if (auto *obj = _scene->nodeGraphicsObject(nodeId); obj != nullptr)
                obj->setSelected(true);
        }
    });

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

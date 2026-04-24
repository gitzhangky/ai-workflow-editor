#include "qtnodes/UndoCommands.hpp"

#include <QtNodes/internal/ConnectionGraphicsObject.hpp>
#include <QtNodes/DataFlowGraphModel>
#include <QtNodes/DataFlowGraphicsScene>

// --- NodeCreateCommand ---

NodeCreateCommand::NodeCreateCommand(QtNodesEditorWidget *editor, QString typeKey, QPointF scenePosition)
    : _editor(editor)
    , _typeKey(std::move(typeKey))
    , _scenePosition(scenePosition)
{}

QtNodes::NodeId NodeCreateCommand::nodeId() const
{
    return _snapshot.has_value() ? _snapshot->nodeId : QtNodes::InvalidNodeId;
}

void NodeCreateCommand::undo()
{
    if (_editor == nullptr || !_snapshot.has_value())
        return;

    _snapshot = _editor->snapshotNode(_snapshot->nodeId);
    _editor->deleteNodeInternal(_snapshot->nodeId);
    _editor->refreshSelectedNodeState();
    Q_EMIT _editor->workflowModified();
}

void NodeCreateCommand::redo()
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

// --- NodeDeleteSelectionCommand ---

NodeDeleteSelectionCommand::NodeDeleteSelectionCommand(QtNodesEditorWidget *editor, bool includeSelectedNodes)
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

void NodeDeleteSelectionCommand::undo()
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

void NodeDeleteSelectionCommand::redo()
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

// --- ConnectionCreateCommand ---

ConnectionCreateCommand::ConnectionCreateCommand(QtNodesEditorWidget *editor, QtNodes::ConnectionId connectionId)
    : _editor(editor)
    , _connectionId(connectionId)
{}

void ConnectionCreateCommand::undo()
{
    if (_editor == nullptr || !_editor->_graphModel->connectionExists(_connectionId))
        return;

    _editor->_graphModel->deleteConnection(_connectionId);
    _editor->refreshValidationForConnectionEndpoints(_connectionId);
    Q_EMIT _editor->workflowModified();
}

void ConnectionCreateCommand::redo()
{
    if (_editor == nullptr || _editor->_graphModel->connectionExists(_connectionId)
        || !_editor->_graphModel->connectionPossible(_connectionId)) {
        return;
    }

    _editor->_graphModel->addConnection(_connectionId);
    _editor->refreshValidationForConnectionEndpoints(_connectionId);
    Q_EMIT _editor->workflowModified();
}

// --- NodeDisplayNameEditCommand ---

NodeDisplayNameEditCommand::NodeDisplayNameEditCommand(QtNodesEditorWidget *editor,
                                                       QtNodes::NodeId nodeId,
                                                       QString oldValue,
                                                       QString newValue)
    : _editor(editor)
    , _nodeId(nodeId)
    , _oldValue(std::move(oldValue))
    , _newValue(std::move(newValue))
{}

int NodeDisplayNameEditCommand::id() const { return 1001; }

bool NodeDisplayNameEditCommand::mergeWith(QUndoCommand const *other)
{
    auto const *command = dynamic_cast<NodeDisplayNameEditCommand const *>(other);
    if (command == nullptr || command->_nodeId != _nodeId)
        return false;

    _newValue = command->_newValue;
    return true;
}

void NodeDisplayNameEditCommand::undo()
{
    if (_editor != nullptr)
        _editor->setNodeDisplayNameInternal(_nodeId, _oldValue);
}

void NodeDisplayNameEditCommand::redo()
{
    if (_editor != nullptr)
        _editor->setNodeDisplayNameInternal(_nodeId, _newValue);
}

// --- NodeDescriptionEditCommand ---

NodeDescriptionEditCommand::NodeDescriptionEditCommand(QtNodesEditorWidget *editor,
                                                       QtNodes::NodeId nodeId,
                                                       QString oldValue,
                                                       QString newValue)
    : _editor(editor)
    , _nodeId(nodeId)
    , _oldValue(std::move(oldValue))
    , _newValue(std::move(newValue))
{}

int NodeDescriptionEditCommand::id() const { return 1002; }

bool NodeDescriptionEditCommand::mergeWith(QUndoCommand const *other)
{
    auto const *command = dynamic_cast<NodeDescriptionEditCommand const *>(other);
    if (command == nullptr || command->_nodeId != _nodeId)
        return false;

    _newValue = command->_newValue;
    return true;
}

void NodeDescriptionEditCommand::undo()
{
    if (_editor != nullptr)
        _editor->setNodeDescriptionInternal(_nodeId, _oldValue);
}

void NodeDescriptionEditCommand::redo()
{
    if (_editor != nullptr)
        _editor->setNodeDescriptionInternal(_nodeId, _newValue);
}

// --- NodePropertyEditCommand ---

NodePropertyEditCommand::NodePropertyEditCommand(QtNodesEditorWidget *editor,
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

int NodePropertyEditCommand::id() const { return 1003; }

bool NodePropertyEditCommand::mergeWith(QUndoCommand const *other)
{
    auto const *command = dynamic_cast<NodePropertyEditCommand const *>(other);
    if (command == nullptr || command->_nodeId != _nodeId || command->_propertyKey != _propertyKey)
        return false;

    _newValue = command->_newValue;
    return true;
}

void NodePropertyEditCommand::undo()
{
    if (_editor != nullptr)
        _editor->setNodePropertyInternal(_nodeId, _propertyKey, _oldValue);
}

void NodePropertyEditCommand::redo()
{
    if (_editor != nullptr)
        _editor->setNodePropertyInternal(_nodeId, _propertyKey, _newValue);
}

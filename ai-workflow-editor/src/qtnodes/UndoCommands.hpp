#pragma once

#include "qtnodes/QtNodesEditorWidget.hpp"

#include <QtNodes/Definitions>
#include <QtNodes/internal/ConnectionIdHash.hpp>

#include <QPointF>
#include <QString>
#include <QUndoCommand>
#include <QVariant>

#include <optional>
#include <unordered_set>
#include <vector>

class NodeCreateCommand : public QUndoCommand
{
public:
    NodeCreateCommand(QtNodesEditorWidget *editor, QString typeKey, QPointF scenePosition);
    QtNodes::NodeId nodeId() const;
    void undo() override;
    void redo() override;

private:
    QtNodesEditorWidget *_editor;
    QString _typeKey;
    QPointF _scenePosition;
    std::optional<QtNodesEditorWidget::NodeSnapshot> _snapshot;
};

class NodeDeleteSelectionCommand : public QUndoCommand
{
public:
    explicit NodeDeleteSelectionCommand(QtNodesEditorWidget *editor, bool includeSelectedNodes);
    void undo() override;
    void redo() override;

private:
    QtNodesEditorWidget *_editor;
    std::vector<QtNodesEditorWidget::NodeSnapshot> _nodeSnapshots;
    std::unordered_set<QtNodes::ConnectionId> _connectionIds;
};

class ConnectionCreateCommand : public QUndoCommand
{
public:
    ConnectionCreateCommand(QtNodesEditorWidget *editor, QtNodes::ConnectionId connectionId);
    void undo() override;
    void redo() override;

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
                               QString newValue);
    int id() const override;
    bool mergeWith(QUndoCommand const *other) override;
    void undo() override;
    void redo() override;

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
                               QString newValue);
    int id() const override;
    bool mergeWith(QUndoCommand const *other) override;
    void undo() override;
    void redo() override;

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
                            QVariant newValue);
    int id() const override;
    bool mergeWith(QUndoCommand const *other) override;
    void undo() override;
    void redo() override;

private:
    QtNodesEditorWidget *_editor;
    QtNodes::NodeId _nodeId;
    QString _propertyKey;
    QVariant _oldValue;
    QVariant _newValue;
};

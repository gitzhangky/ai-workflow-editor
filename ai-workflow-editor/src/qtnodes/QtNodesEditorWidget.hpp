#pragma once

#include "registry/BuiltInNodeRegistry.hpp"

#include <QtNodes/Definitions>
#include <QtNodes/internal/NodeDelegateModel.hpp>

#include <QJsonObject>
#include <QVariant>
#include <QWidget>

#include <optional>
#include <memory>
#include <vector>

class QMimeData;

namespace QtNodes
{
class DataFlowGraphModel;
class DataFlowGraphicsScene;
class GraphicsView;
class NodeDelegateModelRegistry;
class ConnectionGraphicsObject;
}

class QtNodesEditorWidget : public QWidget
{
    Q_OBJECT

    friend class NodeCreateCommand;
    friend class ConnectionCreateCommand;
    friend class NodeDeleteSelectionCommand;
    friend class NodeDisplayNameEditCommand;
    friend class NodeDescriptionEditCommand;
    friend class NodePropertyEditCommand;

public:
    explicit QtNodesEditorWidget(QWidget *parent = nullptr);
    ~QtNodesEditorWidget() override;

    QtNodes::NodeId createNode(QString const &typeKey, QPointF const &scenePosition = QPointF());
    bool connectNodes(QtNodes::NodeId outNodeId,
                      QtNodes::PortIndex outPortIndex,
                      QtNodes::NodeId inNodeId,
                      QtNodes::PortIndex inPortIndex);
    void selectNode(QtNodes::NodeId nodeId);
    void clearWorkflow();
    void deleteSelectedNodes();
    void deleteSelectedConnections();
    void deleteSelection();
    void selectAllNodes();
    void centerSelection();
    void undo();
    void redo();
    std::vector<QtNodes::NodeId> selectedNodeIds() const;
    int nodeCount() const;
    int connectionCount() const;
    bool dropPreviewVisible() const;
    bool usesStyledNodePainter() const;
    qreal styledNodeCornerRadius() const;
    QSize dropPreviewSize() const;
    QSize previewNodeSize(QString const &typeKey) const;
    bool nodeHasGraphicsEffect(QtNodes::NodeId nodeId) const;
    QPointF portPosition(QtNodes::NodeId nodeId, QtNodes::PortType portType, QtNodes::PortIndex portIndex) const;
    QSize nodeSize(QtNodes::NodeId nodeId) const;
    QVariantMap nodeStyle(QtNodes::NodeId nodeId) const;
    QString nodeValidationState(QtNodes::NodeId nodeId) const;
    QString nodeValidationMessage(QtNodes::NodeId nodeId) const;
    QString selectedNodeDisplayName() const;
    QString selectedNodeDescription() const;
    QVariant selectedNodeProperty(QString const &propertyKey) const;
    QStringList workflowDisplayNames() const;
    QString descriptionForDisplayName(QString const &displayName) const;
    QtNodes::NodeId findNodeIdByDisplayName(QString const &displayName) const;
    QPointF portScenePosition(QtNodes::NodeId nodeId, QtNodes::PortType portType, QtNodes::PortIndex portIndex) const;
    bool hasSelection() const;
    bool hasNodes() const;
    bool canUndo() const;
    bool canRedo() const;
    bool isClean() const;
    void markCurrentStateClean();
    void markCurrentStateDirty();
    void setSelectedNodeDisplayName(QString const &displayName);
    void setSelectedNodeDescription(QString const &description);
    void setSelectedNodeProperty(QString const &propertyKey, QVariant const &value);
    bool saveWorkflow(QString const &filePath) const;
    bool loadWorkflow(QString const &filePath);

Q_SIGNALS:
    void dropPreviewMessageChanged(QString const &message);
    void selectedNodeChanged(QString const &typeKey,
                             QString const &displayName,
                             QString const &description,
                             QVariantMap const &properties);
    void selectedNodeValidationChanged(QString const &state, QString const &message, QString const &propertyKey);
    void selectionCleared();
    void workflowModified();
    void cleanStateChanged(bool clean);
    void interactionStateChanged();

protected:
    void changeEvent(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    struct NodeState
    {
        QString typeKey;
        QString displayName;
        QString description;
        QVariantMap properties;
        bool displayNameCustomized;
        bool descriptionCustomized;
    };

    struct NodeSnapshot
    {
        QtNodes::NodeId nodeId;
        QJsonObject nodeJson;
        NodeState state;
    };

    struct ValidationResult
    {
        QtNodes::NodeValidationState::State state = QtNodes::NodeValidationState::State::Valid;
        QString message;
        QString propertyKey;
    };

    std::shared_ptr<QtNodes::NodeDelegateModelRegistry> buildRegistry() const;
    void applyWorkbenchStyles();
    QtNodes::NodeId createNodeInternal(QString const &typeKey, QPointF const &scenePosition = QPointF());
    QtNodes::NodeId restoreNodeInternal(NodeSnapshot const &snapshot);
    bool deleteNodeInternal(QtNodes::NodeId nodeId);
    void setNodeDisplayNameInternal(QtNodes::NodeId nodeId, QString const &displayName);
    void setNodeDescriptionInternal(QtNodes::NodeId nodeId, QString const &description);
    void setNodePropertyInternal(QtNodes::NodeId nodeId, QString const &propertyKey, QVariant const &value);
    NodeSnapshot snapshotNode(QtNodes::NodeId nodeId) const;
    void refreshSelectedNodeState();
    void applyNodeStyle(QtNodes::NodeId nodeId, QString const &typeKey);
    void refreshValidationForNode(QtNodes::NodeId nodeId);
    void refreshValidationForConnectionEndpoints(QtNodes::ConnectionId const &connectionId);
    QVariantMap nodeStyleForType(QString const &typeKey,
                                 QVariantMap const &properties = QVariantMap()) const;
    ValidationResult validationResultFor(QtNodes::NodeId nodeId,
                                         QString const &typeKey,
                                         QVariantMap const &properties) const;
    QtNodes::NodeValidationState validationStateFor(QtNodes::NodeId nodeId,
                                                    QString const &typeKey,
                                                    QVariantMap const &properties) const;
    void updateDropPreview(QString const &typeKey, QPoint const &position, bool viewportCoordinates);
    void clearDropPreview();
    void updateConnectionFeedback(QPoint const &viewportPoint);
    void clearConnectionFeedback();
    void handleNodeSelected(QtNodes::NodeId nodeId);
    void emitSelectedNodeValidation();
    void retranslateBuiltInContent();
    bool acceptNodeDrag(QMimeData const *mimeData, QPoint const &position, bool viewportCoordinates);
    QSize computePreviewNodeSize(QString const &typeKey) const;
    QPointF scenePositionForWidgetPoint(QPoint const &widgetPoint) const;
    QPointF defaultScenePosition() const;
    std::optional<NodeState> selectedState() const;
    QtNodes::ConnectionGraphicsObject *draftConnectionGraphicsObject() const;
    void showCanvasContextMenu(QPoint const &globalPos);
    QList<QtNodes::NodeId> sortedNodeIds() const;

    BuiltInNodeRegistry _builtInNodeRegistry;
    std::shared_ptr<QtNodes::NodeDelegateModelRegistry> _registry;
    std::unique_ptr<QtNodes::DataFlowGraphModel> _graphModel;
    QtNodes::DataFlowGraphicsScene *_scene;
    QtNodes::GraphicsView *_view;
    QWidget *_dropPreview;
    mutable QHash<QString, QSize> _previewSizeCache;
    QHash<QtNodes::NodeId, NodeState> _nodeStates;
    QtNodes::NodeId _selectedNodeId;
    bool _connectionFeedbackActive;
};

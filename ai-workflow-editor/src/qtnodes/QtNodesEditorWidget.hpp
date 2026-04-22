#pragma once

#include "registry/BuiltInNodeRegistry.hpp"

#include <QtNodes/Definitions>

#include <QVariant>
#include <QWidget>

#include <optional>
#include <memory>

class QMimeData;

namespace QtNodes
{
class DataFlowGraphModel;
class DataFlowGraphicsScene;
class GraphicsView;
class NodeDelegateModelRegistry;
}

class QtNodesEditorWidget : public QWidget
{
    Q_OBJECT

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
    QString selectedNodeDisplayName() const;
    QString selectedNodeDescription() const;
    QVariant selectedNodeProperty(QString const &propertyKey) const;
    QStringList workflowDisplayNames() const;
    QString descriptionForDisplayName(QString const &displayName) const;
    QtNodes::NodeId findNodeIdByDisplayName(QString const &displayName) const;
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
    void selectionCleared();

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

    std::shared_ptr<QtNodes::NodeDelegateModelRegistry> buildRegistry() const;
    void applyWorkbenchStyles();
    void applyNodeStyle(QtNodes::NodeId nodeId, QString const &typeKey);
    QVariantMap nodeStyleForType(QString const &typeKey) const;
    void updateDropPreview(QString const &typeKey, QPoint const &position, bool viewportCoordinates);
    void clearDropPreview();
    void handleNodeSelected(QtNodes::NodeId nodeId);
    void retranslateBuiltInContent();
    bool acceptNodeDrag(QMimeData const *mimeData, QPoint const &position, bool viewportCoordinates);
    QSize computePreviewNodeSize(QString const &typeKey) const;
    QPointF scenePositionForWidgetPoint(QPoint const &widgetPoint) const;
    QPointF defaultScenePosition() const;
    std::optional<NodeState> selectedState() const;
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
};

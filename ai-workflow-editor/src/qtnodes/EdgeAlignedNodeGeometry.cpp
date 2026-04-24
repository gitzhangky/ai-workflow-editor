#include "qtnodes/EdgeAlignedNodeGeometry.hpp"

#include <QtNodes/AbstractGraphModel>

#include <QMarginsF>

#include <algorithm>

EdgeAlignedNodeGeometry::EdgeAlignedNodeGeometry(QtNodes::AbstractGraphModel &graphModel)
    : QtNodes::DefaultHorizontalNodeGeometry(graphModel)
{
}

QRectF EdgeAlignedNodeGeometry::boundingRect(QtNodes::NodeId nodeId) const
{
    return QtNodes::DefaultHorizontalNodeGeometry::boundingRect(nodeId).marginsAdded(
        QMarginsF(HorizontalMargin, 0.0, HorizontalMargin, 0.0));
}

void EdgeAlignedNodeGeometry::recomputeSize(QtNodes::NodeId nodeId) const
{
    QtNodes::DefaultHorizontalNodeGeometry::recomputeSize(nodeId);

    QSize size = _graphModel.nodeData<QSize>(nodeId, QtNodes::NodeRole::Size);
    size.setWidth(std::max(size.width(), MinimumCardWidth));
    size.setHeight(std::max(size.height(), MinimumCardHeight));
    _graphModel.setNodeData(nodeId, QtNodes::NodeRole::Size, size);
}

QPointF EdgeAlignedNodeGeometry::portPosition(QtNodes::NodeId nodeId,
                                              QtNodes::PortType portType,
                                              QtNodes::PortIndex index) const
{
    return QtNodes::DefaultHorizontalNodeGeometry::portPosition(nodeId, portType, index);
}

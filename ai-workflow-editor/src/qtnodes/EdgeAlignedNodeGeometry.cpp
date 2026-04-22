#include "qtnodes/EdgeAlignedNodeGeometry.hpp"

#include <QMarginsF>

EdgeAlignedNodeGeometry::EdgeAlignedNodeGeometry(QtNodes::AbstractGraphModel &graphModel)
    : QtNodes::DefaultHorizontalNodeGeometry(graphModel)
{
}

QRectF EdgeAlignedNodeGeometry::boundingRect(QtNodes::NodeId nodeId) const
{
    return QtNodes::DefaultHorizontalNodeGeometry::boundingRect(nodeId).marginsAdded(
        QMarginsF(HorizontalMargin, 0.0, HorizontalMargin, 0.0));
}

QPointF EdgeAlignedNodeGeometry::portPosition(QtNodes::NodeId nodeId,
                                              QtNodes::PortType portType,
                                              QtNodes::PortIndex index) const
{
    return QtNodes::DefaultHorizontalNodeGeometry::portPosition(nodeId, portType, index);
}

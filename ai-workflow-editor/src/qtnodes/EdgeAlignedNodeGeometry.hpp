#pragma once

#include <QtNodes/internal/DefaultHorizontalNodeGeometry.hpp>

class EdgeAlignedNodeGeometry : public QtNodes::DefaultHorizontalNodeGeometry
{
public:
    explicit EdgeAlignedNodeGeometry(QtNodes::AbstractGraphModel &graphModel);

    QRectF boundingRect(QtNodes::NodeId nodeId) const override;
    void recomputeSize(QtNodes::NodeId nodeId) const override;
    QPointF portPosition(QtNodes::NodeId nodeId,
                         QtNodes::PortType portType,
                         QtNodes::PortIndex index) const override;

private:
    static constexpr qreal HorizontalMargin = 8.0;
    static constexpr int MinimumCardWidth = 176;
    static constexpr int MinimumCardHeight = 96;
};

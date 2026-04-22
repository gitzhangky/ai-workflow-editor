#pragma once

#include <QtNodes/AbstractNodePainter>

class StyledNodePainter : public QtNodes::AbstractNodePainter
{
public:
    static constexpr qreal CardCornerRadius = 8.0;

    void paint(QPainter *painter, QtNodes::NodeGraphicsObject &ngo) const override;
    qreal cornerRadius() const { return CardCornerRadius; }

private:
    void drawCard(QPainter *painter, QtNodes::NodeGraphicsObject &ngo) const;
    void drawHeader(QPainter *painter, QtNodes::NodeGraphicsObject &ngo) const;
    void drawTitle(QPainter *painter, QtNodes::NodeGraphicsObject &ngo) const;
    void drawPorts(QPainter *painter, QtNodes::NodeGraphicsObject &ngo) const;
    void drawContentHints(QPainter *painter, QtNodes::NodeGraphicsObject &ngo) const;
};

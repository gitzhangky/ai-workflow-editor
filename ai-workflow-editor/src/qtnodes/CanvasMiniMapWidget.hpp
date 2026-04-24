#pragma once

#include <QRectF>
#include <QVector>
#include <QWidget>

class QGraphicsView;

class CanvasMiniMapWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CanvasMiniMapWidget(QGraphicsView *view, QWidget *parent = nullptr);

    void setSceneSnapshot(QVector<QRectF> const &nodeSceneRects, QRectF const &viewportSceneRect);
    QRectF viewportIndicatorRect() const;
    bool hasContent() const;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    QRectF contentRect() const;
    QRectF displayRect() const;
    QRectF mapSceneRectToDisplay(QRectF const &sceneRect) const;
    QPointF mapDisplayPointToScene(QPointF const &displayPoint) const;
    void centerViewOn(QPoint const &widgetPoint);

    QGraphicsView *_view;
    QVector<QRectF> _nodeSceneRects;
    QRectF _sceneRect;
    QRectF _viewportSceneRect;
    QRectF _viewportIndicatorRect;
    bool _hasContent;
};

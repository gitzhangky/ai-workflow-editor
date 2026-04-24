#include "qtnodes/CanvasMiniMapWidget.hpp"

#include <QGraphicsView>
#include <QMarginsF>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

#include <algorithm>

namespace
{
constexpr qreal OuterRadius = 12.0;
constexpr qreal ContentPadding = 10.0;
constexpr qreal SceneMargin = 80.0;
}

CanvasMiniMapWidget::CanvasMiniMapWidget(QGraphicsView *view, QWidget *parent)
    : QWidget(parent)
    , _view(view)
    , _hasContent(false)
    , _draggingViewport(false)
{
    setObjectName("canvasMiniMap");
    setAttribute(Qt::WA_StyledBackground, false);
    setMouseTracking(true);
    hide();
}

void CanvasMiniMapWidget::setSceneSnapshot(QVector<QRectF> const &nodeSceneRects, QRectF const &viewportSceneRect)
{
    _nodeSceneRects = nodeSceneRects;
    _sceneRect = {};
    _viewportSceneRect = {};
    _viewportIndicatorRect = {};
    _hasContent = false;

    if (_nodeSceneRects.isEmpty())
        return;

    for (auto const &sceneNodeRect : _nodeSceneRects) {
        if (!sceneNodeRect.isValid() || sceneNodeRect.isEmpty())
            continue;

        _sceneRect = _sceneRect.isValid() ? _sceneRect.united(sceneNodeRect) : sceneNodeRect;
    }

    if (!_sceneRect.isValid())
        return;

    _hasContent = true;
    _viewportSceneRect = viewportSceneRect;
    _sceneRect = _sceneRect.united(_viewportSceneRect).marginsAdded(
        QMarginsF(SceneMargin, SceneMargin, SceneMargin, SceneMargin));
    _viewportIndicatorRect = mapSceneRectToDisplay(_viewportSceneRect);
}

QRectF CanvasMiniMapWidget::viewportIndicatorRect() const
{
    return _viewportIndicatorRect;
}

QVector<QRectF> CanvasMiniMapWidget::nodeIndicatorRects() const
{
    QVector<QRectF> mappedRects;
    mappedRects.reserve(_nodeSceneRects.size());

    for (auto const &sceneNodeRect : _nodeSceneRects) {
        const QRectF mappedRect = mapSceneRectToDisplay(sceneNodeRect);
        mappedRects.append(mappedRect.adjusted(0.0,
                                               0.0,
                                               std::max(2.0 - mappedRect.width(), 0.0),
                                               std::max(2.0 - mappedRect.height(), 0.0)));
    }

    return mappedRects;
}

bool CanvasMiniMapWidget::hasContent() const
{
    return _hasContent;
}

void CanvasMiniMapWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    if (!_hasContent)
        return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPainterPath panelPath;
    panelPath.addRoundedRect(rect().adjusted(0, 0, -1, -1), OuterRadius, OuterRadius);
    painter.fillPath(panelPath, QColor(QStringLiteral("#faf6ef")));
    painter.setPen(QPen(QColor(QStringLiteral("#d8cfbf")), 1.0));
    painter.drawPath(panelPath);

    const QRectF display = displayRect();
    painter.fillRect(display, QColor(QStringLiteral("#f2eadf")));

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(QStringLiteral("#b7ab97")));
    for (auto const &nodeRect : nodeIndicatorRects())
        painter.drawRoundedRect(nodeRect, 2.0, 2.0);

    painter.setBrush(QColor(165, 137, 101, 28));
    painter.setPen(QPen(QColor(QStringLiteral("#a58965")), 1.2));
    painter.drawRoundedRect(_viewportIndicatorRect, 4.0, 4.0);
}

void CanvasMiniMapWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (_viewportIndicatorRect.contains(event->pos())) {
            _draggingViewport = true;
            _viewportDragOffset = event->pos() - _viewportIndicatorRect.center();
        } else {
            centerViewOn(event->pos());
        }
    }

    QWidget::mousePressEvent(event);
}

void CanvasMiniMapWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons().testFlag(Qt::LeftButton)) {
        const QPointF targetPoint = _draggingViewport ? event->pos() - _viewportDragOffset : event->pos();
        centerViewOn(targetPoint);
    }

    QWidget::mouseMoveEvent(event);
}

void CanvasMiniMapWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        _draggingViewport = false;

    QWidget::mouseReleaseEvent(event);
}

QRectF CanvasMiniMapWidget::contentRect() const
{
    return rect().adjusted(ContentPadding, ContentPadding, -ContentPadding, -ContentPadding);
}

QRectF CanvasMiniMapWidget::displayRect() const
{
    const QRectF content = contentRect();
    if (!_sceneRect.isValid() || _sceneRect.width() <= 0.0 || _sceneRect.height() <= 0.0)
        return content;

    const qreal sceneAspect = _sceneRect.width() / _sceneRect.height();
    const qreal contentAspect = content.width() / content.height();

    if (sceneAspect > contentAspect) {
        const qreal height = content.width() / sceneAspect;
        return QRectF(content.left(),
                      content.center().y() - (height / 2.0),
                      content.width(),
                      height);
    }

    const qreal width = content.height() * sceneAspect;
    return QRectF(content.center().x() - (width / 2.0),
                  content.top(),
                  width,
                  content.height());
}

QRectF CanvasMiniMapWidget::mapSceneRectToDisplay(QRectF const &sceneRect) const
{
    const QRectF display = displayRect();
    if (!_sceneRect.isValid() || _sceneRect.width() <= 0.0 || _sceneRect.height() <= 0.0)
        return {};

    const qreal scaleX = display.width() / _sceneRect.width();
    const qreal scaleY = display.height() / _sceneRect.height();

    return QRectF(display.left() + ((sceneRect.left() - _sceneRect.left()) * scaleX),
                  display.top() + ((sceneRect.top() - _sceneRect.top()) * scaleY),
                  std::max(sceneRect.width() * scaleX, 2.0),
                  std::max(sceneRect.height() * scaleY, 2.0));
}

QPointF CanvasMiniMapWidget::mapDisplayPointToScene(QPointF const &displayPoint) const
{
    const QRectF display = displayRect();
    if (!_sceneRect.isValid() || _sceneRect.width() <= 0.0 || _sceneRect.height() <= 0.0
        || !display.isValid()) {
        return {};
    }

    const qreal xRatio = std::clamp((displayPoint.x() - display.left()) / display.width(), 0.0, 1.0);
    const qreal yRatio = std::clamp((displayPoint.y() - display.top()) / display.height(), 0.0, 1.0);

    return QPointF(_sceneRect.left() + (_sceneRect.width() * xRatio),
                   _sceneRect.top() + (_sceneRect.height() * yRatio));
}

void CanvasMiniMapWidget::centerViewOn(QPointF const &widgetPoint)
{
    if (!_hasContent || _view == nullptr)
        return;

    _view->centerOn(mapDisplayPointToScene(widgetPoint));
    update();
}

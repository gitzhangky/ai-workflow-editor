#include "app/NodeLibraryListWidget.hpp"

#include <QAbstractItemDelegate>
#include <QDrag>
#include <QEvent>
#include <QFont>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QScrollBar>
#include <QShowEvent>
#include <QStyledItemDelegate>
#include <QTimer>

#include <algorithm>
#include <cmath>

namespace
{
QPainterPath cardPath(QRectF const &rect, qreal radius, bool roundTop, bool roundBottom)
{
    QPainterPath path;
    const qreal left = rect.left();
    const qreal right = rect.right();
    const qreal top = rect.top();
    const qreal bottom = rect.bottom();
    const qreal effectiveRadius = std::min(radius, rect.height() / 2.0);

    path.moveTo(left, bottom);

    if (roundTop) {
        path.lineTo(left, top + effectiveRadius);
        path.quadTo(left, top, left + effectiveRadius, top);
        path.lineTo(right - effectiveRadius, top);
        path.quadTo(right, top, right, top + effectiveRadius);
    } else {
        path.lineTo(left, top);
        path.lineTo(right, top);
    }

    if (roundBottom) {
        path.lineTo(right, bottom - effectiveRadius);
        path.quadTo(right, bottom, right - effectiveRadius, bottom);
        path.lineTo(left + effectiveRadius, bottom);
        path.quadTo(left, bottom, left, bottom - effectiveRadius);
    } else {
        path.lineTo(right, bottom);
        path.lineTo(left, bottom);
    }

    path.closeSubpath();
    return path;
}

class NodeLibraryItemDelegate final : public QStyledItemDelegate
{
public:
    explicit NodeLibraryItemDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
    {
        setObjectName(QStringLiteral("nodeLibraryItemDelegate"));
    }

    void paint(QPainter *painter,
               QStyleOptionViewItem const &option,
               QModelIndex const &index) const override
    {
        const auto itemKind =
            static_cast<NodeLibraryListWidget::ItemKind>(index.data(NodeLibraryListWidget::ItemKindRole).toInt());
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);

        const auto visualState = static_cast<NodeLibraryListWidget::CardVisualState>(
            index.data(NodeLibraryListWidget::CardVisualStateRole).toInt());
        const bool hovered = option.state.testFlag(QStyle::State_MouseOver);
        const bool selected = option.state.testFlag(QStyle::State_Selected);

        if (itemKind == NodeLibraryListWidget::ItemKind::SectionHeader) {
            const bool collapsed = index.data(NodeLibraryListWidget::SectionCollapsedRole).toBool();
            const bool hasChildren = visualState == NodeLibraryListWidget::CardVisualState::SectionTop;

            QRectF rect = NodeLibraryListWidget::alignedCardRect(
                option.rect.adjusted(8, 8, -8, hasChildren ? 0 : -6));
            auto path = cardPath(rect, 12.0, true, !hasChildren);
            QColor fill = hovered ? QColor(QStringLiteral("#f3ecdf")) : QColor(QStringLiteral("#efe7d8"));
            QColor border = QColor(QStringLiteral("#ddd2c2"));
            painter->fillPath(path, fill);
            painter->setPen(QPen(border, 1.0));
            painter->drawPath(path);

            const auto icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
            const QSize iconSize(16, 16);
            QRectF iconRect(rect.left() + 12.0,
                            rect.center().y() - (iconSize.height() / 2.0),
                            iconSize.width(),
                            iconSize.height());
            icon.paint(painter, iconRect.toRect());

            QFont font = option.font;
            font.setBold(true);
            font.setPointSizeF(font.pointSizeF() - 1.0);
            painter->setFont(font);
            painter->setPen(QColor(QStringLiteral("#6d6256")));

            QRectF textRect = rect.adjusted(36.0, 0.0, -34.0, 0.0);
            painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, index.data(Qt::DisplayRole).toString());

            QRectF chevronRect(rect.right() - 20.0, rect.center().y() - 6.0, 10.0, 10.0);
            QPen chevronPen(QColor(QStringLiteral("#8a7d70")));
            chevronPen.setWidthF(1.8);
            chevronPen.setCapStyle(Qt::RoundCap);
            chevronPen.setJoinStyle(Qt::RoundJoin);
            painter->setPen(chevronPen);

            if (collapsed) {
                painter->drawLine(QPointF(chevronRect.left() + 2.0, chevronRect.top() + 1.0),
                                  QPointF(chevronRect.right() - 1.0, chevronRect.center().y()));
                painter->drawLine(QPointF(chevronRect.left() + 2.0, chevronRect.bottom() - 1.0),
                                  QPointF(chevronRect.right() - 1.0, chevronRect.center().y()));
            } else {
                painter->drawLine(QPointF(chevronRect.left() + 1.0, chevronRect.top() + 2.0),
                                  QPointF(chevronRect.center().x(), chevronRect.bottom() - 1.0));
                painter->drawLine(QPointF(chevronRect.right() - 1.0, chevronRect.top() + 2.0),
                                  QPointF(chevronRect.center().x(), chevronRect.bottom() - 1.0));
            }

            if (hasChildren) {
                painter->setPen(QColor(QStringLiteral("#e5dccf")));
                painter->drawLine(QPointF(rect.left() + 12.0, rect.bottom()),
                                  QPointF(rect.right() - 12.0, rect.bottom()));
            }

            painter->restore();
            return;
        }

        const bool isBottom = visualState == NodeLibraryListWidget::CardVisualState::NodeBottom;
        QRectF rect = NodeLibraryListWidget::alignedCardRect(option.rect.adjusted(8, 0, -8, isBottom ? -8 : 0));
        auto path = cardPath(rect, 12.0, false, isBottom);
        QColor fill = selected ? QColor(QStringLiteral("#eaf2ff"))
                               : (hovered ? QColor(QStringLiteral("#f8fbff")) : QColor(QStringLiteral("#fffdf9")));
        QColor border = selected ? QColor(QStringLiteral("#c8d7ef")) : QColor(QStringLiteral("#e2d9cc"));
        painter->fillPath(path, fill);
        painter->setPen(QPen(border, 1.0));
        painter->drawPath(path);

        painter->setPen(QColor(QStringLiteral("#ece4d8")));
        painter->drawLine(QPointF(rect.left() + 46.0, rect.top()), QPointF(rect.right() - 12.0, rect.top()));

        const auto icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
        const QSize iconSize(18, 18);
        QRectF iconRect(rect.left() + 12.0,
                        rect.top() + 10.0,
                        iconSize.width(),
                        iconSize.height());
        icon.paint(painter, iconRect.toRect());

        QFont titleFont = option.font;
        titleFont.setBold(true);
        painter->setFont(titleFont);
        painter->setPen(selected ? QColor(QStringLiteral("#20344f")) : QColor(QStringLiteral("#2d241f")));

        QRectF titleRect(rect.left() + 42.0, rect.top() + 8.0, rect.width() - 54.0, 18.0);
        painter->drawText(titleRect, Qt::AlignLeft | Qt::AlignVCenter, index.data(Qt::DisplayRole).toString());

        QFont detailFont = option.font;
        detailFont.setPointSizeF(detailFont.pointSizeF() - 1.0);
        painter->setFont(detailFont);
        painter->setPen(QColor(QStringLiteral("#7b6f61")));
        QRectF detailRect(rect.left() + 42.0, rect.top() + 26.0, rect.width() - 54.0, 16.0);
        painter->drawText(detailRect,
                          Qt::AlignLeft | Qt::AlignVCenter,
                          painter->fontMetrics().elidedText(index.data(Qt::ToolTipRole).toString(),
                                                            Qt::ElideRight,
                                                            static_cast<int>(detailRect.width())));

        const int inPorts = index.data(NodeLibraryListWidget::InPortCountRole).toInt();
        const int outPorts = index.data(NodeLibraryListWidget::OutPortCountRole).toInt();
        if (inPorts > 0 || outPorts > 0) {
            const QString portText = QStringLiteral("%1→%2").arg(inPorts).arg(outPorts);
            QFont badgeFont = option.font;
            badgeFont.setPointSizeF(badgeFont.pointSizeF() - 2.0);
            badgeFont.setBold(true);
            painter->setFont(badgeFont);
            const QFontMetricsF badgeMetrics(badgeFont);
            const qreal textWidth = badgeMetrics.horizontalAdvance(portText);
            const qreal badgeWidth = textWidth + 10.0;
            const qreal badgeHeight = 16.0;
            QRectF badgeRect(rect.right() - badgeWidth - 10.0,
                             rect.top() + 10.0,
                             badgeWidth,
                             badgeHeight);
            QPainterPath badgePath;
            badgePath.addRoundedRect(badgeRect, 4.0, 4.0);
            painter->setPen(Qt::NoPen);
            painter->setBrush(QColor(QStringLiteral("#efe7d8")));
            painter->drawPath(badgePath);
            painter->setPen(QColor(QStringLiteral("#7a6b5e")));
            painter->drawText(badgeRect, Qt::AlignCenter, portText);
        }

        painter->restore();
    }

    QSize sizeHint(QStyleOptionViewItem const &option, QModelIndex const &index) const override
    {
        const auto itemKind =
            static_cast<NodeLibraryListWidget::ItemKind>(index.data(NodeLibraryListWidget::ItemKindRole).toInt());
        if (itemKind == NodeLibraryListWidget::ItemKind::SectionHeader)
            return QSize(option.rect.width(), 40);

        return QSize(option.rect.width(), 56);
    }
};

class NodeLibraryScrollOverlay final : public QWidget
{
public:
    explicit NodeLibraryScrollOverlay(QScrollBar *scrollBar, QWidget *parent = nullptr)
        : QWidget(parent)
        , _scrollBar(scrollBar)
        , _thumbRect()
        , _dragging(false)
        , _dragOffset(0.0)
    {
        setObjectName(QStringLiteral("nodeLibraryScrollOverlay"));
        setAttribute(Qt::WA_StyledBackground, false);
        setMouseTracking(true);
        hide();
    }

    void syncToScrollBar()
    {
        _thumbRect = thumbRectForValue();
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        if (_scrollBar == nullptr || _scrollBar->maximum() <= 0 || !_thumbRect.isValid())
            return;

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        const QRectF track = alignedRect(trackRect());
        const QRectF thumb = alignedRect(_thumbRect);
        painter.setPen(Qt::NoPen);
        painter.setBrush(NodeLibraryListWidget::scrollIndicatorTrackColor());
        painter.drawRoundedRect(track,
                                NodeLibraryListWidget::scrollIndicatorCornerRadius(),
                                NodeLibraryListWidget::scrollIndicatorCornerRadius());

        painter.setBrush(NodeLibraryListWidget::scrollIndicatorThumbColor());
        painter.drawRoundedRect(thumb,
                                NodeLibraryListWidget::scrollIndicatorCornerRadius(),
                                NodeLibraryListWidget::scrollIndicatorCornerRadius());
    }

    void mousePressEvent(QMouseEvent *event) override
    {
        if (_scrollBar == nullptr || _scrollBar->maximum() <= 0) {
            event->ignore();
            return;
        }

        if (_thumbRect.contains(event->pos())) {
            _dragging = true;
            _dragOffset = event->localPos().y() - _thumbRect.top();
        } else {
            const qreal desiredTop = event->localPos().y() - (_thumbRect.height() / 2.0);
            _dragOffset = _thumbRect.height() / 2.0;
            _dragging = true;
            _scrollBar->setValue(scrollValueForThumbTop(desiredTop));
            syncToScrollBar();
        }

        event->accept();
    }

    void mouseMoveEvent(QMouseEvent *event) override
    {
        if (!_dragging || _scrollBar == nullptr) {
            event->ignore();
            return;
        }

        const qreal desiredTop = event->localPos().y() - _dragOffset;
        _scrollBar->setValue(scrollValueForThumbTop(desiredTop));
        syncToScrollBar();
        event->accept();
    }

    void mouseReleaseEvent(QMouseEvent *event) override
    {
        _dragging = false;
        event->accept();
    }

    void wheelEvent(QWheelEvent *event) override
    {
        event->ignore();
    }

private:
    QRectF trackRect() const
    {
        constexpr qreal sideInset = 1.0;
        constexpr qreal topInset = 1.0;
        const QRectF rect = this->rect();
        return rect.adjusted(sideInset, topInset, -sideInset, -topInset);
    }

    QRectF alignedRect(QRectF const &rect) const
    {
        return rect.adjusted(0.5, 0.5, -0.5, -0.5);
    }

    QRectF thumbRectForValue() const
    {
        const QRectF track = trackRect();
        if (_scrollBar == nullptr || _scrollBar->maximum() <= 0 || track.height() <= 0.0)
            return {};

        const qreal documentExtent = static_cast<qreal>(_scrollBar->maximum() + _scrollBar->pageStep());
        if (documentExtent <= 0.0)
            return {};

        const qreal thumbHeight =
            std::clamp(track.height() * (static_cast<qreal>(_scrollBar->pageStep()) / documentExtent), 34.0, track.height());
        const qreal travel = std::max(0.0, track.height() - thumbHeight);
        const qreal ratio =
            _scrollBar->maximum() > 0 ? static_cast<qreal>(_scrollBar->value()) / static_cast<qreal>(_scrollBar->maximum())
                                      : 0.0;
        return QRectF(track.left(), track.top() + (travel * ratio), track.width(), thumbHeight);
    }

    int scrollValueForThumbTop(qreal thumbTop) const
    {
        const QRectF track = trackRect();
        if (_scrollBar == nullptr || _scrollBar->maximum() <= 0 || _thumbRect.height() <= 0.0)
            return 0;

        const qreal travel = std::max(0.0, track.height() - _thumbRect.height());
        if (travel <= 0.0)
            return 0;

        const qreal clampedTop = std::clamp(thumbTop, track.top(), track.bottom() - _thumbRect.height());
        const qreal ratio = (clampedTop - track.top()) / travel;
        return static_cast<int>(std::lround(ratio * static_cast<qreal>(_scrollBar->maximum())));
    }

    QScrollBar *_scrollBar;
    QRectF _thumbRect;
    bool _dragging;
    qreal _dragOffset;
};
}

NodeLibraryListWidget::NodeLibraryListWidget(QWidget *parent)
    : QListWidget(parent)
    , _filterText()
    , _scrollIndicator(new NodeLibraryScrollOverlay(verticalScrollBar(), this))
    , _viewportHovered(false)
    , _scrollIndicatorHovered(false)
{
    setDragEnabled(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setDefaultDropAction(Qt::CopyAction);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setSpacing(0);
    setItemDelegate(new NodeLibraryItemDelegate(this));
    viewport()->setMouseTracking(true);
    viewport()->installEventFilter(this);
    _scrollIndicator->installEventFilter(this);

    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, [this]() { refreshScrollIndicator(); });
    connect(verticalScrollBar(), &QScrollBar::rangeChanged, this, [this](int, int) { refreshScrollIndicator(); });

    updateScrollIndicatorGeometry();
    refreshScrollIndicator();
}

QPixmap NodeLibraryListWidget::dragPreviewPixmap()
{
    QPixmap pixmap(1, 1);
    pixmap.fill(Qt::transparent);
    return pixmap;
}

QRectF NodeLibraryListWidget::alignedCardRect(QRectF const &rect)
{
    return rect.adjusted(0.5, 0.5, -0.5, -0.5);
}

int NodeLibraryListWidget::scrollIndicatorWidth()
{
    return 8;
}

qreal NodeLibraryListWidget::scrollIndicatorCornerRadius()
{
    return 4.0;
}

QColor NodeLibraryListWidget::scrollIndicatorTrackColor()
{
    return QColor(QStringLiteral("#f4eee5"));
}

QColor NodeLibraryListWidget::scrollIndicatorThumbColor()
{
    return QColor(QStringLiteral("#d7c1aa"));
}

QListWidgetItem *NodeLibraryListWidget::addSectionHeader(QString const &title, QIcon const &icon)
{
    auto *item = new QListWidgetItem(icon, title, this);
    item->setFlags(Qt::ItemIsEnabled);
    item->setData(ItemKindRole, static_cast<int>(ItemKind::SectionHeader));
    item->setData(NodeTypeRole, QString());
    item->setData(SectionCollapsedRole, false);
    item->setData(CardVisualStateRole, static_cast<int>(CardVisualState::Standalone));
    item->setSizeHint(QSize(0, 40));
    queueScrollIndicatorRefresh();
    return item;
}

QListWidgetItem *NodeLibraryListWidget::addNodeEntry(QString const &typeKey,
                                                     QString const &displayName,
                                                     QString const &description,
                                                     QIcon const &icon,
                                                     int inPortCount,
                                                     int outPortCount)
{
    auto *item = new QListWidgetItem(icon, displayName, this);
    item->setData(NodeTypeRole, typeKey);
    item->setData(ItemKindRole, static_cast<int>(ItemKind::NodeEntry));
    item->setToolTip(description);
    item->setData(CardVisualStateRole, static_cast<int>(CardVisualState::NodeBottom));
    item->setData(InPortCountRole, inPortCount);
    item->setData(OutPortCountRole, outPortCount);
    item->setSizeHint(QSize(0, 56));
    queueScrollIndicatorRefresh();
    return item;
}

int NodeLibraryListWidget::visibleNodeCount() const
{
    int count = 0;
    for (int row = 0; row < this->count(); ++row) {
        auto *item = this->item(row);
        if (!item->isHidden()
            && item->data(ItemKindRole).toInt() == static_cast<int>(ItemKind::NodeEntry)) {
            ++count;
        }
    }
    return count;
}

void NodeLibraryListWidget::setFilterText(QString const &filterText)
{
    _filterText = filterText.trimmed();
    applyVisibility();
}

QString NodeLibraryListWidget::filterText() const
{
    return _filterText;
}

void NodeLibraryListWidget::toggleSection(QListWidgetItem *headerItem)
{
    if (headerItem == nullptr
        || headerItem->data(ItemKindRole).toInt() != static_cast<int>(ItemKind::SectionHeader)) {
        return;
    }

    headerItem->setData(SectionCollapsedRole, !headerItem->data(SectionCollapsedRole).toBool());
    applyVisibility();
}

bool NodeLibraryListWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == viewport()) {
        switch (event->type()) {
        case QEvent::Enter:
        case QEvent::MouseMove:
            _viewportHovered = true;
            queueScrollIndicatorRefresh();
            break;
        case QEvent::Leave:
        case QEvent::Hide:
            _viewportHovered = false;
            queueScrollIndicatorRefresh();
            break;
        case QEvent::Show:
            queueScrollIndicatorRefresh();
            break;
        default:
            break;
        }
    } else if (watched == _scrollIndicator) {
        switch (event->type()) {
        case QEvent::Enter:
        case QEvent::MouseMove:
            _scrollIndicatorHovered = true;
            queueScrollIndicatorRefresh();
            break;
        case QEvent::Leave:
        case QEvent::Hide:
            _scrollIndicatorHovered = false;
            queueScrollIndicatorRefresh();
            break;
        case QEvent::Show:
            queueScrollIndicatorRefresh();
            break;
        default:
            break;
        }
    }

    return QListWidget::eventFilter(watched, event);
}

QStringList NodeLibraryListWidget::mimeTypes() const
{
    return {QString::fromUtf8(MimeType)};
}

QMimeData *NodeLibraryListWidget::mimeData(QList<QListWidgetItem *> items) const
{
    auto *mimeData = new QMimeData();

    if (!items.isEmpty()) {
        const auto typeKey = items.front()->data(NodeTypeRole).toString();
        mimeData->setData(QString::fromUtf8(MimeType), typeKey.toUtf8());
        mimeData->setText(items.front()->text());
    }

    return mimeData;
}

Qt::DropActions NodeLibraryListWidget::supportedDragActions() const
{
    return Qt::CopyAction;
}

void NodeLibraryListWidget::resizeEvent(QResizeEvent *event)
{
    QListWidget::resizeEvent(event);
    updateScrollIndicatorGeometry();
    refreshScrollIndicator();
}

void NodeLibraryListWidget::showEvent(QShowEvent *event)
{
    QListWidget::showEvent(event);
    updateScrollIndicatorGeometry();
    queueScrollIndicatorRefresh();
}

void NodeLibraryListWidget::startDrag(Qt::DropActions)
{
    auto *item = currentItem();
    if (item == nullptr || item->data(ItemKindRole).toInt() == static_cast<int>(ItemKind::SectionHeader))
        return;

    auto *drag = new QDrag(this);
    drag->setMimeData(mimeData(selectedItems()));
    drag->setPixmap(dragPreviewPixmap());
    drag->setHotSpot(QPoint(0, 0));
    drag->exec(Qt::CopyAction);
}

void NodeLibraryListWidget::applyVisibility()
{
    const QString normalizedFilter = _filterText.toCaseFolded();

    QList<QListWidgetItem *> currentSectionNodes;
    QListWidgetItem *currentHeader = nullptr;

    auto applySection = [&](QListWidgetItem *header, QList<QListWidgetItem *> const &nodes) {
        if (header == nullptr)
            return;

        const bool hasFilter = !normalizedFilter.isEmpty();
        bool sectionMatches = !hasFilter || header->text().toCaseFolded().contains(normalizedFilter);
        bool hasVisibleChild = false;

        for (auto *node : nodes) {
            const bool nodeMatches = !hasFilter || node->text().toCaseFolded().contains(normalizedFilter)
                                     || node->toolTip().toCaseFolded().contains(normalizedFilter);
            hasVisibleChild = hasVisibleChild || nodeMatches;
            node->setHidden(hasFilter ? !nodeMatches : header->data(SectionCollapsedRole).toBool());
        }

        header->setHidden(hasFilter ? !(sectionMatches || hasVisibleChild) : false);
    };

    for (int row = 0; row < count(); ++row) {
        auto *item = this->item(row);
        const auto kind = static_cast<ItemKind>(item->data(ItemKindRole).toInt());
        if (kind == ItemKind::SectionHeader) {
            applySection(currentHeader, currentSectionNodes);
            currentHeader = item;
            currentSectionNodes.clear();
        } else {
            currentSectionNodes.append(item);
        }
    }

    applySection(currentHeader, currentSectionNodes);

    currentHeader = nullptr;
    currentSectionNodes.clear();

    auto updateSectionCardStates = [&](QListWidgetItem *header, QList<QListWidgetItem *> const &nodes) {
        if (header == nullptr)
            return;

        QList<QListWidgetItem *> visibleNodes;
        for (auto *node : nodes) {
            if (!node->isHidden())
                visibleNodes.append(node);
        }

        header->setData(CardVisualStateRole,
                        static_cast<int>(visibleNodes.isEmpty() ? CardVisualState::Standalone
                                                               : CardVisualState::SectionTop));

        for (int index = 0; index < nodes.size(); ++index) {
            nodes[index]->setData(CardVisualStateRole, static_cast<int>(CardVisualState::NodeBottom));
        }

        for (int index = 0; index < visibleNodes.size(); ++index) {
            visibleNodes[index]->setData(CardVisualStateRole,
                                         static_cast<int>(index == visibleNodes.size() - 1
                                                              ? CardVisualState::NodeBottom
                                                              : CardVisualState::NodeMiddle));
        }
    };

    for (int row = 0; row < count(); ++row) {
        auto *item = this->item(row);
        const auto kind = static_cast<ItemKind>(item->data(ItemKindRole).toInt());
        if (kind == ItemKind::SectionHeader) {
            updateSectionCardStates(currentHeader, currentSectionNodes);
            currentHeader = item;
            currentSectionNodes.clear();
        } else {
            currentSectionNodes.append(item);
        }
    }

    updateSectionCardStates(currentHeader, currentSectionNodes);
    queueScrollIndicatorRefresh();
}

void NodeLibraryListWidget::queueScrollIndicatorRefresh()
{
    QTimer::singleShot(0, this, [this]() { refreshScrollIndicator(); });
}

void NodeLibraryListWidget::refreshScrollIndicator()
{
    updateScrollIndicatorGeometry();

    if (_scrollIndicator == nullptr)
        return;

    const bool hasOverflow = verticalScrollBar()->maximum() > 0;
    const bool hovered = _viewportHovered || _scrollIndicatorHovered;
    const bool shouldShow = isVisible() && hasOverflow && hovered;

    _scrollIndicator->setVisible(shouldShow);
    _scrollIndicator->raise();
    static_cast<NodeLibraryScrollOverlay *>(_scrollIndicator)->syncToScrollBar();
}

void NodeLibraryListWidget::updateScrollIndicatorGeometry()
{
    if (_scrollIndicator == nullptr)
        return;

    const QRect viewportRect = viewport()->geometry();
    const int overlayWidth = scrollIndicatorWidth();
    constexpr int rightInset = 6;
    constexpr int verticalInset = 12;
    const int overlayHeight = std::max(0, viewportRect.height() - (verticalInset * 2));
    const int overlayLeft = viewportRect.right() - overlayWidth - rightInset + 1;
    _scrollIndicator->setGeometry(overlayLeft,
                                  viewportRect.top() + verticalInset,
                                  overlayWidth,
                                  overlayHeight);
}

#include "app/NodeLibraryListWidget.hpp"

#include <QApplication>
#include <QColor>
#include <QEvent>
#include <QListWidgetItem>
#include <QPixmap>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QWidget>
#include <QtTest/QTest>

class NodeLibraryListWidgetTests : public QObject
{
    Q_OBJECT

private slots:
    void usesTransparentDragPreviewPixmap();
    void alignsCardRectsToHalfPixels();
    void marksSectionHeadersAsNonDraggableLibrarySections();
    void usesDedicatedDelegateForSectionAndNodeRows();
    void filtersNodesAndMatchingSections();
    void togglesSectionEntries();
    void assignsGroupedCardStatesForVisibleSections();
    void usesCustomOverlayScrollIndicatorInsteadOfNativeScrollbar();
    void exposesSoftEllipticalScrollIndicatorTokens();
    void usesSlimFlatScrollIndicatorGeometry();
    void showsScrollIndicatorOnlyWhileHoveredWhenContentOverflows();
    void keepsScrollIndicatorHiddenWithoutOverflow();
    void storesPortCountsOnNodeEntries();
    void reportsVisibleNodeCount();
};

void NodeLibraryListWidgetTests::usesTransparentDragPreviewPixmap()
{
    const QPixmap pixmap = NodeLibraryListWidget::dragPreviewPixmap();

    QCOMPARE(pixmap.size(), QSize(1, 1));
    QVERIFY(pixmap.hasAlphaChannel());
    QCOMPARE(pixmap.toImage().pixelColor(0, 0).alpha(), 0);
}

void NodeLibraryListWidgetTests::alignsCardRectsToHalfPixels()
{
    const QRectF inputRect(8.0, 8.0, 220.0, 56.0);
    const QRectF alignedRect = NodeLibraryListWidget::alignedCardRect(inputRect);

    QCOMPARE(alignedRect.left(), 8.5);
    QCOMPARE(alignedRect.top(), 8.5);
    QCOMPARE(alignedRect.right(), 227.5);
    QCOMPARE(alignedRect.bottom(), 63.5);
}

void NodeLibraryListWidgetTests::marksSectionHeadersAsNonDraggableLibrarySections()
{
    NodeLibraryListWidget widget;

    auto *headerItem = widget.addSectionHeader(QStringLiteral("Flow"), QIcon());
    auto *nodeItem = widget.addNodeEntry(QStringLiteral("start"), QStringLiteral("Start"), QString(), QIcon());

    QVERIFY(headerItem != nullptr);
    QVERIFY(nodeItem != nullptr);
    QCOMPARE(headerItem->data(NodeLibraryListWidget::ItemKindRole).toInt(),
             static_cast<int>(NodeLibraryListWidget::ItemKind::SectionHeader));
    QCOMPARE(nodeItem->data(NodeLibraryListWidget::ItemKindRole).toInt(),
             static_cast<int>(NodeLibraryListWidget::ItemKind::NodeEntry));
    QCOMPARE(headerItem->data(NodeLibraryListWidget::NodeTypeRole).toString(), QString());
    QCOMPARE(nodeItem->data(NodeLibraryListWidget::NodeTypeRole).toString(), QString("start"));
    QVERIFY(!(headerItem->flags() & Qt::ItemIsDragEnabled));
    QVERIFY(nodeItem->flags() & Qt::ItemIsDragEnabled);
}

void NodeLibraryListWidgetTests::usesDedicatedDelegateForSectionAndNodeRows()
{
    NodeLibraryListWidget widget;

    QVERIFY(widget.itemDelegate() != nullptr);
    QCOMPARE(widget.itemDelegate()->objectName(), QStringLiteral("nodeLibraryItemDelegate"));
}

void NodeLibraryListWidgetTests::filtersNodesAndMatchingSections()
{
    NodeLibraryListWidget widget;

    auto *flowHeader = widget.addSectionHeader(QStringLiteral("Flow"), QIcon());
    auto *startNode = widget.addNodeEntry(QStringLiteral("start"), QStringLiteral("Start"), QString(), QIcon());
    auto *aiHeader = widget.addSectionHeader(QStringLiteral("AI"), QIcon());
    auto *promptNode = widget.addNodeEntry(QStringLiteral("prompt"), QStringLiteral("Prompt"), QString(), QIcon());
    auto *llmNode = widget.addNodeEntry(QStringLiteral("llm"), QStringLiteral("LLM"), QString(), QIcon());

    widget.setFilterText(QStringLiteral("Prompt"));

    QVERIFY(flowHeader->isHidden());
    QVERIFY(startNode->isHidden());
    QVERIFY(!aiHeader->isHidden());
    QVERIFY(!promptNode->isHidden());
    QVERIFY(llmNode->isHidden());
    QCOMPARE(widget.filterText(), QStringLiteral("Prompt"));
}

void NodeLibraryListWidgetTests::togglesSectionEntries()
{
    NodeLibraryListWidget widget;

    auto *flowHeader = widget.addSectionHeader(QStringLiteral("Flow"), QIcon());
    auto *startNode = widget.addNodeEntry(QStringLiteral("start"), QStringLiteral("Start"), QString(), QIcon());
    auto *outputNode = widget.addNodeEntry(QStringLiteral("output"), QStringLiteral("Output"), QString(), QIcon());

    QVERIFY(!startNode->isHidden());
    QVERIFY(!outputNode->isHidden());

    widget.toggleSection(flowHeader);

    QVERIFY(startNode->isHidden());
    QVERIFY(outputNode->isHidden());
    QCOMPARE(flowHeader->data(NodeLibraryListWidget::SectionCollapsedRole).toBool(), true);

    widget.toggleSection(flowHeader);

    QVERIFY(!startNode->isHidden());
    QVERIFY(!outputNode->isHidden());
    QCOMPARE(flowHeader->data(NodeLibraryListWidget::SectionCollapsedRole).toBool(), false);
}

void NodeLibraryListWidgetTests::assignsGroupedCardStatesForVisibleSections()
{
    NodeLibraryListWidget widget;

    auto *flowHeader = widget.addSectionHeader(QStringLiteral("Flow"), QIcon());
    auto *startNode = widget.addNodeEntry(QStringLiteral("start"), QStringLiteral("Start"), QString(), QIcon());
    auto *outputNode = widget.addNodeEntry(QStringLiteral("output"), QStringLiteral("Output"), QString(), QIcon());

    widget.setFilterText(QString());

    QCOMPARE(flowHeader->data(NodeLibraryListWidget::CardVisualStateRole).toInt(),
             static_cast<int>(NodeLibraryListWidget::CardVisualState::SectionTop));
    QCOMPARE(startNode->data(NodeLibraryListWidget::CardVisualStateRole).toInt(),
             static_cast<int>(NodeLibraryListWidget::CardVisualState::NodeMiddle));
    QCOMPARE(outputNode->data(NodeLibraryListWidget::CardVisualStateRole).toInt(),
             static_cast<int>(NodeLibraryListWidget::CardVisualState::NodeBottom));

    widget.toggleSection(flowHeader);

    QCOMPARE(flowHeader->data(NodeLibraryListWidget::CardVisualStateRole).toInt(),
             static_cast<int>(NodeLibraryListWidget::CardVisualState::Standalone));
}

void NodeLibraryListWidgetTests::usesCustomOverlayScrollIndicatorInsteadOfNativeScrollbar()
{
    NodeLibraryListWidget widget;

    auto *overlay = widget.findChild<QWidget *>(QStringLiteral("nodeLibraryScrollOverlay"));

    QVERIFY(overlay != nullptr);
    QCOMPARE(widget.verticalScrollBarPolicy(), Qt::ScrollBarAlwaysOff);
}

void NodeLibraryListWidgetTests::exposesSoftEllipticalScrollIndicatorTokens()
{
    QCOMPARE(NodeLibraryListWidget::scrollIndicatorWidth(), 8);
    QCOMPARE(NodeLibraryListWidget::scrollIndicatorCornerRadius(), 4.0);
    QCOMPARE(NodeLibraryListWidget::scrollIndicatorTrackColor(), QColor(QStringLiteral("#f4eee5")));
    QCOMPARE(NodeLibraryListWidget::scrollIndicatorThumbColor(), QColor(QStringLiteral("#d7c1aa")));
}

void NodeLibraryListWidgetTests::usesSlimFlatScrollIndicatorGeometry()
{
    QWidget host;
    host.resize(260, 220);

    auto *layout = new QVBoxLayout(&host);
    layout->setContentsMargins(0, 0, 0, 0);

    auto *widget = new NodeLibraryListWidget(&host);
    layout->addWidget(widget);

    for (int index = 0; index < 12; ++index) {
        widget->addNodeEntry(QStringLiteral("node-%1").arg(index),
                             QStringLiteral("Node %1").arg(index),
                             QStringLiteral("Description %1").arg(index),
                             QIcon());
    }

    host.show();
    QVERIFY(QTest::qWaitForWindowExposed(&host));

    auto *overlay = widget->findChild<QWidget *>(QStringLiteral("nodeLibraryScrollOverlay"));
    QVERIFY(overlay != nullptr);
    QVERIFY(widget->verticalScrollBar()->maximum() > 0);

    QCOMPARE(overlay->width(), 8);
}

void NodeLibraryListWidgetTests::showsScrollIndicatorOnlyWhileHoveredWhenContentOverflows()
{
    QWidget host;
    host.resize(260, 220);

    auto *layout = new QVBoxLayout(&host);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto *widget = new NodeLibraryListWidget(&host);
    auto *spacer = new QWidget(&host);
    spacer->setFixedHeight(40);
    layout->addWidget(widget, 1);
    layout->addWidget(spacer);

    for (int index = 0; index < 12; ++index) {
        widget->addNodeEntry(QStringLiteral("node-%1").arg(index),
                             QStringLiteral("Node %1").arg(index),
                             QStringLiteral("Description %1").arg(index),
                             QIcon());
    }

    host.show();
    QVERIFY(QTest::qWaitForWindowExposed(&host));

    auto *overlay = widget->findChild<QWidget *>(QStringLiteral("nodeLibraryScrollOverlay"));
    QVERIFY(overlay != nullptr);
    QVERIFY(widget->verticalScrollBar()->maximum() > 0);
    QVERIFY(!overlay->isVisible());

    QEvent enterEvent(QEvent::Enter);
    QApplication::sendEvent(widget->viewport(), &enterEvent);
    QTRY_VERIFY(overlay->isVisible());

    QEvent leaveEvent(QEvent::Leave);
    QApplication::sendEvent(widget->viewport(), &leaveEvent);
    QApplication::sendEvent(spacer, &enterEvent);
    QTRY_VERIFY(!overlay->isVisible());
}

void NodeLibraryListWidgetTests::keepsScrollIndicatorHiddenWithoutOverflow()
{
    QWidget host;
    host.resize(260, 320);

    auto *layout = new QVBoxLayout(&host);
    layout->setContentsMargins(0, 0, 0, 0);

    auto *widget = new NodeLibraryListWidget(&host);
    layout->addWidget(widget);
    widget->addNodeEntry(QStringLiteral("single"), QStringLiteral("Single"), QStringLiteral("Only row"), QIcon());

    host.show();
    QVERIFY(QTest::qWaitForWindowExposed(&host));

    auto *overlay = widget->findChild<QWidget *>(QStringLiteral("nodeLibraryScrollOverlay"));
    QVERIFY(overlay != nullptr);
    QCOMPARE(widget->verticalScrollBar()->maximum(), 0);

    QEvent enterEvent(QEvent::Enter);
    QApplication::sendEvent(widget->viewport(), &enterEvent);
    QTest::qWait(10);
    QVERIFY(!overlay->isVisible());
}

void NodeLibraryListWidgetTests::storesPortCountsOnNodeEntries()
{
    NodeLibraryListWidget widget;
    widget.addSectionHeader("Test", QIcon());
    auto *item = widget.addNodeEntry("test", "Test Node", "A test node", QIcon(), 2, 3);

    QCOMPARE(item->data(NodeLibraryListWidget::InPortCountRole).toInt(), 2);
    QCOMPARE(item->data(NodeLibraryListWidget::OutPortCountRole).toInt(), 3);
}

void NodeLibraryListWidgetTests::reportsVisibleNodeCount()
{
    NodeLibraryListWidget widget;
    widget.addSectionHeader("Group", QIcon());
    widget.addNodeEntry("alpha", "Alpha", "Alpha node", QIcon(), 1, 1);
    widget.addNodeEntry("beta", "Beta", "Beta node", QIcon(), 0, 1);

    QCOMPARE(widget.visibleNodeCount(), 2);

    widget.setFilterText("Alpha");
    QCOMPARE(widget.visibleNodeCount(), 1);

    widget.setFilterText("zzz_no_match_zzz");
    QCOMPARE(widget.visibleNodeCount(), 0);

    widget.setFilterText("");
    QCOMPARE(widget.visibleNodeCount(), 2);
}

QTEST_MAIN(NodeLibraryListWidgetTests)

#include "NodeLibraryListWidgetTests.moc"

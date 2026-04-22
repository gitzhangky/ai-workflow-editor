#include "app/NodeLibraryListWidget.hpp"

#include <QColor>
#include <QListWidgetItem>
#include <QPixmap>
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

QTEST_MAIN(NodeLibraryListWidgetTests)

#include "NodeLibraryListWidgetTests.moc"

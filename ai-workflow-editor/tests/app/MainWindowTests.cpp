#include "app/MainWindow.hpp"
#include "app/LanguageManager.hpp"
#include "app/NodeLibraryListWidget.hpp"
#include "inspector/InspectorFieldSchema.hpp"
#include "qtnodes/QtNodesEditorWidget.hpp"

#include <QtNodes/ConnectionStyle>
#include <QtNodes/GraphicsViewStyle>
#include <QtNodes/internal/ConnectionGraphicsObject.hpp>
#include <QtNodes/internal/DataFlowGraphicsScene.hpp>

#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QDockWidget>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGraphicsView>
#include <QLabel>
#include <QListWidget>
#include <QLineEdit>
#include <QMenuBar>
#include <QMimeData>
#include <QDoubleSpinBox>
#include <QSettings>
#include <QSet>
#include <QSpinBox>
#include <QStandardPaths>
#include <QStatusBar>
#include <QTableWidget>
#include <QTemporaryDir>
#include <QTabWidget>
#include <QTextEdit>
#include <QTextBrowser>
#include <QTextDocument>
#include <QUrl>
#include <QToolButton>
#include <QToolBar>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QtTest/QTest>
#include <QMouseEvent>

namespace
{
QGraphicsObject *selectNodeGraphicsObject(QtNodesEditorWidget *editor, QtNodes::NodeId nodeId, bool clearExisting = true)
{
    auto *graphicsView = editor != nullptr ? editor->findChild<QGraphicsView *>() : nullptr;
    auto *scene =
        graphicsView != nullptr ? qobject_cast<QtNodes::DataFlowGraphicsScene *>(graphicsView->scene()) : nullptr;
    if (scene != nullptr && clearExisting)
        scene->clearSelection();
    auto *nodeGraphicsObject = scene != nullptr ? scene->nodeGraphicsObject(nodeId) : nullptr;
    if (nodeGraphicsObject != nullptr)
        nodeGraphicsObject->setSelected(true);
    QCoreApplication::processEvents();
    return nodeGraphicsObject;
}

QtNodes::ConnectionGraphicsObject *selectConnectionGraphicsObject(QtNodesEditorWidget *editor,
                                                                  QtNodes::ConnectionId const &connectionId,
                                                                  bool clearExisting = true)
{
    auto *graphicsView = editor != nullptr ? editor->findChild<QGraphicsView *>() : nullptr;
    auto *scene =
        graphicsView != nullptr ? qobject_cast<QtNodes::DataFlowGraphicsScene *>(graphicsView->scene()) : nullptr;
    if (scene != nullptr && clearExisting)
        scene->clearSelection();
    auto *connectionGraphicsObject = scene != nullptr ? scene->connectionGraphicsObject(connectionId) : nullptr;
    if (connectionGraphicsObject != nullptr)
        connectionGraphicsObject->setSelected(true);
    QCoreApplication::processEvents();
    return connectionGraphicsObject;
}

void clearCanvasSelection(QtNodesEditorWidget *editor)
{
    auto *graphicsView = editor != nullptr ? editor->findChild<QGraphicsView *>() : nullptr;
    auto *scene =
        graphicsView != nullptr ? qobject_cast<QtNodes::DataFlowGraphicsScene *>(graphicsView->scene()) : nullptr;
    if (scene != nullptr)
        scene->clearSelection();
    QCoreApplication::processEvents();
}

}

class MainWindowTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();
    void setsExpectedWindowTitle();
    void createsNodeLibraryAndInspectorDocks();
    void createsWorkflowCanvasInCentralArea();
    void opensUserGuideInReusableWorkbenchTab();
    void userGuideCoversCoreWorkflowTasksWithVisualDiagrams();
    void preservesDockVisibilityPreferenceWhenSwitchingHelpTab();
    void appliesLightWorkbenchCanvasBackground();
    void createsPrimaryToolbarAndStatusBar();
    void showsGroupedToolbarLayout();
    void exposesToolbarStylingHooks();
    void exposesStableObjectNamesForWorkbenchChrome();
    void createsMenuBarWithFileViewAndSettingsMenus();
    void exposesCanvasArrangeActionsForMultiSelection();
    void showsProblemsPanelWithWorkflowValidationIssues();
    void problemsPanelShowsCountAndEmptyState();
    void problemsPanelFiltersIssuesBySeverity();
    void activatingProblemSelectsNodeAndHighlightsInspectorField();
    void problemsPanelRefreshesAfterIssueIsFixed();
    void problemsPanelRefreshesAfterTypingFixInInspector();
    void keepsCanvasMiniMapHiddenWhenWorkflowIsEmpty();
    void showsCanvasMiniMapWhenWorkflowHasNodes();
    void clickingCanvasMiniMapRecentersLargeCanvas();
    void draggingCanvasMiniMapViewportPansWithoutInitialJump();
    void updatesCanvasMiniMapWhenNodeGraphicsMove();
    void commitsMovedNodeGraphicsPositionWithUndoRedoOnMouseRelease();
    void fitWorkflowActionShowsEntireLargeWorkflow();
    void alignsSelectedNodesLeftWithUndoRedo();
    void distributesSelectedNodesHorizontallyWithUndoRedo();
    void defaultsWorkbenchTextToChinese();
    void keepsChineseDefaultWhenExternalTranslationFileIsUnavailable();
    void retranslatesWorkbenchTextToEnglishAtRuntime();
    void addsIconsToToolbarAndNodeLibrary();
    void groupsNodeLibraryIntoCategorySections();
    void addsSearchBoxForNodeLibraryFiltering();
    void addsNodeToCanvasFromLibraryAction();
    void acceptsDropOnVisibleCanvasViewport();
    void usesCustomNodeCardPainter();
    void movesPortsCloserToNodeEdges();
    void avoidsExtraNodeGraphicsShadow();
    void syncsSelectedNodeIntoInspectorAndBack();
    void showsTypeSpecificInspectorFieldsForPromptLlmAndTool();
    void showsTypeSpecificInspectorFieldsForMemory();
    void showsTypeSpecificInspectorFieldsForRetriever();
    void showsTypeSpecificInspectorFieldsForTemplateVariables();
    void showsTypeSpecificInspectorFieldsForHttpRequest();
    void showsTypeSpecificInspectorFieldsForJsonTransform();
    void showsTypeSpecificInspectorFieldsForAgent();
    void showsTypeSpecificInspectorFieldsForChatOutput();
    void exposesStableInspectorFieldMetadata();
    void exposesStableInspectorFieldHintMetadata();
    void exposesStableInspectorFieldSchemaObjectNames();
    void exposesStableInspectorSectionSchemaObjectNames();
    void exposesStableInspectorSectionSchemaCopy();
    void retranslatesInspectorFieldLabelsAtRuntime();
    void retranslatesInspectorFieldHintsAtRuntime();
    void showsTypeSpecificInspectorHeaderAndEmptyState();
    void appliesDistinctNodeCardStylesByType();
    void exposesAllCurrentValidationIssues();
    void marksIncompletePromptNodeWithWarningValidationState();
    void marksIncompleteMemoryNodeWithWarningValidationState();
    void marksIncompleteRetrieverNodeWithWarningValidationState();
    void marksIncompleteAgentNodeWithWarningValidationState();
    void marksIncompleteChatOutputNodeWithWarningValidationState();
    void marksInvalidTemplateVariablesNodeWithValidationState();
    void marksInvalidHttpRequestNodeWithValidationState();
    void marksInvalidJsonTransformNodeWithValidationState();
    void marksInvalidToolNodeWithErrorValidationState();
    void marksFlowNodesWithStructuralWarningsBasedOnConnections();
    void showsValidationMessageInInspectorForSelectedNode();
    void highlightsInspectorFieldForRequiredPropertyValidation();
    void highlightsInspectorFieldForInvalidToolJsonMapping();
    void keepsLlmAndToolValidationMessagesConsistentAcrossSurfaces();
    void refreshesFlowValidationAfterConnectionDeletionUndo();
    void updatesFlowValidationSurfacesWhenConnectionFixesNode();
    void showsImmediateStatusFeedbackForInvalidConnectionDrag();
    void showsValidationSummaryForCurrentSelectionInStatusBar();
    void assignsExpectedKeyboardShortcuts();
    void enablesWorkbenchActionsBasedOnEditorState();
    void enablesDeleteActionForSelectedConnections();
    void enforcesConnectionRulesBetweenCompatiblePorts();
    void savesAndLoadsWorkflowJson();
    void savesAndLoadsNodePositionsAtOrigin();
    void savesAndLoadsMemoryNodeProperties();
    void savesAndLoadsRetrieverNodeProperties();
    void savesAndLoadsTemplateVariablesNodeProperties();
    void savesAndLoadsHttpRequestNodeProperties();
    void savesAndLoadsJsonTransformNodeProperties();
    void savesAndLoadsAgentNodeProperties();
    void savesAndLoadsChatOutputNodeProperties();
    void preservesMinimumNodeCardSizeAcrossSaveAndLoad();
    void tracksDirtyStateOnEdits();
    void clearsDirtyStateOnSave();
    void clearsDirtyStateOnLoad();
    void doesNotPopulateUndoHistoryWhenLoadingWorkflow();
    void preservesCurrentWorkflowWhenLoadFails();
    void showsDirtyMarkerInWindowTitle();
    void showsFileNameInWindowTitleAfterSave();
    void tracksRecentFilesAcrossSaveAndLoad();
    void fileMenuContainsSaveAsAndRecentFiles();
    void deletesSelectedNodeFromCanvas();
    void deletesSelectedConnectionFromCanvas();
    void ignoresDeleteWhenSelectionIsEmpty();
    void deletesMixedNodeAndConnectionSelectionTogether();
    void deleteMarksDocumentDirty();
    void editMenuContainsDeleteAction();
    void undoesAndRedoesNodeCreation();
    void undoesAndRedoesPropertyEdit();
    void undoesAndRedoesNodeDeletion();
    void undoesAndRedoesConnectionCreation();
    void undoesAndRedoesConnectionDeletion();
    void restoresDirtyStateWhenUndoAndRedoCrossSavedPropertyEdit();
    void staysDirtyUntilUndoReachesExactSavedPropertyState();
    void restoresDirtyStateWhenUndoAndRedoCrossSavedNodeDeletion();
    void restoresDirtyStateWhenUndoAndRedoCrossSavedConnectionCreation();
    void restoresDirtyStateWhenUndoAndRedoCrossSavedConnectionDeletion();
    void showsZoomIndicatorInStatusBar();
    void nodeLibraryShowsPortCountBadges();
    void nodeLibraryShowsNoResultsWhenFilterHasNoMatch();
    void editMenuContainsCopyPasteDuplicateActions();
    void assignsCopyPasteDuplicateKeyboardShortcuts();
    void enablesCopyPasteDuplicateActionsBasedOnState();
    void copiesAndPastesNodeOnCanvas();
    void duplicatesNodeOnCanvas();
    void allowsCompatiblePortConnections();
    void rejectsIncompatiblePortConnections();
    void fileMenuContainsExportSubmenu();
    void exportActionsDisabledWhenNoNodes();
    void exportActionsEnabledWhenNodesExist();
    void exportReadinessReportsEmptyInvalidAndValidWorkflow();
};

void MainWindowTests::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    QCoreApplication::setOrganizationName("CodexTests");
    QCoreApplication::setApplicationName("AIWorkflowEditorTests");
}

void MainWindowTests::init()
{
    QSettings settings;
    settings.clear();
}

void MainWindowTests::setsExpectedWindowTitle()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    QCOMPARE(window.windowTitle(), QString::fromUtf8("AI 工作流编辑器"));
}

void MainWindowTests::createsNodeLibraryAndInspectorDocks()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    const auto docks = window.findChildren<QDockWidget *>();
    QCOMPARE(docks.size(), 3);
    QVERIFY(window.findChild<QDockWidget *>("nodeLibraryDock") != nullptr);
    QVERIFY(window.findChild<QDockWidget *>("inspectorDock") != nullptr);
    QVERIFY(window.findChild<QDockWidget *>("problemsDock") != nullptr);

    auto *nodeLibrary = window.findChild<QListWidget *>("nodeLibraryList");
    QVERIFY(nodeLibrary != nullptr);
    QCOMPARE(nodeLibrary->count(), 16);
    QCOMPARE(nodeLibrary->item(1)->text(), QString::fromUtf8("开始"));
}

void MainWindowTests::createsWorkflowCanvasInCentralArea()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *tabs = qobject_cast<QTabWidget *>(window.centralWidget());
    QVERIFY(tabs != nullptr);
    QCOMPARE(tabs->objectName(), QString("workbenchTabWidget"));
    QCOMPARE(tabs->count(), 1);
    QCOMPARE(tabs->tabText(0), QString::fromUtf8("工作流"));

    auto *editor = tabs->widget(0);
    QVERIFY(editor != nullptr);
    QCOMPARE(editor->objectName(), QString("workflowCanvas"));
    QVERIFY(editor->findChild<QGraphicsView *>() != nullptr);
}

void MainWindowTests::opensUserGuideInReusableWorkbenchTab()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *tabs = qobject_cast<QTabWidget *>(window.centralWidget());
    auto *helpAction = window.findChild<QAction *>("helpAction");
    QVERIFY(tabs != nullptr);
    QVERIFY(helpAction != nullptr);
    QCOMPARE(tabs->count(), 1);

    helpAction->trigger();
    QCOMPARE(tabs->count(), 2);
    QCOMPARE(tabs->currentIndex(), 1);
    QCOMPARE(tabs->tabText(1), QString::fromUtf8("用户指南"));

    auto *helpWidget = tabs->widget(1);
    QVERIFY(helpWidget != nullptr);
    QCOMPARE(helpWidget->objectName(), QString("helpDocumentWidget"));
    auto *browser = helpWidget->findChild<QTextBrowser *>("helpDocumentBrowser");
    QVERIFY(browser != nullptr);
    QVERIFY(browser->toPlainText().contains(QString::fromUtf8("AI 工作流编辑器")));
    QVERIFY(browser->toPlainText().contains(QString::fromUtf8("画布小地图")));

    helpAction->trigger();
    QCOMPARE(tabs->count(), 2);
    QCOMPARE(tabs->currentIndex(), 1);
}

void MainWindowTests::userGuideCoversCoreWorkflowTasksWithVisualDiagrams()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *tabs = qobject_cast<QTabWidget *>(window.centralWidget());
    auto *helpAction = window.findChild<QAction *>("helpAction");
    QVERIFY(tabs != nullptr);
    QVERIFY(helpAction != nullptr);

    helpAction->trigger();

    auto *helpWidget = tabs->widget(1);
    QVERIFY(helpWidget != nullptr);
    auto *browser = helpWidget->findChild<QTextBrowser *>("helpDocumentBrowser");
    QVERIFY(browser != nullptr);

    const QString plainText = browser->toPlainText();
    QVERIFY(plainText.contains(QString::fromUtf8("图文导览")));
    QVERIFY(plainText.contains(QString::fromUtf8("5 分钟路线")));
    QVERIFY(plainText.contains(QString::fromUtf8("开始 → 提示词 → 大模型 → 输出")));
    QVERIFY(plainText.contains(QString::fromUtf8("连线只允许从输出端口拖到输入端口")));
    QVERIFY(plainText.contains(QString::fromUtf8("节点出现 warning / error")));
    QVERIFY(plainText.contains(QString::fromUtf8("视图 > 整理")));
    QVERIFY(plainText.contains(QString::fromUtf8("左对齐 / 右对齐 / 顶部对齐 / 底部对齐")));
    QVERIFY(plainText.contains(QString::fromUtf8("水平分布 / 垂直分布")));
    QVERIFY(plainText.contains(QString::fromUtf8("问题面板")));
    QVERIFY(plainText.contains(QString::fromUtf8("自动选中对应节点、居中画布")));
    QVERIFY(plainText.contains(QString::fromUtf8("新功能必须同步更新帮助文档")));

    QVERIFY(plainText.contains(QString::fromUtf8("顶部工具栏和菜单")));
    QVERIFY(plainText.contains(QString::fromUtf8("左侧节点库")));
    QVERIFY(plainText.contains(QString::fromUtf8("最小工作流")));
    QVERIFY(plainText.contains(QString::fromUtf8("用户提示模板：空")));

    const QVariant layoutImage =
        browser->document()->resource(QTextDocument::ImageResource, QUrl("help://layout-overview"));
    const QVariant workflowImage =
        browser->document()->resource(QTextDocument::ImageResource, QUrl("help://minimal-workflow"));
    const QVariant validationImage =
        browser->document()->resource(QTextDocument::ImageResource, QUrl("help://validation-card"));
    QVERIFY(layoutImage.canConvert<QImage>());
    QVERIFY(workflowImage.canConvert<QImage>());
    QVERIFY(validationImage.canConvert<QImage>());
    QVERIFY(!layoutImage.value<QImage>().isNull());
    QVERIFY(!workflowImage.value<QImage>().isNull());
    QVERIFY(!validationImage.value<QImage>().isNull());
}

void MainWindowTests::preservesDockVisibilityPreferenceWhenSwitchingHelpTab()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);
    window.show();
    QCoreApplication::processEvents();

    auto *tabs = qobject_cast<QTabWidget *>(window.centralWidget());
    auto *helpAction = window.findChild<QAction *>("helpAction");
    auto *nodeLibraryDock = window.findChild<QDockWidget *>("nodeLibraryDock");
    auto *inspectorDock = window.findChild<QDockWidget *>("inspectorDock");
    auto *problemsDock = window.findChild<QDockWidget *>("problemsDock");
    auto *toggleNodeLibraryAction = window.findChild<QAction *>("toggleNodeLibraryAction");
    auto *toggleInspectorAction = window.findChild<QAction *>("toggleInspectorAction");
    auto *toggleProblemsAction = window.findChild<QAction *>("toggleProblemsAction");
    QVERIFY(tabs != nullptr);
    QVERIFY(helpAction != nullptr);
    QVERIFY(nodeLibraryDock != nullptr);
    QVERIFY(inspectorDock != nullptr);
    QVERIFY(problemsDock != nullptr);
    QVERIFY(toggleNodeLibraryAction != nullptr);
    QVERIFY(toggleInspectorAction != nullptr);
    QVERIFY(toggleProblemsAction != nullptr);

    QVERIFY(nodeLibraryDock->isVisible());
    QVERIFY(inspectorDock->isVisible());
    QVERIFY(problemsDock->isVisible());
    QVERIFY(toggleNodeLibraryAction->isChecked());
    QVERIFY(toggleInspectorAction->isChecked());
    QVERIFY(toggleProblemsAction->isChecked());

    helpAction->trigger();
    QCoreApplication::processEvents();

    QVERIFY(!nodeLibraryDock->isVisible());
    QVERIFY(!inspectorDock->isVisible());
    QVERIFY(!problemsDock->isVisible());
    QVERIFY(toggleNodeLibraryAction->isChecked());
    QVERIFY(toggleInspectorAction->isChecked());
    QVERIFY(toggleProblemsAction->isChecked());

    tabs->setCurrentIndex(0);
    QCoreApplication::processEvents();

    QVERIFY(nodeLibraryDock->isVisible());
    QVERIFY(inspectorDock->isVisible());
    QVERIFY(problemsDock->isVisible());
    QVERIFY(toggleNodeLibraryAction->isChecked());
    QVERIFY(toggleInspectorAction->isChecked());
    QVERIFY(toggleProblemsAction->isChecked());
}

void MainWindowTests::appliesLightWorkbenchCanvasBackground()
{
    QtNodes::GraphicsViewStyle::setStyle(R"({
      "GraphicsViewStyle": {
        "BackgroundColor": [53, 53, 53],
        "FineGridColor": [60, 60, 60],
        "CoarseGridColor": [25, 25, 25]
      }
    })");
    QtNodes::ConnectionStyle::setConnectionStyle(R"({
      "ConnectionStyle": {
        "ConstructionColor": [128, 128, 128],
        "NormalColor": [255, 255, 255],
        "SelectedColor": [255, 255, 255],
        "SelectedHaloColor": [255, 255, 255],
        "HoveredColor": [255, 255, 255],
        "LineWidth": 3.0,
        "ConstructionLineWidth": 2.0,
        "PointDiameter": 10.0,
        "UseDataDefinedColors": false
      }
    })");

    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *graphicsView = window.findChild<QGraphicsView *>();
    QVERIFY(graphicsView != nullptr);

    QCOMPARE(graphicsView->backgroundBrush().color(), QColor(245, 240, 231));
}

void MainWindowTests::createsPrimaryToolbarAndStatusBar()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *toolbar = window.findChild<QToolBar *>("primaryToolBar");
    QVERIFY(toolbar != nullptr);
    QCOMPARE(toolbar->actions().size(), 13);
    QCOMPARE(toolbar->actions().at(0)->text(), QString::fromUtf8("新建"));
    QCOMPARE(toolbar->actions().at(2)->text(), QString::fromUtf8("保存"));

    QVERIFY(window.statusBar() != nullptr);
}

void MainWindowTests::showsGroupedToolbarLayout()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *toolbar = window.findChild<QToolBar *>("primaryToolBar");
    QVERIFY(toolbar != nullptr);

    const auto actions = toolbar->actions();
    QCOMPARE(actions.size(), 13);
    QCOMPARE(actions.at(0)->objectName(), QString("newAction"));
    QCOMPARE(actions.at(1)->objectName(), QString("openAction"));
    QCOMPARE(actions.at(2)->objectName(), QString("saveAction"));
    QVERIFY(actions.at(3)->isSeparator());
    QCOMPARE(actions.at(4)->objectName(), QString("undoAction"));
    QCOMPARE(actions.at(5)->objectName(), QString("redoAction"));
    QCOMPARE(actions.at(6)->objectName(), QString("deleteAction"));
    QVERIFY(actions.at(7)->isSeparator());
    QCOMPARE(actions.at(8)->objectName(), QString("selectAllAction"));
    QCOMPARE(actions.at(9)->objectName(), QString("centerAction"));
    QCOMPARE(actions.at(10)->objectName(), QString("fitWorkflowAction"));
    QVERIFY(actions.at(11)->isSeparator());
    QVERIFY(actions.at(12)->isSeparator() == false);
}

void MainWindowTests::exposesToolbarStylingHooks()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *toolbar = window.findChild<QToolBar *>("primaryToolBar");
    auto *saveAction = window.findChild<QAction *>("saveAction");
    auto *languageButton = window.findChild<QToolButton *>("languageToolButton");
    QVERIFY(toolbar != nullptr);
    QVERIFY(saveAction != nullptr);
    QVERIFY(languageButton != nullptr);

    QCOMPARE(toolbar->property("variant").toString(), QString("workbench"));
    QCOMPARE(saveAction->property("emphasis").toString(), QString("primary"));
    QCOMPARE(languageButton->property("variant").toString(), QString("toolbar-language"));
}

void MainWindowTests::exposesStableObjectNamesForWorkbenchChrome()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *toolbar = window.findChild<QToolBar *>("primaryToolBar");
    auto *nodeLibraryDock = window.findChild<QDockWidget *>("nodeLibraryDock");
    auto *inspectorDock = window.findChild<QDockWidget *>("inspectorDock");
    auto *languageButton = window.findChild<QToolButton *>("languageToolButton");

    QVERIFY(toolbar != nullptr);
    QVERIFY(nodeLibraryDock != nullptr);
    QVERIFY(inspectorDock != nullptr);
    QVERIFY(languageButton != nullptr);
}

void MainWindowTests::createsMenuBarWithFileViewAndSettingsMenus()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *menuBar = window.menuBar();
    QVERIFY(menuBar != nullptr);
    QCOMPARE(menuBar->actions().size(), 5);
    QCOMPARE(menuBar->actions().at(0)->text(), QString::fromUtf8("文件"));
    QCOMPARE(menuBar->actions().at(1)->text(), QString::fromUtf8("编辑"));
    QCOMPARE(menuBar->actions().at(2)->text(), QString::fromUtf8("视图"));
    QCOMPARE(menuBar->actions().at(3)->text(), QString::fromUtf8("设置"));
    QCOMPARE(menuBar->actions().at(4)->text(), QString::fromUtf8("帮助"));

    auto *settingsMenu = menuBar->actions().at(3)->menu();
    QVERIFY(settingsMenu != nullptr);
    QCOMPARE(settingsMenu->actions().size(), 1);
    QCOMPARE(settingsMenu->actions().at(0)->text(), QString::fromUtf8("语言"));
    QVERIFY(settingsMenu->actions().at(0)->menu() != nullptr);
    QCOMPARE(settingsMenu->actions().at(0)->menu()->actions().size(), 2);

    auto *helpMenu = menuBar->actions().at(4)->menu();
    QVERIFY(helpMenu != nullptr);
    QCOMPARE(helpMenu->actions().size(), 1);
    QCOMPARE(helpMenu->actions().at(0)->objectName(), QString("helpAction"));
    QCOMPARE(helpMenu->actions().at(0)->text(), QString::fromUtf8("用户指南"));
}

void MainWindowTests::exposesCanvasArrangeActionsForMultiSelection()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *alignLeftAction = window.findChild<QAction *>("alignLeftAction");
    auto *distributeHorizontalAction = window.findChild<QAction *>("distributeHorizontalAction");
    QVERIFY(editor != nullptr);
    QVERIFY(alignLeftAction != nullptr);
    QVERIFY(distributeHorizontalAction != nullptr);

    QVERIFY(!alignLeftAction->isEnabled());
    QVERIFY(!distributeHorizontalAction->isEnabled());

    const auto startNode = editor->createNode("start", QPointF(320.0, 40.0));
    const auto promptNode = editor->createNode("prompt", QPointF(100.0, 180.0));
    const auto outputNode = editor->createNode("output", QPointF(240.0, 320.0));
    editor->markCurrentStateClean();

    selectNodeGraphicsObject(editor, startNode);
    selectNodeGraphicsObject(editor, promptNode, false);
    QCoreApplication::processEvents();

    QVERIFY(alignLeftAction->isEnabled());
    QVERIFY(!distributeHorizontalAction->isEnabled());

    selectNodeGraphicsObject(editor, outputNode, false);
    QCoreApplication::processEvents();

    QVERIFY(distributeHorizontalAction->isEnabled());

    alignLeftAction->trigger();
    QCOMPARE(editor->nodePosition(startNode).x(), 100.0);
    QCOMPARE(editor->nodePosition(promptNode).x(), 100.0);
    QCOMPARE(editor->nodePosition(outputNode).x(), 100.0);
}

void MainWindowTests::showsProblemsPanelWithWorkflowValidationIssues()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *problemsDock = window.findChild<QDockWidget *>("problemsDock");
    auto *problemsTable = window.findChild<QTableWidget *>("problemsTable");
    QVERIFY(editor != nullptr);
    QVERIFY(problemsDock != nullptr);
    QVERIFY(problemsTable != nullptr);

    QVERIFY(problemsDock->windowTitle().startsWith(QString::fromUtf8("问题")));
    QCOMPARE(problemsTable->rowCount(), 0);
    QVERIFY(problemsTable->hasMouseTracking());
    QVERIFY(problemsTable->viewport()->hasMouseTracking());
    QVERIFY(problemsTable->alternatingRowColors());
    QVERIFY(!problemsTable->showGrid());

    editor->createNode("prompt");
    editor->createNode("output");
    QCoreApplication::processEvents();

    QCOMPARE(problemsTable->rowCount(), 2);
    QCOMPARE(problemsTable->horizontalHeaderItem(0)->text(), QString::fromUtf8("级别"));
    QCOMPARE(problemsTable->horizontalHeaderItem(1)->text(), QString::fromUtf8("节点"));
    QCOMPARE(problemsTable->horizontalHeaderItem(2)->text(), QString::fromUtf8("类型"));
    QCOMPARE(problemsTable->horizontalHeaderItem(3)->text(), QString::fromUtf8("问题"));

    QStringList messages;
    for (int row = 0; row < problemsTable->rowCount(); ++row)
        messages << problemsTable->item(row, 3)->text();
    QVERIFY(messages.contains(QString::fromUtf8("提示词模板为空。")));
    QVERIFY(messages.contains(QString::fromUtf8("输出节点需要输入连接。")));
}

void MainWindowTests::problemsPanelShowsCountAndEmptyState()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *problemsDock = window.findChild<QDockWidget *>("problemsDock");
    auto *problemsTable = window.findChild<QTableWidget *>("problemsTable");
    auto *emptyStateLabel = window.findChild<QLabel *>("problemsEmptyStateLabel");
    QVERIFY(editor != nullptr);
    QVERIFY(problemsDock != nullptr);
    QVERIFY(problemsTable != nullptr);
    QVERIFY(emptyStateLabel != nullptr);

    QCOMPARE(problemsDock->windowTitle(), QString::fromUtf8("问题 (0)"));
    QVERIFY(emptyStateLabel->isVisible());
    QVERIFY(!problemsTable->isVisible());

    editor->createNode("prompt");
    editor->createNode("output");
    QCoreApplication::processEvents();

    QCOMPARE(problemsDock->windowTitle(), QString::fromUtf8("问题 (2)"));
    QVERIFY(!emptyStateLabel->isVisible());
    QVERIFY(problemsTable->isVisible());
}

void MainWindowTests::problemsPanelFiltersIssuesBySeverity()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *problemsTable = window.findChild<QTableWidget *>("problemsTable");
    auto *filterComboBox = window.findChild<QComboBox *>("problemsFilterComboBox");
    QVERIFY(editor != nullptr);
    QVERIFY(problemsTable != nullptr);
    QVERIFY(filterComboBox != nullptr);

    const auto templateVariablesNode = editor->createNode("templateVariables");
    editor->selectNode(templateVariablesNode);
    editor->setSelectedNodeProperty("variablesJson", "{bad json");
    editor->createNode("prompt");
    QCoreApplication::processEvents();
    QCOMPARE(problemsTable->rowCount(), 2);

    filterComboBox->setCurrentIndex(filterComboBox->findData(QString("error")));
    QCoreApplication::processEvents();
    QCOMPARE(problemsTable->rowCount(), 1);
    QCOMPARE(problemsTable->item(0, 0)->text(), QString("error"));

    filterComboBox->setCurrentIndex(filterComboBox->findData(QString("warning")));
    QCoreApplication::processEvents();
    QCOMPARE(problemsTable->rowCount(), 1);
    QCOMPARE(problemsTable->item(0, 0)->text(), QString("warning"));

    filterComboBox->setCurrentIndex(filterComboBox->findData(QString("all")));
    QCoreApplication::processEvents();
    QCOMPARE(problemsTable->rowCount(), 2);
}

void MainWindowTests::activatingProblemSelectsNodeAndHighlightsInspectorField()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);
    window.resize(1280, 840);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *problemsTable = window.findChild<QTableWidget *>("problemsTable");
    auto *promptUserLabel = window.findChild<QLabel *>("inspectorPromptUserTemplateLabel");
    QVERIFY(editor != nullptr);
    QVERIFY(problemsTable != nullptr);
    QVERIFY(promptUserLabel != nullptr);

    const auto promptNode = editor->createNode("prompt", QPointF(1800.0, 1300.0));
    editor->createNode("output", QPointF(40.0, 40.0));
    QCoreApplication::processEvents();

    int promptProblemRow = -1;
    for (int row = 0; row < problemsTable->rowCount(); ++row) {
        if (problemsTable->item(row, 3)->text() == QString::fromUtf8("提示词模板为空。")) {
            promptProblemRow = row;
            break;
        }
    }
    QVERIFY(promptProblemRow >= 0);

    auto *messageItem = problemsTable->item(promptProblemRow, 3);
    QVERIFY(messageItem != nullptr);
    const QPoint activationPoint = problemsTable->visualItemRect(messageItem).center();
    QTest::mouseClick(problemsTable->viewport(), Qt::LeftButton, Qt::NoModifier, activationPoint);
    QCoreApplication::processEvents();

    QCOMPARE(problemsTable->currentRow(), promptProblemRow);
    QCOMPARE(editor->selectedNodeDisplayName(), QString::fromUtf8("提示词"));
    QCOMPARE(promptUserLabel->property("validationState").toString(), QString("warning"));

    const QPointF centered = editor->viewportSceneCenter();
    QVERIFY(qAbs(centered.x() - editor->nodePosition(promptNode).x()) < 400.0);
    QVERIFY(qAbs(centered.y() - editor->nodePosition(promptNode).y()) < 300.0);
}

void MainWindowTests::problemsPanelRefreshesAfterIssueIsFixed()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *problemsTable = window.findChild<QTableWidget *>("problemsTable");
    auto *promptUserEdit = window.findChild<QTextEdit *>("inspectorPromptUserTemplateEdit");
    QVERIFY(editor != nullptr);
    QVERIFY(problemsTable != nullptr);
    QVERIFY(promptUserEdit != nullptr);

    const auto promptNode = editor->createNode("prompt");
    editor->selectNode(promptNode);
    QCoreApplication::processEvents();
    QCOMPARE(problemsTable->rowCount(), 1);
    QCOMPARE(problemsTable->item(0, 3)->text(), QString::fromUtf8("提示词模板为空。"));

    promptUserEdit->setPlainText("Summarize {{input}}");
    QCoreApplication::processEvents();

    QCOMPARE(problemsTable->rowCount(), 0);
}

void MainWindowTests::problemsPanelRefreshesAfterTypingFixInInspector()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *problemsTable = window.findChild<QTableWidget *>("problemsTable");
    auto *promptUserEdit = window.findChild<QTextEdit *>("inspectorPromptUserTemplateEdit");
    QVERIFY(editor != nullptr);
    QVERIFY(problemsTable != nullptr);
    QVERIFY(promptUserEdit != nullptr);

    const auto promptNode = editor->createNode("prompt");
    editor->selectNode(promptNode);
    QCoreApplication::processEvents();
    QCOMPARE(problemsTable->rowCount(), 1);

    promptUserEdit->setFocus();
    QVERIFY(promptUserEdit->hasFocus());
    QTest::keyClicks(promptUserEdit, "Summarize {{input}}");
    QCoreApplication::processEvents();

    QVERIFY(!editor->selectedNodeProperty("userPromptTemplate").toString().trimmed().isEmpty());
    QCOMPARE(problemsTable->rowCount(), 0);
}

void MainWindowTests::keepsCanvasMiniMapHiddenWhenWorkflowIsEmpty()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);
    window.show();
    QCoreApplication::processEvents();

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    QVERIFY(editor != nullptr);
    QVERIFY(!editor->miniMapVisible());
}

void MainWindowTests::showsCanvasMiniMapWhenWorkflowHasNodes()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);
    window.show();
    QCoreApplication::processEvents();

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    QVERIFY(editor != nullptr);

    const auto startNode = editor->createNode("start");
    QVERIFY(startNode != QtNodes::InvalidNodeId);
    QCoreApplication::processEvents();

    QVERIFY(editor->miniMapVisible());
    QVERIFY(editor->miniMapGeometry().isValid());
    QVERIFY(editor->miniMapGeometry().width() >= 120);
    QVERIFY(editor->miniMapGeometry().height() >= 80);
}

void MainWindowTests::clickingCanvasMiniMapRecentersLargeCanvas()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);
    window.resize(1280, 840);
    window.show();
    QCoreApplication::processEvents();

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    QVERIFY(editor != nullptr);

    const auto startNode = editor->createNode("start");
    const auto outputNode = editor->createNode("output");
    QVERIFY(startNode != QtNodes::InvalidNodeId);
    QVERIFY(outputNode != QtNodes::InvalidNodeId);

    editor->setNodePosition(startNode, QPointF(0.0, 0.0));
    editor->setNodePosition(outputNode, QPointF(2400.0, 1600.0));
    editor->selectNode(startNode);
    editor->centerSelection();
    QCoreApplication::processEvents();

    auto *miniMap = window.findChild<QWidget *>("canvasMiniMap");
    QVERIFY(miniMap != nullptr);
    QVERIFY(editor->miniMapVisible());

    const QPointF beforeCenter = editor->viewportSceneCenter();
    const QPoint clickPoint(miniMap->width() - 12, miniMap->height() - 12);
    QTest::mouseClick(miniMap, Qt::LeftButton, Qt::NoModifier, clickPoint);
    QCoreApplication::processEvents();

    const QPointF afterCenter = editor->viewportSceneCenter();
    QVERIFY(afterCenter.x() > beforeCenter.x() + 400.0);
    QVERIFY(afterCenter.y() > beforeCenter.y() + 250.0);
}

void MainWindowTests::draggingCanvasMiniMapViewportPansWithoutInitialJump()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);
    window.resize(1280, 840);
    window.show();
    QCoreApplication::processEvents();

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    QVERIFY(editor != nullptr);

    const auto startNode = editor->createNode("start");
    const auto outputNode = editor->createNode("output");
    QVERIFY(startNode != QtNodes::InvalidNodeId);
    QVERIFY(outputNode != QtNodes::InvalidNodeId);

    editor->setNodePosition(startNode, QPointF(0.0, 0.0));
    editor->setNodePosition(outputNode, QPointF(2400.0, 1600.0));
    editor->selectNode(startNode);
    editor->centerSelection();
    QCoreApplication::processEvents();

    auto *miniMap = window.findChild<QWidget *>("canvasMiniMap");
    QVERIFY(miniMap != nullptr);
    QVERIFY(editor->miniMapVisible());

    const QRectF indicatorRect = editor->miniMapViewportIndicatorRect();
    QVERIFY(indicatorRect.isValid());

    const QPointF beforeCenter = editor->viewportSceneCenter();
    const QPoint pressPoint = (indicatorRect.topLeft() + QPointF(3.0, 3.0)).toPoint();
    QTest::mousePress(miniMap, Qt::LeftButton, Qt::NoModifier, pressPoint);
    QCoreApplication::processEvents();

    const QPointF afterPressCenter = editor->viewportSceneCenter();
    QVERIFY(qAbs(afterPressCenter.x() - beforeCenter.x()) < 20.0);
    QVERIFY(qAbs(afterPressCenter.y() - beforeCenter.y()) < 20.0);

    const QPoint dragPoint = pressPoint + QPoint(24, 18);
    QMouseEvent dragEvent(QEvent::MouseMove,
                          dragPoint,
                          miniMap->mapToGlobal(dragPoint),
                          Qt::NoButton,
                          Qt::LeftButton,
                          Qt::NoModifier);
    QApplication::sendEvent(miniMap, &dragEvent);
    QTest::mouseRelease(miniMap, Qt::LeftButton, Qt::NoModifier, dragPoint);
    QCoreApplication::processEvents();

    const QPointF afterDragCenter = editor->viewportSceneCenter();
    QVERIFY(afterDragCenter.x() > beforeCenter.x() + 120.0);
    QVERIFY(afterDragCenter.y() > beforeCenter.y() + 80.0);
}

void MainWindowTests::updatesCanvasMiniMapWhenNodeGraphicsMove()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);
    window.resize(1280, 840);
    window.show();
    QCoreApplication::processEvents();

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    QVERIFY(editor != nullptr);

    const auto startNode = editor->createNode("start");
    const auto outputNode = editor->createNode("output");
    QVERIFY(startNode != QtNodes::InvalidNodeId);
    QVERIFY(outputNode != QtNodes::InvalidNodeId);

    editor->setNodePosition(startNode, QPointF(0.0, 0.0));
    editor->setNodePosition(outputNode, QPointF(2400.0, 1600.0));
    QCoreApplication::processEvents();

    const auto beforeRects = editor->miniMapNodeIndicatorRects();
    QVERIFY(beforeRects.size() >= 2);
    const QPointF beforeStartCenter = beforeRects.front().center();

    auto *startGraphicsObject = selectNodeGraphicsObject(editor, startNode);
    QVERIFY(startGraphicsObject != nullptr);
    startGraphicsObject->setPos(QPointF(900.0, 700.0));
    QCoreApplication::processEvents();

    const auto afterRects = editor->miniMapNodeIndicatorRects();
    QVERIFY(afterRects.size() >= 2);
    const QPointF afterStartCenter = afterRects.front().center();
    QVERIFY(afterStartCenter.x() > beforeStartCenter.x() + 20.0);
    QVERIFY(afterStartCenter.y() > beforeStartCenter.y() + 20.0);
}

void MainWindowTests::commitsMovedNodeGraphicsPositionWithUndoRedoOnMouseRelease()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);
    window.resize(1280, 840);
    window.show();
    QCoreApplication::processEvents();

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *graphicsView = editor != nullptr ? editor->findChild<QGraphicsView *>() : nullptr;
    QVERIFY(editor != nullptr);
    QVERIFY(graphicsView != nullptr);

    const auto startNode = editor->createNode("start", QPointF(80.0, 90.0));
    QVERIFY(startNode != QtNodes::InvalidNodeId);
    editor->markCurrentStateClean();

    auto *startGraphicsObject = selectNodeGraphicsObject(editor, startNode);
    QVERIFY(startGraphicsObject != nullptr);
    startGraphicsObject->setPos(QPointF(340.0, 260.0));

    QMouseEvent releaseEvent(QEvent::MouseButtonRelease,
                             QPoint(20, 20),
                             graphicsView->viewport()->mapToGlobal(QPoint(20, 20)),
                             Qt::LeftButton,
                             Qt::NoButton,
                             Qt::NoModifier);
    QVERIFY(QCoreApplication::sendEvent(graphicsView->viewport(), &releaseEvent));
    QCoreApplication::processEvents();

    QCOMPARE(editor->nodePosition(startNode), QPointF(340.0, 260.0));
    QVERIFY(editor->canUndo());
    QVERIFY(!editor->isClean());

    editor->undo();
    QCOMPARE(editor->nodePosition(startNode), QPointF(80.0, 90.0));
    QVERIFY(editor->isClean());

    editor->redo();
    QCOMPARE(editor->nodePosition(startNode), QPointF(340.0, 260.0));
}

void MainWindowTests::fitWorkflowActionShowsEntireLargeWorkflow()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);
    window.resize(1280, 840);
    window.show();
    QCoreApplication::processEvents();

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *fitWorkflowAction = window.findChild<QAction *>("fitWorkflowAction");
    QVERIFY(editor != nullptr);
    QVERIFY(fitWorkflowAction != nullptr);

    const auto startNode = editor->createNode("start");
    const auto outputNode = editor->createNode("output");
    QVERIFY(startNode != QtNodes::InvalidNodeId);
    QVERIFY(outputNode != QtNodes::InvalidNodeId);

    editor->setNodePosition(startNode, QPointF(0.0, 0.0));
    editor->setNodePosition(outputNode, QPointF(2400.0, 1600.0));
    editor->selectNode(startNode);
    editor->centerSelection();
    QCoreApplication::processEvents();

    const QPointF beforeCenter = editor->viewportSceneCenter();
    fitWorkflowAction->trigger();
    QCoreApplication::processEvents();

    const QPointF afterCenter = editor->viewportSceneCenter();
    QVERIFY(afterCenter.x() > beforeCenter.x() + 400.0);
    QVERIFY(afterCenter.y() > beforeCenter.y() + 250.0);
}

void MainWindowTests::alignsSelectedNodesLeftWithUndoRedo()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    QVERIFY(editor != nullptr);

    const auto startNode = editor->createNode("start", QPointF(320.0, 40.0));
    const auto promptNode = editor->createNode("prompt", QPointF(100.0, 180.0));
    const auto outputNode = editor->createNode("output", QPointF(240.0, 320.0));
    QVERIFY(startNode != QtNodes::InvalidNodeId);
    QVERIFY(promptNode != QtNodes::InvalidNodeId);
    QVERIFY(outputNode != QtNodes::InvalidNodeId);

    editor->markCurrentStateClean();
    selectNodeGraphicsObject(editor, startNode);
    selectNodeGraphicsObject(editor, promptNode, false);
    selectNodeGraphicsObject(editor, outputNode, false);

    editor->alignSelectedNodes(QtNodesEditorWidget::Alignment::Left);

    QCOMPARE(editor->nodePosition(startNode).x(), 100.0);
    QCOMPARE(editor->nodePosition(promptNode).x(), 100.0);
    QCOMPARE(editor->nodePosition(outputNode).x(), 100.0);
    QCOMPARE(editor->nodePosition(startNode).y(), 40.0);
    QCOMPARE(editor->nodePosition(promptNode).y(), 180.0);
    QCOMPARE(editor->nodePosition(outputNode).y(), 320.0);
    QVERIFY(editor->canUndo());
    QVERIFY(!editor->isClean());

    editor->undo();
    QCOMPARE(editor->nodePosition(startNode), QPointF(320.0, 40.0));
    QCOMPARE(editor->nodePosition(promptNode), QPointF(100.0, 180.0));
    QCOMPARE(editor->nodePosition(outputNode), QPointF(240.0, 320.0));
    QVERIFY(editor->isClean());

    editor->redo();
    QCOMPARE(editor->nodePosition(startNode).x(), 100.0);
    QCOMPARE(editor->nodePosition(promptNode).x(), 100.0);
    QCOMPARE(editor->nodePosition(outputNode).x(), 100.0);
}

void MainWindowTests::distributesSelectedNodesHorizontallyWithUndoRedo()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    QVERIFY(editor != nullptr);

    const auto startNode = editor->createNode("start", QPointF(100.0, 40.0));
    const auto promptNode = editor->createNode("prompt", QPointF(460.0, 180.0));
    const auto outputNode = editor->createNode("output", QPointF(220.0, 320.0));
    QVERIFY(startNode != QtNodes::InvalidNodeId);
    QVERIFY(promptNode != QtNodes::InvalidNodeId);
    QVERIFY(outputNode != QtNodes::InvalidNodeId);

    editor->markCurrentStateClean();
    selectNodeGraphicsObject(editor, startNode);
    selectNodeGraphicsObject(editor, promptNode, false);
    selectNodeGraphicsObject(editor, outputNode, false);

    editor->distributeSelectedNodes(QtNodesEditorWidget::Distribution::Horizontal);

    QCOMPARE(editor->nodePosition(startNode), QPointF(100.0, 40.0));
    QCOMPARE(editor->nodePosition(outputNode), QPointF(280.0, 320.0));
    QCOMPARE(editor->nodePosition(promptNode), QPointF(460.0, 180.0));
    QVERIFY(editor->canUndo());
    QVERIFY(!editor->isClean());

    editor->undo();
    QCOMPARE(editor->nodePosition(startNode), QPointF(100.0, 40.0));
    QCOMPARE(editor->nodePosition(outputNode), QPointF(220.0, 320.0));
    QCOMPARE(editor->nodePosition(promptNode), QPointF(460.0, 180.0));
    QVERIFY(editor->isClean());

    editor->redo();
    QCOMPARE(editor->nodePosition(outputNode), QPointF(280.0, 320.0));
}

void MainWindowTests::defaultsWorkbenchTextToChinese()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *toolbar = window.findChild<QToolBar *>("primaryToolBar");
    auto *nodeLibraryDock = window.findChild<QDockWidget *>("nodeLibraryDock");
    auto *inspectorDock = window.findChild<QDockWidget *>("inspectorDock");
    auto *hintLabel = window.findChild<QLabel *>("inspectorHintLabel");

    QVERIFY(toolbar != nullptr);
    QVERIFY(nodeLibraryDock != nullptr);
    QVERIFY(inspectorDock != nullptr);
    QVERIFY(hintLabel != nullptr);

    QCOMPARE(window.windowTitle(), QString::fromUtf8("AI 工作流编辑器"));
    QCOMPARE(toolbar->actions().at(0)->text(), QString::fromUtf8("新建"));
    QCOMPARE(nodeLibraryDock->windowTitle(), QString::fromUtf8("节点库"));
    QCOMPARE(inspectorDock->windowTitle(), QString::fromUtf8("属性面板"));
    QCOMPARE(hintLabel->text(), QString::fromUtf8("选择一个节点以编辑其配置"));
    QCOMPARE(window.statusBar()->currentMessage(), QString::fromUtf8("就绪"));
}

void MainWindowTests::keepsChineseDefaultWhenExternalTranslationFileIsUnavailable()
{
    const QString translationPath = QDir(QCoreApplication::applicationDirPath())
                                        .absoluteFilePath(QStringLiteral("../i18n/ai_workflow_editor_zh_CN.qm"));
    const QString backupPath = translationPath + QStringLiteral(".bak");
    struct RestoreTranslationFile
    {
        QString originalPath;
        QString backupPath;
        ~RestoreTranslationFile()
        {
            if (QFile::exists(backupPath) && !QFile::exists(originalPath))
                QFile::rename(backupPath, originalPath);
        }
    } restoreTranslationFile{translationPath, backupPath};

    if (QFile::exists(backupPath) && !QFile::exists(translationPath))
        QVERIFY(QFile::rename(backupPath, translationPath));
    else if (QFile::exists(backupPath))
        QVERIFY(QFile::remove(backupPath));

    QVERIFY(QFile::exists(translationPath));
    QVERIFY(!QFile::exists(backupPath));
    QVERIFY(QFile::rename(translationPath, backupPath));

    LanguageManager languageManager;
    MainWindow window(&languageManager);

    QCOMPARE(window.windowTitle(), QString::fromUtf8("AI 工作流编辑器"));
    QCOMPARE(window.statusBar()->currentMessage(), QString::fromUtf8("就绪"));
}

void MainWindowTests::retranslatesWorkbenchTextToEnglishAtRuntime()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *toolbar = window.findChild<QToolBar *>("primaryToolBar");
    auto *nodeLibraryDock = window.findChild<QDockWidget *>("nodeLibraryDock");
    auto *inspectorDock = window.findChild<QDockWidget *>("inspectorDock");
    auto *nodeLibrary = window.findChild<QListWidget *>("nodeLibraryList");
    auto *hintLabel = window.findChild<QLabel *>("inspectorHintLabel");
    auto *helpAction = window.findChild<QAction *>("helpAction");
    auto *tabs = qobject_cast<QTabWidget *>(window.centralWidget());

    QVERIFY(toolbar != nullptr);
    QVERIFY(nodeLibraryDock != nullptr);
    QVERIFY(inspectorDock != nullptr);
    QVERIFY(nodeLibrary != nullptr);
    QVERIFY(hintLabel != nullptr);
    QVERIFY(helpAction != nullptr);
    QVERIFY(tabs != nullptr);

    helpAction->trigger();
    QCOMPARE(tabs->tabText(1), QString::fromUtf8("用户指南"));

    QVERIFY(languageManager.setLanguage(LanguageManager::Language::English));

    QCOMPARE(window.windowTitle(), QString("AI Workflow Editor"));
    QCOMPARE(tabs->tabText(0), QString("Workflow"));
    QCOMPARE(tabs->tabText(1), QString("User Guide"));
    QCOMPARE(toolbar->actions().at(0)->text(), QString("New"));
    QCOMPARE(nodeLibraryDock->windowTitle(), QString("Node Library"));
    QCOMPARE(inspectorDock->windowTitle(), QString("Inspector"));
    QCOMPARE(nodeLibrary->item(1)->text(), QString("Start"));
    QCOMPARE(hintLabel->text(), QString("Select a node to edit its configuration"));
    QCOMPARE(window.statusBar()->currentMessage(), QString("Ready"));
    QCOMPARE(window.menuBar()->actions().at(0)->text(), QString("File"));
    QCOMPARE(window.menuBar()->actions().at(3)->text(), QString("Settings"));
    QCOMPARE(window.menuBar()->actions().at(4)->text(), QString("Help"));
    QCOMPARE(helpAction->text(), QString("User Guide"));
}

void MainWindowTests::addsIconsToToolbarAndNodeLibrary()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *toolbar = window.findChild<QToolBar *>("primaryToolBar");
    auto *nodeLibrary = window.findChild<QListWidget *>("nodeLibraryList");
    auto *languageButton = window.findChild<QToolButton *>("languageToolButton");

    QVERIFY(toolbar != nullptr);
    QVERIFY(nodeLibrary != nullptr);
    QVERIFY(languageButton != nullptr);

    QVERIFY(!toolbar->actions().at(0)->icon().isNull());
    QVERIFY(!toolbar->actions().at(1)->icon().isNull());
    QVERIFY(!toolbar->actions().at(2)->icon().isNull());
    QVERIFY(!toolbar->actions().at(5)->icon().isNull());
    QVERIFY(!languageButton->icon().isNull());
    QVERIFY(!nodeLibrary->item(0)->icon().isNull());
    QVERIFY(!nodeLibrary->item(1)->icon().isNull());
    QVERIFY(!nodeLibrary->item(4)->icon().isNull());
    QVERIFY(!nodeLibrary->item(6)->icon().isNull());
    QVERIFY(!nodeLibrary->item(12)->icon().isNull());
}

void MainWindowTests::groupsNodeLibraryIntoCategorySections()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *nodeLibrary = window.findChild<QListWidget *>("nodeLibraryList");
    QVERIFY(nodeLibrary != nullptr);

    QCOMPARE(nodeLibrary->count(), 16);
    QCOMPARE(nodeLibrary->item(0)->text(), QString::fromUtf8("流程"));
    QCOMPARE(nodeLibrary->item(0)->data(Qt::UserRole + 1).toString(), QString());
    QVERIFY(!(nodeLibrary->item(0)->flags() & Qt::ItemIsDragEnabled));
    QCOMPARE(nodeLibrary->item(1)->text(), QString::fromUtf8("开始"));
    QCOMPARE(nodeLibrary->item(2)->text(), QString::fromUtf8("条件"));
    QCOMPARE(nodeLibrary->item(3)->text(), QString::fromUtf8("聊天输出"));
    QCOMPARE(nodeLibrary->item(4)->text(), QString::fromUtf8("输出"));
    QCOMPARE(nodeLibrary->item(5)->text(), QString::fromUtf8("AI"));
    QCOMPARE(nodeLibrary->item(6)->text(), QString::fromUtf8("提示词"));
    QCOMPARE(nodeLibrary->item(7)->text(), QString::fromUtf8("大模型"));
    QCOMPARE(nodeLibrary->item(8)->text(), QString::fromUtf8("Agent"));
    QCOMPARE(nodeLibrary->item(9)->text(), QString::fromUtf8("记忆"));
    QCOMPARE(nodeLibrary->item(10)->text(), QString::fromUtf8("检索器"));
    QCOMPARE(nodeLibrary->item(11)->text(), QString::fromUtf8("模板变量"));
    QCOMPARE(nodeLibrary->item(12)->text(), QString::fromUtf8("集成"));
    QCOMPARE(nodeLibrary->item(13)->text(), QString::fromUtf8("HTTP 请求"));
    QCOMPARE(nodeLibrary->item(14)->text(), QString::fromUtf8("JSON 转换"));
    QCOMPARE(nodeLibrary->item(15)->text(), QString::fromUtf8("工具"));
}

void MainWindowTests::addsSearchBoxForNodeLibraryFiltering()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *searchEdit = window.findChild<QLineEdit *>("nodeLibrarySearchEdit");
    auto *nodeLibrary = window.findChild<QListWidget *>("nodeLibraryList");
    QVERIFY(searchEdit != nullptr);
    QVERIFY(nodeLibrary != nullptr);

    QCOMPARE(searchEdit->placeholderText(), QString::fromUtf8("搜索节点"));

    searchEdit->setText(QString::fromUtf8("提示"));

    QVERIFY(nodeLibrary->item(0)->isHidden());
    QVERIFY(!nodeLibrary->item(5)->isHidden());
    QVERIFY(!nodeLibrary->item(6)->isHidden());
    QVERIFY(nodeLibrary->item(7)->isHidden());
}

void MainWindowTests::addsNodeToCanvasFromLibraryAction()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    QVERIFY(editor != nullptr);
    QCOMPARE(editor->nodeCount(), 0);

    window.addNodeFromType("prompt");

    QCOMPARE(editor->nodeCount(), 1);
    QCOMPARE(editor->selectedNodeDisplayName(), QString::fromUtf8("提示词"));
}

void MainWindowTests::acceptsDropOnVisibleCanvasViewport()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *graphicsView = editor->findChild<QGraphicsView *>();

    QVERIFY(editor != nullptr);
    QVERIFY(graphicsView != nullptr);
    QCOMPARE(editor->nodeCount(), 0);

    QMimeData mimeData;
    mimeData.setData(QString::fromUtf8(NodeLibraryListWidget::MimeType), QByteArray("prompt"));

    const QPoint viewportPoint = graphicsView->viewport()->rect().center();
    QDragEnterEvent dragEnterEvent(viewportPoint, Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
    QVERIFY(QCoreApplication::sendEvent(graphicsView->viewport(), &dragEnterEvent));
    QVERIFY(dragEnterEvent.isAccepted());
    QVERIFY(editor->dropPreviewVisible());
    QCOMPARE(editor->dropPreviewSize(), editor->previewNodeSize(QStringLiteral("prompt")));
    QVERIFY(window.statusBar()->currentMessage().contains(QString::fromUtf8("提示词")));

    QDropEvent dropEvent(viewportPoint, Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
    QVERIFY(QCoreApplication::sendEvent(graphicsView->viewport(), &dropEvent));

    QCOMPARE(editor->nodeCount(), 1);
    QCOMPARE(editor->selectedNodeDisplayName(), QString::fromUtf8("提示词"));
    QVERIFY(!editor->dropPreviewVisible());
}

void MainWindowTests::usesCustomNodeCardPainter()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    QVERIFY(editor != nullptr);

    QVERIFY(editor->usesStyledNodePainter());
    QCOMPARE(editor->styledNodeCornerRadius(), 8.0);
}

void MainWindowTests::movesPortsCloserToNodeEdges()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    QVERIFY(editor != nullptr);

    const auto promptNode = editor->createNode("prompt");
    QVERIFY(promptNode != QtNodes::InvalidNodeId);

    const QPointF inPortPosition = editor->portPosition(promptNode, QtNodes::PortType::In, 0);
    const QPointF outPortPosition = editor->portPosition(promptNode, QtNodes::PortType::Out, 0);
    const QSize nodeSize = editor->nodeSize(promptNode);

    QCOMPARE(inPortPosition.x(), 0.0);
    QCOMPARE(outPortPosition.x(), static_cast<qreal>(nodeSize.width()));
}

void MainWindowTests::avoidsExtraNodeGraphicsShadow()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    QVERIFY(editor != nullptr);

    const auto promptNode = editor->createNode("prompt");
    QVERIFY(promptNode != QtNodes::InvalidNodeId);

    QVERIFY(!editor->nodeHasGraphicsEffect(promptNode));
}

void MainWindowTests::syncsSelectedNodeIntoInspectorAndBack()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *displayNameEdit = window.findChild<QLineEdit *>("inspectorDisplayNameEdit");
    auto *descriptionEdit = window.findChild<QTextEdit *>("inspectorDescriptionEdit");

    QVERIFY(editor != nullptr);
    QVERIFY(displayNameEdit != nullptr);
    QVERIFY(descriptionEdit != nullptr);

    const auto nodeId = editor->createNode("tool");
    QVERIFY(nodeId != QtNodes::InvalidNodeId);

    editor->selectNode(nodeId);

    QCOMPARE(displayNameEdit->text(), QString::fromUtf8("工具"));
    QCOMPARE(descriptionEdit->toPlainText(), QString::fromUtf8("工具调用节点"));

    displayNameEdit->setText("Search Tool");
    descriptionEdit->setPlainText("Runs a search connector.");

    QCOMPARE(editor->selectedNodeDisplayName(), QString("Search Tool"));
    QCOMPARE(editor->selectedNodeDescription(), QString("Runs a search connector."));
}

void MainWindowTests::showsTypeSpecificInspectorFieldsForPromptLlmAndTool()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *promptSystemEdit = window.findChild<QTextEdit *>("inspectorPromptSystemEdit");
    auto *promptUserEdit = window.findChild<QTextEdit *>("inspectorPromptUserTemplateEdit");
    auto *llmModelEdit = window.findChild<QLineEdit *>("inspectorLlmModelNameEdit");
    auto *llmTemperatureSpin = window.findChild<QDoubleSpinBox *>("inspectorLlmTemperatureSpin");
    auto *llmMaxTokensSpin = window.findChild<QSpinBox *>("inspectorLlmMaxTokensSpin");
    auto *jsonTransformEdit = window.findChild<QTextEdit *>("inspectorJsonTransformEdit");
    auto *toolNameEdit = window.findChild<QLineEdit *>("inspectorToolNameEdit");
    auto *toolTimeoutSpin = window.findChild<QSpinBox *>("inspectorToolTimeoutSpin");
    auto *toolInputMappingEdit = window.findChild<QTextEdit *>("inspectorToolInputMappingEdit");
    auto *chatOutputRoleEdit = window.findChild<QLineEdit *>("inspectorChatOutputRoleEdit");
    auto *chatOutputTemplateEdit = window.findChild<QTextEdit *>("inspectorChatOutputTemplateEdit");

    QVERIFY(editor != nullptr);
    QVERIFY(promptSystemEdit != nullptr);
    QVERIFY(promptUserEdit != nullptr);
    QVERIFY(llmModelEdit != nullptr);
    QVERIFY(llmTemperatureSpin != nullptr);
    QVERIFY(llmMaxTokensSpin != nullptr);
    QVERIFY(jsonTransformEdit != nullptr);
    QVERIFY(toolNameEdit != nullptr);
    QVERIFY(toolTimeoutSpin != nullptr);
    QVERIFY(toolInputMappingEdit != nullptr);

    const auto promptNode = editor->createNode("prompt");
    editor->selectNode(promptNode);

    QVERIFY(promptSystemEdit->isVisibleTo(&window));
    QVERIFY(promptUserEdit->isVisibleTo(&window));
    QVERIFY(!llmModelEdit->isVisibleTo(&window));
    QVERIFY(!toolNameEdit->isVisibleTo(&window));

    promptSystemEdit->setPlainText("You are a planning assistant.");
    promptUserEdit->setPlainText("Summarize: {{input}}");

    QCOMPARE(editor->selectedNodeProperty("systemPrompt").toString(), QString("You are a planning assistant."));
    QCOMPARE(editor->selectedNodeProperty("userPromptTemplate").toString(), QString("Summarize: {{input}}"));

    const auto llmNode = editor->createNode("llm");
    editor->selectNode(llmNode);

    QVERIFY(!promptSystemEdit->isVisibleTo(&window));
    QVERIFY(llmModelEdit->isVisibleTo(&window));
    QVERIFY(llmTemperatureSpin->isVisibleTo(&window));
    QVERIFY(llmMaxTokensSpin->isVisibleTo(&window));
    QVERIFY(!toolNameEdit->isVisibleTo(&window));

    llmModelEdit->setText("demo-model");
    llmTemperatureSpin->setValue(0.7);
    llmMaxTokensSpin->setValue(2048);

    QCOMPARE(editor->selectedNodeProperty("modelName").toString(), QString("demo-model"));
    QCOMPARE(editor->selectedNodeProperty("temperature").toDouble(), 0.7);
    QCOMPARE(editor->selectedNodeProperty("maxTokens").toInt(), 2048);

    const auto toolNode = editor->createNode("tool");
    editor->selectNode(toolNode);

    QVERIFY(!promptSystemEdit->isVisibleTo(&window));
    QVERIFY(!llmModelEdit->isVisibleTo(&window));
    QVERIFY(toolNameEdit->isVisibleTo(&window));
    QVERIFY(toolTimeoutSpin->isVisibleTo(&window));
    QVERIFY(toolInputMappingEdit->isVisibleTo(&window));

    toolNameEdit->setText("web_search");
    toolTimeoutSpin->setValue(15000);
    toolInputMappingEdit->setPlainText("{\"query\": \"{{input}}\"}");

    QCOMPARE(editor->selectedNodeProperty("toolName").toString(), QString("web_search"));
    QCOMPARE(editor->selectedNodeProperty("timeoutMs").toInt(), 15000);
    QCOMPARE(editor->selectedNodeProperty("inputMapping").toString(), QString("{\"query\": \"{{input}}\"}"));
}

void MainWindowTests::showsTypeSpecificInspectorFieldsForMemory()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *memoryKeyLabel = window.findChild<QLabel *>("inspectorMemoryKeyLabel");
    auto *memoryKeyEdit = window.findChild<QLineEdit *>("inspectorMemoryKeyEdit");
    auto *toolNameEdit = window.findChild<QLineEdit *>("inspectorToolNameEdit");

    QVERIFY(editor != nullptr);
    QVERIFY(memoryKeyLabel != nullptr);
    QVERIFY(memoryKeyEdit != nullptr);
    QVERIFY(toolNameEdit != nullptr);

    const auto memoryNode = editor->createNode("memory");
    editor->selectNode(memoryNode);

    QVERIFY(memoryKeyLabel->isVisibleTo(&window));
    QVERIFY(memoryKeyEdit->isVisibleTo(&window));
    QVERIFY(!toolNameEdit->isVisibleTo(&window));

    memoryKeyEdit->setText("conversation_summary");
    QCOMPARE(editor->selectedNodeProperty("memoryKey").toString(), QString("conversation_summary"));
}

void MainWindowTests::showsTypeSpecificInspectorFieldsForRetriever()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *retrieverKeyLabel = window.findChild<QLabel *>("inspectorRetrieverKeyLabel");
    auto *retrieverKeyEdit = window.findChild<QLineEdit *>("inspectorRetrieverKeyEdit");
    auto *toolNameEdit = window.findChild<QLineEdit *>("inspectorToolNameEdit");

    QVERIFY(editor != nullptr);
    QVERIFY(retrieverKeyLabel != nullptr);
    QVERIFY(retrieverKeyEdit != nullptr);
    QVERIFY(toolNameEdit != nullptr);

    const auto retrieverNode = editor->createNode("retriever");
    editor->selectNode(retrieverNode);

    QVERIFY(retrieverKeyLabel->isVisibleTo(&window));
    QVERIFY(retrieverKeyEdit->isVisibleTo(&window));
    QVERIFY(!toolNameEdit->isVisibleTo(&window));

    retrieverKeyEdit->setText("knowledge_base_search");
    QCOMPARE(editor->selectedNodeProperty("retrieverKey").toString(), QString("knowledge_base_search"));
}

void MainWindowTests::showsTypeSpecificInspectorFieldsForTemplateVariables()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *variablesLabel = window.findChild<QLabel *>("inspectorTemplateVariablesLabel");
    auto *variablesEdit = window.findChild<QTextEdit *>("inspectorTemplateVariablesEdit");
    auto *toolNameEdit = window.findChild<QLineEdit *>("inspectorToolNameEdit");

    QVERIFY(editor != nullptr);
    QVERIFY(variablesLabel != nullptr);
    QVERIFY(variablesEdit != nullptr);
    QVERIFY(toolNameEdit != nullptr);

    const auto templateVariablesNode = editor->createNode("templateVariables");
    editor->selectNode(templateVariablesNode);

    QVERIFY(variablesLabel->isVisibleTo(&window));
    QVERIFY(variablesEdit->isVisibleTo(&window));
    QVERIFY(!toolNameEdit->isVisibleTo(&window));

    variablesEdit->setPlainText("{\"topic\": \"{{input}}\"}");
    QCOMPARE(editor->selectedNodeProperty("variablesJson").toString(), QString("{\"topic\": \"{{input}}\"}"));
}

void MainWindowTests::showsTypeSpecificInspectorFieldsForHttpRequest()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *methodLabel = window.findChild<QLabel *>("inspectorHttpRequestMethodLabel");
    auto *methodEdit = window.findChild<QLineEdit *>("inspectorHttpRequestMethodEdit");
    auto *urlLabel = window.findChild<QLabel *>("inspectorHttpRequestUrlLabel");
    auto *urlEdit = window.findChild<QLineEdit *>("inspectorHttpRequestUrlEdit");
    auto *headersEdit = window.findChild<QTextEdit *>("inspectorHttpRequestHeadersEdit");
    auto *bodyEdit = window.findChild<QTextEdit *>("inspectorHttpRequestBodyEdit");
    auto *timeoutSpin = window.findChild<QSpinBox *>("inspectorHttpRequestTimeoutSpin");
    auto *toolNameEdit = window.findChild<QLineEdit *>("inspectorToolNameEdit");

    QVERIFY(editor != nullptr);
    QVERIFY(methodLabel != nullptr);
    QVERIFY(methodEdit != nullptr);
    QVERIFY(urlLabel != nullptr);
    QVERIFY(urlEdit != nullptr);
    QVERIFY(headersEdit != nullptr);
    QVERIFY(bodyEdit != nullptr);
    QVERIFY(timeoutSpin != nullptr);
    QVERIFY(toolNameEdit != nullptr);

    const auto httpRequestNode = editor->createNode("httpRequest");
    editor->selectNode(httpRequestNode);

    QVERIFY(methodLabel->isVisibleTo(&window));
    QVERIFY(methodEdit->isVisibleTo(&window));
    QVERIFY(urlLabel->isVisibleTo(&window));
    QVERIFY(urlEdit->isVisibleTo(&window));
    QVERIFY(headersEdit->isVisibleTo(&window));
    QVERIFY(bodyEdit->isVisibleTo(&window));
    QVERIFY(timeoutSpin->isVisibleTo(&window));
    QVERIFY(!toolNameEdit->isVisibleTo(&window));

    methodEdit->setText("POST");
    urlEdit->setText("https://api.example.com/search");
    headersEdit->setPlainText("{\"Authorization\": \"Bearer {{token}}\"}");
    bodyEdit->setPlainText("{\"query\": \"{{input}}\"}");
    timeoutSpin->setValue(45000);

    QCOMPARE(editor->selectedNodeProperty("method").toString(), QString("POST"));
    QCOMPARE(editor->selectedNodeProperty("url").toString(), QString("https://api.example.com/search"));
    QCOMPARE(editor->selectedNodeProperty("headersJson").toString(), QString("{\"Authorization\": \"Bearer {{token}}\"}"));
    QCOMPARE(editor->selectedNodeProperty("bodyTemplate").toString(), QString("{\"query\": \"{{input}}\"}"));
    QCOMPARE(editor->selectedNodeProperty("timeoutMs").toInt(), 45000);
}

void MainWindowTests::showsTypeSpecificInspectorFieldsForJsonTransform()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *transformLabel = window.findChild<QLabel *>("inspectorJsonTransformLabel");
    auto *transformEdit = window.findChild<QTextEdit *>("inspectorJsonTransformEdit");
    auto *toolNameEdit = window.findChild<QLineEdit *>("inspectorToolNameEdit");

    QVERIFY(editor != nullptr);
    QVERIFY(transformLabel != nullptr);
    QVERIFY(transformEdit != nullptr);
    QVERIFY(toolNameEdit != nullptr);

    const auto jsonTransformNode = editor->createNode("jsonTransform");
    editor->selectNode(jsonTransformNode);

    QVERIFY(transformLabel->isVisibleTo(&window));
    QVERIFY(transformEdit->isVisibleTo(&window));
    QVERIFY(!toolNameEdit->isVisibleTo(&window));

    transformEdit->setPlainText("{\"summary\": \"{{http.response.summary}}\"}");
    QCOMPARE(editor->selectedNodeProperty("transformJson").toString(),
             QString("{\"summary\": \"{{http.response.summary}}\"}"));
}

void MainWindowTests::showsTypeSpecificInspectorFieldsForAgent()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *instructionsLabel = window.findChild<QLabel *>("inspectorAgentInstructionsLabel");
    auto *instructionsEdit = window.findChild<QTextEdit *>("inspectorAgentInstructionsEdit");
    auto *modelNameLabel = window.findChild<QLabel *>("inspectorAgentModelNameLabel");
    auto *modelNameEdit = window.findChild<QLineEdit *>("inspectorAgentModelNameEdit");
    auto *maxIterationsSpin = window.findChild<QSpinBox *>("inspectorAgentMaxIterationsSpin");
    auto *toolNameEdit = window.findChild<QLineEdit *>("inspectorToolNameEdit");

    QVERIFY(editor != nullptr);
    QVERIFY(instructionsLabel != nullptr);
    QVERIFY(instructionsEdit != nullptr);
    QVERIFY(modelNameLabel != nullptr);
    QVERIFY(modelNameEdit != nullptr);
    QVERIFY(maxIterationsSpin != nullptr);
    QVERIFY(toolNameEdit != nullptr);

    const auto agentNode = editor->createNode("agent");
    editor->selectNode(agentNode);

    QVERIFY(instructionsLabel->isVisibleTo(&window));
    QVERIFY(instructionsEdit->isVisibleTo(&window));
    QVERIFY(modelNameLabel->isVisibleTo(&window));
    QVERIFY(modelNameEdit->isVisibleTo(&window));
    QVERIFY(maxIterationsSpin->isVisibleTo(&window));
    QVERIFY(!toolNameEdit->isVisibleTo(&window));

    instructionsEdit->setPlainText("Use tools when necessary and produce a final answer.");
    modelNameEdit->setText("gpt-5.4");
    maxIterationsSpin->setValue(8);

    QCOMPARE(editor->selectedNodeProperty("agentInstructions").toString(),
             QString("Use tools when necessary and produce a final answer."));
    QCOMPARE(editor->selectedNodeProperty("modelName").toString(), QString("gpt-5.4"));
    QCOMPARE(editor->selectedNodeProperty("maxIterations").toInt(), 8);
}

void MainWindowTests::showsTypeSpecificInspectorFieldsForChatOutput()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *roleLabel = window.findChild<QLabel *>("inspectorChatOutputRoleLabel");
    auto *roleEdit = window.findChild<QLineEdit *>("inspectorChatOutputRoleEdit");
    auto *templateLabel = window.findChild<QLabel *>("inspectorChatOutputTemplateLabel");
    auto *templateEdit = window.findChild<QTextEdit *>("inspectorChatOutputTemplateEdit");
    auto *toolNameEdit = window.findChild<QLineEdit *>("inspectorToolNameEdit");

    QVERIFY(editor != nullptr);
    QVERIFY(roleLabel != nullptr);
    QVERIFY(roleEdit != nullptr);
    QVERIFY(templateLabel != nullptr);
    QVERIFY(templateEdit != nullptr);
    QVERIFY(toolNameEdit != nullptr);

    const auto chatOutputNode = editor->createNode("chatOutput");
    editor->selectNode(chatOutputNode);

    QVERIFY(roleLabel->isVisibleTo(&window));
    QVERIFY(roleEdit->isVisibleTo(&window));
    QVERIFY(templateLabel->isVisibleTo(&window));
    QVERIFY(templateEdit->isVisibleTo(&window));
    QVERIFY(!toolNameEdit->isVisibleTo(&window));

    roleEdit->setText("assistant");
    templateEdit->setPlainText("{{agent.final_answer}}");

    QCOMPARE(editor->selectedNodeProperty("messageRole").toString(), QString("assistant"));
    QCOMPARE(editor->selectedNodeProperty("messageTemplate").toString(), QString("{{agent.final_answer}}"));
}

void MainWindowTests::exposesStableInspectorFieldMetadata()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *promptSystemEdit = window.findChild<QTextEdit *>("inspectorPromptSystemEdit");
    auto *promptUserEdit = window.findChild<QTextEdit *>("inspectorPromptUserTemplateEdit");
    auto *llmModelEdit = window.findChild<QLineEdit *>("inspectorLlmModelNameEdit");
    auto *llmTemperatureSpin = window.findChild<QDoubleSpinBox *>("inspectorLlmTemperatureSpin");
    auto *llmMaxTokensSpin = window.findChild<QSpinBox *>("inspectorLlmMaxTokensSpin");
    auto *jsonTransformEdit = window.findChild<QTextEdit *>("inspectorJsonTransformEdit");
    auto *toolNameEdit = window.findChild<QLineEdit *>("inspectorToolNameEdit");
    auto *toolTimeoutSpin = window.findChild<QSpinBox *>("inspectorToolTimeoutSpin");
    auto *toolInputMappingEdit = window.findChild<QTextEdit *>("inspectorToolInputMappingEdit");
    auto *chatOutputRoleEdit = window.findChild<QLineEdit *>("inspectorChatOutputRoleEdit");
    auto *chatOutputTemplateEdit = window.findChild<QTextEdit *>("inspectorChatOutputTemplateEdit");

    QVERIFY(promptSystemEdit != nullptr);
    QVERIFY(promptUserEdit != nullptr);
    QVERIFY(llmModelEdit != nullptr);
    QVERIFY(llmTemperatureSpin != nullptr);
    QVERIFY(llmMaxTokensSpin != nullptr);
    QVERIFY(jsonTransformEdit != nullptr);
    auto *agentInstructionsEdit = window.findChild<QTextEdit *>("inspectorAgentInstructionsEdit");
    auto *agentModelEdit = window.findChild<QLineEdit *>("inspectorAgentModelNameEdit");
    auto *agentMaxIterationsSpin = window.findChild<QSpinBox *>("inspectorAgentMaxIterationsSpin");
    QVERIFY(toolNameEdit != nullptr);
    QVERIFY(toolTimeoutSpin != nullptr);
    QVERIFY(toolInputMappingEdit != nullptr);
    QVERIFY(agentInstructionsEdit != nullptr);
    QVERIFY(agentModelEdit != nullptr);
    QVERIFY(agentMaxIterationsSpin != nullptr);
    QVERIFY(chatOutputRoleEdit != nullptr);
    QVERIFY(chatOutputTemplateEdit != nullptr);

    QCOMPARE(promptSystemEdit->property("inspectorPropertyKey").toString(), QString("systemPrompt"));
    QCOMPARE(promptSystemEdit->property("inspectorTypeKey").toString(), QString("prompt"));
    QCOMPARE(promptUserEdit->property("inspectorPropertyKey").toString(), QString("userPromptTemplate"));
    QCOMPARE(promptUserEdit->property("inspectorTypeKey").toString(), QString("prompt"));
    QCOMPARE(llmModelEdit->property("inspectorPropertyKey").toString(), QString("modelName"));
    QCOMPARE(llmModelEdit->property("inspectorTypeKey").toString(), QString("llm"));
    QCOMPARE(llmTemperatureSpin->property("inspectorPropertyKey").toString(), QString("temperature"));
    QCOMPARE(llmTemperatureSpin->property("inspectorTypeKey").toString(), QString("llm"));
    QCOMPARE(llmMaxTokensSpin->property("inspectorPropertyKey").toString(), QString("maxTokens"));
    QCOMPARE(llmMaxTokensSpin->property("inspectorTypeKey").toString(), QString("llm"));
    QCOMPARE(agentInstructionsEdit->property("inspectorPropertyKey").toString(), QString("agentInstructions"));
    QCOMPARE(agentInstructionsEdit->property("inspectorTypeKey").toString(), QString("agent"));
    QCOMPARE(agentModelEdit->property("inspectorPropertyKey").toString(), QString("modelName"));
    QCOMPARE(agentModelEdit->property("inspectorTypeKey").toString(), QString("agent"));
    QCOMPARE(agentMaxIterationsSpin->property("inspectorPropertyKey").toString(), QString("maxIterations"));
    QCOMPARE(agentMaxIterationsSpin->property("inspectorTypeKey").toString(), QString("agent"));
    QCOMPARE(chatOutputRoleEdit->property("inspectorPropertyKey").toString(), QString("messageRole"));
    QCOMPARE(chatOutputRoleEdit->property("inspectorTypeKey").toString(), QString("chatOutput"));
    QCOMPARE(chatOutputTemplateEdit->property("inspectorPropertyKey").toString(), QString("messageTemplate"));
    QCOMPARE(chatOutputTemplateEdit->property("inspectorTypeKey").toString(), QString("chatOutput"));
    QCOMPARE(jsonTransformEdit->property("inspectorPropertyKey").toString(), QString("transformJson"));
    QCOMPARE(jsonTransformEdit->property("inspectorTypeKey").toString(), QString("jsonTransform"));
    QCOMPARE(toolNameEdit->property("inspectorPropertyKey").toString(), QString("toolName"));
    QCOMPARE(toolNameEdit->property("inspectorTypeKey").toString(), QString("tool"));
    QCOMPARE(toolTimeoutSpin->property("inspectorPropertyKey").toString(), QString("timeoutMs"));
    QCOMPARE(toolTimeoutSpin->property("inspectorTypeKey").toString(), QString("tool"));
    QCOMPARE(toolInputMappingEdit->property("inspectorPropertyKey").toString(), QString("inputMapping"));
    QCOMPARE(toolInputMappingEdit->property("inspectorTypeKey").toString(), QString("tool"));
}

void MainWindowTests::exposesStableInspectorFieldHintMetadata()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *promptUserEdit = window.findChild<QTextEdit *>("inspectorPromptUserTemplateEdit");
    auto *llmModelEdit = window.findChild<QLineEdit *>("inspectorLlmModelNameEdit");
    auto *jsonTransformEdit = window.findChild<QTextEdit *>("inspectorJsonTransformEdit");
    auto *toolInputMappingEdit = window.findChild<QTextEdit *>("inspectorToolInputMappingEdit");
    auto *chatOutputRoleEdit = window.findChild<QLineEdit *>("inspectorChatOutputRoleEdit");
    auto *chatOutputTemplateEdit = window.findChild<QTextEdit *>("inspectorChatOutputTemplateEdit");

    QVERIFY(promptUserEdit != nullptr);
    QVERIFY(llmModelEdit != nullptr);
    auto *agentInstructionsEdit = window.findChild<QTextEdit *>("inspectorAgentInstructionsEdit");
    auto *agentModelEdit = window.findChild<QLineEdit *>("inspectorAgentModelNameEdit");
    QVERIFY(jsonTransformEdit != nullptr);
    QVERIFY(toolInputMappingEdit != nullptr);
    QVERIFY(agentInstructionsEdit != nullptr);
    QVERIFY(agentModelEdit != nullptr);
    QVERIFY(chatOutputRoleEdit != nullptr);
    QVERIFY(chatOutputTemplateEdit != nullptr);

    QCOMPARE(promptUserEdit->property("inspectorPlaceholderTextSource").toString(),
             QString("Example: Summarize the following content: {{input}}"));
    QCOMPARE(promptUserEdit->property("inspectorHelpTextSource").toString(),
             QString("Required. Supports workflow variables such as {{input}}."));
    QCOMPARE(llmModelEdit->property("inspectorPlaceholderTextSource").toString(),
             QString("Enter model identifier"));
    QCOMPARE(llmModelEdit->property("inspectorHelpTextSource").toString(),
             QString("Required. Use the model identifier configured by your runtime/backend."));
    QCOMPARE(agentInstructionsEdit->property("inspectorPlaceholderTextSource").toString(),
             QString("Example: Triage the request, decide whether to call tools, then summarize the result."));
    QCOMPARE(agentInstructionsEdit->property("inspectorHelpTextSource").toString(),
             QString("Required. Describes how this agent should reason, when it should use tools, and what it should return."));
    QCOMPARE(agentModelEdit->property("inspectorPlaceholderTextSource").toString(),
             QString("Example: gpt-5.4"));
    QCOMPARE(agentModelEdit->property("inspectorHelpTextSource").toString(),
             QString("Required. Choose the model that powers this agent node."));
    QCOMPARE(chatOutputRoleEdit->property("inspectorPlaceholderTextSource").toString(),
             QString("Example: assistant"));
    QCOMPARE(chatOutputRoleEdit->property("inspectorHelpTextSource").toString(),
             QString("Optional. Labels the chat speaker role for this output message."));
    QCOMPARE(chatOutputTemplateEdit->property("inspectorPlaceholderTextSource").toString(),
             QString("Example: {{agent.final_answer}}"));
    QCOMPARE(chatOutputTemplateEdit->property("inspectorHelpTextSource").toString(),
             QString("Required. Template text for the final chat message emitted by this workflow."));
    QCOMPARE(jsonTransformEdit->property("inspectorPlaceholderTextSource").toString(),
             QString("Example: {\"summary\": \"{{http.response.summary}}\"}"));
    QCOMPARE(jsonTransformEdit->property("inspectorHelpTextSource").toString(),
             QString("Required. Provide a JSON object that reshapes workflow data into a new object."));
    QCOMPARE(toolInputMappingEdit->property("inspectorPlaceholderTextSource").toString(),
             QString("Example: {\"query\": \"{{input}}\"}"));
    QCOMPARE(toolInputMappingEdit->property("inspectorHelpTextSource").toString(),
             QString("Optional. Provide a JSON object that maps workflow values into tool inputs."));
}

void MainWindowTests::exposesStableInspectorFieldSchemaObjectNames()
{
    const auto schemas = builtInInspectorFieldSchemas();
    QVERIFY(!schemas.empty());

    QSet<QString> uniqueFieldKeys;
    for (auto const &schema : schemas) {
        QVERIFY(!schema.typeKey.isEmpty());
        QVERIFY(!schema.propertyKey.isEmpty());
        QVERIFY(!schema.labelObjectName.isEmpty());
        QVERIFY(!schema.widgetObjectName.isEmpty());
        const QString fieldKey = schema.typeKey + QStringLiteral("/") + schema.propertyKey;
        QVERIFY(!uniqueFieldKeys.contains(fieldKey));
        uniqueFieldKeys.insert(fieldKey);
    }

    auto const promptUserSchema = std::find_if(schemas.cbegin(), schemas.cend(), [](InspectorFieldSchema const &schema) {
        return schema.typeKey == QStringLiteral("prompt") && schema.propertyKey == QStringLiteral("userPromptTemplate");
    });
    QVERIFY(promptUserSchema != schemas.cend());
    QCOMPARE(promptUserSchema->labelObjectName, QStringLiteral("inspectorPromptUserTemplateLabel"));
    QCOMPARE(promptUserSchema->widgetObjectName, QStringLiteral("inspectorPromptUserTemplateEdit"));
}

void MainWindowTests::exposesStableInspectorSectionSchemaObjectNames()
{
    const auto schemas = builtInInspectorSectionSchemas();
    QCOMPARE(schemas.size(), 13);

    QSet<QString> uniqueTypeKeys;
    for (auto const &schema : schemas) {
        QVERIFY(!schema.typeKey.isEmpty());
        QVERIFY(!schema.displayName.isEmpty());
        QVERIFY(!schema.sectionTitle.isEmpty());
        QVERIFY(!uniqueTypeKeys.contains(schema.typeKey));
        uniqueTypeKeys.insert(schema.typeKey);
        if (schema.typeKey == QStringLiteral("prompt") || schema.typeKey == QStringLiteral("llm")
            || schema.typeKey == QStringLiteral("agent")
            || schema.typeKey == QStringLiteral("chatOutput")
            || schema.typeKey == QStringLiteral("memory")
            || schema.typeKey == QStringLiteral("retriever")
            || schema.typeKey == QStringLiteral("templateVariables")
            || schema.typeKey == QStringLiteral("httpRequest")
            || schema.typeKey == QStringLiteral("jsonTransform")
            || schema.typeKey == QStringLiteral("tool")) {
            QVERIFY(!schema.sectionObjectName.isEmpty());
        }
    }

    auto const toolSchema = std::find_if(schemas.cbegin(), schemas.cend(), [](InspectorSectionSchema const &schema) {
        return schema.typeKey == QStringLiteral("tool");
    });
    QVERIFY(toolSchema != schemas.cend());
    QCOMPARE(toolSchema->sectionObjectName, QStringLiteral("inspectorToolSection"));
    QCOMPARE(toolSchema->sectionTitle, QStringLiteral("Tool Settings"));
}

void MainWindowTests::exposesStableInspectorSectionSchemaCopy()
{
    const auto schemas = builtInInspectorSectionSchemas();

    auto const promptSchema = std::find_if(schemas.cbegin(), schemas.cend(), [](InspectorSectionSchema const &schema) {
        return schema.typeKey == QStringLiteral("prompt");
    });
    auto const memorySchema = std::find_if(schemas.cbegin(), schemas.cend(), [](InspectorSectionSchema const &schema) {
        return schema.typeKey == QStringLiteral("memory");
    });
    auto const retrieverSchema = std::find_if(schemas.cbegin(), schemas.cend(), [](InspectorSectionSchema const &schema) {
        return schema.typeKey == QStringLiteral("retriever");
    });
    auto const templateVariablesSchema =
        std::find_if(schemas.cbegin(), schemas.cend(), [](InspectorSectionSchema const &schema) {
            return schema.typeKey == QStringLiteral("templateVariables");
        });
    auto const httpRequestSchema = std::find_if(schemas.cbegin(), schemas.cend(), [](InspectorSectionSchema const &schema) {
        return schema.typeKey == QStringLiteral("httpRequest");
    });
    auto const jsonTransformSchema = std::find_if(schemas.cbegin(), schemas.cend(), [](InspectorSectionSchema const &schema) {
        return schema.typeKey == QStringLiteral("jsonTransform");
    });
    auto const chatOutputSchema = std::find_if(schemas.cbegin(), schemas.cend(), [](InspectorSectionSchema const &schema) {
        return schema.typeKey == QStringLiteral("chatOutput");
    });
    auto const outputSchema = std::find_if(schemas.cbegin(), schemas.cend(), [](InspectorSectionSchema const &schema) {
        return schema.typeKey == QStringLiteral("output");
    });

    QVERIFY(promptSchema != schemas.cend());
    QVERIFY(memorySchema != schemas.cend());
    QVERIFY(retrieverSchema != schemas.cend());
    QVERIFY(templateVariablesSchema != schemas.cend());
    QVERIFY(httpRequestSchema != schemas.cend());
    QVERIFY(jsonTransformSchema != schemas.cend());
    QVERIFY(chatOutputSchema != schemas.cend());
    QVERIFY(outputSchema != schemas.cend());

    QCOMPARE(promptSchema->summaryText, QStringLiteral("Edit the prompt template and runtime text for this node."));
    QCOMPARE(promptSchema->displayName, QStringLiteral("Prompt"));
    QVERIFY(!promptSchema->showsEmptyState);
    QCOMPARE(memorySchema->displayName, QStringLiteral("Memory"));
    QCOMPARE(memorySchema->sectionTitle, QStringLiteral("Memory Settings"));
    QVERIFY(!memorySchema->showsEmptyState);
    QCOMPARE(retrieverSchema->displayName, QStringLiteral("Retriever"));
    QCOMPARE(retrieverSchema->sectionTitle, QStringLiteral("Retriever Settings"));
    QVERIFY(!retrieverSchema->showsEmptyState);
    QCOMPARE(templateVariablesSchema->displayName, QStringLiteral("Template Variables"));
    QCOMPARE(templateVariablesSchema->sectionTitle, QStringLiteral("Template Variables Settings"));
    QVERIFY(!templateVariablesSchema->showsEmptyState);
    QCOMPARE(httpRequestSchema->displayName, QStringLiteral("HTTP Request"));
    QCOMPARE(httpRequestSchema->sectionTitle, QStringLiteral("HTTP Request Settings"));
    QVERIFY(!httpRequestSchema->showsEmptyState);
    QCOMPARE(jsonTransformSchema->displayName, QStringLiteral("JSON Transform"));
    QCOMPARE(jsonTransformSchema->sectionTitle, QStringLiteral("JSON Transform Settings"));
    QVERIFY(!jsonTransformSchema->showsEmptyState);
    QCOMPARE(chatOutputSchema->displayName, QStringLiteral("Chat Output"));
    QCOMPARE(chatOutputSchema->sectionTitle, QStringLiteral("Chat Output Settings"));
    QVERIFY(!chatOutputSchema->showsEmptyState);
    QCOMPARE(outputSchema->displayName, QStringLiteral("Output"));
    QCOMPARE(outputSchema->sectionTitle, QStringLiteral("General Settings"));
    QCOMPARE(outputSchema->summaryText, QStringLiteral("This node represents the workflow result."));
    QVERIFY(outputSchema->showsEmptyState);
    QCOMPARE(outputSchema->emptyStateText, QStringLiteral("This node has no advanced settings."));
}

void MainWindowTests::retranslatesInspectorFieldLabelsAtRuntime()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *promptSystemLabel = window.findChild<QLabel *>("inspectorPromptSystemLabel");
    auto *promptUserTemplateLabel = window.findChild<QLabel *>("inspectorPromptUserTemplateLabel");
    auto *llmModelNameLabel = window.findChild<QLabel *>("inspectorLlmModelNameLabel");
    auto *llmTemperatureLabel = window.findChild<QLabel *>("inspectorLlmTemperatureLabel");
    auto *llmMaxTokensLabel = window.findChild<QLabel *>("inspectorLlmMaxTokensLabel");
    auto *jsonTransformLabel = window.findChild<QLabel *>("inspectorJsonTransformLabel");
    auto *toolNameLabel = window.findChild<QLabel *>("inspectorToolNameLabel");
    auto *toolTimeoutLabel = window.findChild<QLabel *>("inspectorToolTimeoutLabel");
    auto *toolInputMappingLabel = window.findChild<QLabel *>("inspectorToolInputMappingLabel");

    QVERIFY(promptSystemLabel != nullptr);
    QVERIFY(promptUserTemplateLabel != nullptr);
    QVERIFY(llmModelNameLabel != nullptr);
    QVERIFY(llmTemperatureLabel != nullptr);
    QVERIFY(llmMaxTokensLabel != nullptr);
    QVERIFY(jsonTransformLabel != nullptr);
    QVERIFY(toolNameLabel != nullptr);
    QVERIFY(toolTimeoutLabel != nullptr);
    QVERIFY(toolInputMappingLabel != nullptr);

    QCOMPARE(promptSystemLabel->text(), QString::fromUtf8("系统提示词"));
    QCOMPARE(promptUserTemplateLabel->text(), QString::fromUtf8("用户提示模板"));
    QCOMPARE(llmModelNameLabel->text(), QString::fromUtf8("模型名称"));
    QCOMPARE(llmTemperatureLabel->text(), QString::fromUtf8("温度"));
    QCOMPARE(llmMaxTokensLabel->text(), QString::fromUtf8("最大令牌数"));
    QCOMPARE(jsonTransformLabel->text(), QString::fromUtf8("转换 JSON"));
    QCOMPARE(toolNameLabel->text(), QString::fromUtf8("工具名称"));
    QCOMPARE(toolTimeoutLabel->text(), QString::fromUtf8("超时（毫秒）"));
    QCOMPARE(toolInputMappingLabel->text(), QString::fromUtf8("输入映射"));

    QVERIFY(languageManager.setLanguage(LanguageManager::Language::English));

    QCOMPARE(promptSystemLabel->text(), QString("System Prompt"));
    QCOMPARE(promptUserTemplateLabel->text(), QString("User Prompt Template"));
    QCOMPARE(llmModelNameLabel->text(), QString("Model Name"));
    QCOMPARE(llmTemperatureLabel->text(), QString("Temperature"));
    QCOMPARE(llmMaxTokensLabel->text(), QString("Max Tokens"));
    QCOMPARE(jsonTransformLabel->text(), QString("Transform JSON"));
    QCOMPARE(toolNameLabel->text(), QString("Tool Name"));
    QCOMPARE(toolTimeoutLabel->text(), QString("Timeout (ms)"));
    QCOMPARE(toolInputMappingLabel->text(), QString("Input Mapping"));
}

void MainWindowTests::retranslatesInspectorFieldHintsAtRuntime()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *promptUserEdit = window.findChild<QTextEdit *>("inspectorPromptUserTemplateEdit");
    auto *llmModelEdit = window.findChild<QLineEdit *>("inspectorLlmModelNameEdit");
    auto *llmTemperatureSpin = window.findChild<QDoubleSpinBox *>("inspectorLlmTemperatureSpin");
    auto *jsonTransformEdit = window.findChild<QTextEdit *>("inspectorJsonTransformEdit");
    auto *toolInputMappingEdit = window.findChild<QTextEdit *>("inspectorToolInputMappingEdit");

    QVERIFY(promptUserEdit != nullptr);
    QVERIFY(llmModelEdit != nullptr);
    QVERIFY(llmTemperatureSpin != nullptr);
    QVERIFY(jsonTransformEdit != nullptr);
    QVERIFY(toolInputMappingEdit != nullptr);

    QCOMPARE(promptUserEdit->placeholderText(), QString::fromUtf8("例如：总结以下内容：{{input}}"));
    QCOMPARE(llmModelEdit->placeholderText(), QString::fromUtf8("输入模型标识"));
    QCOMPARE(llmTemperatureSpin->toolTip(), QString::fromUtf8("较低的温度更稳定，较高的温度更发散。"));
    QCOMPARE(jsonTransformEdit->placeholderText(), QString::fromUtf8("例如：{\"summary\": \"{{http.response.summary}}\"}"));
    QCOMPARE(jsonTransformEdit->toolTip(), QString::fromUtf8("必填。填写 JSON 对象，把工作流数据重组为新的对象。"));
    QCOMPARE(toolInputMappingEdit->placeholderText(), QString::fromUtf8("例如：{\"query\": \"{{input}}\"}"));
    QCOMPARE(toolInputMappingEdit->toolTip(), QString::fromUtf8("可选。填写 JSON 对象，将工作流变量映射到工具输入。"));

    QVERIFY(languageManager.setLanguage(LanguageManager::Language::English));

    QCOMPARE(promptUserEdit->placeholderText(), QString("Example: Summarize the following content: {{input}}"));
    QCOMPARE(llmModelEdit->placeholderText(), QString("Enter model identifier"));
    QCOMPARE(llmTemperatureSpin->toolTip(),
             QString("Lower temperatures are more deterministic; higher temperatures are more creative."));
    QCOMPARE(jsonTransformEdit->placeholderText(), QString("Example: {\"summary\": \"{{http.response.summary}}\"}"));
    QCOMPARE(jsonTransformEdit->toolTip(),
             QString("Required. Provide a JSON object that reshapes workflow data into a new object."));
    QCOMPARE(toolInputMappingEdit->placeholderText(), QString("Example: {\"query\": \"{{input}}\"}"));
    QCOMPARE(toolInputMappingEdit->toolTip(),
             QString("Optional. Provide a JSON object that maps workflow values into tool inputs."));
}

void MainWindowTests::showsTypeSpecificInspectorHeaderAndEmptyState()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *typeBadgeLabel = window.findChild<QLabel *>("inspectorTypeBadgeLabel");
    auto *typeSummaryLabel = window.findChild<QLabel *>("inspectorTypeSummaryLabel");
    auto *sectionTitleLabel = window.findChild<QLabel *>("inspectorSectionTitleLabel");
    auto *emptyStateLabel = window.findChild<QLabel *>("inspectorEmptyStateLabel");

    QVERIFY(editor != nullptr);
    QVERIFY(typeBadgeLabel != nullptr);
    QVERIFY(typeSummaryLabel != nullptr);
    QVERIFY(sectionTitleLabel != nullptr);
    QVERIFY(emptyStateLabel != nullptr);

    const auto promptNode = editor->createNode("prompt");
    editor->selectNode(promptNode);

    QCOMPARE(typeBadgeLabel->text(), QString::fromUtf8("提示词"));
    QCOMPARE(sectionTitleLabel->text(), QString::fromUtf8("提示词配置"));
    QVERIFY(typeSummaryLabel->text().contains(QString::fromUtf8("提示词")));
    QVERIFY(!emptyStateLabel->isVisibleTo(&window));

    const auto llmNode = editor->createNode("llm");
    editor->selectNode(llmNode);

    QCOMPARE(typeBadgeLabel->text(), QString::fromUtf8("大模型"));
    QCOMPARE(sectionTitleLabel->text(), QString::fromUtf8("模型配置"));
    QVERIFY(typeSummaryLabel->text().contains(QString::fromUtf8("模型")));
    QVERIFY(!emptyStateLabel->isVisibleTo(&window));

    const auto startNode = editor->createNode("start");
    editor->selectNode(startNode);

    QCOMPARE(typeBadgeLabel->text(), QString::fromUtf8("开始"));
    QCOMPARE(sectionTitleLabel->text(), QString::fromUtf8("通用配置"));
    QVERIFY(typeSummaryLabel->text().contains(QString::fromUtf8("入口")));
    QVERIFY(emptyStateLabel->isVisibleTo(&window));
    QCOMPARE(emptyStateLabel->text(), QString::fromUtf8("当前节点没有高级配置项。"));
}

void MainWindowTests::appliesDistinctNodeCardStylesByType()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    QVERIFY(editor != nullptr);

    const auto startNode = editor->createNode("start");
    const auto promptNode = editor->createNode("prompt");
    const auto llmNode = editor->createNode("llm");
    const auto toolNode = editor->createNode("tool");

    QVERIFY(startNode != QtNodes::InvalidNodeId);
    QVERIFY(promptNode != QtNodes::InvalidNodeId);
    QVERIFY(llmNode != QtNodes::InvalidNodeId);
    QVERIFY(toolNode != QtNodes::InvalidNodeId);

    const QVariantMap startStyle = editor->nodeStyle(startNode);
    const QVariantMap promptStyle = editor->nodeStyle(promptNode);
    const QVariantMap llmStyle = editor->nodeStyle(llmNode);
    const QVariantMap toolStyle = editor->nodeStyle(toolNode);

    QVERIFY(!startStyle.isEmpty());
    QVERIFY(!promptStyle.isEmpty());
    QVERIFY(!llmStyle.isEmpty());
    QVERIFY(!toolStyle.isEmpty());

    QVERIFY(startStyle != promptStyle);
    QVERIFY(promptStyle != llmStyle);
    QVERIFY(llmStyle != toolStyle);
}

void MainWindowTests::exposesAllCurrentValidationIssues()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    QVERIFY(editor != nullptr);

    const auto promptNode = editor->createNode("prompt");
    const auto outputNode = editor->createNode("output");
    QVERIFY(promptNode != QtNodes::InvalidNodeId);
    QVERIFY(outputNode != QtNodes::InvalidNodeId);

    const auto issues = editor->validationIssues();
    QCOMPARE(issues.size(), 2);

    auto promptIssueIt = std::find_if(issues.cbegin(), issues.cend(), [promptNode](auto const &issue) {
        return issue.nodeId == promptNode;
    });
    auto outputIssueIt = std::find_if(issues.cbegin(), issues.cend(), [outputNode](auto const &issue) {
        return issue.nodeId == outputNode;
    });
    QVERIFY(promptIssueIt != issues.cend());
    QVERIFY(outputIssueIt != issues.cend());

    QCOMPARE(promptIssueIt->typeKey, QString("prompt"));
    QCOMPARE(promptIssueIt->state, QString("warning"));
    QCOMPARE(promptIssueIt->propertyKey, QString("userPromptTemplate"));
    QCOMPARE(promptIssueIt->message, QString::fromUtf8("提示词模板为空。"));
    QCOMPARE(outputIssueIt->typeKey, QString("output"));
    QCOMPARE(outputIssueIt->state, QString("warning"));
    QCOMPARE(outputIssueIt->message, QString::fromUtf8("输出节点需要输入连接。"));
}

void MainWindowTests::marksIncompletePromptNodeWithWarningValidationState()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    QVERIFY(editor != nullptr);

    const auto promptNode = editor->createNode("prompt");
    QCOMPARE(editor->nodeValidationState(promptNode), QString("warning"));

    editor->selectNode(promptNode);
    auto *promptUserEdit = window.findChild<QTextEdit *>("inspectorPromptUserTemplateEdit");
    QVERIFY(promptUserEdit != nullptr);
    promptUserEdit->setPlainText("Summarize {{input}}");

    QCOMPARE(editor->nodeValidationState(promptNode), QString("valid"));
}

void MainWindowTests::marksIncompleteMemoryNodeWithWarningValidationState()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *validationLabel = window.findChild<QLabel *>("inspectorValidationLabel");
    auto *summaryLabel = window.findChild<QLabel *>("selectionValidationSummaryLabel");
    auto *memoryKeyLabel = window.findChild<QLabel *>("inspectorMemoryKeyLabel");
    auto *memoryKeyEdit = window.findChild<QLineEdit *>("inspectorMemoryKeyEdit");
    QVERIFY(editor != nullptr);
    QVERIFY(validationLabel != nullptr);
    QVERIFY(summaryLabel != nullptr);
    QVERIFY(memoryKeyLabel != nullptr);
    QVERIFY(memoryKeyEdit != nullptr);

    const auto memoryNode = editor->createNode("memory");
    editor->selectNode(memoryNode);

    QCOMPARE(validationLabel->text(), QString::fromUtf8("记忆键不能为空。"));
    QVERIFY(summaryLabel->text().contains(QString::fromUtf8("记忆键不能为空")));
    QCOMPARE(memoryKeyLabel->property("validationState").toString(), QString("warning"));
    QCOMPARE(memoryKeyEdit->property("validationState").toString(), QString("warning"));

    memoryKeyEdit->setText("conversation_summary");

    QVERIFY(validationLabel->isHidden());
    QCOMPARE(memoryKeyLabel->property("validationState").toString(), QString());
    QCOMPARE(memoryKeyEdit->property("validationState").toString(), QString());
}

void MainWindowTests::marksIncompleteRetrieverNodeWithWarningValidationState()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *validationLabel = window.findChild<QLabel *>("inspectorValidationLabel");
    auto *summaryLabel = window.findChild<QLabel *>("selectionValidationSummaryLabel");
    auto *retrieverKeyLabel = window.findChild<QLabel *>("inspectorRetrieverKeyLabel");
    auto *retrieverKeyEdit = window.findChild<QLineEdit *>("inspectorRetrieverKeyEdit");
    QVERIFY(editor != nullptr);
    QVERIFY(validationLabel != nullptr);
    QVERIFY(summaryLabel != nullptr);
    QVERIFY(retrieverKeyLabel != nullptr);
    QVERIFY(retrieverKeyEdit != nullptr);

    const auto retrieverNode = editor->createNode("retriever");
    editor->selectNode(retrieverNode);

    QCOMPARE(validationLabel->text(), QString::fromUtf8("检索器键不能为空。"));
    QVERIFY(summaryLabel->text().contains(QString::fromUtf8("检索器键不能为空")));
    QCOMPARE(retrieverKeyLabel->property("validationState").toString(), QString("warning"));
    QCOMPARE(retrieverKeyEdit->property("validationState").toString(), QString("warning"));

    retrieverKeyEdit->setText("knowledge_base_search");

    QVERIFY(validationLabel->isHidden());
    QCOMPARE(retrieverKeyLabel->property("validationState").toString(), QString());
    QCOMPARE(retrieverKeyEdit->property("validationState").toString(), QString());
}

void MainWindowTests::marksIncompleteAgentNodeWithWarningValidationState()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *validationLabel = window.findChild<QLabel *>("inspectorValidationLabel");
    auto *summaryLabel = window.findChild<QLabel *>("selectionValidationSummaryLabel");
    auto *instructionsLabel = window.findChild<QLabel *>("inspectorAgentInstructionsLabel");
    auto *instructionsEdit = window.findChild<QTextEdit *>("inspectorAgentInstructionsEdit");
    auto *modelNameLabel = window.findChild<QLabel *>("inspectorAgentModelNameLabel");
    auto *modelNameEdit = window.findChild<QLineEdit *>("inspectorAgentModelNameEdit");
    QVERIFY(editor != nullptr);
    QVERIFY(validationLabel != nullptr);
    QVERIFY(summaryLabel != nullptr);
    QVERIFY(instructionsLabel != nullptr);
    QVERIFY(instructionsEdit != nullptr);
    QVERIFY(modelNameLabel != nullptr);
    QVERIFY(modelNameEdit != nullptr);

    const auto agentNode = editor->createNode("agent");
    editor->selectNode(agentNode);

    QCOMPARE(validationLabel->text(), QString::fromUtf8("Agent 指令不能为空。"));
    QVERIFY(summaryLabel->text().contains(QString::fromUtf8("Agent 指令不能为空")));
    QCOMPARE(instructionsLabel->property("validationState").toString(), QString("warning"));
    QCOMPARE(instructionsEdit->property("validationState").toString(), QString("warning"));

    instructionsEdit->setPlainText("Plan and answer using available tools when needed.");
    QCOMPARE(validationLabel->text(), QString::fromUtf8("Agent 模型名称不能为空。"));
    QVERIFY(summaryLabel->text().contains(QString::fromUtf8("Agent 模型名称不能为空")));
    QCOMPARE(modelNameLabel->property("validationState").toString(), QString("warning"));
    QCOMPARE(modelNameEdit->property("validationState").toString(), QString("warning"));

    modelNameEdit->setText("gpt-5.4");

    QVERIFY(validationLabel->isHidden());
    QCOMPARE(instructionsLabel->property("validationState").toString(), QString());
    QCOMPARE(instructionsEdit->property("validationState").toString(), QString());
    QCOMPARE(modelNameLabel->property("validationState").toString(), QString());
    QCOMPARE(modelNameEdit->property("validationState").toString(), QString());
    QCOMPARE(editor->nodeValidationState(agentNode), QString("valid"));
}

void MainWindowTests::marksIncompleteChatOutputNodeWithWarningValidationState()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *validationLabel = window.findChild<QLabel *>("inspectorValidationLabel");
    auto *summaryLabel = window.findChild<QLabel *>("selectionValidationSummaryLabel");
    auto *templateLabel = window.findChild<QLabel *>("inspectorChatOutputTemplateLabel");
    auto *templateEdit = window.findChild<QTextEdit *>("inspectorChatOutputTemplateEdit");
    QVERIFY(editor != nullptr);
    QVERIFY(validationLabel != nullptr);
    QVERIFY(summaryLabel != nullptr);
    QVERIFY(templateLabel != nullptr);
    QVERIFY(templateEdit != nullptr);

    const auto startNode = editor->createNode("start");
    const auto chatOutputNode = editor->createNode("chatOutput");
    editor->selectNode(chatOutputNode);

    QCOMPARE(validationLabel->text(), QString::fromUtf8("聊天输出模板不能为空。"));
    QVERIFY(summaryLabel->text().contains(QString::fromUtf8("聊天输出模板不能为空")));
    QCOMPARE(templateLabel->property("validationState").toString(), QString("warning"));
    QCOMPARE(templateEdit->property("validationState").toString(), QString("warning"));

    templateEdit->setPlainText("{{agent.final_answer}}");

    QCOMPARE(validationLabel->text(), QString::fromUtf8("聊天输出节点需要输入连接。"));
    QCOMPARE(templateLabel->property("validationState").toString(), QString());
    QCOMPARE(templateEdit->property("validationState").toString(), QString());

    QVERIFY(editor->connectNodes(startNode, 0, chatOutputNode, 0));

    QVERIFY(validationLabel->isHidden());
    QCOMPARE(editor->nodeValidationState(chatOutputNode), QString("valid"));
}

void MainWindowTests::marksInvalidTemplateVariablesNodeWithValidationState()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *validationLabel = window.findChild<QLabel *>("inspectorValidationLabel");
    auto *summaryLabel = window.findChild<QLabel *>("selectionValidationSummaryLabel");
    auto *variablesLabel = window.findChild<QLabel *>("inspectorTemplateVariablesLabel");
    auto *variablesEdit = window.findChild<QTextEdit *>("inspectorTemplateVariablesEdit");
    QVERIFY(editor != nullptr);
    QVERIFY(validationLabel != nullptr);
    QVERIFY(summaryLabel != nullptr);
    QVERIFY(variablesLabel != nullptr);
    QVERIFY(variablesEdit != nullptr);

    const auto templateVariablesNode = editor->createNode("templateVariables");
    editor->selectNode(templateVariablesNode);

    QCOMPARE(validationLabel->text(), QString::fromUtf8("模板变量不能为空。"));
    QVERIFY(summaryLabel->text().contains(QString::fromUtf8("模板变量不能为空")));
    QCOMPARE(variablesLabel->property("validationState").toString(), QString("warning"));
    QCOMPARE(variablesEdit->property("validationState").toString(), QString("warning"));

    variablesEdit->setPlainText("{bad json}");

    QCOMPARE(validationLabel->text(), QString::fromUtf8("模板变量必须是合法的 JSON 对象。"));
    QVERIFY(summaryLabel->text().contains(QString::fromUtf8("模板变量必须是合法的 JSON 对象")));
    QCOMPARE(variablesLabel->property("validationState").toString(), QString("error"));
    QCOMPARE(variablesEdit->property("validationState").toString(), QString("error"));

    variablesEdit->setPlainText("{\"topic\": \"{{input}}\"}");

    QVERIFY(validationLabel->isHidden());
    QCOMPARE(variablesLabel->property("validationState").toString(), QString());
    QCOMPARE(variablesEdit->property("validationState").toString(), QString());
}

void MainWindowTests::marksInvalidHttpRequestNodeWithValidationState()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *validationLabel = window.findChild<QLabel *>("inspectorValidationLabel");
    auto *summaryLabel = window.findChild<QLabel *>("selectionValidationSummaryLabel");
    auto *urlLabel = window.findChild<QLabel *>("inspectorHttpRequestUrlLabel");
    auto *urlEdit = window.findChild<QLineEdit *>("inspectorHttpRequestUrlEdit");
    auto *headersLabel = window.findChild<QLabel *>("inspectorHttpRequestHeadersLabel");
    auto *headersEdit = window.findChild<QTextEdit *>("inspectorHttpRequestHeadersEdit");
    QVERIFY(editor != nullptr);
    QVERIFY(validationLabel != nullptr);
    QVERIFY(summaryLabel != nullptr);
    QVERIFY(urlLabel != nullptr);
    QVERIFY(urlEdit != nullptr);
    QVERIFY(headersLabel != nullptr);
    QVERIFY(headersEdit != nullptr);

    const auto httpRequestNode = editor->createNode("httpRequest");
    editor->selectNode(httpRequestNode);

    QCOMPARE(validationLabel->text(), QString::fromUtf8("请求 URL 不能为空。"));
    QVERIFY(summaryLabel->text().contains(QString::fromUtf8("请求 URL 不能为空")));
    QCOMPARE(urlLabel->property("validationState").toString(), QString("warning"));
    QCOMPARE(urlEdit->property("validationState").toString(), QString("warning"));

    urlEdit->setText("https://api.example.com/search");
    headersEdit->setPlainText("{bad json}");

    QCOMPARE(validationLabel->text(), QString::fromUtf8("请求头必须是合法的 JSON 对象。"));
    QVERIFY(summaryLabel->text().contains(QString::fromUtf8("请求头必须是合法的 JSON 对象")));
    QCOMPARE(headersLabel->property("validationState").toString(), QString("error"));
    QCOMPARE(headersEdit->property("validationState").toString(), QString("error"));

    headersEdit->setPlainText("{\"Authorization\": \"Bearer {{token}}\"}");

    QVERIFY(validationLabel->isHidden());
    QCOMPARE(headersLabel->property("validationState").toString(), QString());
    QCOMPARE(headersEdit->property("validationState").toString(), QString());
}

void MainWindowTests::marksInvalidJsonTransformNodeWithValidationState()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *validationLabel = window.findChild<QLabel *>("inspectorValidationLabel");
    auto *summaryLabel = window.findChild<QLabel *>("selectionValidationSummaryLabel");
    auto *transformLabel = window.findChild<QLabel *>("inspectorJsonTransformLabel");
    auto *transformEdit = window.findChild<QTextEdit *>("inspectorJsonTransformEdit");
    QVERIFY(editor != nullptr);
    QVERIFY(validationLabel != nullptr);
    QVERIFY(summaryLabel != nullptr);
    QVERIFY(transformLabel != nullptr);
    QVERIFY(transformEdit != nullptr);

    const auto jsonTransformNode = editor->createNode("jsonTransform");
    editor->selectNode(jsonTransformNode);

    QCOMPARE(validationLabel->text(), QString::fromUtf8("转换 JSON 不能为空。"));
    QVERIFY(summaryLabel->text().contains(QString::fromUtf8("转换 JSON 不能为空")));
    QCOMPARE(transformLabel->property("validationState").toString(), QString("warning"));
    QCOMPARE(transformEdit->property("validationState").toString(), QString("warning"));

    transformEdit->setPlainText("{bad json}");

    QCOMPARE(validationLabel->text(), QString::fromUtf8("转换 JSON 必须是合法的 JSON 对象。"));
    QVERIFY(summaryLabel->text().contains(QString::fromUtf8("转换 JSON 必须是合法的 JSON 对象")));
    QCOMPARE(transformLabel->property("validationState").toString(), QString("error"));
    QCOMPARE(transformEdit->property("validationState").toString(), QString("error"));

    transformEdit->setPlainText("{\"summary\": \"{{http.response.summary}}\"}");

    QVERIFY(validationLabel->isHidden());
    QCOMPARE(transformLabel->property("validationState").toString(), QString());
    QCOMPARE(transformEdit->property("validationState").toString(), QString());
}

void MainWindowTests::marksInvalidToolNodeWithErrorValidationState()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    QVERIFY(editor != nullptr);

    const auto toolNode = editor->createNode("tool");
    editor->selectNode(toolNode);

    auto *toolNameEdit = window.findChild<QLineEdit *>("inspectorToolNameEdit");
    auto *toolInputMappingEdit = window.findChild<QTextEdit *>("inspectorToolInputMappingEdit");
    QVERIFY(toolNameEdit != nullptr);
    QVERIFY(toolInputMappingEdit != nullptr);

    toolNameEdit->setText("web_search");
    toolInputMappingEdit->setPlainText("{bad json}");

    QCOMPARE(editor->nodeValidationState(toolNode), QString("error"));
}

void MainWindowTests::marksFlowNodesWithStructuralWarningsBasedOnConnections()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    QVERIFY(editor != nullptr);

    const auto startNode = editor->createNode("start");
    const auto conditionNode = editor->createNode("condition");
    const auto trueOutputNode = editor->createNode("output");
    const auto falseOutputNode = editor->createNode("output");

    QCOMPARE(editor->nodeValidationState(startNode), QString("warning"));
    QCOMPARE(editor->nodeValidationMessage(startNode), QString::fromUtf8("开始节点需要连接到下一步。"));
    QCOMPARE(editor->nodeValidationState(conditionNode), QString("warning"));
    QCOMPARE(editor->nodeValidationMessage(conditionNode), QString::fromUtf8("条件节点需要输入连接。"));
    QCOMPARE(editor->nodeValidationState(trueOutputNode), QString("warning"));
    QCOMPARE(editor->nodeValidationMessage(trueOutputNode), QString::fromUtf8("输出节点需要输入连接。"));

    QVERIFY(editor->connectNodes(startNode, 0, conditionNode, 0));
    QCOMPARE(editor->nodeValidationState(startNode), QString("valid"));
    QCOMPARE(editor->nodeValidationState(conditionNode), QString("warning"));
    QCOMPARE(editor->nodeValidationMessage(conditionNode), QString::fromUtf8("条件节点需要同时连接 True 和 False 分支。"));

    QVERIFY(editor->connectNodes(conditionNode, 0, trueOutputNode, 0));
    QCOMPARE(editor->nodeValidationState(conditionNode), QString("warning"));
    QCOMPARE(editor->nodeValidationMessage(conditionNode), QString::fromUtf8("条件节点需要同时连接 True 和 False 分支。"));
    QCOMPARE(editor->nodeValidationState(trueOutputNode), QString("valid"));

    QVERIFY(editor->connectNodes(conditionNode, 1, falseOutputNode, 0));
    QCOMPARE(editor->nodeValidationState(conditionNode), QString("valid"));
    QCOMPARE(editor->nodeValidationState(falseOutputNode), QString("valid"));
}

void MainWindowTests::updatesFlowValidationSurfacesWhenConnectionFixesNode()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);
    window.resize(1280, 840);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *problemsTable = window.findChild<QTableWidget *>("problemsTable");
    QVERIFY(editor != nullptr);
    QVERIFY(problemsTable != nullptr);

    const auto startNode = editor->createNode("start", QPointF(120.0, 120.0));
    const auto conditionNode = editor->createNode("condition", QPointF(420.0, 120.0));
    editor->selectNode(startNode);
    editor->centerSelection();
    QCoreApplication::processEvents();

    QCOMPARE(editor->nodeValidationState(startNode), QString("warning"));
    const int problemCountBefore = problemsTable->rowCount();
    QVERIFY(problemCountBefore >= 2);

    QVERIFY(editor->connectNodes(startNode, 0, conditionNode, 0));
    QCoreApplication::processEvents();

    QCOMPARE(editor->connectionCount(), 1);
    QCOMPARE(editor->nodeValidationState(startNode), QString("valid"));
    QVERIFY(problemsTable->rowCount() < problemCountBefore);
}

void MainWindowTests::showsValidationMessageInInspectorForSelectedNode()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *validationLabel = window.findChild<QLabel *>("inspectorValidationLabel");
    auto *promptUserEdit = window.findChild<QTextEdit *>("inspectorPromptUserTemplateEdit");
    QVERIFY(editor != nullptr);
    QVERIFY(validationLabel != nullptr);
    QVERIFY(promptUserEdit != nullptr);

    const auto promptNode = editor->createNode("prompt");
    editor->selectNode(promptNode);

    QVERIFY(!validationLabel->isHidden());
    QCOMPARE(validationLabel->text(), QString::fromUtf8("提示词模板为空。"));

    promptUserEdit->setPlainText("Summarize {{input}}");

    QVERIFY(validationLabel->isHidden());
    QVERIFY(validationLabel->text().isEmpty());
}

void MainWindowTests::highlightsInspectorFieldForRequiredPropertyValidation()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *modelNameLabel = window.findChild<QLabel *>("inspectorLlmModelNameLabel");
    auto *modelNameEdit = window.findChild<QLineEdit *>("inspectorLlmModelNameEdit");
    QVERIFY(editor != nullptr);
    QVERIFY(modelNameLabel != nullptr);
    QVERIFY(modelNameEdit != nullptr);

    const auto llmNode = editor->createNode("llm");
    editor->selectNode(llmNode);
    modelNameEdit->clear();

    QCOMPARE(modelNameLabel->property("validationState").toString(), QString("warning"));
    QCOMPARE(modelNameEdit->property("validationState").toString(), QString("warning"));

    modelNameEdit->setText("demo-model");

    QCOMPARE(modelNameLabel->property("validationState").toString(), QString());
    QCOMPARE(modelNameEdit->property("validationState").toString(), QString());
}

void MainWindowTests::highlightsInspectorFieldForInvalidToolJsonMapping()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *toolNameEdit = window.findChild<QLineEdit *>("inspectorToolNameEdit");
    auto *inputMappingLabel = window.findChild<QLabel *>("inspectorToolInputMappingLabel");
    auto *inputMappingEdit = window.findChild<QTextEdit *>("inspectorToolInputMappingEdit");
    QVERIFY(editor != nullptr);
    QVERIFY(toolNameEdit != nullptr);
    QVERIFY(inputMappingLabel != nullptr);
    QVERIFY(inputMappingEdit != nullptr);

    const auto toolNode = editor->createNode("tool");
    editor->selectNode(toolNode);
    toolNameEdit->setText("web_search");
    inputMappingEdit->setPlainText("{bad json}");

    QCOMPARE(inputMappingLabel->property("validationState").toString(), QString("error"));
    QCOMPARE(inputMappingEdit->property("validationState").toString(), QString("error"));

    inputMappingEdit->setPlainText("{\"query\":\"{{input}}\"}");

    QCOMPARE(inputMappingLabel->property("validationState").toString(), QString());
    QCOMPARE(inputMappingEdit->property("validationState").toString(), QString());
}

void MainWindowTests::keepsLlmAndToolValidationMessagesConsistentAcrossSurfaces()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *validationLabel = window.findChild<QLabel *>("inspectorValidationLabel");
    auto *summaryLabel = window.findChild<QLabel *>("selectionValidationSummaryLabel");
    auto *llmModelEdit = window.findChild<QLineEdit *>("inspectorLlmModelNameEdit");
    auto *toolNameEdit = window.findChild<QLineEdit *>("inspectorToolNameEdit");
    auto *toolInputMappingEdit = window.findChild<QTextEdit *>("inspectorToolInputMappingEdit");
    QVERIFY(editor != nullptr);
    QVERIFY(validationLabel != nullptr);
    QVERIFY(summaryLabel != nullptr);
    QVERIFY(llmModelEdit != nullptr);
    QVERIFY(toolNameEdit != nullptr);
    QVERIFY(toolInputMappingEdit != nullptr);

    const auto llmNode = editor->createNode("llm");
    editor->selectNode(llmNode);
    llmModelEdit->clear();

    QCOMPARE(validationLabel->text(), QString::fromUtf8("模型名称不能为空。"));
    QVERIFY(summaryLabel->text().contains(QString::fromUtf8("模型名称不能为空")));

    llmModelEdit->setText("demo-model");
    QVERIFY(validationLabel->isHidden());
    QVERIFY(summaryLabel->text().contains(QString::fromUtf8("已通过检查")));

    const auto toolNode = editor->createNode("tool");
    editor->selectNode(toolNode);

    QCOMPARE(validationLabel->text(), QString::fromUtf8("工具名称不能为空。"));
    QVERIFY(summaryLabel->text().contains(QString::fromUtf8("工具名称不能为空")));

    toolNameEdit->setText("web_search");
    toolInputMappingEdit->setPlainText("{bad json}");

    QCOMPARE(validationLabel->text(), QString::fromUtf8("输入映射必须是合法的 JSON 对象。"));
    QVERIFY(summaryLabel->text().contains(QString::fromUtf8("输入映射必须是合法的 JSON 对象")));
}

void MainWindowTests::refreshesFlowValidationAfterConnectionDeletionUndo()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *deleteAction = window.findChild<QAction *>("deleteAction");
    auto *undoAction = window.findChild<QAction *>("undoAction");
    auto *validationLabel = window.findChild<QLabel *>("inspectorValidationLabel");
    auto *summaryLabel = window.findChild<QLabel *>("selectionValidationSummaryLabel");
    QVERIFY(editor != nullptr);
    QVERIFY(deleteAction != nullptr);
    QVERIFY(undoAction != nullptr);
    QVERIFY(validationLabel != nullptr);
    QVERIFY(summaryLabel != nullptr);

    const auto startNode = editor->createNode("start");
    const auto outputNode = editor->createNode("output");
    const QtNodes::ConnectionId connectionId{startNode, 0, outputNode, 0};

    QVERIFY(editor->connectNodes(startNode, 0, outputNode, 0));
    editor->selectNode(startNode);
    QVERIFY(validationLabel->isHidden());
    QVERIFY(summaryLabel->text().contains(QString::fromUtf8("已通过检查")));

    QVERIFY(selectConnectionGraphicsObject(editor, connectionId) != nullptr);
    deleteAction->trigger();

    editor->selectNode(startNode);
    QCOMPARE(validationLabel->text(), QString::fromUtf8("开始节点需要连接到下一步。"));
    QVERIFY(summaryLabel->text().contains(QString::fromUtf8("开始节点需要连接到下一步。")));

    undoAction->trigger();
    editor->selectNode(startNode);
    QVERIFY(validationLabel->isHidden());
    QVERIFY(summaryLabel->text().contains(QString::fromUtf8("已通过检查")));
}

void MainWindowTests::showsImmediateStatusFeedbackForInvalidConnectionDrag()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *graphicsView = editor->findChild<QGraphicsView *>();
    QVERIFY(editor != nullptr);
    QVERIFY(graphicsView != nullptr);

    const auto promptNode = editor->createNode("prompt", QPointF(320.0, 120.0));
    const auto startNode = editor->createNode("start", QPointF(80.0, 120.0));
    QVERIFY(promptNode != QtNodes::InvalidNodeId);
    QVERIFY(startNode != QtNodes::InvalidNodeId);

    const QPoint sourcePoint = graphicsView->mapFromScene(editor->portScenePosition(promptNode, QtNodes::PortType::Out, 0));
    const QPoint targetPoint = graphicsView->mapFromScene(editor->portScenePosition(startNode, QtNodes::PortType::Out, 0));

    QMouseEvent pressEvent(QEvent::MouseButtonPress,
                           sourcePoint,
                           graphicsView->viewport()->mapToGlobal(sourcePoint),
                           Qt::LeftButton,
                           Qt::LeftButton,
                           Qt::NoModifier);
    QVERIFY(QCoreApplication::sendEvent(graphicsView->viewport(), &pressEvent));

    QMouseEvent moveEvent(QEvent::MouseMove,
                          targetPoint,
                          graphicsView->viewport()->mapToGlobal(targetPoint),
                          Qt::NoButton,
                          Qt::LeftButton,
                          Qt::NoModifier);
    QVERIFY(QCoreApplication::sendEvent(graphicsView->viewport(), &moveEvent));
    QCoreApplication::processEvents();

    QCOMPARE(window.statusBar()->currentMessage(), QString::fromUtf8("此端口不能接受当前连接。"));

    QMouseEvent releaseEvent(QEvent::MouseButtonRelease,
                             targetPoint,
                             graphicsView->viewport()->mapToGlobal(targetPoint),
                             Qt::LeftButton,
                             Qt::NoButton,
                             Qt::NoModifier);
    QVERIFY(QCoreApplication::sendEvent(graphicsView->viewport(), &releaseEvent));
    QCoreApplication::processEvents();

    QCOMPARE(window.statusBar()->currentMessage(), QString::fromUtf8("就绪"));
}

void MainWindowTests::showsValidationSummaryForCurrentSelectionInStatusBar()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *summaryLabel = window.findChild<QLabel *>("selectionValidationSummaryLabel");
    auto *promptUserEdit = window.findChild<QTextEdit *>("inspectorPromptUserTemplateEdit");
    QVERIFY(editor != nullptr);
    QVERIFY(summaryLabel != nullptr);
    QVERIFY(promptUserEdit != nullptr);

    const auto promptNode = editor->createNode("prompt");
    editor->selectNode(promptNode);

    QVERIFY(summaryLabel->text().contains(QString::fromUtf8("提示词")));
    QVERIFY(summaryLabel->text().contains(QString::fromUtf8("提示词模板为空")));

    promptUserEdit->setPlainText("Summarize {{input}}");

    QVERIFY(summaryLabel->text().contains(QString::fromUtf8("已通过检查")));
}

void MainWindowTests::assignsExpectedKeyboardShortcuts()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *newAction = window.findChild<QAction *>("newAction");
    auto *openAction = window.findChild<QAction *>("openAction");
    auto *saveAction = window.findChild<QAction *>("saveAction");
    auto *saveAsAction = window.findChild<QAction *>("saveAsAction");
    auto *deleteAction = window.findChild<QAction *>("deleteAction");
    auto *undoAction = window.findChild<QAction *>("undoAction");
    auto *redoAction = window.findChild<QAction *>("redoAction");
    auto *centerAction = window.findChild<QAction *>("centerAction");
    auto *fitWorkflowAction = window.findChild<QAction *>("fitWorkflowAction");
    auto *selectAllAction = window.findChild<QAction *>("selectAllAction");
    auto *helpAction = window.findChild<QAction *>("helpAction");

    QVERIFY(newAction != nullptr);
    QVERIFY(openAction != nullptr);
    QVERIFY(saveAction != nullptr);
    QVERIFY(saveAsAction != nullptr);
    QVERIFY(deleteAction != nullptr);
    QVERIFY(undoAction != nullptr);
    QVERIFY(redoAction != nullptr);
    QVERIFY(centerAction != nullptr);
    QVERIFY(fitWorkflowAction != nullptr);
    QVERIFY(selectAllAction != nullptr);
    QVERIFY(helpAction != nullptr);

    QCOMPARE(newAction->shortcut(), QKeySequence::keyBindings(QKeySequence::New).constFirst());
    QCOMPARE(openAction->shortcut(), QKeySequence::keyBindings(QKeySequence::Open).constFirst());
    QCOMPARE(saveAction->shortcut(), QKeySequence::keyBindings(QKeySequence::Save).constFirst());
    QCOMPARE(saveAsAction->shortcut(), QKeySequence::keyBindings(QKeySequence::SaveAs).constFirst());
    QCOMPARE(deleteAction->shortcut(), QKeySequence(Qt::Key_Delete));
    QCOMPARE(undoAction->shortcut(), QKeySequence::keyBindings(QKeySequence::Undo).constFirst());
    QCOMPARE(redoAction->shortcut(), QKeySequence::keyBindings(QKeySequence::Redo).constFirst());
    QCOMPARE(centerAction->shortcut(), QKeySequence(Qt::Key_Space));
    QCOMPARE(fitWorkflowAction->shortcut(), QKeySequence(Qt::CTRL | Qt::Key_0));
    QCOMPARE(selectAllAction->shortcut(), QKeySequence::keyBindings(QKeySequence::SelectAll).constFirst());
    QCOMPARE(helpAction->shortcut(),
             QKeySequence::keyBindings(QKeySequence::HelpContents).value(0, QKeySequence(Qt::Key_F1)));
}

void MainWindowTests::enablesWorkbenchActionsBasedOnEditorState()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *deleteAction = window.findChild<QAction *>("deleteAction");
    auto *undoAction = window.findChild<QAction *>("undoAction");
    auto *redoAction = window.findChild<QAction *>("redoAction");
    auto *centerAction = window.findChild<QAction *>("centerAction");
    auto *fitWorkflowAction = window.findChild<QAction *>("fitWorkflowAction");
    auto *selectAllAction = window.findChild<QAction *>("selectAllAction");
    QVERIFY(editor != nullptr);
    QVERIFY(deleteAction != nullptr);
    QVERIFY(undoAction != nullptr);
    QVERIFY(redoAction != nullptr);
    QVERIFY(centerAction != nullptr);
    QVERIFY(fitWorkflowAction != nullptr);
    QVERIFY(selectAllAction != nullptr);

    QVERIFY(!undoAction->isEnabled());
    QVERIFY(!redoAction->isEnabled());
    QVERIFY(!deleteAction->isEnabled());
    QVERIFY(!centerAction->isEnabled());
    QVERIFY(!fitWorkflowAction->isEnabled());
    QVERIFY(!selectAllAction->isEnabled());

    const auto promptNode = editor->createNode("prompt");
    QVERIFY(promptNode != QtNodes::InvalidNodeId);

    QVERIFY(undoAction->isEnabled());
    QVERIFY(!redoAction->isEnabled());
    QVERIFY(deleteAction->isEnabled());
    QVERIFY(centerAction->isEnabled());
    QVERIFY(fitWorkflowAction->isEnabled());
    QVERIFY(selectAllAction->isEnabled());

    clearCanvasSelection(editor);

    QVERIFY(undoAction->isEnabled());
    QVERIFY(!redoAction->isEnabled());
    QVERIFY(!deleteAction->isEnabled());
    QVERIFY(centerAction->isEnabled());
    QVERIFY(fitWorkflowAction->isEnabled());
    QVERIFY(selectAllAction->isEnabled());

    undoAction->trigger();

    QVERIFY(!undoAction->isEnabled());
    QVERIFY(redoAction->isEnabled());
    QVERIFY(!deleteAction->isEnabled());
    QVERIFY(!centerAction->isEnabled());
    QVERIFY(!fitWorkflowAction->isEnabled());
    QVERIFY(!selectAllAction->isEnabled());
}

void MainWindowTests::enablesDeleteActionForSelectedConnections()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *deleteAction = window.findChild<QAction *>("deleteAction");
    QVERIFY(editor != nullptr);
    QVERIFY(deleteAction != nullptr);

    const auto startNode = editor->createNode("start");
    const auto outputNode = editor->createNode("output");
    const QtNodes::ConnectionId connectionId{startNode, 0, outputNode, 0};
    QVERIFY(editor->connectNodes(startNode, 0, outputNode, 0));

    clearCanvasSelection(editor);
    QVERIFY(!deleteAction->isEnabled());

    QVERIFY(selectConnectionGraphicsObject(editor, connectionId) != nullptr);
    QCoreApplication::processEvents();
    QVERIFY(deleteAction->isEnabled());
}

void MainWindowTests::enforcesConnectionRulesBetweenCompatiblePorts()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);
    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");

    QVERIFY(editor != nullptr);

    const auto startNode = editor->createNode("start");
    const auto promptNode = editor->createNode("prompt");
    const auto outputNode = editor->createNode("output");

    QVERIFY(editor->connectNodes(startNode, 0, promptNode, 0));
    QVERIFY(editor->connectNodes(promptNode, 0, outputNode, 0));
    QVERIFY(!editor->connectNodes(outputNode, 0, startNode, 0));
    QCOMPARE(editor->connectionCount(), 2);
}

void MainWindowTests::savesAndLoadsWorkflowJson()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString workflowPath = QDir(tempDir.path()).filePath("workflow.json");

    LanguageManager languageManager;
    MainWindow window(&languageManager);
    window.show();
    QCoreApplication::processEvents();
    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    QVERIFY(editor != nullptr);

    const auto promptNode = editor->createNode("prompt");
    const auto llmNode = editor->createNode("llm");

    QVERIFY(editor->connectNodes(promptNode, 0, llmNode, 0));
    editor->selectNode(promptNode);

    auto *displayNameEdit = window.findChild<QLineEdit *>("inspectorDisplayNameEdit");
    auto *descriptionEdit = window.findChild<QTextEdit *>("inspectorDescriptionEdit");
    auto *promptSystemEdit = window.findChild<QTextEdit *>("inspectorPromptSystemEdit");
    auto *promptUserEdit = window.findChild<QTextEdit *>("inspectorPromptUserTemplateEdit");
    auto *llmModelEdit = window.findChild<QLineEdit *>("inspectorLlmModelNameEdit");
    auto *llmTemperatureSpin = window.findChild<QDoubleSpinBox *>("inspectorLlmTemperatureSpin");
    auto *llmMaxTokensSpin = window.findChild<QSpinBox *>("inspectorLlmMaxTokensSpin");
    QVERIFY(displayNameEdit != nullptr);
    QVERIFY(descriptionEdit != nullptr);
    QVERIFY(promptSystemEdit != nullptr);
    QVERIFY(promptUserEdit != nullptr);
    QVERIFY(llmModelEdit != nullptr);
    QVERIFY(llmTemperatureSpin != nullptr);
    QVERIFY(llmMaxTokensSpin != nullptr);

    displayNameEdit->setText("Draft Prompt");
    descriptionEdit->setPlainText("Builds the user prompt.");
    promptSystemEdit->setPlainText("You are a careful assistant.");
    promptUserEdit->setPlainText("Write a summary for {{input}}.");

    editor->selectNode(llmNode);
    llmModelEdit->setText("demo-model");
    llmTemperatureSpin->setValue(0.35);
    llmMaxTokensSpin->setValue(4096);

    QVERIFY(window.saveWorkflowToPath(workflowPath));
    QVERIFY(QFileInfo::exists(workflowPath));

    MainWindow restoredWindow(&languageManager);
    QVERIFY(restoredWindow.loadWorkflowFromPath(workflowPath));

    auto *restoredEditor = restoredWindow.findChild<QtNodesEditorWidget *>("workflowCanvas");
    QVERIFY(restoredEditor != nullptr);

    QCOMPARE(restoredEditor->nodeCount(), 2);
    QCOMPARE(restoredEditor->connectionCount(), 1);
    QCOMPARE(restoredEditor->workflowDisplayNames(), QStringList({"Draft Prompt", QString::fromUtf8("大模型")}));
    QCOMPARE(restoredEditor->descriptionForDisplayName("Draft Prompt"), QString("Builds the user prompt."));

    auto const restoredPromptNode = restoredEditor->findNodeIdByDisplayName("Draft Prompt");
    QVERIFY(restoredPromptNode != QtNodes::InvalidNodeId);
    restoredEditor->selectNode(restoredPromptNode);
    QCOMPARE(restoredEditor->selectedNodeProperty("systemPrompt").toString(), QString("You are a careful assistant."));
    QCOMPARE(restoredEditor->selectedNodeProperty("userPromptTemplate").toString(),
             QString("Write a summary for {{input}}."));

    auto const restoredLlmNode = restoredEditor->findNodeIdByDisplayName(QString::fromUtf8("大模型"));
    QVERIFY(restoredLlmNode != QtNodes::InvalidNodeId);
    restoredEditor->selectNode(restoredLlmNode);
    QCOMPARE(restoredEditor->selectedNodeProperty("modelName").toString(), QString("demo-model"));
    QCOMPARE(restoredEditor->selectedNodeProperty("temperature").toDouble(), 0.35);
    QCOMPARE(restoredEditor->selectedNodeProperty("maxTokens").toInt(), 4096);
}

void MainWindowTests::savesAndLoadsNodePositionsAtOrigin()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString workflowPath = QDir(tempDir.path()).filePath("workflow-positions.json");

    LanguageManager languageManager;
    MainWindow window(&languageManager);
    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    QVERIFY(editor != nullptr);

    const auto promptNode = editor->createNode("prompt");
    const auto outputNode = editor->createNode("output");
    QVERIFY(promptNode != QtNodes::InvalidNodeId);
    QVERIFY(outputNode != QtNodes::InvalidNodeId);

    const QPointF promptPosition(0.0, 0.0);
    const QPointF outputPosition(240.0, 160.0);
    editor->setNodePosition(promptNode, promptPosition);
    editor->setNodePosition(outputNode, outputPosition);

    QVERIFY(window.saveWorkflowToPath(workflowPath));

    MainWindow restoredWindow(&languageManager);
    restoredWindow.show();
    QCoreApplication::processEvents();
    QVERIFY(restoredWindow.loadWorkflowFromPath(workflowPath));

    auto *restoredEditor = restoredWindow.findChild<QtNodesEditorWidget *>("workflowCanvas");
    QVERIFY(restoredEditor != nullptr);

    const auto restoredPromptNode = restoredEditor->findNodeIdByDisplayName(QString::fromUtf8("提示词"));
    const auto restoredOutputNode = restoredEditor->findNodeIdByDisplayName(QString::fromUtf8("输出"));
    QVERIFY(restoredPromptNode != QtNodes::InvalidNodeId);
    QVERIFY(restoredOutputNode != QtNodes::InvalidNodeId);

    QCOMPARE(restoredEditor->nodePosition(restoredPromptNode), promptPosition);
    QCOMPARE(restoredEditor->nodePosition(restoredOutputNode), outputPosition);
}

void MainWindowTests::savesAndLoadsMemoryNodeProperties()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString workflowPath = QDir(tempDir.path()).filePath("memory-workflow.json");

    LanguageManager languageManager;
    MainWindow window(&languageManager);
    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *memoryKeyEdit = window.findChild<QLineEdit *>("inspectorMemoryKeyEdit");
    QVERIFY(editor != nullptr);
    QVERIFY(memoryKeyEdit != nullptr);

    const auto memoryNode = editor->createNode("memory");
    editor->selectNode(memoryNode);
    memoryKeyEdit->setText("conversation_summary");

    QVERIFY(window.saveWorkflowToPath(workflowPath));

    MainWindow restoredWindow(&languageManager);
    QVERIFY(restoredWindow.loadWorkflowFromPath(workflowPath));

    auto *restoredEditor = restoredWindow.findChild<QtNodesEditorWidget *>("workflowCanvas");
    QVERIFY(restoredEditor != nullptr);

    const auto restoredMemoryNode = restoredEditor->findNodeIdByDisplayName(QString::fromUtf8("记忆"));
    QVERIFY(restoredMemoryNode != QtNodes::InvalidNodeId);
    restoredEditor->selectNode(restoredMemoryNode);
    QCOMPARE(restoredEditor->selectedNodeProperty("memoryKey").toString(), QString("conversation_summary"));
}

void MainWindowTests::savesAndLoadsRetrieverNodeProperties()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString workflowPath = QDir(tempDir.path()).filePath("retriever-workflow.json");

    LanguageManager languageManager;
    MainWindow window(&languageManager);
    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *retrieverKeyEdit = window.findChild<QLineEdit *>("inspectorRetrieverKeyEdit");
    QVERIFY(editor != nullptr);
    QVERIFY(retrieverKeyEdit != nullptr);

    const auto retrieverNode = editor->createNode("retriever");
    editor->selectNode(retrieverNode);
    retrieverKeyEdit->setText("knowledge_base_search");

    QVERIFY(window.saveWorkflowToPath(workflowPath));

    MainWindow restoredWindow(&languageManager);
    QVERIFY(restoredWindow.loadWorkflowFromPath(workflowPath));

    auto *restoredEditor = restoredWindow.findChild<QtNodesEditorWidget *>("workflowCanvas");
    QVERIFY(restoredEditor != nullptr);

    const auto restoredRetrieverNode = restoredEditor->findNodeIdByDisplayName(QString::fromUtf8("检索器"));
    QVERIFY(restoredRetrieverNode != QtNodes::InvalidNodeId);
    restoredEditor->selectNode(restoredRetrieverNode);
    QCOMPARE(restoredEditor->selectedNodeProperty("retrieverKey").toString(), QString("knowledge_base_search"));
}

void MainWindowTests::savesAndLoadsTemplateVariablesNodeProperties()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString workflowPath = QDir(tempDir.path()).filePath("template-variables-workflow.json");

    LanguageManager languageManager;
    MainWindow window(&languageManager);
    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *variablesEdit = window.findChild<QTextEdit *>("inspectorTemplateVariablesEdit");
    QVERIFY(editor != nullptr);
    QVERIFY(variablesEdit != nullptr);

    const auto templateVariablesNode = editor->createNode("templateVariables");
    editor->selectNode(templateVariablesNode);
    variablesEdit->setPlainText("{\"topic\": \"{{input}}\"}");

    QVERIFY(window.saveWorkflowToPath(workflowPath));

    MainWindow restoredWindow(&languageManager);
    QVERIFY(restoredWindow.loadWorkflowFromPath(workflowPath));

    auto *restoredEditor = restoredWindow.findChild<QtNodesEditorWidget *>("workflowCanvas");
    QVERIFY(restoredEditor != nullptr);

    const auto restoredTemplateVariablesNode =
        restoredEditor->findNodeIdByDisplayName(QString::fromUtf8("模板变量"));
    QVERIFY(restoredTemplateVariablesNode != QtNodes::InvalidNodeId);
    restoredEditor->selectNode(restoredTemplateVariablesNode);
    QCOMPARE(restoredEditor->selectedNodeProperty("variablesJson").toString(), QString("{\"topic\": \"{{input}}\"}"));
}

void MainWindowTests::savesAndLoadsHttpRequestNodeProperties()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString workflowPath = QDir(tempDir.path()).filePath("http-request-workflow.json");

    LanguageManager languageManager;
    MainWindow window(&languageManager);
    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *methodEdit = window.findChild<QLineEdit *>("inspectorHttpRequestMethodEdit");
    auto *urlEdit = window.findChild<QLineEdit *>("inspectorHttpRequestUrlEdit");
    auto *headersEdit = window.findChild<QTextEdit *>("inspectorHttpRequestHeadersEdit");
    auto *bodyEdit = window.findChild<QTextEdit *>("inspectorHttpRequestBodyEdit");
    auto *timeoutSpin = window.findChild<QSpinBox *>("inspectorHttpRequestTimeoutSpin");
    QVERIFY(editor != nullptr);
    QVERIFY(methodEdit != nullptr);
    QVERIFY(urlEdit != nullptr);
    QVERIFY(headersEdit != nullptr);
    QVERIFY(bodyEdit != nullptr);
    QVERIFY(timeoutSpin != nullptr);

    const auto httpRequestNode = editor->createNode("httpRequest");
    editor->selectNode(httpRequestNode);
    methodEdit->setText("POST");
    urlEdit->setText("https://api.example.com/search");
    headersEdit->setPlainText("{\"Authorization\": \"Bearer {{token}}\"}");
    bodyEdit->setPlainText("{\"query\": \"{{input}}\"}");
    timeoutSpin->setValue(45000);

    QVERIFY(window.saveWorkflowToPath(workflowPath));

    MainWindow restoredWindow(&languageManager);
    QVERIFY(restoredWindow.loadWorkflowFromPath(workflowPath));

    auto *restoredEditor = restoredWindow.findChild<QtNodesEditorWidget *>("workflowCanvas");
    QVERIFY(restoredEditor != nullptr);

    const auto restoredHttpRequestNode = restoredEditor->findNodeIdByDisplayName(QString::fromUtf8("HTTP 请求"));
    QVERIFY(restoredHttpRequestNode != QtNodes::InvalidNodeId);
    restoredEditor->selectNode(restoredHttpRequestNode);
    QCOMPARE(restoredEditor->selectedNodeProperty("method").toString(), QString("POST"));
    QCOMPARE(restoredEditor->selectedNodeProperty("url").toString(), QString("https://api.example.com/search"));
    QCOMPARE(restoredEditor->selectedNodeProperty("headersJson").toString(),
             QString("{\"Authorization\": \"Bearer {{token}}\"}"));
    QCOMPARE(restoredEditor->selectedNodeProperty("bodyTemplate").toString(), QString("{\"query\": \"{{input}}\"}"));
    QCOMPARE(restoredEditor->selectedNodeProperty("timeoutMs").toInt(), 45000);
}

void MainWindowTests::savesAndLoadsJsonTransformNodeProperties()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString workflowPath = QDir(tempDir.path()).filePath("json-transform-workflow.json");

    LanguageManager languageManager;
    MainWindow window(&languageManager);
    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *transformEdit = window.findChild<QTextEdit *>("inspectorJsonTransformEdit");
    QVERIFY(editor != nullptr);
    QVERIFY(transformEdit != nullptr);

    const auto jsonTransformNode = editor->createNode("jsonTransform");
    editor->selectNode(jsonTransformNode);
    transformEdit->setPlainText("{\"summary\": \"{{http.response.summary}}\"}");

    QVERIFY(window.saveWorkflowToPath(workflowPath));

    MainWindow restoredWindow(&languageManager);
    QVERIFY(restoredWindow.loadWorkflowFromPath(workflowPath));

    auto *restoredEditor = restoredWindow.findChild<QtNodesEditorWidget *>("workflowCanvas");
    QVERIFY(restoredEditor != nullptr);

    const auto restoredJsonTransformNode = restoredEditor->findNodeIdByDisplayName(QString::fromUtf8("JSON 转换"));
    QVERIFY(restoredJsonTransformNode != QtNodes::InvalidNodeId);
    restoredEditor->selectNode(restoredJsonTransformNode);
    QCOMPARE(restoredEditor->selectedNodeProperty("transformJson").toString(),
             QString("{\"summary\": \"{{http.response.summary}}\"}"));
}

void MainWindowTests::savesAndLoadsAgentNodeProperties()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString workflowPath = QDir(tempDir.path()).filePath("agent-workflow.json");

    LanguageManager languageManager;
    MainWindow window(&languageManager);
    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *instructionsEdit = window.findChild<QTextEdit *>("inspectorAgentInstructionsEdit");
    auto *modelNameEdit = window.findChild<QLineEdit *>("inspectorAgentModelNameEdit");
    auto *maxIterationsSpin = window.findChild<QSpinBox *>("inspectorAgentMaxIterationsSpin");
    QVERIFY(editor != nullptr);
    QVERIFY(instructionsEdit != nullptr);
    QVERIFY(modelNameEdit != nullptr);
    QVERIFY(maxIterationsSpin != nullptr);

    const auto agentNode = editor->createNode("agent");
    editor->selectNode(agentNode);
    instructionsEdit->setPlainText("Triage the task, call tools when needed, then produce a concise answer.");
    modelNameEdit->setText("gpt-5.4");
    maxIterationsSpin->setValue(7);

    QVERIFY(window.saveWorkflowToPath(workflowPath));

    MainWindow restoredWindow(&languageManager);
    QVERIFY(restoredWindow.loadWorkflowFromPath(workflowPath));

    auto *restoredEditor = restoredWindow.findChild<QtNodesEditorWidget *>("workflowCanvas");
    QVERIFY(restoredEditor != nullptr);

    const auto restoredAgentNode = restoredEditor->findNodeIdByDisplayName(QString::fromUtf8("Agent"));
    QVERIFY(restoredAgentNode != QtNodes::InvalidNodeId);
    restoredEditor->selectNode(restoredAgentNode);
    QCOMPARE(restoredEditor->selectedNodeProperty("agentInstructions").toString(),
             QString("Triage the task, call tools when needed, then produce a concise answer."));
    QCOMPARE(restoredEditor->selectedNodeProperty("modelName").toString(), QString("gpt-5.4"));
    QCOMPARE(restoredEditor->selectedNodeProperty("maxIterations").toInt(), 7);
}

void MainWindowTests::savesAndLoadsChatOutputNodeProperties()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString workflowPath = QDir(tempDir.path()).filePath("chat-output-workflow.json");

    LanguageManager languageManager;
    MainWindow window(&languageManager);
    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *roleEdit = window.findChild<QLineEdit *>("inspectorChatOutputRoleEdit");
    auto *templateEdit = window.findChild<QTextEdit *>("inspectorChatOutputTemplateEdit");
    QVERIFY(editor != nullptr);
    QVERIFY(roleEdit != nullptr);
    QVERIFY(templateEdit != nullptr);

    const auto chatOutputNode = editor->createNode("chatOutput");
    editor->selectNode(chatOutputNode);
    roleEdit->setText("assistant");
    templateEdit->setPlainText("{{agent.final_answer}}");

    QVERIFY(window.saveWorkflowToPath(workflowPath));

    MainWindow restoredWindow(&languageManager);
    QVERIFY(restoredWindow.loadWorkflowFromPath(workflowPath));

    auto *restoredEditor = restoredWindow.findChild<QtNodesEditorWidget *>("workflowCanvas");
    QVERIFY(restoredEditor != nullptr);

    const auto restoredChatOutputNode = restoredEditor->findNodeIdByDisplayName(QString::fromUtf8("聊天输出"));
    QVERIFY(restoredChatOutputNode != QtNodes::InvalidNodeId);
    restoredEditor->selectNode(restoredChatOutputNode);
    QCOMPARE(restoredEditor->selectedNodeProperty("messageRole").toString(), QString("assistant"));
    QCOMPARE(restoredEditor->selectedNodeProperty("messageTemplate").toString(), QString("{{agent.final_answer}}"));
}

void MainWindowTests::preservesMinimumNodeCardSizeAcrossSaveAndLoad()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString workflowPath = QDir(tempDir.path()).filePath("workflow-card-size.json");

    LanguageManager languageManager;
    MainWindow window(&languageManager);
    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *displayNameEdit = window.findChild<QLineEdit *>("inspectorDisplayNameEdit");
    QVERIFY(editor != nullptr);
    QVERIFY(displayNameEdit != nullptr);

    const auto promptNode = editor->createNode("prompt");
    QVERIFY(promptNode != QtNodes::InvalidNodeId);
    editor->selectNode(promptNode);
    displayNameEdit->setText(QString::fromUtf8("一个更长的提示词节点标题"));

    const QSize originalSize = editor->nodeSize(promptNode);
    QVERIFY(originalSize.width() >= 176);
    QVERIFY(originalSize.height() >= 96);

    QVERIFY(window.saveWorkflowToPath(workflowPath));

    MainWindow restoredWindow(&languageManager);
    QVERIFY(restoredWindow.loadWorkflowFromPath(workflowPath));

    auto *restoredEditor = restoredWindow.findChild<QtNodesEditorWidget *>("workflowCanvas");
    QVERIFY(restoredEditor != nullptr);

    const auto restoredPromptNode =
        restoredEditor->findNodeIdByDisplayName(QString::fromUtf8("一个更长的提示词节点标题"));
    QVERIFY(restoredPromptNode != QtNodes::InvalidNodeId);

    const QSize restoredSize = restoredEditor->nodeSize(restoredPromptNode);
    QCOMPARE(restoredSize, originalSize);
    QVERIFY(restoredSize.width() >= 176);
    QVERIFY(restoredSize.height() >= 96);
}

void MainWindowTests::tracksDirtyStateOnEdits()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    QVERIFY(!window.isDirty());

    window.addNodeFromType("prompt");
    QVERIFY(window.isDirty());
}

void MainWindowTests::clearsDirtyStateOnSave()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    LanguageManager languageManager;
    MainWindow window(&languageManager);

    window.addNodeFromType("prompt");
    QVERIFY(window.isDirty());

    QVERIFY(window.saveWorkflowToPath(QDir(tempDir.path()).filePath("test.json")));
    QVERIFY(!window.isDirty());
}

void MainWindowTests::clearsDirtyStateOnLoad()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString workflowPath = QDir(tempDir.path()).filePath("workflow.json");

    LanguageManager languageManager;
    MainWindow window(&languageManager);

    window.addNodeFromType("prompt");
    QVERIFY(window.saveWorkflowToPath(workflowPath));

    window.addNodeFromType("llm");
    QVERIFY(window.isDirty());

    QVERIFY(window.loadWorkflowFromPath(workflowPath));
    QVERIFY(!window.isDirty());
}

void MainWindowTests::doesNotPopulateUndoHistoryWhenLoadingWorkflow()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString workflowPath = QDir(tempDir.path()).filePath("workflow.json");

    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *undoAction = window.findChild<QAction *>("undoAction");
    QVERIFY(editor != nullptr);
    QVERIFY(undoAction != nullptr);

    const auto startNode = editor->createNode("start");
    const auto outputNode = editor->createNode("output");
    QVERIFY(editor->connectNodes(startNode, 0, outputNode, 0));
    QVERIFY(window.saveWorkflowToPath(workflowPath));

    MainWindow restoredWindow(&languageManager);
    QVERIFY(restoredWindow.loadWorkflowFromPath(workflowPath));

    auto *restoredEditor = restoredWindow.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *restoredUndoAction = restoredWindow.findChild<QAction *>("undoAction");
    QVERIFY(restoredEditor != nullptr);
    QVERIFY(restoredUndoAction != nullptr);

    QCOMPARE(restoredEditor->nodeCount(), 2);
    QCOMPARE(restoredEditor->connectionCount(), 1);
    QVERIFY(!restoredEditor->canUndo());

    restoredUndoAction->trigger();

    QCOMPARE(restoredEditor->nodeCount(), 2);
    QCOMPARE(restoredEditor->connectionCount(), 1);
}

void MainWindowTests::preservesCurrentWorkflowWhenLoadFails()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString invalidWorkflowPath = QDir(tempDir.path()).filePath("invalid-workflow.json");
    QFile invalidWorkflowFile(invalidWorkflowPath);
    QVERIFY(invalidWorkflowFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    invalidWorkflowFile.write(R"({
  "version": 1,
  "nodes": [
    {
      "id": 1,
      "type": "prompt",
      "displayName": "Broken Prompt",
      "description": "",
      "properties": {
        "userPromptTemplate": "Hello"
      },
      "position": {
        "x": 0,
        "y": 0
      }
    }
  ],
  "connections": [
    {
      "outNodeId": 1,
      "outPortIndex": 0,
      "inNodeId": 999,
      "inPortIndex": 0
    }
  ]
})");
    invalidWorkflowFile.close();

    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    QVERIFY(editor != nullptr);

    const auto promptNode = editor->createNode("prompt");
    editor->selectNode(promptNode);
    editor->setSelectedNodeDisplayName("Draft Prompt");
    editor->setSelectedNodeProperty("userPromptTemplate", "Summarize {{input}}");

    QVERIFY(window.isDirty());
    QCOMPARE(editor->nodeCount(), 1);
    QCOMPARE(editor->workflowDisplayNames(), QStringList({"Draft Prompt"}));

    QVERIFY(!window.loadWorkflowFromPath(invalidWorkflowPath));
    QVERIFY(window.isDirty());
    QCOMPARE(editor->nodeCount(), 1);
    QCOMPARE(editor->connectionCount(), 0);
    QCOMPARE(editor->workflowDisplayNames(), QStringList({"Draft Prompt"}));
    QCOMPARE(editor->selectedNodeProperty("userPromptTemplate").toString(), QString("Summarize {{input}}"));
}

void MainWindowTests::showsDirtyMarkerInWindowTitle()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    const QString cleanTitle = window.windowTitle();
    QVERIFY(!cleanTitle.startsWith("*"));

    window.addNodeFromType("prompt");
    QVERIFY(window.windowTitle().startsWith("*"));
}

void MainWindowTests::showsFileNameInWindowTitleAfterSave()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    LanguageManager languageManager;
    MainWindow window(&languageManager);

    window.addNodeFromType("prompt");
    QVERIFY(window.saveWorkflowToPath(QDir(tempDir.path()).filePath("my_flow.json")));

    QVERIFY(window.windowTitle().contains("my_flow.json"));
    QVERIFY(!window.windowTitle().startsWith("*"));
}

void MainWindowTests::tracksRecentFilesAcrossSaveAndLoad()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    LanguageManager languageManager;
    MainWindow window(&languageManager);

    QVERIFY(window.recentFiles().isEmpty());

    const QString path1 = QDir(tempDir.path()).filePath("flow1.json");
    const QString path2 = QDir(tempDir.path()).filePath("flow2.json");

    window.addNodeFromType("prompt");
    QVERIFY(window.saveWorkflowToPath(path1));

    QCOMPARE(window.recentFiles().size(), 1);
    QCOMPARE(window.recentFiles().first(), path1);

    window.addNodeFromType("llm");
    QVERIFY(window.saveWorkflowToPath(path2));

    QCOMPARE(window.recentFiles().size(), 2);
    QCOMPARE(window.recentFiles().first(), path2);
}

void MainWindowTests::fileMenuContainsSaveAsAndRecentFiles()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *fileMenu = window.findChild<QMenu *>("fileMenu");
    QVERIFY(fileMenu != nullptr);

    bool hasSaveAs = false;
    bool hasRecentFiles = false;
    for (auto *action : fileMenu->actions()) {
        if (action->objectName() == "saveAsAction")
            hasSaveAs = true;
        if (action->menu() != nullptr && action->menu()->objectName() == "recentFilesMenu")
            hasRecentFiles = true;
    }

    QVERIFY(hasSaveAs);
    QVERIFY(hasRecentFiles);
}

void MainWindowTests::deletesSelectedNodeFromCanvas()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    QVERIFY(editor != nullptr);

    const auto nodeA = editor->createNode("prompt");
    const auto nodeB = editor->createNode("llm");
    QCOMPARE(editor->nodeCount(), 2);

    editor->selectNode(nodeA);
    editor->deleteSelectedNodes();

    QCOMPARE(editor->nodeCount(), 1);
    QCOMPARE(editor->workflowDisplayNames().size(), 1);
}

void MainWindowTests::deletesSelectedConnectionFromCanvas()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    QVERIFY(editor != nullptr);

    const auto startNode = editor->createNode("start");
    const auto promptNode = editor->createNode("prompt");
    QVERIFY(editor->connectNodes(startNode, 0, promptNode, 0));
    QCOMPARE(editor->connectionCount(), 1);

    const QtNodes::ConnectionId connectionId{startNode, 0, promptNode, 0};
    QVERIFY(selectConnectionGraphicsObject(editor, connectionId) != nullptr);

    editor->deleteSelectedConnections();
    QCOMPARE(editor->connectionCount(), 0);
    QCOMPARE(editor->nodeCount(), 2);
}

void MainWindowTests::ignoresDeleteWhenSelectionIsEmpty()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    QVERIFY(editor != nullptr);

    const auto promptNode = editor->createNode("prompt");
    QVERIFY(promptNode != QtNodes::InvalidNodeId);
    QVERIFY(window.saveWorkflowToPath(QDir(tempDir.path()).filePath("empty-selection-delete.json")));
    QVERIFY(!window.isDirty());

    clearCanvasSelection(editor);
    editor->deleteSelection();

    QCOMPARE(editor->nodeCount(), 1);
    QCOMPARE(editor->connectionCount(), 0);
    QVERIFY(!window.isDirty());
}

void MainWindowTests::deletesMixedNodeAndConnectionSelectionTogether()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *deleteAction = window.findChild<QAction *>("deleteAction");
    auto *undoAction = window.findChild<QAction *>("undoAction");
    QVERIFY(editor != nullptr);
    QVERIFY(deleteAction != nullptr);
    QVERIFY(undoAction != nullptr);

    const auto startNode = editor->createNode("start");
    const auto promptNode = editor->createNode("prompt");
    const auto toolNode = editor->createNode("tool");
    const auto outputNode = editor->createNode("output");
    const QtNodes::ConnectionId promptConnectionId{startNode, 0, promptNode, 0};
    const QtNodes::ConnectionId toolConnectionId{toolNode, 0, outputNode, 0};
    QVERIFY(editor->connectNodes(startNode, 0, promptNode, 0));
    QVERIFY(editor->connectNodes(toolNode, 0, outputNode, 0));
    QCOMPARE(editor->nodeCount(), 4);
    QCOMPARE(editor->connectionCount(), 2);

    QVERIFY(selectNodeGraphicsObject(editor, promptNode) != nullptr);
    QVERIFY(selectConnectionGraphicsObject(editor, toolConnectionId, false) != nullptr);
    QVERIFY(deleteAction->isEnabled());

    deleteAction->trigger();

    QCOMPARE(editor->nodeCount(), 3);
    QCOMPARE(editor->connectionCount(), 0);
    QCOMPARE(editor->findNodeIdByDisplayName(QString::fromUtf8("提示词")), QtNodes::InvalidNodeId);

    undoAction->trigger();

    QCOMPARE(editor->nodeCount(), 4);
    QCOMPARE(editor->connectionCount(), 2);
    QVERIFY(selectConnectionGraphicsObject(editor, promptConnectionId) != nullptr);
    QVERIFY(selectConnectionGraphicsObject(editor, toolConnectionId) != nullptr);
}

void MainWindowTests::deleteMarksDocumentDirty()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    QVERIFY(editor != nullptr);

    editor->createNode("prompt");
    QVERIFY(window.saveWorkflowToPath(QDir(tempDir.path()).filePath("test.json")));
    QVERIFY(!window.isDirty());

    editor->selectNode(editor->findNodeIdByDisplayName(QString::fromUtf8("提示词")));
    editor->deleteSelectedNodes();
    QVERIFY(window.isDirty());
}

void MainWindowTests::editMenuContainsDeleteAction()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editMenu = window.findChild<QMenu *>("editMenu");
    QVERIFY(editMenu != nullptr);

    bool hasDelete = false;
    for (auto *action : editMenu->actions()) {
        if (action->objectName() == "deleteAction")
            hasDelete = true;
    }
    QVERIFY(hasDelete);
}

void MainWindowTests::undoesAndRedoesNodeCreation()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *undoAction = window.findChild<QAction *>("undoAction");
    auto *redoAction = window.findChild<QAction *>("redoAction");
    QVERIFY(editor != nullptr);
    QVERIFY(undoAction != nullptr);
    QVERIFY(redoAction != nullptr);

    QCOMPARE(editor->nodeCount(), 0);
    window.addNodeFromType("prompt");
    QCOMPARE(editor->nodeCount(), 1);

    undoAction->trigger();
    QCOMPARE(editor->nodeCount(), 0);

    redoAction->trigger();
    QCOMPARE(editor->nodeCount(), 1);
}

void MainWindowTests::undoesAndRedoesPropertyEdit()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *undoAction = window.findChild<QAction *>("undoAction");
    auto *redoAction = window.findChild<QAction *>("redoAction");
    auto *promptUserEdit = window.findChild<QTextEdit *>("inspectorPromptUserTemplateEdit");
    QVERIFY(editor != nullptr);
    QVERIFY(undoAction != nullptr);
    QVERIFY(redoAction != nullptr);
    QVERIFY(promptUserEdit != nullptr);

    const auto promptNode = editor->createNode("prompt");
    editor->selectNode(promptNode);
    promptUserEdit->setPlainText("Summarize {{input}}");
    QCOMPARE(editor->selectedNodeProperty("userPromptTemplate").toString(), QString("Summarize {{input}}"));

    undoAction->trigger();
    QCOMPARE(editor->selectedNodeProperty("userPromptTemplate").toString(), QString());

    redoAction->trigger();
    QCOMPARE(editor->selectedNodeProperty("userPromptTemplate").toString(), QString("Summarize {{input}}"));
}

void MainWindowTests::undoesAndRedoesNodeDeletion()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *undoAction = window.findChild<QAction *>("undoAction");
    auto *redoAction = window.findChild<QAction *>("redoAction");
    QVERIFY(editor != nullptr);
    QVERIFY(undoAction != nullptr);
    QVERIFY(redoAction != nullptr);

    const auto promptNode = editor->createNode("prompt");
    QVERIFY(promptNode != QtNodes::InvalidNodeId);
    QCOMPARE(editor->nodeCount(), 1);

    editor->selectNode(promptNode);
    editor->deleteSelectedNodes();
    QCOMPARE(editor->nodeCount(), 0);

    undoAction->trigger();
    QCOMPARE(editor->nodeCount(), 1);
    QCOMPARE(editor->workflowDisplayNames(), QStringList({QString::fromUtf8("提示词")}));

    redoAction->trigger();
    QCOMPARE(editor->nodeCount(), 0);
}

void MainWindowTests::undoesAndRedoesConnectionCreation()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *undoAction = window.findChild<QAction *>("undoAction");
    auto *redoAction = window.findChild<QAction *>("redoAction");
    QVERIFY(editor != nullptr);
    QVERIFY(undoAction != nullptr);
    QVERIFY(redoAction != nullptr);

    const auto startNode = editor->createNode("start");
    const auto promptNode = editor->createNode("prompt");
    QVERIFY(editor->connectNodes(startNode, 0, promptNode, 0));
    QCOMPARE(editor->connectionCount(), 1);

    undoAction->trigger();
    QCOMPARE(editor->connectionCount(), 0);
    QCOMPARE(editor->nodeCount(), 2);

    redoAction->trigger();
    QCOMPARE(editor->connectionCount(), 1);
    QCOMPARE(editor->nodeCount(), 2);
}

void MainWindowTests::undoesAndRedoesConnectionDeletion()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *deleteAction = window.findChild<QAction *>("deleteAction");
    auto *undoAction = window.findChild<QAction *>("undoAction");
    auto *redoAction = window.findChild<QAction *>("redoAction");
    QVERIFY(editor != nullptr);
    QVERIFY(deleteAction != nullptr);
    QVERIFY(undoAction != nullptr);
    QVERIFY(redoAction != nullptr);

    const auto startNode = editor->createNode("start");
    const auto promptNode = editor->createNode("prompt");
    const QtNodes::ConnectionId connectionId{startNode, 0, promptNode, 0};
    QVERIFY(editor->connectNodes(startNode, 0, promptNode, 0));
    QCOMPARE(editor->connectionCount(), 1);

    QVERIFY(selectConnectionGraphicsObject(editor, connectionId) != nullptr);
    deleteAction->trigger();
    QCOMPARE(editor->connectionCount(), 0);
    QCOMPARE(editor->nodeCount(), 2);

    undoAction->trigger();
    QCOMPARE(editor->connectionCount(), 1);
    QVERIFY(selectConnectionGraphicsObject(editor, connectionId) != nullptr);

    redoAction->trigger();
    QCOMPARE(editor->connectionCount(), 0);
    QCOMPARE(editor->nodeCount(), 2);
}

void MainWindowTests::restoresDirtyStateWhenUndoAndRedoCrossSavedPropertyEdit()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *undoAction = window.findChild<QAction *>("undoAction");
    auto *redoAction = window.findChild<QAction *>("redoAction");
    auto *promptUserEdit = window.findChild<QTextEdit *>("inspectorPromptUserTemplateEdit");
    QVERIFY(editor != nullptr);
    QVERIFY(undoAction != nullptr);
    QVERIFY(redoAction != nullptr);
    QVERIFY(promptUserEdit != nullptr);

    const auto promptNode = editor->createNode("prompt");
    editor->selectNode(promptNode);
    promptUserEdit->setPlainText("Summarize {{input}}");
    QVERIFY(window.saveWorkflowToPath(QDir(tempDir.path()).filePath("property-edit.json")));
    QVERIFY(!window.isDirty());
    QVERIFY(!window.windowTitle().startsWith("*"));

    promptUserEdit->setPlainText("Summarize {{document}}");
    QVERIFY(window.isDirty());
    QVERIFY(window.windowTitle().startsWith("*"));

    undoAction->trigger();
    QVERIFY(!window.isDirty());
    QVERIFY(!window.windowTitle().startsWith("*"));
    QCOMPARE(editor->selectedNodeProperty("userPromptTemplate").toString(), QString("Summarize {{input}}"));

    redoAction->trigger();
    QVERIFY(window.isDirty());
    QVERIFY(window.windowTitle().startsWith("*"));
    QCOMPARE(editor->selectedNodeProperty("userPromptTemplate").toString(), QString("Summarize {{document}}"));
}

void MainWindowTests::staysDirtyUntilUndoReachesExactSavedPropertyState()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *undoAction = window.findChild<QAction *>("undoAction");
    auto *redoAction = window.findChild<QAction *>("redoAction");
    auto *promptSystemEdit = window.findChild<QTextEdit *>("inspectorPromptSystemEdit");
    auto *promptUserEdit = window.findChild<QTextEdit *>("inspectorPromptUserTemplateEdit");
    QVERIFY(editor != nullptr);
    QVERIFY(undoAction != nullptr);
    QVERIFY(redoAction != nullptr);
    QVERIFY(promptSystemEdit != nullptr);
    QVERIFY(promptUserEdit != nullptr);

    const auto promptNode = editor->createNode("prompt");
    editor->selectNode(promptNode);
    promptUserEdit->setPlainText("Version A");
    QVERIFY(window.saveWorkflowToPath(QDir(tempDir.path()).filePath("property-edit-multi-step.json")));
    QVERIFY(!window.isDirty());
    QVERIFY(!window.windowTitle().startsWith("*"));

    promptSystemEdit->setPlainText("System B");
    promptUserEdit->setPlainText("Version C");
    QVERIFY(window.isDirty());
    QCOMPARE(editor->selectedNodeProperty("userPromptTemplate").toString(), QString("Version C"));
    QCOMPARE(editor->selectedNodeProperty("systemPrompt").toString(), QString("System B"));

    undoAction->trigger();
    QVERIFY(window.isDirty());
    QVERIFY(window.windowTitle().startsWith("*"));
    QCOMPARE(editor->selectedNodeProperty("userPromptTemplate").toString(), QString("Version A"));
    QCOMPARE(editor->selectedNodeProperty("systemPrompt").toString(), QString("System B"));

    undoAction->trigger();
    QVERIFY(!window.isDirty());
    QVERIFY(!window.windowTitle().startsWith("*"));
    QCOMPARE(editor->selectedNodeProperty("userPromptTemplate").toString(), QString("Version A"));
    QCOMPARE(editor->selectedNodeProperty("systemPrompt").toString(), QString());

    redoAction->trigger();
    QVERIFY(window.isDirty());
    QVERIFY(window.windowTitle().startsWith("*"));
    QCOMPARE(editor->selectedNodeProperty("userPromptTemplate").toString(), QString("Version A"));
    QCOMPARE(editor->selectedNodeProperty("systemPrompt").toString(), QString("System B"));
}

void MainWindowTests::restoresDirtyStateWhenUndoAndRedoCrossSavedNodeDeletion()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *undoAction = window.findChild<QAction *>("undoAction");
    auto *redoAction = window.findChild<QAction *>("redoAction");
    QVERIFY(editor != nullptr);
    QVERIFY(undoAction != nullptr);
    QVERIFY(redoAction != nullptr);

    const auto promptNode = editor->createNode("prompt");
    QVERIFY(promptNode != QtNodes::InvalidNodeId);
    QVERIFY(window.saveWorkflowToPath(QDir(tempDir.path()).filePath("node-delete.json")));
    QVERIFY(!window.isDirty());
    QVERIFY(!window.windowTitle().startsWith("*"));

    editor->selectNode(promptNode);
    editor->deleteSelectedNodes();
    QVERIFY(window.isDirty());
    QVERIFY(window.windowTitle().startsWith("*"));
    QCOMPARE(editor->nodeCount(), 0);

    undoAction->trigger();
    QVERIFY(!window.isDirty());
    QVERIFY(!window.windowTitle().startsWith("*"));
    QCOMPARE(editor->nodeCount(), 1);

    redoAction->trigger();
    QVERIFY(window.isDirty());
    QVERIFY(window.windowTitle().startsWith("*"));
    QCOMPARE(editor->nodeCount(), 0);
}

void MainWindowTests::restoresDirtyStateWhenUndoAndRedoCrossSavedConnectionCreation()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *undoAction = window.findChild<QAction *>("undoAction");
    auto *redoAction = window.findChild<QAction *>("redoAction");
    QVERIFY(editor != nullptr);
    QVERIFY(undoAction != nullptr);
    QVERIFY(redoAction != nullptr);

    const auto startNode = editor->createNode("start");
    const auto promptNode = editor->createNode("prompt");
    QVERIFY(window.saveWorkflowToPath(QDir(tempDir.path()).filePath("connection-create.json")));
    QVERIFY(!window.isDirty());
    QVERIFY(!window.windowTitle().startsWith("*"));

    QVERIFY(editor->connectNodes(startNode, 0, promptNode, 0));
    QVERIFY(window.isDirty());
    QVERIFY(window.windowTitle().startsWith("*"));
    QCOMPARE(editor->connectionCount(), 1);

    undoAction->trigger();
    QVERIFY(!window.isDirty());
    QVERIFY(!window.windowTitle().startsWith("*"));
    QCOMPARE(editor->connectionCount(), 0);

    redoAction->trigger();
    QVERIFY(window.isDirty());
    QVERIFY(window.windowTitle().startsWith("*"));
    QCOMPARE(editor->connectionCount(), 1);
}

void MainWindowTests::restoresDirtyStateWhenUndoAndRedoCrossSavedConnectionDeletion()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *deleteAction = window.findChild<QAction *>("deleteAction");
    auto *undoAction = window.findChild<QAction *>("undoAction");
    auto *redoAction = window.findChild<QAction *>("redoAction");
    QVERIFY(editor != nullptr);
    QVERIFY(deleteAction != nullptr);
    QVERIFY(undoAction != nullptr);
    QVERIFY(redoAction != nullptr);

    const auto startNode = editor->createNode("start");
    const auto promptNode = editor->createNode("prompt");
    const QtNodes::ConnectionId connectionId{startNode, 0, promptNode, 0};
    QVERIFY(editor->connectNodes(startNode, 0, promptNode, 0));
    QVERIFY(window.saveWorkflowToPath(QDir(tempDir.path()).filePath("connection-delete.json")));
    QVERIFY(!window.isDirty());
    QVERIFY(!window.windowTitle().startsWith("*"));

    QVERIFY(selectConnectionGraphicsObject(editor, connectionId) != nullptr);
    deleteAction->trigger();
    QVERIFY(window.isDirty());
    QVERIFY(window.windowTitle().startsWith("*"));

    undoAction->trigger();
    QVERIFY(!window.isDirty());
    QVERIFY(!window.windowTitle().startsWith("*"));
    QCOMPARE(editor->connectionCount(), 1);

    redoAction->trigger();
    QVERIFY(window.isDirty());
    QVERIFY(window.windowTitle().startsWith("*"));
    QCOMPARE(editor->connectionCount(), 0);
}

void MainWindowTests::showsZoomIndicatorInStatusBar()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);
    window.show();
    QCoreApplication::processEvents();

    auto *zoomLabel = window.findChild<QLabel *>("zoomIndicatorLabel");
    QVERIFY(zoomLabel != nullptr);
    QVERIFY(!zoomLabel->text().isEmpty());
    QVERIFY(zoomLabel->text().contains('%'));
}

void MainWindowTests::nodeLibraryShowsPortCountBadges()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);
    window.show();
    QCoreApplication::processEvents();

    auto *list = window.findChild<NodeLibraryListWidget *>("nodeLibraryList");
    QVERIFY(list != nullptr);

    bool foundPortCounts = false;
    for (int row = 0; row < list->count(); ++row) {
        auto *item = list->item(row);
        if (item->data(NodeLibraryListWidget::ItemKindRole).toInt()
            != static_cast<int>(NodeLibraryListWidget::ItemKind::NodeEntry)) {
            continue;
        }

        const int inPorts = item->data(NodeLibraryListWidget::InPortCountRole).toInt();
        const int outPorts = item->data(NodeLibraryListWidget::OutPortCountRole).toInt();
        if (inPorts > 0 || outPorts > 0)
            foundPortCounts = true;
    }

    QVERIFY(foundPortCounts);
}

void MainWindowTests::nodeLibraryShowsNoResultsWhenFilterHasNoMatch()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);
    window.show();
    QCoreApplication::processEvents();

    auto *searchEdit = window.findChild<QLineEdit *>("nodeLibrarySearchEdit");
    auto *noResultsLabel = window.findChild<QLabel *>("noSearchResultsLabel");
    auto *list = window.findChild<NodeLibraryListWidget *>("nodeLibraryList");
    QVERIFY(searchEdit != nullptr);
    QVERIFY(noResultsLabel != nullptr);
    QVERIFY(list != nullptr);

    QVERIFY(!noResultsLabel->isVisible());

    searchEdit->setText("zzz_nonexistent_node_type_zzz");
    QCoreApplication::processEvents();
    QCOMPARE(list->visibleNodeCount(), 0);
    QVERIFY(noResultsLabel->isVisible());

    searchEdit->clear();
    QCoreApplication::processEvents();
    QVERIFY(!noResultsLabel->isVisible());
    QVERIFY(list->visibleNodeCount() > 0);
}

void MainWindowTests::editMenuContainsCopyPasteDuplicateActions()
{
    LanguageManager languageManager;
    languageManager.setLanguage(LanguageManager::Language::English);
    MainWindow window(&languageManager);

    auto *editMenu = window.findChild<QMenu *>("editMenu");
    QVERIFY(editMenu != nullptr);

    QStringList actionTexts;
    for (auto *action : editMenu->actions()) {
        if (!action->isSeparator())
            actionTexts.append(action->text());
    }

    QVERIFY(actionTexts.contains("Copy"));
    QVERIFY(actionTexts.contains("Paste"));
    QVERIFY(actionTexts.contains("Duplicate"));
}

void MainWindowTests::assignsCopyPasteDuplicateKeyboardShortcuts()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *copyAction = window.findChild<QAction *>("copyAction");
    auto *pasteAction = window.findChild<QAction *>("pasteAction");
    auto *duplicateAction = window.findChild<QAction *>("duplicateAction");
    QVERIFY(copyAction != nullptr);
    QVERIFY(pasteAction != nullptr);
    QVERIFY(duplicateAction != nullptr);

    QCOMPARE(copyAction->shortcut(), QKeySequence::keyBindings(QKeySequence::Copy).constFirst());
    QCOMPARE(pasteAction->shortcut(), QKeySequence::keyBindings(QKeySequence::Paste).constFirst());
    QCOMPARE(duplicateAction->shortcut(), QKeySequence(Qt::CTRL | Qt::Key_D));
}

void MainWindowTests::enablesCopyPasteDuplicateActionsBasedOnState()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    auto *copyAction = window.findChild<QAction *>("copyAction");
    auto *pasteAction = window.findChild<QAction *>("pasteAction");
    auto *duplicateAction = window.findChild<QAction *>("duplicateAction");
    QVERIFY(editor != nullptr);
    QVERIFY(copyAction != nullptr);
    QVERIFY(pasteAction != nullptr);
    QVERIFY(duplicateAction != nullptr);

    QVERIFY(!copyAction->isEnabled());
    QVERIFY(!duplicateAction->isEnabled());

    const auto promptNode = editor->createNode("prompt");
    QVERIFY(promptNode != QtNodes::InvalidNodeId);

    QVERIFY(copyAction->isEnabled());
    QVERIFY(duplicateAction->isEnabled());

    clearCanvasSelection(editor);

    QVERIFY(!copyAction->isEnabled());
    QVERIFY(!duplicateAction->isEnabled());
}

void MainWindowTests::copiesAndPastesNodeOnCanvas()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    QVERIFY(editor != nullptr);

    const auto promptNode = editor->createNode("prompt");
    QVERIFY(promptNode != QtNodes::InvalidNodeId);
    QCOMPARE(editor->nodeCount(), 1);

    editor->copySelection();
    QVERIFY(editor->canPaste());

    editor->pasteClipboard();
    QCOMPARE(editor->nodeCount(), 2);
}

void MainWindowTests::duplicatesNodeOnCanvas()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    QVERIFY(editor != nullptr);

    const auto promptNode = editor->createNode("prompt");
    QVERIFY(promptNode != QtNodes::InvalidNodeId);
    QCOMPARE(editor->nodeCount(), 1);

    editor->duplicateSelection();
    QCOMPARE(editor->nodeCount(), 2);
}

void MainWindowTests::allowsCompatiblePortConnections()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    QVERIFY(editor != nullptr);

    // start(out:flow) -> prompt(in:flow) — same type
    const auto startNode = editor->createNode("start");
    const auto promptNode = editor->createNode("prompt");
    QVERIFY(editor->connectNodes(startNode, 0, promptNode, 0));

    // prompt(out:text) -> llm(in:text) — same type
    const auto llmNode = editor->createNode("llm");
    QVERIFY(editor->connectNodes(promptNode, 0, llmNode, 0));

    // llm(success:completion) -> output(in:flow) — flow accepts anything
    const auto outputNode = editor->createNode("output");
    QVERIFY(editor->connectNodes(llmNode, 0, outputNode, 0));
}

void MainWindowTests::rejectsIncompatiblePortConnections()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    QVERIFY(editor != nullptr);

    // llm(success:completion) -> llm2(in:text) — completion != text
    const auto llmNode1 = editor->createNode("llm");
    const auto llmNode2 = editor->createNode("llm");
    QVERIFY(!editor->connectNodes(llmNode1, 0, llmNode2, 0));

    // prompt(out:text) -> prompt2(in:flow) — text -> flow is accepted (flow accepts all)
    const auto promptNode1 = editor->createNode("prompt");
    const auto promptNode2 = editor->createNode("prompt");
    QVERIFY(editor->connectNodes(promptNode1, 0, promptNode2, 0));

    // httpRequest(success:http_response) -> llm(in:text) — incompatible
    const auto httpNode = editor->createNode("httpRequest");
    const auto llmNode3 = editor->createNode("llm");
    QVERIFY(!editor->connectNodes(httpNode, 0, llmNode3, 0));
}

void MainWindowTests::fileMenuContainsExportSubmenu()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *fileMenu = window.findChild<QMenu *>("fileMenu");
    QVERIFY(fileMenu != nullptr);

    bool foundExport = false;
    for (auto *action : fileMenu->actions()) {
        if (action->menu() != nullptr && action->menu()->objectName() == "exportMenu") {
            foundExport = true;
            break;
        }
    }
    QVERIFY(foundExport);

    auto *langChainAction = window.findChild<QAction *>("exportLangChainAction");
    auto *pythonAction = window.findChild<QAction *>("exportPythonAction");
    QVERIFY(langChainAction != nullptr);
    QVERIFY(pythonAction != nullptr);
}

void MainWindowTests::exportActionsDisabledWhenNoNodes()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *langChainAction = window.findChild<QAction *>("exportLangChainAction");
    auto *pythonAction = window.findChild<QAction *>("exportPythonAction");
    QVERIFY(langChainAction != nullptr);
    QVERIFY(pythonAction != nullptr);
    QVERIFY(!langChainAction->isEnabled());
    QVERIFY(!pythonAction->isEnabled());
}

void MainWindowTests::exportActionsEnabledWhenNodesExist()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>();
    QVERIFY(editor != nullptr);
    editor->createNode("start");

    auto *langChainAction = window.findChild<QAction *>("exportLangChainAction");
    auto *pythonAction = window.findChild<QAction *>("exportPythonAction");
    QVERIFY(langChainAction != nullptr);
    QVERIFY(pythonAction != nullptr);
    QVERIFY(langChainAction->isEnabled());
    QVERIFY(pythonAction->isEnabled());
}

void MainWindowTests::exportReadinessReportsEmptyInvalidAndValidWorkflow()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *editor = window.findChild<QtNodesEditorWidget *>("workflowCanvas");
    QVERIFY(editor != nullptr);

    QVERIFY(window.exportReadinessMessageForCurrentWorkflow().contains(QString::fromUtf8("没有可导出的工作流")));

    editor->createNode("prompt");
    QCoreApplication::processEvents();
    const QString invalidMessage = window.exportReadinessMessageForCurrentWorkflow();
    QVERIFY(invalidMessage.contains(QString::fromUtf8("请先修复")));
    QVERIFY(invalidMessage.contains(QString::fromUtf8("提示词模板为空。")));

    editor->clearWorkflow();
    const auto startNode = editor->createNode("start");
    const auto outputNode = editor->createNode("output");
    QVERIFY(editor->connectNodes(startNode, 0, outputNode, 0));
    QCoreApplication::processEvents();

    QCOMPARE(window.exportReadinessMessageForCurrentWorkflow(), QString());
}

QTEST_MAIN(MainWindowTests)

#include "MainWindowTests.moc"

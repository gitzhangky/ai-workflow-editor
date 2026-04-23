#include "app/MainWindow.hpp"
#include "app/LanguageManager.hpp"
#include "app/NodeLibraryListWidget.hpp"
#include "qtnodes/QtNodesEditorWidget.hpp"

#include <QtNodes/ConnectionStyle>
#include <QtNodes/GraphicsViewStyle>
#include <QtNodes/internal/ConnectionGraphicsObject.hpp>
#include <QtNodes/internal/DataFlowGraphicsScene.hpp>

#include <QAction>
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
#include <QSpinBox>
#include <QStandardPaths>
#include <QStatusBar>
#include <QTemporaryDir>
#include <QTextEdit>
#include <QToolButton>
#include <QToolBar>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QtTest/QTest>
#include <QMouseEvent>

namespace
{
QtNodes::ConnectionGraphicsObject *selectConnectionGraphicsObject(QtNodesEditorWidget *editor,
                                                                  QtNodes::ConnectionId const &connectionId)
{
    auto *graphicsView = editor != nullptr ? editor->findChild<QGraphicsView *>() : nullptr;
    auto *scene =
        graphicsView != nullptr ? qobject_cast<QtNodes::DataFlowGraphicsScene *>(graphicsView->scene()) : nullptr;
    if (scene != nullptr)
        scene->clearSelection();
    auto *connectionGraphicsObject = scene != nullptr ? scene->connectionGraphicsObject(connectionId) : nullptr;
    if (connectionGraphicsObject != nullptr)
        connectionGraphicsObject->setSelected(true);
    return connectionGraphicsObject;
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
    void appliesLightWorkbenchCanvasBackground();
    void createsPrimaryToolbarAndStatusBar();
    void showsGroupedToolbarLayout();
    void exposesToolbarStylingHooks();
    void exposesStableObjectNamesForWorkbenchChrome();
    void createsMenuBarWithFileViewAndSettingsMenus();
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
    void showsTypeSpecificInspectorHeaderAndEmptyState();
    void appliesDistinctNodeCardStylesByType();
    void marksIncompletePromptNodeWithWarningValidationState();
    void marksInvalidToolNodeWithErrorValidationState();
    void showsValidationMessageInInspectorForSelectedNode();
    void showsImmediateStatusFeedbackForInvalidConnectionDrag();
    void showsValidationSummaryForCurrentSelectionInStatusBar();
    void assignsExpectedKeyboardShortcuts();
    void enforcesConnectionRulesBetweenCompatiblePorts();
    void savesAndLoadsWorkflowJson();
    void tracksDirtyStateOnEdits();
    void clearsDirtyStateOnSave();
    void clearsDirtyStateOnLoad();
    void showsDirtyMarkerInWindowTitle();
    void showsFileNameInWindowTitleAfterSave();
    void tracksRecentFilesAcrossSaveAndLoad();
    void fileMenuContainsSaveAsAndRecentFiles();
    void deletesSelectedNodeFromCanvas();
    void deletesSelectedConnectionFromCanvas();
    void deleteMarksDocumentDirty();
    void editMenuContainsDeleteAction();
    void undoesAndRedoesNodeCreation();
    void undoesAndRedoesPropertyEdit();
    void undoesAndRedoesNodeDeletion();
    void undoesAndRedoesConnectionDeletion();
    void restoresDirtyStateWhenUndoAndRedoCrossSavedConnectionDeletion();
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
    QCOMPARE(docks.size(), 2);

    auto *nodeLibrary = window.findChild<QListWidget *>("nodeLibraryList");
    QVERIFY(nodeLibrary != nullptr);
    QCOMPARE(nodeLibrary->count(), 9);
    QCOMPARE(nodeLibrary->item(1)->text(), QString::fromUtf8("开始"));
}

void MainWindowTests::createsWorkflowCanvasInCentralArea()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    QVERIFY(window.centralWidget() != nullptr);
    QCOMPARE(window.centralWidget()->objectName(), QString("workflowCanvas"));
    QVERIFY(window.centralWidget()->findChild<QGraphicsView *>() != nullptr);
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
    QCOMPARE(toolbar->actions().size(), 12);
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
    QCOMPARE(actions.size(), 12);
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
    QVERIFY(actions.at(10)->isSeparator());
    QVERIFY(actions.at(11)->isSeparator() == false);
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
    QCOMPARE(menuBar->actions().size(), 4);
    QCOMPARE(menuBar->actions().at(0)->text(), QString::fromUtf8("文件"));
    QCOMPARE(menuBar->actions().at(1)->text(), QString::fromUtf8("编辑"));
    QCOMPARE(menuBar->actions().at(2)->text(), QString::fromUtf8("视图"));
    QCOMPARE(menuBar->actions().at(3)->text(), QString::fromUtf8("设置"));

    auto *settingsMenu = menuBar->actions().at(3)->menu();
    QVERIFY(settingsMenu != nullptr);
    QCOMPARE(settingsMenu->actions().size(), 1);
    QCOMPARE(settingsMenu->actions().at(0)->text(), QString::fromUtf8("语言"));
    QVERIFY(settingsMenu->actions().at(0)->menu() != nullptr);
    QCOMPARE(settingsMenu->actions().at(0)->menu()->actions().size(), 2);
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

    QVERIFY(toolbar != nullptr);
    QVERIFY(nodeLibraryDock != nullptr);
    QVERIFY(inspectorDock != nullptr);
    QVERIFY(nodeLibrary != nullptr);
    QVERIFY(hintLabel != nullptr);

    QVERIFY(languageManager.setLanguage(LanguageManager::Language::English));

    QCOMPARE(window.windowTitle(), QString("AI Workflow Editor"));
    QCOMPARE(toolbar->actions().at(0)->text(), QString("New"));
    QCOMPARE(nodeLibraryDock->windowTitle(), QString("Node Library"));
    QCOMPARE(inspectorDock->windowTitle(), QString("Inspector"));
    QCOMPARE(nodeLibrary->item(1)->text(), QString("Start"));
    QCOMPARE(hintLabel->text(), QString("Select a node to edit its configuration"));
    QCOMPARE(window.statusBar()->currentMessage(), QString("Ready"));
    QCOMPARE(window.menuBar()->actions().at(0)->text(), QString("File"));
    QCOMPARE(window.menuBar()->actions().at(3)->text(), QString("Settings"));
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
}

void MainWindowTests::groupsNodeLibraryIntoCategorySections()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *nodeLibrary = window.findChild<QListWidget *>("nodeLibraryList");
    QVERIFY(nodeLibrary != nullptr);

    QCOMPARE(nodeLibrary->count(), 9);
    QCOMPARE(nodeLibrary->item(0)->text(), QString::fromUtf8("流程"));
    QCOMPARE(nodeLibrary->item(0)->data(Qt::UserRole + 1).toString(), QString());
    QVERIFY(!(nodeLibrary->item(0)->flags() & Qt::ItemIsDragEnabled));
    QCOMPARE(nodeLibrary->item(1)->text(), QString::fromUtf8("开始"));
    QCOMPARE(nodeLibrary->item(2)->text(), QString::fromUtf8("条件"));
    QCOMPARE(nodeLibrary->item(3)->text(), QString::fromUtf8("输出"));
    QCOMPARE(nodeLibrary->item(4)->text(), QString::fromUtf8("AI"));
    QCOMPARE(nodeLibrary->item(5)->text(), QString::fromUtf8("提示词"));
    QCOMPARE(nodeLibrary->item(6)->text(), QString::fromUtf8("大模型"));
    QCOMPARE(nodeLibrary->item(8)->text(), QString::fromUtf8("工具"));
    QCOMPARE(nodeLibrary->item(7)->text(), QString::fromUtf8("集成"));
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
    QVERIFY(!nodeLibrary->item(4)->isHidden());
    QVERIFY(!nodeLibrary->item(5)->isHidden());
    QVERIFY(nodeLibrary->item(6)->isHidden());
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
    auto *toolNameEdit = window.findChild<QLineEdit *>("inspectorToolNameEdit");
    auto *toolTimeoutSpin = window.findChild<QSpinBox *>("inspectorToolTimeoutSpin");
    auto *toolInputMappingEdit = window.findChild<QTextEdit *>("inspectorToolInputMappingEdit");

    QVERIFY(editor != nullptr);
    QVERIFY(promptSystemEdit != nullptr);
    QVERIFY(promptUserEdit != nullptr);
    QVERIFY(llmModelEdit != nullptr);
    QVERIFY(llmTemperatureSpin != nullptr);
    QVERIFY(llmMaxTokensSpin != nullptr);
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
    auto *selectAllAction = window.findChild<QAction *>("selectAllAction");

    QVERIFY(newAction != nullptr);
    QVERIFY(openAction != nullptr);
    QVERIFY(saveAction != nullptr);
    QVERIFY(saveAsAction != nullptr);
    QVERIFY(deleteAction != nullptr);
    QVERIFY(undoAction != nullptr);
    QVERIFY(redoAction != nullptr);
    QVERIFY(centerAction != nullptr);
    QVERIFY(selectAllAction != nullptr);

    QCOMPARE(newAction->shortcut(), QKeySequence::keyBindings(QKeySequence::New).constFirst());
    QCOMPARE(openAction->shortcut(), QKeySequence::keyBindings(QKeySequence::Open).constFirst());
    QCOMPARE(saveAction->shortcut(), QKeySequence::keyBindings(QKeySequence::Save).constFirst());
    QCOMPARE(saveAsAction->shortcut(), QKeySequence::keyBindings(QKeySequence::SaveAs).constFirst());
    QCOMPARE(deleteAction->shortcut(), QKeySequence(Qt::Key_Delete));
    QCOMPARE(undoAction->shortcut(), QKeySequence::keyBindings(QKeySequence::Undo).constFirst());
    QCOMPARE(redoAction->shortcut(), QKeySequence::keyBindings(QKeySequence::Redo).constFirst());
    QCOMPARE(centerAction->shortcut(), QKeySequence(Qt::Key_Space));
    QCOMPARE(selectAllAction->shortcut(), QKeySequence::keyBindings(QKeySequence::SelectAll).constFirst());
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

QTEST_MAIN(MainWindowTests)

#include "MainWindowTests.moc"

#include "app/MainWindow.hpp"

#include "app/HelpDocumentWidget.hpp"
#include "app/NodeLibraryListWidget.hpp"
#include "app/WorkbenchTabWidget.hpp"
#include "inspector/InspectorPanel.hpp"
#include "qtnodes/QtNodesEditorWidget.hpp"
#include "registry/BuiltInNodeRegistry.hpp"

#include <QAction>
#include <QAbstractItemView>
#include <QCloseEvent>
#include <QColor>
#include <QComboBox>
#include <QCoreApplication>
#include <QDockWidget>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QListWidgetItem>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include <QSignalBlocker>
#include <QStatusBar>
#include <QTabBar>
#include <QTableWidget>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

namespace
{
QIcon toolbarIcon(QString const &iconName)
{
    return QIcon(QStringLiteral(":/app-theme/icons/toolbar/%1.svg").arg(iconName));
}

QIcon nodeLibraryIcon(QString const &iconName)
{
    return QIcon(QStringLiteral(":/app-theme/icons/nodes/%1.svg").arg(iconName));
}

QStringList categoryOrder()
{
    return {QStringLiteral("flow"), QStringLiteral("ai"), QStringLiteral("integration")};
}
}

MainWindow::MainWindow(LanguageManager *languageManager, QWidget *parent)
    : QMainWindow(parent)
    , _languageManager(languageManager)
    , _fileMenu(nullptr)
    , _editMenu(nullptr)
    , _viewMenu(nullptr)
    , _settingsMenu(nullptr)
    , _languageMenu(nullptr)
    , _recentFilesMenu(nullptr)
    , _arrangeMenu(nullptr)
    , _primaryToolBar(nullptr)
    , _nodeLibraryDock(nullptr)
    , _inspectorDock(nullptr)
    , _problemsDock(nullptr)
    , _nodeLibraryPanel(nullptr)
    , _nodeLibraryList(nullptr)
    , _nodeLibrarySearchEdit(nullptr)
    , _inspectorPanel(nullptr)
    , _tabWidget(nullptr)
    , _editorWidget(nullptr)
    , _helpWidget(nullptr)
    , _selectionValidationSummaryLabel(nullptr)
    , _problemsSummaryLabel(nullptr)
    , _problemsFilterComboBox(nullptr)
    , _problemsEmptyStateLabel(nullptr)
    , _problemsTable(nullptr)
    , _newAction(nullptr)
    , _openAction(nullptr)
    , _saveAction(nullptr)
    , _saveAsAction(nullptr)
    , _exportMenu(nullptr)
    , _exportLangChainAction(nullptr)
    , _exportPythonAction(nullptr)
    , _exportLangGraphAction(nullptr)
    , _exportCrewAIAction(nullptr)
    , _copyAction(nullptr)
    , _pasteAction(nullptr)
    , _duplicateAction(nullptr)
    , _deleteAction(nullptr)
    , _selectAllAction(nullptr)
    , _undoAction(nullptr)
    , _redoAction(nullptr)
    , _centerAction(nullptr)
    , _fitWorkflowAction(nullptr)
    , _alignLeftAction(nullptr)
    , _alignRightAction(nullptr)
    , _alignTopAction(nullptr)
    , _alignBottomAction(nullptr)
    , _distributeHorizontalAction(nullptr)
    , _distributeVerticalAction(nullptr)
    , _toggleNodeLibraryAction(nullptr)
    , _toggleInspectorAction(nullptr)
    , _toggleProblemsAction(nullptr)
    , _languageMenuAction(nullptr)
    , _languageChineseAction(nullptr)
    , _languageEnglishAction(nullptr)
    , _helpMenu(nullptr)
    , _helpAction(nullptr)
    , _languageToolButton(nullptr)
    , _currentWorkflowPath()
    , _currentSelectedNodeDisplayName()
    , _dirty(false)
{
    setObjectName("appMainWindow");
    resize(1280, 840);

    _fileMenu = menuBar()->addMenu(QString());
    _fileMenu->setObjectName("fileMenu");
    _editMenu = menuBar()->addMenu(QString());
    _editMenu->setObjectName("editMenu");
    _viewMenu = menuBar()->addMenu(QString());
    _viewMenu->setObjectName("viewMenu");
    _arrangeMenu = new QMenu(this);
    _arrangeMenu->setObjectName("arrangeMenu");
    _settingsMenu = menuBar()->addMenu(QString());
    _settingsMenu->setObjectName("settingsMenu");
    _languageMenu = _settingsMenu->addMenu(QString());
    _languageMenu->setObjectName("languageMenu");
    _helpMenu = menuBar()->addMenu(QString());
    _helpMenu->setObjectName("helpMenu");
    _helpAction = new QAction(this);
    _helpAction->setObjectName("helpAction");

    _primaryToolBar = addToolBar(QStringLiteral("Primary"));
    _primaryToolBar->setObjectName("primaryToolBar");
    _primaryToolBar->setMovable(false);
    _primaryToolBar->setFloatable(false);
    _primaryToolBar->setProperty("variant", QStringLiteral("workbench"));
    _primaryToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    _primaryToolBar->setIconSize(QSize(18, 18));

    _newAction = _primaryToolBar->addAction(toolbarIcon(QStringLiteral("new")), QString());
    _newAction->setObjectName("newAction");
    _openAction = _primaryToolBar->addAction(toolbarIcon(QStringLiteral("open")), QString());
    _openAction->setObjectName("openAction");
    _saveAction = _primaryToolBar->addAction(toolbarIcon(QStringLiteral("save")), QString());
    _saveAction->setObjectName("saveAction");
    _saveAction->setProperty("emphasis", QStringLiteral("primary"));
    _primaryToolBar->addSeparator();
    _saveAsAction = new QAction(this);
    _saveAsAction->setObjectName("saveAsAction");
    _exportMenu = new QMenu(this);
    _exportMenu->setObjectName("exportMenu");
    _exportLangChainAction = new QAction(this);
    _exportLangChainAction->setObjectName("exportLangChainAction");
    _exportPythonAction = new QAction(this);
    _exportPythonAction->setObjectName("exportPythonAction");
    _exportLangGraphAction = new QAction(this);
    _exportLangGraphAction->setObjectName("exportLangGraphAction");
    _exportCrewAIAction = new QAction(this);
    _exportCrewAIAction->setObjectName("exportCrewAIAction");
    _copyAction = new QAction(this);
    _copyAction->setObjectName("copyAction");
    _pasteAction = new QAction(this);
    _pasteAction->setObjectName("pasteAction");
    _duplicateAction = new QAction(this);
    _duplicateAction->setObjectName("duplicateAction");
    _deleteAction = new QAction(toolbarIcon(QStringLiteral("delete")), QString(), this);
    _deleteAction->setObjectName("deleteAction");
    _selectAllAction = new QAction(toolbarIcon(QStringLiteral("select-all")), QString(), this);
    _selectAllAction->setObjectName("selectAllAction");
    _undoAction = _primaryToolBar->addAction(toolbarIcon(QStringLiteral("undo")), QString());
    _undoAction->setObjectName("undoAction");
    _redoAction = _primaryToolBar->addAction(toolbarIcon(QStringLiteral("redo")), QString());
    _redoAction->setObjectName("redoAction");
    _primaryToolBar->addAction(_deleteAction);
    _primaryToolBar->addSeparator();
    _primaryToolBar->addAction(_selectAllAction);
    _centerAction = _primaryToolBar->addAction(toolbarIcon(QStringLiteral("center")), QString());
    _centerAction->setObjectName("centerAction");
    _fitWorkflowAction = _primaryToolBar->addAction(toolbarIcon(QStringLiteral("fit-workflow")), QString());
    _fitWorkflowAction->setObjectName("fitWorkflowAction");
    _alignLeftAction = new QAction(this);
    _alignLeftAction->setObjectName("alignLeftAction");
    _alignRightAction = new QAction(this);
    _alignRightAction->setObjectName("alignRightAction");
    _alignTopAction = new QAction(this);
    _alignTopAction->setObjectName("alignTopAction");
    _alignBottomAction = new QAction(this);
    _alignBottomAction->setObjectName("alignBottomAction");
    _distributeHorizontalAction = new QAction(this);
    _distributeHorizontalAction->setObjectName("distributeHorizontalAction");
    _distributeVerticalAction = new QAction(this);
    _distributeVerticalAction->setObjectName("distributeVerticalAction");
    _primaryToolBar->addSeparator();

    _toggleNodeLibraryAction = new QAction(this);
    _toggleNodeLibraryAction->setObjectName("toggleNodeLibraryAction");
    _toggleNodeLibraryAction->setCheckable(true);
    _toggleInspectorAction = new QAction(this);
    _toggleInspectorAction->setObjectName("toggleInspectorAction");
    _toggleInspectorAction->setCheckable(true);
    _toggleProblemsAction = new QAction(this);
    _toggleProblemsAction->setObjectName("toggleProblemsAction");
    _toggleProblemsAction->setCheckable(true);

    _languageToolButton = new QToolButton(this);
    _languageToolButton->setObjectName("languageToolButton");
    _languageToolButton->setProperty("variant", QStringLiteral("toolbar-language"));
    _languageToolButton->setIcon(toolbarIcon(QStringLiteral("language")));
    _languageToolButton->setPopupMode(QToolButton::InstantPopup);

    auto *languageMenu = new QMenu(_languageToolButton);
    _languageChineseAction = languageMenu->addAction(QStringLiteral("中文"));
    _languageChineseAction->setCheckable(true);
    _languageChineseAction->setObjectName("languageChineseAction");
    _languageEnglishAction = languageMenu->addAction(QStringLiteral("English"));
    _languageEnglishAction->setCheckable(true);
    _languageEnglishAction->setObjectName("languageEnglishAction");
    _languageToolButton->setMenu(languageMenu);
    _primaryToolBar->addWidget(_languageToolButton);
    _languageMenuAction = _settingsMenu->addMenu(_languageMenu);
    if (auto *saveButton = qobject_cast<QToolButton *>(_primaryToolBar->widgetForAction(_saveAction)); saveButton != nullptr)
        saveButton->setProperty("emphasis", QStringLiteral("primary"));

    _nodeLibraryDock = new QDockWidget(this);
    _nodeLibraryDock->setObjectName("nodeLibraryDock");
    _nodeLibraryList = createNodeLibrary();
    _nodeLibraryPanel = new QWidget(_nodeLibraryDock);
    _nodeLibraryPanel->setObjectName("nodeLibraryPanel");
    auto *nodeLibraryLayout = new QVBoxLayout(_nodeLibraryPanel);
    nodeLibraryLayout->setContentsMargins(12, 12, 12, 12);
    nodeLibraryLayout->setSpacing(10);
    _nodeLibrarySearchEdit = new QLineEdit(_nodeLibraryPanel);
    _nodeLibrarySearchEdit->setObjectName("nodeLibrarySearchEdit");
    nodeLibraryLayout->addWidget(_nodeLibrarySearchEdit);
    _noSearchResultsLabel = new QLabel(_nodeLibraryPanel);
    _noSearchResultsLabel->setObjectName("noSearchResultsLabel");
    _noSearchResultsLabel->setAlignment(Qt::AlignCenter);
    _noSearchResultsLabel->setWordWrap(true);
    _noSearchResultsLabel->hide();
    nodeLibraryLayout->addWidget(_nodeLibraryList, 1);
    nodeLibraryLayout->addWidget(_noSearchResultsLabel);
    _nodeLibraryDock->setWidget(_nodeLibraryPanel);
    addDockWidget(Qt::LeftDockWidgetArea, _nodeLibraryDock);
    _toggleNodeLibraryAction->setChecked(true);

    _inspectorDock = new QDockWidget(this);
    _inspectorDock->setObjectName("inspectorDock");
    _inspectorPanel = new InspectorPanel(_inspectorDock);
    _inspectorDock->setWidget(_inspectorPanel);
    addDockWidget(Qt::RightDockWidgetArea, _inspectorDock);
    _toggleInspectorAction->setChecked(true);

    _problemsDock = new QDockWidget(this);
    _problemsDock->setObjectName("problemsDock");
    auto *problemsPanel = new QWidget(_problemsDock);
    problemsPanel->setObjectName("problemsPanel");
    auto *problemsLayout = new QVBoxLayout(problemsPanel);
    problemsLayout->setContentsMargins(12, 10, 12, 12);
    problemsLayout->setSpacing(8);

    auto *problemsHeader = new QWidget(problemsPanel);
    problemsHeader->setObjectName("problemsHeader");
    auto *problemsHeaderLayout = new QHBoxLayout(problemsHeader);
    problemsHeaderLayout->setContentsMargins(0, 0, 0, 0);
    problemsHeaderLayout->setSpacing(8);
    _problemsSummaryLabel = new QLabel(problemsHeader);
    _problemsSummaryLabel->setObjectName("problemsSummaryLabel");
    _problemsFilterComboBox = new QComboBox(problemsHeader);
    _problemsFilterComboBox->setObjectName("problemsFilterComboBox");
    problemsHeaderLayout->addWidget(_problemsSummaryLabel, 1);
    problemsHeaderLayout->addWidget(_problemsFilterComboBox);
    problemsLayout->addWidget(problemsHeader);

    _problemsEmptyStateLabel = new QLabel(problemsPanel);
    _problemsEmptyStateLabel->setObjectName("problemsEmptyStateLabel");
    _problemsEmptyStateLabel->setAlignment(Qt::AlignCenter);
    _problemsEmptyStateLabel->setWordWrap(true);
    problemsLayout->addWidget(_problemsEmptyStateLabel, 1);

    _problemsTable = new QTableWidget(problemsPanel);
    _problemsTable->setObjectName("problemsTable");
    _problemsTable->setColumnCount(4);
    _problemsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _problemsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    _problemsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    _problemsTable->setAlternatingRowColors(true);
    _problemsTable->setMouseTracking(true);
    _problemsTable->viewport()->setMouseTracking(true);
    _problemsTable->setShowGrid(false);
    _problemsTable->verticalHeader()->hide();
    _problemsTable->horizontalHeader()->setStretchLastSection(true);
    _problemsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    _problemsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    _problemsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    problemsLayout->addWidget(_problemsTable, 1);
    _problemsDock->setWidget(problemsPanel);
    addDockWidget(Qt::BottomDockWidgetArea, _problemsDock);
    _toggleProblemsAction->setChecked(true);

    _tabWidget = new WorkbenchTabWidget(this);
    _editorWidget = new QtNodesEditorWidget(_tabWidget);
    _helpWidget = nullptr;
    _tabWidget->addTab(_editorWidget, tr("Workflow"));
    _tabWidget->tabBar()->setTabButton(0, QTabBar::RightSide, nullptr);
    _tabWidget->tabBar()->setTabButton(0, QTabBar::LeftSide, nullptr);
    setCentralWidget(_tabWidget);
    connect(_tabWidget, &QTabWidget::currentChanged, this, &MainWindow::handleTabChanged);
    statusBar()->setObjectName("primaryStatusBar");
    _selectionValidationSummaryLabel = new QLabel(statusBar());
    _selectionValidationSummaryLabel->setObjectName("selectionValidationSummaryLabel");
    statusBar()->addPermanentWidget(_selectionValidationSummaryLabel, 1);

    _zoomIndicatorLabel = new QLabel(statusBar());
    _zoomIndicatorLabel->setObjectName("zoomIndicatorLabel");
    _zoomIndicatorLabel->setText(QStringLiteral("100%"));
    statusBar()->addPermanentWidget(_zoomIndicatorLabel);

    connect(_nodeLibraryList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item) {
        addNodeFromType(item->data(NodeLibraryListWidget::NodeTypeRole).toString());
    });
    connect(_nodeLibraryList, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        if (item != nullptr
            && item->data(NodeLibraryListWidget::ItemKindRole).toInt()
                   == static_cast<int>(NodeLibraryListWidget::ItemKind::SectionHeader)) {
            _nodeLibraryList->toggleSection(item);
        }
    });
    connect(_nodeLibrarySearchEdit, &QLineEdit::textChanged, this, [this](QString const &text) {
        _nodeLibraryList->setFilterText(text);
        const bool noResults = !text.trimmed().isEmpty() && _nodeLibraryList->visibleNodeCount() == 0;
        _noSearchResultsLabel->setVisible(noResults);
    });

    connect(_editorWidget,
            &QtNodesEditorWidget::dropPreviewMessageChanged,
            this,
            [this](QString const &message) { statusBar()->showMessage(message); });
    connect(_editorWidget,
            &QtNodesEditorWidget::selectedNodeChanged,
            this,
            [this](QString const &typeKey,
                   QString const &displayName,
                   QString const &description,
                   QVariantMap const &properties) {
                Q_UNUSED(typeKey);
                Q_UNUSED(description);
                Q_UNUSED(properties);
                _currentSelectedNodeDisplayName = displayName;
                _inspectorPanel->setSelectedNode(typeKey, displayName, description, properties);
            });
    connect(_editorWidget,
            &QtNodesEditorWidget::selectedNodeValidationChanged,
            this,
            [this](QString const &state, QString const &message, QString const &propertyKey) {
                _inspectorPanel->setValidationFeedback(state, message, propertyKey);
                updateSelectionValidationSummary(state, message);
                updateProblemsPanel();
            });
    connect(_editorWidget, &QtNodesEditorWidget::selectionCleared, this, [this]() {
        _currentSelectedNodeDisplayName.clear();
        _inspectorPanel->clearSelection();
        updateSelectionValidationSummary(QString(), QString());
    });
    connect(_inspectorPanel,
            &InspectorPanel::displayNameEdited,
            _editorWidget,
            &QtNodesEditorWidget::setSelectedNodeDisplayName);
    connect(_inspectorPanel,
            &InspectorPanel::descriptionEdited,
            _editorWidget,
            &QtNodesEditorWidget::setSelectedNodeDescription);
    connect(_inspectorPanel,
            &InspectorPanel::propertyEdited,
            _editorWidget,
            &QtNodesEditorWidget::setSelectedNodeProperty);

    connect(_editorWidget, &QtNodesEditorWidget::zoomLevelChanged, this, [this](int zoomPercent) {
        _zoomIndicatorLabel->setText(QStringLiteral("%1%").arg(zoomPercent));
    });
    connect(_editorWidget, &QtNodesEditorWidget::workflowModified, this, &MainWindow::markDirty);
    connect(_editorWidget, &QtNodesEditorWidget::workflowModified, this, &MainWindow::updateProblemsPanel);
    connect(_editorWidget, &QtNodesEditorWidget::cleanStateChanged, this, [this](bool clean) {
        if (clean)
            clearDirty();
        else
            markDirty();
    });
    connect(_editorWidget, &QtNodesEditorWidget::interactionStateChanged, this, &MainWindow::updateWorkbenchActionStates);

    _recentFilesMenu = new QMenu(this);
    _recentFilesMenu->setObjectName("recentFilesMenu");

    connect(_newAction, &QAction::triggered, this, [this]() {
        if (!maybeSave())
            return;
        _editorWidget->clearWorkflow();
        _editorWidget->markCurrentStateClean();
        _currentWorkflowPath.clear();
        clearDirty();
        updateProblemsPanel();
        statusBar()->showMessage(tr("New workflow"));
    });

    connect(_openAction, &QAction::triggered, this, [this]() {
        if (!maybeSave())
            return;
        const auto filePath = QFileDialog::getOpenFileName(this,
                                                           tr("Open Workflow"),
                                                           QString(),
                                                           tr("Workflow Files (*.json)"));
        if (!filePath.isEmpty())
            loadWorkflowFromPath(filePath);
    });

    connect(_saveAction, &QAction::triggered, this, [this]() {
        QString filePath = _currentWorkflowPath;
        if (filePath.isEmpty()) {
            filePath = QFileDialog::getSaveFileName(this,
                                                    tr("Save Workflow"),
                                                    QStringLiteral("workflow.json"),
                                                    tr("Workflow Files (*.json)"));
        }

        if (!filePath.isEmpty())
            saveWorkflowToPath(filePath);
    });

    connect(_saveAsAction, &QAction::triggered, this, [this]() {
        const auto filePath = QFileDialog::getSaveFileName(this,
                                                           tr("Save Workflow"),
                                                           QStringLiteral("workflow.json"),
                                                           tr("Workflow Files (*.json)"));
        if (!filePath.isEmpty())
            saveWorkflowToPath(filePath);
    });

    connect(_exportLangChainAction, &QAction::triggered, this, [this]() {
        exportWorkflow(WorkflowExporter::Format::PythonLangChain);
    });
    connect(_exportPythonAction, &QAction::triggered, this, [this]() {
        exportWorkflow(WorkflowExporter::Format::PythonScript);
    });
    connect(_exportLangGraphAction, &QAction::triggered, this, [this]() {
        exportWorkflow(WorkflowExporter::Format::PythonLangGraph);
    });
    connect(_exportCrewAIAction, &QAction::triggered, this, [this]() {
        exportWorkflow(WorkflowExporter::Format::PythonCrewAI);
    });

    connect(_copyAction, &QAction::triggered, _editorWidget, &QtNodesEditorWidget::copySelection);
    connect(_pasteAction, &QAction::triggered, _editorWidget, &QtNodesEditorWidget::pasteClipboard);
    connect(_duplicateAction, &QAction::triggered, _editorWidget, &QtNodesEditorWidget::duplicateSelection);
    connect(_deleteAction, &QAction::triggered, _editorWidget, &QtNodesEditorWidget::deleteSelection);
    connect(_selectAllAction, &QAction::triggered, _editorWidget, &QtNodesEditorWidget::selectAllNodes);
    connect(_undoAction, &QAction::triggered, _editorWidget, &QtNodesEditorWidget::undo);
    connect(_redoAction, &QAction::triggered, _editorWidget, &QtNodesEditorWidget::redo);
    connect(_centerAction, &QAction::triggered, _editorWidget, &QtNodesEditorWidget::centerSelection);
    connect(_fitWorkflowAction, &QAction::triggered, _editorWidget, &QtNodesEditorWidget::fitWorkflow);
    connect(_alignLeftAction, &QAction::triggered, _editorWidget, [this]() {
        _editorWidget->alignSelectedNodes(QtNodesEditorWidget::Alignment::Left);
    });
    connect(_alignRightAction, &QAction::triggered, _editorWidget, [this]() {
        _editorWidget->alignSelectedNodes(QtNodesEditorWidget::Alignment::Right);
    });
    connect(_alignTopAction, &QAction::triggered, _editorWidget, [this]() {
        _editorWidget->alignSelectedNodes(QtNodesEditorWidget::Alignment::Top);
    });
    connect(_alignBottomAction, &QAction::triggered, _editorWidget, [this]() {
        _editorWidget->alignSelectedNodes(QtNodesEditorWidget::Alignment::Bottom);
    });
    connect(_distributeHorizontalAction, &QAction::triggered, _editorWidget, [this]() {
        _editorWidget->distributeSelectedNodes(QtNodesEditorWidget::Distribution::Horizontal);
    });
    connect(_distributeVerticalAction, &QAction::triggered, _editorWidget, [this]() {
        _editorWidget->distributeSelectedNodes(QtNodesEditorWidget::Distribution::Vertical);
    });

    connect(_languageChineseAction, &QAction::triggered, this, [this]() {
        if (_languageManager != nullptr)
            _languageManager->setLanguage(LanguageManager::Language::Chinese);
    });
    connect(_languageEnglishAction, &QAction::triggered, this, [this]() {
        if (_languageManager != nullptr)
            _languageManager->setLanguage(LanguageManager::Language::English);
    });

    connect(_toggleNodeLibraryAction, &QAction::toggled, _nodeLibraryDock, &QDockWidget::setVisible);
    connect(_toggleInspectorAction, &QAction::toggled, _inspectorDock, &QDockWidget::setVisible);
    connect(_toggleProblemsAction, &QAction::toggled, _problemsDock, &QDockWidget::setVisible);
    connect(_nodeLibraryDock, &QDockWidget::visibilityChanged, _toggleNodeLibraryAction, &QAction::setChecked);
    connect(_inspectorDock, &QDockWidget::visibilityChanged, _toggleInspectorAction, &QAction::setChecked);
    connect(_problemsDock, &QDockWidget::visibilityChanged, _toggleProblemsAction, &QAction::setChecked);
    connect(_problemsFilterComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            [this](int) { updateProblemsPanel(); });
    connect(_problemsTable, &QTableWidget::cellClicked, this, [this](int row, int) {
        activateProblemRow(row);
    });
    connect(_problemsTable, &QTableWidget::cellActivated, this, [this](int row, int) {
        activateProblemRow(row);
    });
    connect(_problemsTable, &QTableWidget::cellDoubleClicked, this, [this](int row, int) {
        activateProblemRow(row);
    });
    connect(_helpAction, &QAction::triggered, this, &MainWindow::openHelpTab);

    if (_languageManager != nullptr) {
        connect(_languageManager, &LanguageManager::languageChanged, this, [this](LanguageManager::Language) {
            updateLanguageActions();
            _languageToolButton->setText(_languageManager->currentLanguage() == LanguageManager::Language::Chinese
                                             ? QStringLiteral("中文")
                                             : QStringLiteral("English"));
            QEvent languageChangeEvent(QEvent::LanguageChange);
            QCoreApplication::sendEvent(this, &languageChangeEvent);
            QCoreApplication::sendEvent(_inspectorPanel, &languageChangeEvent);
            QCoreApplication::sendEvent(_editorWidget, &languageChangeEvent);
        });
    }

    retranslateUi();
    populateNodeLibrary();
    updateProblemsPanel();
    updateLanguageActions();
    updateWorkbenchActionStates();
}

MainWindow::~MainWindow()
{
    if (_editorWidget != nullptr)
        disconnect(_editorWidget, nullptr, this, nullptr);
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();

    QMainWindow::changeEvent(event);
}

void MainWindow::addNodeFromType(QString const &typeKey)
{
    _editorWidget->createNode(typeKey);
}

bool MainWindow::saveWorkflowToPath(QString const &filePath)
{
    const bool saved = _editorWidget->saveWorkflow(filePath);
    if (saved) {
        _currentWorkflowPath = filePath;
        _editorWidget->markCurrentStateClean();
        clearDirty();
        addToRecentFiles(filePath);
        statusBar()->showMessage(tr("Saved %1").arg(filePath));
    }

    return saved;
}

bool MainWindow::loadWorkflowFromPath(QString const &filePath)
{
    const bool loaded = _editorWidget->loadWorkflow(filePath);
    if (loaded) {
        _currentWorkflowPath = filePath;
        _editorWidget->markCurrentStateClean();
        clearDirty();
        addToRecentFiles(filePath);
        updateProblemsPanel();
        statusBar()->showMessage(tr("Loaded %1").arg(filePath));
    }

    return loaded;
}

void MainWindow::updateWorkbenchActionStates()
{
    if (_editorWidget == nullptr)
        return;

    _undoAction->setEnabled(_editorWidget->canUndo());
    _redoAction->setEnabled(_editorWidget->canRedo());
    _copyAction->setEnabled(_editorWidget->hasSelection());
    _duplicateAction->setEnabled(_editorWidget->hasSelection());
    _pasteAction->setEnabled(_editorWidget->canPaste());
    _deleteAction->setEnabled(_editorWidget->hasSelection());
    _selectAllAction->setEnabled(_editorWidget->hasNodes());
    _centerAction->setEnabled(_editorWidget->hasNodes());
    _fitWorkflowAction->setEnabled(_editorWidget->hasNodes());
    const int selectedNodeCount = static_cast<int>(_editorWidget->selectedNodeIds().size());
    const bool canAlign = selectedNodeCount >= 2;
    const bool canDistribute = selectedNodeCount >= 3;
    _arrangeMenu->setEnabled(canAlign || canDistribute);
    _alignLeftAction->setEnabled(canAlign);
    _alignRightAction->setEnabled(canAlign);
    _alignTopAction->setEnabled(canAlign);
    _alignBottomAction->setEnabled(canAlign);
    _distributeHorizontalAction->setEnabled(canDistribute);
    _distributeVerticalAction->setEnabled(canDistribute);
    const bool hasNodes = _editorWidget->hasNodes();
    _exportMenu->setEnabled(hasNodes);
    _exportLangChainAction->setEnabled(hasNodes);
    _exportPythonAction->setEnabled(hasNodes);
    _exportLangGraphAction->setEnabled(hasNodes);
    _exportCrewAIAction->setEnabled(hasNodes);
}

void MainWindow::updateProblemsPanel()
{
    if (_editorWidget == nullptr || _problemsTable == nullptr)
        return;

    const QSignalBlocker blocker(_problemsTable);
    int selectedNodeId = static_cast<int>(QtNodes::InvalidNodeId);
    QString selectedPropertyKey;
    QString selectedMessage;
    const int selectedRow = _problemsTable->currentRow();
    if (selectedRow >= 0) {
        if (auto *levelItem = _problemsTable->item(selectedRow, 0); levelItem != nullptr) {
            selectedNodeId = levelItem->data(Qt::UserRole).toInt();
            selectedPropertyKey = levelItem->data(Qt::UserRole + 1).toString();
        }
        if (auto *messageItem = _problemsTable->item(selectedRow, 3); messageItem != nullptr)
            selectedMessage = messageItem->text();
    }

    const auto issues = _editorWidget->validationIssues();
    int warningCount = 0;
    int errorCount = 0;
    for (auto const &issue : issues) {
        if (issue.state == QStringLiteral("error"))
            ++errorCount;
        else if (issue.state == QStringLiteral("warning"))
            ++warningCount;
    }

    if (_problemsDock != nullptr)
        _problemsDock->setWindowTitle(tr("Problems (%1)").arg(issues.size()));
    if (_problemsSummaryLabel != nullptr) {
        _problemsSummaryLabel->setText(
            tr("%1 problems: %2 errors, %3 warnings").arg(issues.size()).arg(errorCount).arg(warningCount));
    }

    const QString filterKey = _problemsFilterComboBox != nullptr
                                  ? _problemsFilterComboBox->currentData().toString()
                                  : QStringLiteral("all");
    QList<QtNodesEditorWidget::ValidationIssue> visibleIssues;
    for (auto const &issue : issues) {
        if (filterKey == QStringLiteral("all") || issue.state == filterKey)
            visibleIssues.push_back(issue);
    }

    _problemsTable->setRowCount(visibleIssues.size());

    int rowToSelect = -1;
    for (int row = 0; row < visibleIssues.size(); ++row) {
        auto const &issue = visibleIssues.at(row);
        auto *levelItem = new QTableWidgetItem(issue.state);
        levelItem->setData(Qt::UserRole, static_cast<int>(issue.nodeId));
        levelItem->setData(Qt::UserRole + 1, issue.propertyKey);

        auto *nodeItem = new QTableWidgetItem(issue.displayName);
        auto *typeItem = new QTableWidgetItem(issue.typeKey);
        auto *messageItem = new QTableWidgetItem(issue.message);

        _problemsTable->setItem(row, 0, levelItem);
        _problemsTable->setItem(row, 1, nodeItem);
        _problemsTable->setItem(row, 2, typeItem);
        _problemsTable->setItem(row, 3, messageItem);

        if (static_cast<int>(issue.nodeId) == selectedNodeId && issue.propertyKey == selectedPropertyKey
            && issue.message == selectedMessage) {
            rowToSelect = row;
        }
    }

    const bool hasVisibleIssues = !visibleIssues.isEmpty();
    _problemsTable->setVisible(hasVisibleIssues);
    if (_problemsEmptyStateLabel != nullptr) {
        _problemsEmptyStateLabel->setText(issues.isEmpty()
                                              ? tr("No workflow problems. The current workflow is ready to save or export.")
                                              : tr("No problems match the current filter."));
        _problemsEmptyStateLabel->setVisible(!hasVisibleIssues);
    }

    if (rowToSelect >= 0) {
        _problemsTable->selectRow(rowToSelect);
        _problemsTable->setCurrentCell(rowToSelect, 0);
    } else {
        _problemsTable->clearSelection();
        _problemsTable->setCurrentCell(-1, -1);
    }

    _problemsTable->resizeRowsToContents();
    _problemsTable->viewport()->update();
}

void MainWindow::activateProblemRow(int row)
{
    if (_editorWidget == nullptr || _problemsTable == nullptr || row < 0 || row >= _problemsTable->rowCount())
        return;

    auto *item = _problemsTable->item(row, 0);
    if (item == nullptr)
        return;

    const auto nodeId = static_cast<QtNodes::NodeId>(item->data(Qt::UserRole).toInt());
    if (nodeId == QtNodes::InvalidNodeId)
        return;

    _editorWidget->selectNode(nodeId);
    _editorWidget->centerSelection();
}

NodeLibraryListWidget *MainWindow::createNodeLibrary()
{
    auto *list = new NodeLibraryListWidget(this);
    list->setObjectName("nodeLibraryList");
    return list;
}

void MainWindow::populateNodeLibrary()
{
    _nodeLibraryList->clear();

    BuiltInNodeRegistry registry;
    auto const definitions = registry.definitions();

    for (auto const &categoryKey : categoryOrder()) {
        QString categoryDisplayName;
        for (auto const &definition : definitions) {
            if (definition.categoryKey == categoryKey) {
                categoryDisplayName = definition.categoryDisplayName;
                break;
            }
        }

        if (categoryDisplayName.isEmpty())
            continue;

        _nodeLibraryList->addSectionHeader(categoryDisplayName, nodeLibraryIcon(categoryKey));

        for (auto const &definition : definitions) {
            if (definition.categoryKey != categoryKey)
                continue;

            _nodeLibraryList->addNodeEntry(definition.typeKey,
                                           definition.displayName,
                                           definition.description,
                                           nodeLibraryIcon(definition.typeKey),
                                           definition.inputPorts.size(),
                                           definition.outputPorts.size());
        }
    }

    if (_nodeLibrarySearchEdit != nullptr)
        _nodeLibraryList->setFilterText(_nodeLibrarySearchEdit->text());
}

void MainWindow::retranslateUi()
{
    _fileMenu->setTitle(tr("File"));
    _editMenu->setTitle(tr("Edit"));
    _viewMenu->setTitle(tr("View"));
    _settingsMenu->setTitle(tr("Settings"));
    _languageMenu->setTitle(tr("Language"));
    _helpMenu->setTitle(tr("Help"));
    _helpAction->setText(tr("User Guide"));
    _helpAction->setShortcut(QKeySequence::keyBindings(QKeySequence::HelpContents).value(0, QKeySequence(Qt::Key_F1)));
    _tabWidget->setTabText(0, tr("Workflow"));

    _newAction->setText(tr("New"));
    _openAction->setText(tr("Open"));
    _saveAction->setText(tr("Save"));
    _saveAsAction->setText(tr("Save As..."));
    _exportMenu->setTitle(tr("Export"));
    _exportLangChainAction->setText(tr("Python (LangChain)"));
    _exportPythonAction->setText(tr("Python Script"));
    _exportLangGraphAction->setText(tr("Python (LangGraph)"));
    _exportCrewAIAction->setText(tr("Python (CrewAI)"));
    _recentFilesMenu->setTitle(tr("Recent Files"));
    _copyAction->setText(tr("Copy"));
    _pasteAction->setText(tr("Paste"));
    _duplicateAction->setText(tr("Duplicate"));
    _deleteAction->setText(tr("Delete"));
    _selectAllAction->setText(tr("Select All"));
    _newAction->setShortcut(QKeySequence::keyBindings(QKeySequence::New).constFirst());
    _openAction->setShortcut(QKeySequence::keyBindings(QKeySequence::Open).constFirst());
    _saveAction->setShortcut(QKeySequence::keyBindings(QKeySequence::Save).constFirst());
    _saveAsAction->setShortcut(QKeySequence::keyBindings(QKeySequence::SaveAs).constFirst());
    _copyAction->setShortcut(QKeySequence::keyBindings(QKeySequence::Copy).constFirst());
    _pasteAction->setShortcut(QKeySequence::keyBindings(QKeySequence::Paste).constFirst());
    _duplicateAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_D));
    _deleteAction->setShortcut(QKeySequence(Qt::Key_Delete));
    _selectAllAction->setShortcut(QKeySequence::keyBindings(QKeySequence::SelectAll).constFirst());
    _undoAction->setText(tr("Undo"));
    _redoAction->setText(tr("Redo"));
    _centerAction->setText(tr("Center"));
    _fitWorkflowAction->setText(tr("Fit Workflow"));
    _arrangeMenu->setTitle(tr("Arrange"));
    _alignLeftAction->setText(tr("Align Left"));
    _alignRightAction->setText(tr("Align Right"));
    _alignTopAction->setText(tr("Align Top"));
    _alignBottomAction->setText(tr("Align Bottom"));
    _distributeHorizontalAction->setText(tr("Distribute Horizontally"));
    _distributeVerticalAction->setText(tr("Distribute Vertically"));
    _undoAction->setShortcut(QKeySequence::keyBindings(QKeySequence::Undo).constFirst());
    _redoAction->setShortcut(QKeySequence::keyBindings(QKeySequence::Redo).constFirst());
    _centerAction->setShortcut(QKeySequence(Qt::Key_Space));
    _fitWorkflowAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0));
    _toggleNodeLibraryAction->setText(tr("Show Node Library"));
    _toggleInspectorAction->setText(tr("Show Inspector"));
    _toggleProblemsAction->setText(tr("Show Problems"));
    _languageToolButton->setToolTip(tr("Language"));
    _languageToolButton->setText(_languageManager != nullptr
                                     && _languageManager->currentLanguage() == LanguageManager::Language::English
                                 ? QStringLiteral("English")
                                 : QStringLiteral("中文"));
    _nodeLibraryDock->setWindowTitle(tr("Node Library"));
    _inspectorDock->setWindowTitle(tr("Inspector"));
    _problemsDock->setWindowTitle(tr("Problems"));
    if (_problemsFilterComboBox != nullptr) {
        const QString currentFilter = _problemsFilterComboBox->currentData().toString();
        const QSignalBlocker blocker(_problemsFilterComboBox);
        _problemsFilterComboBox->clear();
        _problemsFilterComboBox->addItem(tr("All"), QStringLiteral("all"));
        _problemsFilterComboBox->addItem(tr("Errors"), QStringLiteral("error"));
        _problemsFilterComboBox->addItem(tr("Warnings"), QStringLiteral("warning"));
        const int filterIndex = _problemsFilterComboBox->findData(currentFilter.isEmpty() ? QStringLiteral("all") : currentFilter);
        _problemsFilterComboBox->setCurrentIndex(filterIndex >= 0 ? filterIndex : 0);
    }
    if (_problemsTable != nullptr) {
        _problemsTable->setHorizontalHeaderLabels(
            {tr("Level"), tr("Node"), tr("Type"), tr("Problem")});
    }
    if (_nodeLibrarySearchEdit != nullptr)
        _nodeLibrarySearchEdit->setPlaceholderText(
            _languageManager != nullptr && _languageManager->currentLanguage() == LanguageManager::Language::English
                ? QStringLiteral("Search Nodes")
                : QString::fromUtf8("搜索节点"));
    _noSearchResultsLabel->setText(tr("No matching nodes found"));
    statusBar()->showMessage(tr("Ready"));

    _fileMenu->clear();
    _fileMenu->addAction(_newAction);
    _fileMenu->addAction(_openAction);
    _fileMenu->addAction(_saveAction);
    _fileMenu->addAction(_saveAsAction);
    _fileMenu->addSeparator();
    _exportMenu->clear();
    _exportMenu->addAction(_exportLangChainAction);
    _exportMenu->addAction(_exportLangGraphAction);
    _exportMenu->addAction(_exportCrewAIAction);
    _exportMenu->addSeparator();
    _exportMenu->addAction(_exportPythonAction);
    _fileMenu->addMenu(_exportMenu);
    _fileMenu->addSeparator();
    _fileMenu->addMenu(_recentFilesMenu);

    _editMenu->clear();
    _editMenu->addAction(_undoAction);
    _editMenu->addAction(_redoAction);
    _editMenu->addSeparator();
    _editMenu->addAction(_copyAction);
    _editMenu->addAction(_pasteAction);
    _editMenu->addAction(_duplicateAction);
    _editMenu->addSeparator();
    _editMenu->addAction(_selectAllAction);
    _editMenu->addAction(_deleteAction);

    _viewMenu->clear();
    _viewMenu->addAction(_centerAction);
    _viewMenu->addAction(_fitWorkflowAction);
    _viewMenu->addSeparator();
    _arrangeMenu->clear();
    _arrangeMenu->addAction(_alignLeftAction);
    _arrangeMenu->addAction(_alignRightAction);
    _arrangeMenu->addAction(_alignTopAction);
    _arrangeMenu->addAction(_alignBottomAction);
    _arrangeMenu->addSeparator();
    _arrangeMenu->addAction(_distributeHorizontalAction);
    _arrangeMenu->addAction(_distributeVerticalAction);
    _viewMenu->addMenu(_arrangeMenu);
    _viewMenu->addSeparator();
    _viewMenu->addAction(_toggleNodeLibraryAction);
    _viewMenu->addAction(_toggleInspectorAction);
    _viewMenu->addAction(_toggleProblemsAction);

    _settingsMenu->clear();
    _languageMenuAction = _settingsMenu->addMenu(_languageMenu);

    _languageMenu->clear();
    _languageMenu->addAction(_languageChineseAction);
    _languageMenu->addAction(_languageEnglishAction);

    _helpMenu->clear();
    _helpMenu->addAction(_helpAction);

    if (_helpWidget != nullptr) {
        const int helpIndex = _tabWidget->indexOf(_helpWidget);
        if (helpIndex >= 0)
            _tabWidget->setTabText(helpIndex, tr("User Guide"));
        _helpWidget->retranslateUi();
    }

    updateWindowTitle();
    rebuildRecentFilesMenu();
    populateNodeLibrary();
    updateLanguageActions();
    updateSelectionValidationSummary(QString(), QString());
    updateProblemsPanel();
}

void MainWindow::updateLanguageActions()
{
    const bool chineseSelected =
        _languageManager == nullptr || _languageManager->currentLanguage() == LanguageManager::Language::Chinese;
    _languageChineseAction->setChecked(chineseSelected);
    _languageEnglishAction->setChecked(!chineseSelected);
}

bool MainWindow::isDirty() const
{
    return _dirty;
}

QStringList MainWindow::recentFiles() const
{
    QSettings settings;
    return settings.value(QStringLiteral("recentFiles")).toStringList();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave())
        event->accept();
    else
        event->ignore();
}

void MainWindow::markDirty()
{
    if (!_dirty) {
        _dirty = true;
        updateWindowTitle();
    }
}

void MainWindow::clearDirty()
{
    _dirty = false;
    updateWindowTitle();
}

void MainWindow::updateWindowTitle()
{
    QString title = tr("AI Workflow Editor");
    if (!_currentWorkflowPath.isEmpty())
        title = QFileInfo(_currentWorkflowPath).fileName() + QStringLiteral(" - ") + title;
    if (_dirty)
        title = QStringLiteral("* ") + title;
    setWindowTitle(title);
}

void MainWindow::updateSelectionValidationSummary(QString const &state, QString const &message)
{
    if (_selectionValidationSummaryLabel == nullptr)
        return;

    if (_currentSelectedNodeDisplayName.isEmpty()) {
        _selectionValidationSummaryLabel->clear();
        return;
    }

    if (state == QStringLiteral("warning") || state == QStringLiteral("error")) {
        _selectionValidationSummaryLabel->setText(
            tr("%1: %2").arg(_currentSelectedNodeDisplayName, message));
        return;
    }

    _selectionValidationSummaryLabel->setText(
        tr("%1: Passed validation").arg(_currentSelectedNodeDisplayName));
}

bool MainWindow::maybeSave()
{
    if (!_dirty)
        return true;

    const auto result = QMessageBox::warning(this,
                                              tr("Unsaved Changes"),
                                              tr("The current workflow has unsaved changes.\n"
                                                 "Do you want to save before continuing?"),
                                              QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                                              QMessageBox::Save);

    if (result == QMessageBox::Cancel)
        return false;

    if (result == QMessageBox::Save) {
        QString filePath = _currentWorkflowPath;
        if (filePath.isEmpty()) {
            filePath = QFileDialog::getSaveFileName(this,
                                                    tr("Save Workflow"),
                                                    QStringLiteral("workflow.json"),
                                                    tr("Workflow Files (*.json)"));
        }

        if (filePath.isEmpty())
            return false;

        return saveWorkflowToPath(filePath);
    }

    return true;
}

void MainWindow::addToRecentFiles(QString const &filePath)
{
    QSettings settings;
    QStringList files = settings.value(QStringLiteral("recentFiles")).toStringList();
    files.removeAll(filePath);
    files.prepend(filePath);
    while (files.size() > MaxRecentFiles)
        files.removeLast();
    settings.setValue(QStringLiteral("recentFiles"), files);
    rebuildRecentFilesMenu();
}

void MainWindow::rebuildRecentFilesMenu()
{
    _recentFilesMenu->clear();
    const QStringList files = recentFiles();

    for (auto const &filePath : files) {
        const QString label = QFileInfo(filePath).fileName();
        QAction *action = _recentFilesMenu->addAction(label);
        connect(action, &QAction::triggered, this, [this, filePath]() {
            if (!maybeSave())
                return;
            loadWorkflowFromPath(filePath);
        });
    }

    _recentFilesMenu->setEnabled(!files.isEmpty());
}

void MainWindow::exportWorkflow(WorkflowExporter::Format format)
{
    const QString readinessMessage = exportReadinessMessageForCurrentWorkflow();
    if (!readinessMessage.isEmpty()) {
        QMessageBox::warning(this, tr("Export Preflight"), readinessMessage);
        return;
    }

    const auto filePath = QFileDialog::getSaveFileName(
        this,
        tr("Export Workflow"),
        QStringLiteral("workflow.py"),
        WorkflowExporter::formatFileFilter(format));

    if (filePath.isEmpty())
        return;

    const auto workflow = _editorWidget->workflowToJson();
    const auto result = WorkflowExporter::exportWorkflow(workflow, format);

    if (!result.success) {
        QMessageBox::warning(this, tr("Export Failed"), result.errorMessage);
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::warning(this, tr("Export Failed"), tr("Cannot write to file: %1").arg(filePath));
        return;
    }

    file.write(result.code.toUtf8());
    statusBar()->showMessage(tr("Exported to %1").arg(filePath));
}

QString MainWindow::exportReadinessMessageForCurrentWorkflow() const
{
    if (_editorWidget == nullptr || _editorWidget->nodeCount() == 0)
        return tr("No workflow to export.");

    const auto issues = _editorWidget->validationIssues();
    if (issues.isEmpty())
        return {};

    QStringList lines;
    lines << tr("Please fix %1 workflow problems before exporting:").arg(issues.size());
    for (auto const &issue : issues)
        lines << tr("- %1: %2").arg(issue.displayName, issue.message);
    return lines.join(QLatin1Char('\n'));
}

void MainWindow::openHelpTab()
{
    if (_helpWidget == nullptr) {
        _helpWidget = new HelpDocumentWidget(_tabWidget);
        connect(_helpWidget, &QObject::destroyed, this, [this]() { _helpWidget = nullptr; });
        _tabWidget->addClosableTab(_helpWidget, tr("User Guide"));
    } else {
        _tabWidget->activateOrAddTab(_helpWidget, tr("User Guide"));
    }
}

void MainWindow::handleTabChanged(int index)
{
    const bool isCanvas = (index == 0);
    const QSignalBlocker inspectorBlocker(_inspectorDock);
    const QSignalBlocker nodeLibraryBlocker(_nodeLibraryDock);
    const QSignalBlocker problemsBlocker(_problemsDock);

    _inspectorDock->setVisible(isCanvas && _toggleInspectorAction->isChecked());
    _nodeLibraryDock->setVisible(isCanvas && _toggleNodeLibraryAction->isChecked());
    _problemsDock->setVisible(isCanvas && _toggleProblemsAction->isChecked());
    _toggleInspectorAction->setEnabled(isCanvas);
    _toggleNodeLibraryAction->setEnabled(isCanvas);
    _toggleProblemsAction->setEnabled(isCanvas);
}

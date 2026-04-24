#include "app/MainWindow.hpp"

#include "app/NodeLibraryListWidget.hpp"
#include "inspector/InspectorPanel.hpp"
#include "qtnodes/QtNodesEditorWidget.hpp"
#include "registry/BuiltInNodeRegistry.hpp"

#include <QAction>
#include <QCloseEvent>
#include <QColor>
#include <QCoreApplication>
#include <QDockWidget>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QListWidgetItem>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include <QStatusBar>
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
    , _primaryToolBar(nullptr)
    , _nodeLibraryDock(nullptr)
    , _inspectorDock(nullptr)
    , _nodeLibraryPanel(nullptr)
    , _nodeLibraryList(nullptr)
    , _nodeLibrarySearchEdit(nullptr)
    , _inspectorPanel(nullptr)
    , _editorWidget(nullptr)
    , _selectionValidationSummaryLabel(nullptr)
    , _newAction(nullptr)
    , _openAction(nullptr)
    , _saveAction(nullptr)
    , _saveAsAction(nullptr)
    , _deleteAction(nullptr)
    , _selectAllAction(nullptr)
    , _undoAction(nullptr)
    , _redoAction(nullptr)
    , _centerAction(nullptr)
    , _toggleNodeLibraryAction(nullptr)
    , _toggleInspectorAction(nullptr)
    , _languageMenuAction(nullptr)
    , _languageChineseAction(nullptr)
    , _languageEnglishAction(nullptr)
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
    _settingsMenu = menuBar()->addMenu(QString());
    _settingsMenu->setObjectName("settingsMenu");
    _languageMenu = _settingsMenu->addMenu(QString());
    _languageMenu->setObjectName("languageMenu");

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
    _primaryToolBar->addSeparator();

    _toggleNodeLibraryAction = new QAction(this);
    _toggleNodeLibraryAction->setObjectName("toggleNodeLibraryAction");
    _toggleNodeLibraryAction->setCheckable(true);
    _toggleInspectorAction = new QAction(this);
    _toggleInspectorAction->setObjectName("toggleInspectorAction");
    _toggleInspectorAction->setCheckable(true);

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

    _editorWidget = new QtNodesEditorWidget(this);
    setCentralWidget(_editorWidget);
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

    connect(_deleteAction, &QAction::triggered, _editorWidget, &QtNodesEditorWidget::deleteSelection);
    connect(_selectAllAction, &QAction::triggered, _editorWidget, &QtNodesEditorWidget::selectAllNodes);
    connect(_undoAction, &QAction::triggered, _editorWidget, &QtNodesEditorWidget::undo);
    connect(_redoAction, &QAction::triggered, _editorWidget, &QtNodesEditorWidget::redo);
    connect(_centerAction, &QAction::triggered, _editorWidget, &QtNodesEditorWidget::centerSelection);

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
    connect(_nodeLibraryDock, &QDockWidget::visibilityChanged, _toggleNodeLibraryAction, &QAction::setChecked);
    connect(_inspectorDock, &QDockWidget::visibilityChanged, _toggleInspectorAction, &QAction::setChecked);

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
    _deleteAction->setEnabled(_editorWidget->hasSelection());
    _selectAllAction->setEnabled(_editorWidget->hasNodes());
    _centerAction->setEnabled(_editorWidget->hasNodes());
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

    _newAction->setText(tr("New"));
    _openAction->setText(tr("Open"));
    _saveAction->setText(tr("Save"));
    _saveAsAction->setText(tr("Save As..."));
    _recentFilesMenu->setTitle(tr("Recent Files"));
    _deleteAction->setText(tr("Delete"));
    _selectAllAction->setText(tr("Select All"));
    _newAction->setShortcut(QKeySequence::keyBindings(QKeySequence::New).constFirst());
    _openAction->setShortcut(QKeySequence::keyBindings(QKeySequence::Open).constFirst());
    _saveAction->setShortcut(QKeySequence::keyBindings(QKeySequence::Save).constFirst());
    _saveAsAction->setShortcut(QKeySequence::keyBindings(QKeySequence::SaveAs).constFirst());
    _deleteAction->setShortcut(QKeySequence(Qt::Key_Delete));
    _selectAllAction->setShortcut(QKeySequence::keyBindings(QKeySequence::SelectAll).constFirst());
    _undoAction->setText(tr("Undo"));
    _redoAction->setText(tr("Redo"));
    _centerAction->setText(tr("Center"));
    _undoAction->setShortcut(QKeySequence::keyBindings(QKeySequence::Undo).constFirst());
    _redoAction->setShortcut(QKeySequence::keyBindings(QKeySequence::Redo).constFirst());
    _centerAction->setShortcut(QKeySequence(Qt::Key_Space));
    _toggleNodeLibraryAction->setText(tr("Show Node Library"));
    _toggleInspectorAction->setText(tr("Show Inspector"));
    _languageToolButton->setToolTip(tr("Language"));
    _languageToolButton->setText(_languageManager != nullptr
                                     && _languageManager->currentLanguage() == LanguageManager::Language::English
                                 ? QStringLiteral("English")
                                 : QStringLiteral("中文"));
    _nodeLibraryDock->setWindowTitle(tr("Node Library"));
    _inspectorDock->setWindowTitle(tr("Inspector"));
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
    _fileMenu->addMenu(_recentFilesMenu);

    _editMenu->clear();
    _editMenu->addAction(_undoAction);
    _editMenu->addAction(_redoAction);
    _editMenu->addSeparator();
    _editMenu->addAction(_selectAllAction);
    _editMenu->addAction(_deleteAction);

    _viewMenu->clear();
    _viewMenu->addAction(_centerAction);
    _viewMenu->addSeparator();
    _viewMenu->addAction(_toggleNodeLibraryAction);
    _viewMenu->addAction(_toggleInspectorAction);

    _settingsMenu->clear();
    _languageMenuAction = _settingsMenu->addMenu(_languageMenu);

    _languageMenu->clear();
    _languageMenu->addAction(_languageChineseAction);
    _languageMenu->addAction(_languageEnglishAction);

    updateWindowTitle();
    rebuildRecentFilesMenu();
    populateNodeLibrary();
    updateLanguageActions();
    updateSelectionValidationSummary(QString(), QString());
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

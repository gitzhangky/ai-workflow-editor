#pragma once

#include "app/LanguageManager.hpp"
#include "export/WorkflowExporter.hpp"

#include <QMainWindow>
#include <QStringList>

class QAction;
class QCloseEvent;
class InspectorPanel;
class NodeLibraryListWidget;
class QDockWidget;
class QEvent;
class QLabel;
class QLineEdit;
class QMenu;
class QToolBar;
class QToolButton;
class QWidget;
class QtNodesEditorWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(LanguageManager *languageManager, QWidget *parent = nullptr);
    ~MainWindow() override;
    void addNodeFromType(QString const &typeKey);
    bool saveWorkflowToPath(QString const &filePath);
    bool loadWorkflowFromPath(QString const &filePath);
    bool isDirty() const;
    QStringList recentFiles() const;

protected:
    void changeEvent(QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private:
    NodeLibraryListWidget *createNodeLibrary();
    void populateNodeLibrary();
    void retranslateUi();
    void updateLanguageActions();
    void markDirty();
    void clearDirty();
    void updateWindowTitle();
    void updateWorkbenchActionStates();
    void updateSelectionValidationSummary(QString const &state, QString const &message);
    bool maybeSave();
    void addToRecentFiles(QString const &filePath);
    void rebuildRecentFilesMenu();
    void exportWorkflow(WorkflowExporter::Format format);

    LanguageManager *_languageManager;
    QMenu *_fileMenu;
    QMenu *_editMenu;
    QMenu *_viewMenu;
    QMenu *_settingsMenu;
    QMenu *_languageMenu;
    QMenu *_recentFilesMenu;
    QToolBar *_primaryToolBar;
    QDockWidget *_nodeLibraryDock;
    QDockWidget *_inspectorDock;
    QWidget *_nodeLibraryPanel;
    NodeLibraryListWidget *_nodeLibraryList;
    QLineEdit *_nodeLibrarySearchEdit;
    QLabel *_noSearchResultsLabel;
    InspectorPanel *_inspectorPanel;
    QtNodesEditorWidget *_editorWidget;
    QLabel *_selectionValidationSummaryLabel;
    QLabel *_zoomIndicatorLabel;
    QAction *_newAction;
    QAction *_openAction;
    QAction *_saveAction;
    QAction *_saveAsAction;
    QMenu *_exportMenu;
    QAction *_exportLangChainAction;
    QAction *_exportPythonAction;
    QAction *_exportLangGraphAction;
    QAction *_exportCrewAIAction;
    QAction *_copyAction;
    QAction *_pasteAction;
    QAction *_duplicateAction;
    QAction *_deleteAction;
    QAction *_selectAllAction;
    QAction *_undoAction;
    QAction *_redoAction;
    QAction *_centerAction;
    QAction *_fitWorkflowAction;
    QAction *_toggleNodeLibraryAction;
    QAction *_toggleInspectorAction;
    QAction *_languageMenuAction;
    QAction *_languageChineseAction;
    QAction *_languageEnglishAction;
    QToolButton *_languageToolButton;
    QString _currentWorkflowPath;
    QString _currentSelectedNodeDisplayName;
    bool _dirty;
    static constexpr int MaxRecentFiles = 10;
};

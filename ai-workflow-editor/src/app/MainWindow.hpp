#pragma once

#include "app/LanguageManager.hpp"

#include <QMainWindow>

class QAction;
class InspectorPanel;
class NodeLibraryListWidget;
class QDockWidget;
class QEvent;
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
    void addNodeFromType(QString const &typeKey);
    bool saveWorkflowToPath(QString const &filePath);
    bool loadWorkflowFromPath(QString const &filePath);

protected:
    void changeEvent(QEvent *event) override;

private:
    NodeLibraryListWidget *createNodeLibrary();
    void populateNodeLibrary();
    void retranslateUi();
    void updateLanguageActions();

    LanguageManager *_languageManager;
    QMenu *_fileMenu;
    QMenu *_editMenu;
    QMenu *_viewMenu;
    QMenu *_settingsMenu;
    QMenu *_languageMenu;
    QToolBar *_primaryToolBar;
    QDockWidget *_nodeLibraryDock;
    QDockWidget *_inspectorDock;
    QWidget *_nodeLibraryPanel;
    NodeLibraryListWidget *_nodeLibraryList;
    QLineEdit *_nodeLibrarySearchEdit;
    InspectorPanel *_inspectorPanel;
    QtNodesEditorWidget *_editorWidget;
    QAction *_newAction;
    QAction *_openAction;
    QAction *_saveAction;
    QAction *_undoAction;
    QAction *_redoAction;
    QAction *_centerAction;
    QAction *_toggleNodeLibraryAction;
    QAction *_toggleInspectorAction;
    QAction *_languageMenuAction;
    QAction *_languageChineseAction;
    QAction *_languageEnglishAction;
    QToolButton *_languageToolButton;
    QString _currentWorkflowPath;
};

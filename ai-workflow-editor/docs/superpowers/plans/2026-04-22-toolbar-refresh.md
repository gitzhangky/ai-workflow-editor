# Toolbar Refresh Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Refresh the main workbench toolbar so it feels lighter, grouped, and more aligned with the existing AI workflow editor workbench.

**Architecture:** Keep the implementation inside the application shell and stylesheet layer. Use `MainWindow.cpp` to regroup toolbar actions and attach stable object names/properties, and use `workbench.qss` to restyle the toolbar into a quieter grouped control rail with a restrained save emphasis and a distinct language entry.

**Tech Stack:** Qt Widgets, C++, QToolBar/QToolButton, Qt stylesheets, Qt Test, CMake

---

### Task 1: Lock In Toolbar Grouping Expectations

**Files:**
- Modify: `ai-workflow-editor/tests/app/MainWindowTests.cpp`
- Test: `ai-workflow-editor/tests/app/MainWindowTests.cpp`

- [ ] **Step 1: Write the failing test**

```cpp
void MainWindowTests::showsGroupedToolbarLayout()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *toolbar = window.findChild<QToolBar *>("primaryToolBar");
    auto *saveAction = window.findChild<QAction *>("saveAction");
    auto *deleteAction = window.findChild<QAction *>("deleteAction");
    auto *selectAllAction = window.findChild<QAction *>("selectAllAction");
    QVERIFY(toolbar != nullptr);
    QVERIFY(saveAction != nullptr);
    QVERIFY(deleteAction != nullptr);
    QVERIFY(selectAllAction != nullptr);

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
}
```

- [ ] **Step 2: Run test to verify it fails**

Run:

```bash
ctest --test-dir /Users/zhangkaiyuan/Documents/Codex/2026-04-21-github-qt-c-nodeeditor/ai-workflow-editor/build --output-on-failure -R ai_workflow_editor_app_tests
```

Expected: `MainWindowTests::showsGroupedToolbarLayout()` fails because `deleteAction` and `selectAllAction` are not on the toolbar and the action count/order does not match.

- [ ] **Step 3: Write minimal implementation**

```cpp
_newAction = _primaryToolBar->addAction(toolbarIcon(QStringLiteral("new")), QString());
_openAction = _primaryToolBar->addAction(toolbarIcon(QStringLiteral("open")), QString());
_saveAction = _primaryToolBar->addAction(toolbarIcon(QStringLiteral("save")), QString());
_primaryToolBar->addSeparator();
_undoAction = _primaryToolBar->addAction(toolbarIcon(QStringLiteral("undo")), QString());
_redoAction = _primaryToolBar->addAction(toolbarIcon(QStringLiteral("redo")), QString());
_deleteAction = _primaryToolBar->addAction(toolbarIcon(QStringLiteral("delete")), QString());
_primaryToolBar->addSeparator();
_selectAllAction = _primaryToolBar->addAction(toolbarIcon(QStringLiteral("select-all")), QString());
_centerAction = _primaryToolBar->addAction(toolbarIcon(QStringLiteral("center")), QString());
_primaryToolBar->addSeparator();
_primaryToolBar->addWidget(_languageToolButton);
```

- [ ] **Step 4: Run test to verify it passes**

Run:

```bash
ctest --test-dir /Users/zhangkaiyuan/Documents/Codex/2026-04-21-github-qt-c-nodeeditor/ai-workflow-editor/build --output-on-failure -R ai_workflow_editor_app_tests
```

Expected: the new grouping test passes and no existing toolbar structure tests regress.

- [ ] **Step 5: Commit**

```bash
git -C /Users/zhangkaiyuan/Documents/Codex/2026-04-21-github-qt-c-nodeeditor add ai-workflow-editor/tests/app/MainWindowTests.cpp ai-workflow-editor/src/app/MainWindow.cpp
git -C /Users/zhangkaiyuan/Documents/Codex/2026-04-21-github-qt-c-nodeeditor commit -m "test: lock toolbar grouping layout"
```

### Task 2: Lock In Toolbar Styling Hooks

**Files:**
- Modify: `ai-workflow-editor/tests/app/MainWindowTests.cpp`
- Modify: `ai-workflow-editor/src/app/MainWindow.cpp`
- Test: `ai-workflow-editor/tests/app/MainWindowTests.cpp`

- [ ] **Step 1: Write the failing test**

```cpp
void MainWindowTests::exposesToolbarStylingHooks()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *toolbar = window.findChild<QToolBar *>("primaryToolBar");
    auto *languageButton = window.findChild<QToolButton *>("languageToolButton");
    QVERIFY(toolbar != nullptr);
    QVERIFY(languageButton != nullptr);

    QCOMPARE(toolbar->property("variant").toString(), QString("workbench"));
    QCOMPARE(window.findChild<QAction *>("saveAction")->property("emphasis").toString(), QString("primary"));
    QCOMPARE(languageButton->property("variant").toString(), QString("toolbar-language"));
}
```

- [ ] **Step 2: Run test to verify it fails**

Run:

```bash
ctest --test-dir /Users/zhangkaiyuan/Documents/Codex/2026-04-21-github-qt-c-nodeeditor/ai-workflow-editor/build --output-on-failure -R ai_workflow_editor_app_tests
```

Expected: the test fails because the toolbar, save action, and language button do not expose the styling properties yet.

- [ ] **Step 3: Write minimal implementation**

```cpp
_primaryToolBar->setProperty("variant", QStringLiteral("workbench"));
_saveAction->setProperty("emphasis", QStringLiteral("primary"));
_languageToolButton->setProperty("variant", QStringLiteral("toolbar-language"));
```

- [ ] **Step 4: Run test to verify it passes**

Run:

```bash
ctest --test-dir /Users/zhangkaiyuan/Documents/Codex/2026-04-21-github-qt-c-nodeeditor/ai-workflow-editor/build --output-on-failure -R ai_workflow_editor_app_tests
```

Expected: the new styling-hook test passes.

- [ ] **Step 5: Commit**

```bash
git -C /Users/zhangkaiyuan/Documents/Codex/2026-04-21-github-qt-c-nodeeditor add ai-workflow-editor/tests/app/MainWindowTests.cpp ai-workflow-editor/src/app/MainWindow.cpp
git -C /Users/zhangkaiyuan/Documents/Codex/2026-04-21-github-qt-c-nodeeditor commit -m "test: add toolbar styling hooks"
```

### Task 3: Restyle Toolbar Into Grouped Workbench Rail

**Files:**
- Modify: `ai-workflow-editor/src/resources/styles/workbench.qss`
- Test: `ai-workflow-editor/tests/app/MainWindowTests.cpp`

- [ ] **Step 1: Write the failing test**

```cpp
void MainWindowTests::usesGroupedWorkbenchToolbarStyling()
{
    LanguageManager languageManager;
    MainWindow window(&languageManager);

    auto *toolbar = window.findChild<QToolBar *>("primaryToolBar");
    auto *languageButton = window.findChild<QToolButton *>("languageToolButton");
    QVERIFY(toolbar != nullptr);
    QVERIFY(languageButton != nullptr);

    QVERIFY(toolbar->styleSheet().isEmpty());
    QCOMPARE(toolbar->property("variant").toString(), QString("workbench"));
    QCOMPARE(languageButton->property("variant").toString(), QString("toolbar-language"));
}
```

- [ ] **Step 2: Run test to verify it fails**

Run:

```bash
ctest --test-dir /Users/zhangkaiyuan/Documents/Codex/2026-04-21-github-qt-c-nodeeditor/ai-workflow-editor/build --output-on-failure -R ai_workflow_editor_app_tests
```

Expected: this stays red until Task 2 is complete; after Task 2 it serves as a regression guard while styles are updated.

- [ ] **Step 3: Write minimal implementation**

```css
QToolBar#primaryToolBar[variant="workbench"] {
    background: #fbf8f2;
    border: none;
    border-bottom: 1px solid #ddd5c9;
    padding: 8px 16px;
    spacing: 6px;
}

QToolBar#primaryToolBar[variant="workbench"] QToolButton {
    background: transparent;
    border: 1px solid transparent;
    border-radius: 8px;
    min-height: 30px;
    padding: 0 12px;
}

QToolBar#primaryToolBar[variant="workbench"] QToolButton:hover {
    background: #f3ede2;
    border-color: #d8ccba;
}

QToolBar#primaryToolBar[variant="workbench"] QToolButton[emphasis="primary"] {
    background: #efe6d7;
    border-color: #d7c7ae;
}

QToolButton#languageToolButton[variant="toolbar-language"] {
    background: #f5efe5;
    border: 1px solid #ddd1c0;
    border-radius: 9px;
    padding: 0 10px;
}
```

- [ ] **Step 4: Run test to verify it passes**

Run:

```bash
ctest --test-dir /Users/zhangkaiyuan/Documents/Codex/2026-04-21-github-qt-c-nodeeditor/ai-workflow-editor/build --output-on-failure -R ai_workflow_editor_app_tests
```

Expected: toolbar-focused app tests pass while the visual behavior is now driven by the new grouped toolbar selectors.

- [ ] **Step 5: Commit**

```bash
git -C /Users/zhangkaiyuan/Documents/Codex/2026-04-21-github-qt-c-nodeeditor add ai-workflow-editor/src/resources/styles/workbench.qss
git -C /Users/zhangkaiyuan/Documents/Codex/2026-04-21-github-qt-c-nodeeditor commit -m "style: refresh workbench toolbar"
```

### Task 4: Full Verification

**Files:**
- Modify: none
- Test: `ai-workflow-editor/tests/app/MainWindowTests.cpp`

- [ ] **Step 1: Run the required build**

Run:

```bash
cmake --build /Users/zhangkaiyuan/Documents/Codex/2026-04-21-github-qt-c-nodeeditor/ai-workflow-editor/build --target ai_workflow_editor_domain_tests ai_workflow_editor_app_tests ai_workflow_editor_theme_tests ai_workflow_editor_widget_tests ai-workflow-editor
```

Expected: build exits with code `0`.

- [ ] **Step 2: Run the required full test suite**

Run:

```bash
ctest --test-dir /Users/zhangkaiyuan/Documents/Codex/2026-04-21-github-qt-c-nodeeditor/ai-workflow-editor/build --output-on-failure
```

Expected: all test suites pass.

- [ ] **Step 3: Review the toolbar against the spec**

Check:

```text
- grouped layout is visible
- save has restrained emphasis
- language button feels distinct
- toolbar no longer reads like a row of isolated beige cards
```

- [ ] **Step 4: Commit**

```bash
git -C /Users/zhangkaiyuan/Documents/Codex/2026-04-21-github-qt-c-nodeeditor commit -am "feat: refresh main toolbar styling"
```

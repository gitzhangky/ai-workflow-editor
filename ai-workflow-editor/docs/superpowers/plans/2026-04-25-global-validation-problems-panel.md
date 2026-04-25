# Global Validation Problems Panel Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a bottom Problems panel that lists every current workflow warning/error and lets users jump directly to the affected node.

**Architecture:** Reuse the existing `QtNodesEditorWidget::validationResultFor()` path so node cards, Inspector feedback, status bar summaries, and the Problems panel all report the same validation state. Keep UI ownership in `MainWindow`; keep graph validation data in the QtNodes adapter.

**Tech Stack:** Qt Widgets, QtNodes adapter layer, Qt Test, CMake

---

### Task 1: Editor Validation Issue API

**Files:**
- Modify: `ai-workflow-editor/src/qtnodes/QtNodesEditorWidget.hpp`
- Modify: `ai-workflow-editor/src/qtnodes/QtNodesEditorWidget.cpp`
- Test: `ai-workflow-editor/tests/app/MainWindowTests.cpp`

- [ ] Add a failing test that creates invalid `prompt` and `output` nodes and expects `editor->validationIssues()` to return warning entries with node id, display name, type key, message, and property key.
- [ ] Implement a public `ValidationIssue` struct and `QList<ValidationIssue> validationIssues() const`.
- [ ] Run the focused test and confirm it passes.

### Task 2: Problems Dock UI

**Files:**
- Modify: `ai-workflow-editor/src/app/MainWindow.hpp`
- Modify: `ai-workflow-editor/src/app/MainWindow.cpp`
- Modify: `ai-workflow-editor/src/resources/styles/workbench.qss`
- Test: `ai-workflow-editor/tests/app/MainWindowTests.cpp`

- [ ] Add a failing test that finds `problemsDock` and `problemsTable`, then verifies invalid workflow nodes appear as rows.
- [ ] Create a bottom `QDockWidget` titled `Problems` containing a read-only `QTableWidget`.
- [ ] Refresh rows after workflow changes, selection changes, load, undo, redo, and Inspector edits.
- [ ] Run the focused test and confirm it passes.

### Task 3: Click-To-Fix Navigation

**Files:**
- Modify: `ai-workflow-editor/src/app/MainWindow.cpp`
- Modify: `ai-workflow-editor/src/qtnodes/QtNodesEditorWidget.hpp`
- Modify: `ai-workflow-editor/src/qtnodes/QtNodesEditorWidget.cpp`
- Test: `ai-workflow-editor/tests/app/MainWindowTests.cpp`

- [ ] Add a failing test that clicks a problem row and expects the node to become selected, centered, and reflected in the Inspector.
- [ ] Store node ids and property keys on table items using `Qt::UserRole`.
- [ ] On row activation, call `selectNode()` and `centerSelection()`.
- [ ] Run the focused test and confirm it passes.

### Task 4: Documentation, I18n, Full Verification

**Files:**
- Modify: `ai-workflow-editor/docs/user-guide.md`
- Modify: `ai-workflow-editor/src/app/HelpDocumentWidget.cpp`
- Modify: `ai-workflow-editor/src/i18n/ai_workflow_editor_zh_CN.ts`
- Test: `ai-workflow-editor/tests/app/MainWindowTests.cpp`

- [ ] Add help text for the Problems panel and assert it appears in the in-app guide test.
- [ ] Add Chinese translations for new menu/dock/table labels.
- [ ] Run `cmake --build build -j4`.
- [ ] Run `ctest --test-dir build --output-on-failure`.
- [ ] Commit and push after verification.

# Document Diagnostics Export Preflight Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Extract a focused workflow document boundary, upgrade workflow diagnostics, and prevent exporting workflows with unresolved validation issues.

**Architecture:** Keep QtNodes and graphics behavior inside `QtNodesEditorWidget`, but move workflow JSON document records into `src/workflow/WorkflowDocument.*`. Keep diagnostics UI in `MainWindow` because it owns docks/actions. Export preflight reads the same validation issues as the Problems panel so all user-facing surfaces agree.

**Tech Stack:** C++17, Qt Widgets, Qt Test, CMake, JSON persistence

---

### Task 1: WorkflowDocument Boundary

**Files:**
- Create: `src/workflow/WorkflowDocument.hpp`
- Create: `src/workflow/WorkflowDocument.cpp`
- Test: `tests/workflow/WorkflowDocumentTests.cpp`
- Modify: `CMakeLists.txt`
- Modify: `tests/CMakeLists.txt`
- Modify: `src/qtnodes/QtNodesEditorWidget.hpp`
- Modify: `src/qtnodes/QtNodesEditorWidget.cpp`

- [ ] Write `WorkflowDocumentTests` that parse a v2 workflow JSON object with one node and one connection.
- [ ] Run `cmake --build build --target ai_workflow_editor_workflow_tests -j4 && ./build/tests/ai_workflow_editor_workflow_tests` and verify it fails because `WorkflowDocument` does not exist.
- [ ] Implement `WorkflowDocument::fromJson`, `WorkflowDocument::toJson`, record structs, and error handling.
- [ ] Replace inline save/load parsing in `QtNodesEditorWidget` with `WorkflowDocument`.
- [ ] Run workflow tests plus focused save/load app tests.

### Task 2: Problems Panel Diagnostics

**Files:**
- Modify: `src/app/MainWindow.hpp`
- Modify: `src/app/MainWindow.cpp`
- Modify: `src/resources/styles/workbench.qss`
- Modify: `tests/app/MainWindowTests.cpp`
- Modify: `src/i18n/ai_workflow_editor_zh_CN.ts`

- [ ] Add tests for Problems title count, warning/error filter, and empty state.
- [ ] Run focused app tests and verify they fail before implementation.
- [ ] Add a filter combo box and empty-state label above the existing table.
- [ ] Keep click/double-click activation behavior and preserve selection when visible issues remain.
- [ ] Update translations and styles.
- [ ] Run focused app tests.

### Task 3: Export Preflight

**Files:**
- Modify: `src/app/MainWindow.hpp`
- Modify: `src/app/MainWindow.cpp`
- Modify: `tests/app/MainWindowTests.cpp`
- Modify: `src/i18n/ai_workflow_editor_zh_CN.ts`
- Modify: `docs/user-guide.md`

- [ ] Add tests for export readiness message when workflow is empty, invalid, and valid.
- [ ] Run focused app tests and verify they fail before implementation.
- [ ] Add `MainWindow::exportReadinessMessageForCurrentWorkflow()` and call it before opening the export file dialog.
- [ ] Reuse Problems panel validation issue text in the preflight message.
- [ ] Update user guide to mention fixing Problems before export.
- [ ] Run focused app tests.

### Task 4: Final Verification

**Files:**
- Verify all touched files.

- [ ] Run `cmake --build build -j4`.
- [ ] Run `ctest --test-dir build --output-on-failure`.
- [ ] Commit and push if verification passes.

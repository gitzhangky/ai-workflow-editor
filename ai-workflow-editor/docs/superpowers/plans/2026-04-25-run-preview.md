# Run Preview Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a local mock Run Preview so users can test a workflow and inspect each node's input/output without turning the app into a model runtime.

**Architecture:** Add a deterministic `WorkflowRunner` in `src/runtime/` that consumes saved workflow JSON and returns trace records. Add a `RunPreviewWidget` in `src/app/` for input, run button, trace table, and final output. `MainWindow` opens the widget as a central tab and passes the current workflow JSON before running.

**Tech Stack:** C++17, Qt Widgets, Qt Test, CMake

---

### Task 1: Runtime Core

**Files:**
- Create: `src/runtime/WorkflowRunner.hpp`
- Create: `src/runtime/WorkflowRunner.cpp`
- Create: `tests/runtime/WorkflowRunnerTests.cpp`
- Modify: `CMakeLists.txt`
- Modify: `tests/CMakeLists.txt`

- [x] Write a failing test for `start -> prompt -> llm -> output`.
- [x] Implement deterministic local execution with trace records.
- [x] Run `ai_workflow_editor_runtime_tests`.

### Task 2: Run Preview Tab

**Files:**
- Create: `src/app/RunPreviewWidget.hpp`
- Create: `src/app/RunPreviewWidget.cpp`
- Modify: `src/app/MainWindow.hpp`
- Modify: `src/app/MainWindow.cpp`
- Modify: `tests/app/MainWindowTests.cpp`

- [x] Write failing app tests for opening the Run Preview tab and producing output.
- [x] Add toolbar/menu action `runPreviewAction`.
- [x] Add the central Run Preview tab with input editor, run button, trace table, and output editor.
- [x] Run focused app tests.

### Task 3: Docs And Verification

**Files:**
- Modify: `docs/user-guide.md`
- Modify: `src/app/HelpDocumentWidget.cpp`
- Modify: `src/i18n/ai_workflow_editor_zh_CN.ts`
- Modify: `src/resources/styles/workbench.qss`

- [x] Document mock run preview scope and limits.
- [x] Add Chinese translations for new UI strings.
- [x] Add Run Preview styling consistent with the workbench.
- [x] Run `cmake --build build -j4`.
- [x] Run `ctest --test-dir build --output-on-failure`.

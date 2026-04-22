# AI Workflow Editor Claude Code Handoff

## Purpose

This document is the working handoff for continuing development in Claude Code.
It summarizes the current product state, code layout, build commands, compatibility constraints, and the recommended next sequence of work.

The goal is to let a new coding agent pick up the project without reconstructing the whole conversation history.

## Product Direction Lock

This project direction must not drift.

`AI Workflow Editor` is a desktop visual orchestration editor for AI workflows.
It is used to connect and configure prompts, LLMs, tools, conditions, memory, retrieval, and outputs into executable workflow structures.

It is **not**:

- a large model implementation project
- a generic low-code/BPM platform
- a chat client shell
- a plugin marketplace project
- a general-purpose business workflow engine

All future features should be evaluated against this question:

`Does this improve the user's ability to design, inspect, validate, debug, and manage AI workflows?`

If the answer is not clearly yes, the feature is likely out of scope or should be postponed.

## Project Boundaries

- Product app: `ai-workflow-editor/`
- Third-party dependency: `third_party/nodeeditor/`
- Do not place product docs inside `third_party/nodeeditor/`.
- Prefer implementing product behavior in `ai-workflow-editor/src/...` and only touch `third_party/nodeeditor/` if there is no reasonable adapter-layer alternative.

## Current Product State

The app is a standalone Qt Widgets desktop application built on top of `QtNodes`.

Implemented today:

- Standalone CMake project with Qt 5 and Qt 6 compatible `find_package` flow.
- Light-themed workbench shell.
- Top toolbar, menu bar, left node library, center node canvas, right inspector, bottom status bar.
- Built-in node types:
  - `start`
  - `prompt`
  - `llm`
  - `tool`
  - `condition`
  - `output`
- Drag nodes from the left library to the canvas.
- Visual drop preview on the canvas.
- Node selection synced to inspector.
- Type-specific inspector fields for:
  - `prompt`
  - `llm`
  - `tool`
- Structured node property persistence via JSON save/load.
- Basic connection validation.
- Runtime language switching between Chinese and English.
- Default language is Chinese and language preference is persisted.
- Custom node card painter and custom node geometry for improved visual styling.
- Left node library supports:
  - grouped categories
  - collapse/expand
  - search
  - card-style grouped rendering
- Basic automated tests for app shell, theme, node library, language behavior, save/load, and canvas behaviors.

Implemented in Phase 1 (2026-04-22):

- Dirty-state tracking: `workflowModified()` signal from `QtNodesEditorWidget`, `_dirty` flag in `MainWindow`
- Window title shows `*` prefix when unsaved, filename after save
- Close/new/open confirmation dialog when document is dirty (`maybeSave()`)
- Save As action in File menu
- Recent files menu (persisted via QSettings, max 10 entries)
- Chinese translations for all new UI strings

Not implemented yet:

- Workflow execution engine
- Undo/redo wiring to real editing commands
- Context menus and deletion flows
- Rich node validation and inline error presentation
- Search/filter inside the canvas
- Property schemas for all node types
- Plugin system
- Packaging/deployment
- Windows CI

## Important Product Decisions

- This is a product project, not a fork of `QtNodes`.
- The visual direction is a light, professional workbench.
- Default UI language is Chinese.
- More languages may be added later.
- The editor currently focuses on composition and persistence, not execution.
- The long-term role of the product is to orchestrate external AI capabilities, not to implement the models themselves.
- `QVariantMap` is intentionally used for node property storage at this stage because node types are still evolving.

## Code Map

### App Shell

- `src/main.cpp`
  - App bootstrap
  - Creates `LanguageManager`
  - Applies `LightTheme`
  - Shows `MainWindow`

- `src/app/MainWindow.hpp`
- `src/app/MainWindow.cpp`
  - Main workbench shell
  - Menus, toolbar, docks, language menu, file open/save/save-as actions
  - Dirty-state tracking (`markDirty`, `clearDirty`, `updateWindowTitle`)
  - Close/new/open unsaved confirmation (`maybeSave`, `closeEvent`)
  - Recent files menu (`addToRecentFiles`, `rebuildRecentFilesMenu`, persisted in QSettings)
  - Node library creation and population
  - Wires editor and inspector together

- `src/app/LanguageManager.hpp`
- `src/app/LanguageManager.cpp`
  - Default Chinese
  - Runtime language switching
  - Uses embedded `.qm` resources
  - Stores preference in `QSettings`

- `src/app/LightTheme.hpp`
- `src/app/LightTheme.cpp`
  - Applies palette and QSS
  - Initializes `breeze` and `app_theme` resources

### Node Library

- `src/app/NodeLibraryListWidget.hpp`
- `src/app/NodeLibraryListWidget.cpp`
  - Left library widget
  - Drag MIME payload
  - Search filtering
  - Section collapse state
  - Custom delegate drawing for grouped cards

### Domain and Registry

- `src/domain/WorkflowNodeDefinition.hpp`
- `src/domain/WorkflowNodeDefinition.cpp`
  - Node definition data structure

- `src/registry/BuiltInNodeRegistry.hpp`
- `src/registry/BuiltInNodeRegistry.cpp`
  - Built-in node definitions
  - Category names, display names, descriptions, default properties, ports

### Inspector

- `src/inspector/InspectorPanel.hpp`
- `src/inspector/InspectorPanel.cpp`
  - Shared name/description editing
  - Type-specific sections for prompt, llm, tool
  - Runtime retranslation

### QtNodes Adapter Layer

- `src/qtnodes/QtNodesEditorWidget.hpp`
- `src/qtnodes/QtNodesEditorWidget.cpp`
  - Scene/view container
  - Node creation
  - Drag/drop acceptance
  - Save/load workflow JSON
  - Selection propagation
  - Node style application
  - Emits `workflowModified()` on any document mutation

- `src/qtnodes/StaticNodeDelegateModel.hpp`
- `src/qtnodes/StaticNodeDelegateModel.cpp`
  - Delegate model implementation for built-in nodes

- `src/qtnodes/StyledNodePainter.hpp`
- `src/qtnodes/StyledNodePainter.cpp`
  - Custom node card rendering

- `src/qtnodes/EdgeAlignedNodeGeometry.hpp`
- `src/qtnodes/EdgeAlignedNodeGeometry.cpp`
  - Port positioning and edge alignment behavior

### Resources

- `src/resources/styles/workbench.qss`
  - App-wide light workbench styling

- `src/resources/app_theme.qrc`
  - Toolbar icons, node icons, QSS

- `src/i18n/ai_workflow_editor_zh_CN.ts`
  - Chinese translation source

### Tests

- `tests/app/MainWindowTests.cpp`
- `tests/app/LightThemeTests.cpp`
- `tests/app/NodeLibraryListWidgetTests.cpp`
- `tests/domain/BuiltInNodeRegistryTests.cpp`

## Build and Test

From the repository root:

```bash
cmake -S ai-workflow-editor -B ai-workflow-editor/build
cmake --build ai-workflow-editor/build --target ai-workflow-editor
ctest --test-dir ai-workflow-editor/build --output-on-failure
```

To launch:

```bash
./ai-workflow-editor/build/ai-workflow-editor
```

## Compatibility Notes

Current status:

- Static review does not show an obvious source-level blocker for Qt 5.13.2.
- The app has no product-layer macOS-only APIs.
- The app should remain portable to Windows with Qt Widgets.

Compatibility work already done:

- Top-level `cmake_minimum_required` reduced to `3.16`.
- `Qt::Test` is no longer required in non-test builds.

Still important for Windows + Qt 5.13.2:

- Ensure the environment provides:
  - `Qt Core`
  - `Qt Gui`
  - `Qt Widgets`
  - `Qt Svg`
  - `LinguistTools`
- `third_party/nodeeditor` requires OpenGL support.
- `BreezeStyleSheets` is fetched during configure, so the build machine needs `git` and network access unless the dependency is vendored locally.
- Real Windows CI has not been set up yet, so Windows compatibility is not yet continuously verified.

## Known Technical Debt

- `Undo` and `Redo` actions are present in UI but not wired to actual command stacks.
- Save/load currently lives in the editor adapter layer rather than a stronger document model.
- Node properties are only typed at the form level, not via a formal schema engine.
- Left node library visuals have been iterated heavily; if restyling again, keep the custom delegate as the single source of truth.
- There is no explicit document object like `WorkflowDocument` yet. If `MainWindow.cpp` grows past ~600 lines, extract one.
- No node/connection deletion flow yet — users cannot remove items from the canvas.

## Recommended Next Development Order

The next coding agent should continue in this order:

1. Workflow document behavior
2. Node editing affordances
3. Validation and status feedback
4. Node type expansion
5. Cross-platform and release hardening

See the companion plan file:

- `docs/plans/2026-04-22-next-development-plan.md`

## Suggested Rules For The Next Agent

- Keep changes inside `ai-workflow-editor/` unless a dependency change is truly necessary.
- Preserve the light visual language.
- Keep Chinese as the default UI language.
- Run targeted tests after each UI or behavior change.
- Run the full test suite before claiming completion.
- When changing left node library visuals, inspect both:
  - `NodeLibraryListWidget.cpp`
  - `workbench.qss`
- When changing node card visuals, inspect both:
  - `StyledNodePainter.cpp`
  - `EdgeAlignedNodeGeometry.cpp`
- When changing inspector behavior, inspect both:
  - `InspectorPanel.cpp`
  - `QtNodesEditorWidget.cpp`
- Any operation that mutates document state must emit `workflowModified()` from `QtNodesEditorWidget`.
- Add Chinese translations to `ai_workflow_editor_zh_CN.ts` for all new user-facing strings.
- See `CLAUDE.md` at repo root for the full agent guide.

## Best Immediate Next Task

If an agent is going to continue immediately, the best next task is:

`Add delete selected node/connection, right-click context menu, and keyboard shortcuts.`

Why this should go next:

- Users can create nodes and connections but cannot remove them without editing the JSON file.
- This is the most basic editing operation still missing.
- It does not require architectural changes — deletion goes through `DataFlowGraphModel` and `_nodeStates`.

## Verification Baseline

Before starting new work, confirm these still pass:

```bash
cmake -S ai-workflow-editor -B ai-workflow-editor/build
cmake --build ai-workflow-editor/build --target ai_workflow_editor_domain_tests ai_workflow_editor_app_tests ai_workflow_editor_theme_tests ai_workflow_editor_widget_tests ai-workflow-editor
ctest --test-dir ai-workflow-editor/build --output-on-failure
```

Expected current result:

- `4/4` test suites passing
- App test suite includes 30 test functions (23 original + 7 Phase 1 additions)

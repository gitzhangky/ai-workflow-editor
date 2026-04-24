# AI Workflow Editor Claude Code Handoff

## Purpose

This document is the working handoff for continuing development in Claude Code.
It summarizes the current product state, code layout, build commands, compatibility constraints, and the recommended next sequence of work.

The goal is to let a new coding agent pick up the project without reconstructing the whole conversation history.

If the goal is to understand how the current app is supposed to be used, read `docs/user-guide.md` first.

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

- Repository root: `/Users/zhangkaiyuan/Documents/Codex/2026-04-21-github-qt-c-nodeeditor`
- Product project: `/Users/zhangkaiyuan/Documents/Codex/2026-04-21-github-qt-c-nodeeditor/ai-workflow-editor`
- Product app: `ai-workflow-editor/`
- Third-party dependency: `third_party/nodeeditor/`
- Do not place product docs inside `third_party/nodeeditor/`.
- Prefer implementing product behavior in `ai-workflow-editor/src/...` and only touch `third_party/nodeeditor/` if there is no reasonable adapter-layer alternative.

## Current Product State (updated 2026-04-24)

The app is a standalone Qt Widgets desktop application built on top of `QtNodes`.

### Fully implemented

- Standalone CMake project with Qt 5 and Qt 6 compatible `find_package` flow
- Light-themed workbench shell with menus, toolbar, docks, status bar
- **13 built-in node types**: start, prompt, llm, agent, memory, retriever, templateVariables, httpRequest, jsonTransform, tool, condition, chatOutput, output
- **Port data type system**: 5 data types (flow, text, completion, error, http_response) with compatibility constraints — incompatible connections are rejected at the graph model level
- **WorkflowGraphModel**: custom DataFlowGraphModel subclass that overrides `connectionPossible()` with type-aware compatibility checks
- Drag/drop from library to canvas with centered visual preview
- Custom node card painter and edge-aligned port geometry
- Type-specific inspector for all 13 node types via InspectorFieldSchema
- **Copy/paste/duplicate**: Ctrl+C/V/D, Edit menu, right-click context menu; paste places nodes at mouse cursor position
- **JSON save/load** with full property round-trip (file format version 2, accepts v1)
- **Undo/redo**: full QUndoStack covering create, delete, edit, connect, disconnect — commands extracted to `UndoCommands.hpp/.cpp`
- **Dirty-state tracking** aligned with undo stack clean state
- **Close/new/open confirmation** prompts before discarding unsaved work
- **Recent files menu** persisted via QSettings
- **Save As** action
- **Delete selection**: via Edit menu, keyboard shortcuts (Delete/Backspace), right-click context menu
- **Node-level validation**: inline badges, border color changes, inspector field highlighting, status bar summary
- Runtime Chinese/English language switching (default Chinese)
- Grouped node library with search, collapse, port count badges, no-results empty state
- Zoom indicator in status bar
- **Cross-platform CI**: GitHub Actions for Linux/Windows/macOS × Qt 5.15/6.5

### Not implemented yet

- Workflow execution engine
- Workflow export to executable format (LangChain JSON, Python, etc.)
- Canvas internal search/filter
- Node grouping / sub-workflows
- Multi-node batch operations beyond delete
- Plugin system
- Packaging / deployment
- Mini-map

## Important Product Decisions

- This is a product project, not a fork of `QtNodes`.
- The visual direction is a light, professional workbench.
- Default UI language is Chinese.
- The editor currently focuses on composition and persistence, not execution.
- `QVariantMap` is intentionally used for node property storage at this stage because node types are still evolving.
- Port data types are defined in `PortDataTypes.hpp`; the compatibility rule is: `flow` inputs accept any output type, other types require exact match.
- File format version is 2; loading rejects version > 2 and accepts version 1 (backward compatible).

## Code Map

### App Shell

- `src/main.cpp` — bootstrap, creates LanguageManager, applies LightTheme, shows MainWindow
- `src/app/MainWindow.hpp/.cpp` (~695 lines) — menus, toolbar, docks, language menu, file actions, dirty-state, recent files, action enablement, copy/paste/duplicate wiring
- `src/app/LanguageManager.hpp/.cpp` — runtime language switching, persisted preference
- `src/app/LightTheme.hpp/.cpp` — palette and QSS application

### Node Library

- `src/app/NodeLibraryListWidget.hpp/.cpp` — left panel, drag MIME, search filter, section collapse, port count badges, custom delegate

### Domain and Registry

- `src/domain/WorkflowNodeDefinition.hpp/.cpp` — node definition struct with `WorkflowPortDefinition` (id, label, dataTypeId)
- `src/domain/PortDataTypes.hpp` — port data type constants (flow, text, completion, error, http_response) and `areCompatible()` function
- `src/registry/BuiltInNodeRegistry.hpp/.cpp` — 13 built-in node definitions with port type assignments

### Inspector

- `src/inspector/InspectorPanel.hpp/.cpp` (~607 lines) — shared name/description editing, schema-driven fields, field-level validation highlighting
- `src/inspector/InspectorFieldSchema.hpp/.cpp` — inspector field and section schema definitions

### QtNodes Adapter Layer

- `src/qtnodes/QtNodesEditorWidget.hpp/.cpp` (~1711 lines) — scene/view container, node CRUD, drag/drop, save/load (v2), selection, validation, copy/paste, context menu
- `src/qtnodes/UndoCommands.hpp/.cpp` (~258 lines) — 6 QUndoCommand subclasses: NodeCreate, NodeDeleteSelection, ConnectionCreate, NodeDisplayNameEdit, NodeDescriptionEdit, NodePropertyEdit
- `src/qtnodes/WorkflowGraphModel.hpp/.cpp` — DataFlowGraphModel subclass with port type compatibility in `connectionPossible()`
- `src/qtnodes/StaticNodeDelegateModel.hpp/.cpp` — delegate model returning per-port data types
- `src/qtnodes/StyledNodePainter.hpp/.cpp` — custom node card rendering
- `src/qtnodes/EdgeAlignedNodeGeometry.hpp/.cpp` — port positioning

### Resources

- `src/resources/styles/workbench.qss` — light workbench styling
- `src/resources/app_theme.qrc` — toolbar icons, 16 node SVG icons, QSS
- `src/i18n/ai_workflow_editor_zh_CN.ts` — Chinese translations

### Tests

- `tests/app/MainWindowTests.cpp` — ~107 test methods covering full integration
- `tests/app/LightThemeTests.cpp` — 3 tests
- `tests/app/NodeLibraryListWidgetTests.cpp` — 14 tests
- `tests/domain/BuiltInNodeRegistryTests.cpp` — 10 tests
- **4/4 test suites, all passing**

## Build and Test

From the repository root:

```bash
cmake -S ai-workflow-editor -B ai-workflow-editor/build
cmake --build ai-workflow-editor/build
ctest --test-dir ai-workflow-editor/build --output-on-failure
```

To launch:

```bash
./ai-workflow-editor/build/ai-workflow-editor
```

## Compatibility Notes

- Qt 5.15 on macOS is the primary development environment
- CI tests Linux/Windows/macOS with both Qt 5.15 and Qt 6.5
- `third_party/nodeeditor` requires OpenGL support
- `BreezeStyleSheets` is fetched during configure (needs git + network)

## Recommended Next Development Directions

The original 6-phase plan is fully complete. The following directions were identified from an objective assessment:

1. **Workflow export** — allow exporting workflows to executable formats (LangChain JSON, Python code, etc.)
2. **Further QtNodesEditorWidget decomposition** — extract WorkflowDocument abstraction for document model + serialization (still ~1711 lines)
3. **Multi-node operations** — rubber band selection, batch move, alignment
4. **Node grouping / sub-workflows**
5. **Canvas mini-map**

## Rules For The Next Agent

1. Keep changes inside `ai-workflow-editor/` unless a dependency change is truly necessary.
2. Run the full test suite before AND after making changes.
3. When changing inspector behavior, check both `InspectorPanel.cpp` and `QtNodesEditorWidget.cpp`.
4. When changing node card visuals, check both `StyledNodePainter.cpp` and `EdgeAlignedNodeGeometry.cpp`.
5. When changing node library visuals, check both `NodeLibraryListWidget.cpp` and `workbench.qss`.
6. Add Chinese translations to `ai_workflow_editor_zh_CN.ts` for any new user-facing strings.
7. Emit `workflowModified()` from `QtNodesEditorWidget` for any operation that changes document state.
8. Write Qt Test cases for new behavior in the appropriate test file under `tests/`.
9. When adding new port types, update `PortDataTypes.hpp` and the compatibility function.
10. When adding new node types, assign `dataTypeId` to every port in `BuiltInNodeRegistry.cpp`.
11. See `CLAUDE.md` at repo root for the full agent guide.

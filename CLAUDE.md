# AI Workflow Editor — Agent Guide

> This file is the canonical entry point for any AI coding agent (Claude Code, Codex, etc.)
> working in this repository. Read it before doing anything.

## What This Project Is

A standalone Qt Widgets desktop application for visual AI workflow authoring, built on top of the
[QtNodes](https://github.com/paceholder/nodeeditor) graph library. The app lets users compose
prompt/LLM/tool pipelines by dragging nodes onto a canvas, connecting them, and configuring
properties in a side inspector.

## Repository Layout

```
.
├── ai-workflow-editor/          # Product application (all your work goes here)
│   ├── CMakeLists.txt           # Build entry point
│   ├── src/
│   │   ├── main.cpp             # Bootstrap: language, theme, window
│   │   ├── app/                 # Shell: MainWindow, NodeLibrary, LanguageManager, LightTheme
│   │   ├── domain/              # Data: WorkflowNodeDefinition
│   │   ├── inspector/           # Right panel: InspectorPanel (type-specific property forms)
│   │   ├── qtnodes/             # Adapter layer over QtNodes (editor widget, painter, geometry)
│   │   ├── registry/            # BuiltInNodeRegistry (node definitions)
│   │   ├── resources/           # QSS, icons (SVG), QRC
│   │   └── i18n/                # Qt .ts translation source (Chinese ↔ English)
│   ├── tests/                   # Qt Test suites
│   └── docs/                    # Design docs and development plans
├── third_party/nodeeditor/      # Git submodule — upstream QtNodes library (avoid modifying)
└── build/                       # ⛔ CMake build output — NEVER read, commit, or modify
```

## Build & Test

```bash
# Configure (once, or after CMakeLists changes)
cmake -S ai-workflow-editor -B ai-workflow-editor/build

# Build everything
cmake --build ai-workflow-editor/build

# Run all tests (expect 4/4 passing)
ctest --test-dir ai-workflow-editor/build --output-on-failure
```

Target names for selective builds:
- `ai-workflow-editor` — the executable
- `ai_workflow_editor_app_tests` — MainWindow integration tests
- `ai_workflow_editor_domain_tests` — domain/registry unit tests
- `ai_workflow_editor_theme_tests` — theme tests
- `ai_workflow_editor_widget_tests` — node library widget tests

## Key Constraints

- **Default UI language is Chinese.** English is supported via runtime switching.
- **Light theme only.** All visuals use a warm, light professional palette.
- **Keep changes inside `ai-workflow-editor/`.** Only touch `third_party/nodeeditor/` if there
  is no adapter-layer alternative.
- **`build/` directories are generated artifacts.** Never read, search, or commit them.
- **`QVariantMap` for node properties** — intentional at this stage; don't replace with a typed
  schema engine unless the plan explicitly calls for it.
- **Qt 5.15 on macOS** is the primary development environment. CI tests Linux, Windows,
  and macOS with both Qt 5.15 and Qt 6.5.

## Architecture Quick Reference

| Concern | Primary file(s) |
|---|---|
| App shell, menus, toolbar, docks | `src/app/MainWindow.hpp/.cpp` |
| Node canvas, drag/drop, save/load, dirty state signaling | `src/qtnodes/QtNodesEditorWidget.hpp/.cpp` |
| Undo commands | `src/qtnodes/UndoCommands.hpp/.cpp` |
| Graph model with port type constraints | `src/qtnodes/WorkflowGraphModel.hpp/.cpp` |
| Port data type definitions and compatibility | `src/domain/PortDataTypes.hpp` |
| Inspector (property editing) | `src/inspector/InspectorPanel.hpp/.cpp` |
| Inspector field/section schemas | `src/inspector/InspectorFieldSchema.hpp/.cpp` |
| Node definitions (types, ports, defaults, port types) | `src/registry/BuiltInNodeRegistry.hpp/.cpp` |
| Custom node card rendering | `src/qtnodes/StyledNodePainter.hpp/.cpp` |
| Port positioning | `src/qtnodes/EdgeAlignedNodeGeometry.hpp/.cpp` |
| Node library (left panel) | `src/app/NodeLibraryListWidget.hpp/.cpp` |
| Language switching | `src/app/LanguageManager.hpp/.cpp` |
| Theme | `src/app/LightTheme.hpp/.cpp` |
| Translations | `src/i18n/ai_workflow_editor_zh_CN.ts` |
| Stylesheet | `src/resources/styles/workbench.qss` |

## Signal Flow

```
NodeLibrary (drag/double-click)
    → MainWindow::addNodeFromType()
    → QtNodesEditorWidget::createNode()
    → emits workflowModified()        ← MainWindow tracks dirty state
    → emits selectedNodeChanged()     ← InspectorPanel updates fields

InspectorPanel (user edits a field)
    → emits displayNameEdited / descriptionEdited / propertyEdited
    → QtNodesEditorWidget updates NodeState
    → emits workflowModified()        ← MainWindow tracks dirty state
```

## What Has Been Completed

- [x] Main workbench shell with menus, toolbar, docks
- [x] 13 built-in node types: start, prompt, llm, agent, memory, retriever, templateVariables, httpRequest, jsonTransform, tool, condition, chatOutput, output
- [x] Drag/drop from library to canvas with visual preview
- [x] Custom node card painter and edge-aligned port geometry
- [x] Type-specific inspector for all node types via InspectorFieldSchema
- [x] JSON save/load with full property round-trip
- [x] Runtime Chinese/English language switching
- [x] Grouped node library with search and collapse
- [x] **Dirty-state tracking** — window title shows `*` on unsaved changes
- [x] **Close/new/open confirmation** — prompts before discarding unsaved work
- [x] **Recent files menu** — persisted via QSettings, shown in File menu
- [x] **Save As** action
- [x] **Delete selected node/connection** — via Edit menu, keyboard shortcuts (Delete/Backspace), and right-click context menu
- [x] **Right-click context menu** — delete node, delete connection, select all
- [x] **Keyboard shortcuts** — Delete and Backspace to remove selected items
- [x] **Undo/redo** — full QUndoStack with create, delete, edit commands and merge support
- [x] **Node-level validation** — inline badges, border color changes, inspector field highlighting
- [x] **Inspector field schema** — data-driven property field definitions via InspectorFieldSchema
- [x] **Expanded node set** — agent, memory, retriever, templateVariables, httpRequest, jsonTransform, chatOutput with SVG icons
- [x] **Zoom indicator** — real-time zoom percentage in status bar
- [x] **Port count badges** — input→output port counts on library entries
- [x] **No-results empty state** — message when library search has no matches
- [x] **Cross-platform CI** — GitHub Actions for Linux/Windows/macOS with Qt 5.15 and Qt 6.5
- [x] **Copy/paste/duplicate** — Ctrl+C/V/D, Edit menu, right-click context menu
- [x] **Port data types** — flow, text, completion, error, http_response with compatibility constraints
- [x] **WorkflowGraphModel** — custom DataFlowGraphModel subclass enforcing port type compatibility
- [x] **Undo commands extracted** — moved to `UndoCommands.hpp/.cpp` (QtNodesEditorWidget 2005→1711 lines)
- [x] **File format versioning** — save format version 2, accepts v1 files with forward compatibility

## What To Build Next

See `docs/plans/2026-04-22-next-development-plan.md` for full details. The recommended
immediate task sequence (items 1–6 are done):

1. ~~dirty-state tracking~~ ✅
2. ~~close/open unsaved confirmation~~ ✅
3. ~~recent files menu~~ ✅
4. ~~delete selected node and connection~~ ✅
5. ~~validation markers~~ ✅
6. ~~expand built-in node set~~ ✅
7. ~~canvas and library polish~~ ✅
8. ~~cross-platform CI~~ ✅

## Rules For Agents

1. Run the full test suite before AND after making changes.
2. When changing inspector behavior, check both `InspectorPanel.cpp` and `QtNodesEditorWidget.cpp`.
3. When changing node card visuals, check both `StyledNodePainter.cpp` and `EdgeAlignedNodeGeometry.cpp`.
4. When changing node library visuals, check both `NodeLibraryListWidget.cpp` and `workbench.qss`.
5. Add Chinese translations to `ai_workflow_editor_zh_CN.ts` for any new user-facing strings.
6. Emit `workflowModified()` from `QtNodesEditorWidget` for any operation that changes document state.
7. Write Qt Test cases for new behavior in the appropriate test file under `tests/`.
8. Keep `MainWindow.cpp` from growing too large — if it exceeds ~600 lines, consider extracting
   a `WorkflowDocument` abstraction.

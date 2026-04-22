# AI Workflow Editor Next Development Plan

> **For agentic workers:** Use this as the primary sequencing guide for the next implementation wave. Prefer small, test-backed increments.

**Goal:** Continue the editor from a polished visual prototype into a more reliable product-grade workflow authoring tool.

**Architecture:** Keep the current product boundary: `ai-workflow-editor` owns application logic and visual behavior, while `third_party/nodeeditor` remains a dependency. Favor improving the adapter and app layers before touching the third-party library.

**Tech Stack:** CMake, Qt Widgets, QtNodes, Qt translation resources, JSON persistence, Qt Test

---

## Current Baseline

Already working:

- main workbench shell
- node library
- drag/drop into canvas
- custom node cards
- type-specific inspector fields for prompt, llm, tool
- JSON save/load
- runtime Chinese/English switching
- grouped library search and collapse

Main gap:

The app is visually strong enough to demo, but still weak in document lifecycle, editing reliability, validation feedback, and production workflow ergonomics.

## Phase 1: Document Lifecycle

### Objective

Make the editor safe to use for real workflow authoring sessions.

### Scope

- add dirty-state tracking
- mark the window title when unsaved changes exist
- prompt before closing unsaved work
- prompt before opening another workflow if current work is dirty
- add recent files list
- restore last opened workflow path if desired

### Files To Expect

- Modify: `src/app/MainWindow.hpp`
- Modify: `src/app/MainWindow.cpp`
- Modify: `src/qtnodes/QtNodesEditorWidget.hpp`
- Modify: `src/qtnodes/QtNodesEditorWidget.cpp`
- Modify: `tests/app/MainWindowTests.cpp`
- Maybe create: `src/app/RecentFilesManager.hpp`
- Maybe create: `src/app/RecentFilesManager.cpp`

### Acceptance Criteria

- editing node title, description, properties, structure, or connections marks document dirty
- saving clears dirty state
- loading a workflow clears dirty state after success
- closing dirty work triggers confirmation
- recent files appear in `File`

## Phase 2: Editing Affordances

### Objective

Make core editing operations feel complete instead of prototype-like.

### Scope

- delete selected node
- delete selected connection
- right-click context menu in canvas
- keyboard shortcuts for delete
- actual undo/redo wiring if feasible with current graph model
- center on selection

### Files To Expect

- Modify: `src/app/MainWindow.cpp`
- Modify: `src/qtnodes/QtNodesEditorWidget.hpp`
- Modify: `src/qtnodes/QtNodesEditorWidget.cpp`
- Modify: `tests/app/MainWindowTests.cpp`

### Acceptance Criteria

- selected items can be removed without manual JSON editing
- toolbar `Undo` and `Redo` are either fully working or explicitly disabled until implemented
- context actions are discoverable

## Phase 3: Validation And Feedback

### Objective

Make the workflow visually explain what is valid, invalid, or incomplete.

### Scope

- node-level validation states
- inline status badges or outlines on invalid nodes
- illegal connection feedback during drag
- inspector-side validation hints for malformed property input
- status bar summary for current selection

### Files To Expect

- Modify: `src/qtnodes/StyledNodePainter.cpp`
- Modify: `src/qtnodes/QtNodesEditorWidget.cpp`
- Modify: `src/inspector/InspectorPanel.cpp`
- Modify: `src/registry/BuiltInNodeRegistry.cpp`
- Modify: `tests/app/MainWindowTests.cpp`

### Acceptance Criteria

- invalid configuration becomes visible without saving
- users can tell why a node is incomplete
- connection errors are visible at interaction time

## Phase 4: Node Type Expansion

### Objective

Make the built-in workflow language feel more useful for real AI orchestration.

### Scope

Candidate nodes:

- `agent`
- `memory`
- `retriever`
- `template variables`
- `http request`
- `json transform`
- `chat output`

### Files To Expect

- Modify: `src/registry/BuiltInNodeRegistry.cpp`
- Modify: `src/inspector/InspectorPanel.cpp`
- Modify: `src/resources/app_theme.qrc`
- Add matching SVG assets under `src/resources/icons/nodes/`
- Modify tests in:
  - `tests/domain/BuiltInNodeRegistryTests.cpp`
  - `tests/app/MainWindowTests.cpp`

### Acceptance Criteria

- every new node has:
  - category
  - display strings
  - ports
  - default properties
  - icon
  - inspector support
  - persistence coverage

## Phase 5: Canvas And Library Polish

### Objective

Push the editing experience from polished prototype to mature product feel.

### Scope

- foldable node categories with smoother transitions
- stronger hover/selection states
- optional mini-map or zoom indicator
- library item badges for port counts or capability type
- better empty states and inline help

### Files To Expect

- Modify: `src/app/NodeLibraryListWidget.cpp`
- Modify: `src/resources/styles/workbench.qss`
- Modify: `src/qtnodes/StyledNodePainter.cpp`
- Modify: `src/app/MainWindow.cpp`

### Acceptance Criteria

- left library is easy to scan
- canvas states remain visually consistent with side panels
- hover, focus, and selection are distinct but restrained

## Phase 6: Cross-Platform Hardening

### Objective

Move from “likely portable” to “continuously proven portable”.

### Scope

- add Windows CI
- test with Qt 5.13.2 or closest available supported CI image
- add a no-tests build configuration
- document dependency requirements
- verify translation generation on Windows

### Files To Expect

- Modify: `CMakeLists.txt`
- Modify: `tests/CMakeLists.txt`
- Add CI config under:
  - `.github/workflows/` if GitHub Actions is used
- Add docs updates in:
  - `docs/2026-04-22-claude-code-handoff.md`

### Acceptance Criteria

- Windows build is automated
- at least one Qt 5 line is tested automatically
- failures are actionable from CI logs

## Recommended Immediate Task Sequence

Use this exact order:

1. ~~dirty-state tracking~~ ✅ completed 2026-04-22
2. ~~close/open unsaved confirmation~~ ✅ completed 2026-04-22
3. ~~recent files menu~~ ✅ completed 2026-04-22
4. ~~delete selected node and connection~~ ✅ completed 2026-04-22
5. validation markers ← **START HERE**
6. expand built-in node set
7. Windows CI

## Stop Conditions

Pause and reassess before further feature growth if any of these happen:

- `MainWindow.cpp` becomes too large to reason about safely
- `QtNodesEditorWidget.cpp` starts mixing document logic, UI logic, and serialization too heavily
- inspector property handling becomes repetitive enough to justify a field-schema layer

If that happens, introduce a focused `WorkflowDocument` abstraction before adding more features.


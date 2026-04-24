# AI Workflow Editor Beta Usability Priority Plan

> **For agentic workers:** This plan supersedes the old “immediate next” order from `2026-04-22-next-development-plan.md` for the next implementation wave. Prefer small, test-backed increments and keep changes inside `ai-workflow-editor/`.

**Goal:** Move the editor from a polished prototype to a reliable beta-quality AI workflow authoring tool before expanding the node language further.

**Architecture:** Keep product behavior in the app and adapter layers. In the short term, consolidate validation and editing semantics around the existing `QtNodesEditorWidget`; in the medium term, introduce a focused document abstraction before file, undo/redo, and validation logic spread further across `MainWindow` and `QtNodesEditorWidget`.

**Tech Stack:** CMake, Qt Widgets, QtNodes, Qt translation resources, JSON persistence, Qt Test

---

## Why This Plan Reorders Priorities

The project already has more editing surface than the previous plan reflects:

- dirty state, recent files, save/load lifecycle are done
- delete node / delete connection / context menu / keyboard shortcuts are done
- undo/redo already covers node creation, property edits, node deletion, and connection deletion
- dirty state is now aligned with undo stack clean state
- toolbar polish and app-shell ergonomics are one step further along

The current bottleneck is no longer “basic editing affordances exist or not.”

The real bottleneck is this:

`Can a user build, inspect, fix, save, reopen, and trust an AI workflow without getting confused by missing validation, inconsistent state, or incomplete editor feedback?`

That is why the next wave should prioritize reliability, validation clarity, and document semantics over immediate node expansion.

---

## Current Reality Check

### Strong Areas

- Workbench shell is coherent and visually credible.
- Drag/drop, selection, inspector sync, save/load, and runtime i18n are in place.
- Core command stack behavior exists and already has meaningful automated coverage.
- Validation feedback exists in three surfaces:
  - inspector validation message
  - node card warning/error treatment
  - status bar summary for current selection

### Fragile Areas

- Validation logic is still implicit and UI-driven instead of document-driven.
- Save/load, dirty state, undo stack, and node state all still live in the adapter/widget layer.
- `QtNodesEditorWidget.cpp` is taking on too many responsibilities:
  - document mutation
  - serialization
  - validation
  - selection sync
  - context menu behavior
  - undo/redo orchestration
- Inspector forms are still manually wired per field and will become repetitive once more node types arrive.
- The old handoff document now understates what has already been implemented and overstates what is still missing.

---

## Recommended Development Order

Use this sequence for the next wave.

### ~~Phase A: Validation Engine Consolidation~~ ✅ completed 2026-04-24

Centralized validation rule path per node type, unified across inspector, node card, and status bar. All 13 node types have validation rules. Field-level validation highlighting in inspector.

### ~~Phase B: Document Semantics And Save/Undo Reliability~~ ✅ partially completed 2026-04-24

Undo commands extracted to `UndoCommands.hpp/.cpp`. File format versioning added (v2). Dirty/clean behavior stable. Full WorkflowDocument abstraction deferred — QtNodesEditorWidget still at ~1711 lines (down from 2005) which is manageable for now.

### ~~Phase C: Editing Reliability Completion~~ ✅ completed 2026-04-24

All core structural edits (create, delete, connect, disconnect, property edit) participate in undo/redo. Copy/paste/duplicate added (Ctrl+C/V/D). Context menu has copy/paste/duplicate/delete entries. Action states are fully synced with editor state.

### ~~Phase D: Inspector Maintainability And Schema Preparation~~ ✅ completed 2026-04-24

InspectorFieldSchema extracted — data-driven field and section definitions. Adding a new node type's inspector fields is now schema-driven rather than manual widget wiring. Field-level validation highlighting supported.

### ~~Phase E: Node Language Expansion~~ ✅ completed 2026-04-24

All 7 new node types implemented: memory, retriever, templateVariables, httpRequest, jsonTransform, agent, chatOutput. Each has definition, ports with data type IDs, default properties, SVG icon, inspector support, persistence, validation rules, and tests.

### ~~Phase F: Cross-Platform Hardening~~ ✅ completed 2026-04-24

GitHub Actions CI matrix covers Linux/Windows/macOS × Qt 5.15/6.5. Separate build-no-tests job. Docs updated.

---

---

## Post-Plan: Additional Completions (2026-04-24)

Beyond the original A–F phases, the following were also implemented:

- **Port data type system** — 5 types (flow, text, completion, error, http_response) with compatibility constraints
- **WorkflowGraphModel** — custom DataFlowGraphModel subclass enforcing type-aware connection rules
- **Copy/paste/duplicate** — Ctrl+C/V/D, Edit menu, right-click context menu, paste at mouse position
- **Undo command extraction** — 6 commands moved to UndoCommands.hpp/.cpp (widget 2005→1711 lines)
- **File format versioning** — version 2, backward-compatible with v1
- **Workflow export** — File > Export with Python (LangChain) and Python Script formats, topological sort, all 13 node types, 25 unit tests

## Recommended Next Directions

All original phases and workflow export are complete. Future development should focus on:

1. **Additional export formats** — LangGraph, CrewAI, etc.
2. **WorkflowDocument abstraction** — further decompose QtNodesEditorWidget
3. **Multi-node operations** — rubber band selection, batch move, alignment
4. **Node grouping / sub-workflows**
5. **Canvas mini-map**

## Stop Conditions

Pause and reassess before continuing if any of these become true:

- `QtNodesEditorWidget.cpp` exceeds ~2000 lines again
- `MainWindow.cpp` starts owning document state transitions that should belong elsewhere
- validation rules are duplicated across painter, inspector, and adapter code

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

### Phase A: Validation Engine Consolidation

**Objective**

Turn today’s scattered validation behavior into a more explicit, reusable workflow validation layer so node cards, inspector, and status bar all report from the same source of truth.

**Why it goes first**

- It directly improves authoring trust.
- It reduces UI-specific special cases before more node types multiply the problem.
- It is the highest product-value step that still fits the current architecture.

**Scope**

- define a clearer validation rule path per node type
- centralize validation result creation
- ensure node card, inspector, and status bar consume the same validation result
- expose clearer invalid-reason messages for missing required properties and illegal structural states
- cover validation on:
  - prompt
  - llm
  - tool
  - flow-only nodes with structural requirements if applicable

**Likely files**

- `src/qtnodes/QtNodesEditorWidget.cpp`
- `src/qtnodes/QtNodesEditorWidget.hpp`
- `src/inspector/InspectorPanel.cpp`
- `src/qtnodes/StyledNodePainter.cpp`
- `src/registry/BuiltInNodeRegistry.cpp`
- `tests/app/MainWindowTests.cpp`

**Acceptance criteria**

- every visible validation message comes from one consistent rule path
- the same invalid state produces the same message in inspector, card, and status bar
- validation updates immediately after property edit, selection change, undo, redo, save/load restore

### Phase B: Document Semantics And Save/Undo Reliability

**Objective**

Strengthen the meaning of “document state” so save/load/new/reopen/undo/redo all behave predictably and are easier to evolve.

**Why it goes second**

- validation improvements will expose more document-state edge cases
- the current adapter-layer document logic is already dense
- this phase reduces risk before adding search/filter, more node types, or more complex editing commands

**Scope**

- introduce a focused `WorkflowDocument` or equivalent narrow abstraction if file growth justifies it
- separate document concerns from pure scene/view concerns
- make save/load/reset/current-path/current-clean-state semantics more explicit
- verify command stack behavior around:
  - new document
  - load document
  - save after undo/redo
  - delete/restore after save point

**Likely files**

- `src/app/MainWindow.cpp`
- `src/app/MainWindow.hpp`
- `src/qtnodes/QtNodesEditorWidget.cpp`
- `src/qtnodes/QtNodesEditorWidget.hpp`
- maybe new:
  - `src/app/WorkflowDocument.hpp`
  - `src/app/WorkflowDocument.cpp`
- `tests/app/MainWindowTests.cpp`

**Acceptance criteria**

- dirty/clean behavior remains stable across all supported editing actions
- document lifecycle logic becomes easier to reason about than today’s widget-centric flow
- no regression in save/load/undo/redo tests

### Phase C: Editing Reliability Completion

**Objective**

Close the remaining editing gaps so command behavior feels complete and trustworthy.

**Why it goes third**

- after Phase B, command-related work will be safer
- this finishes the “can users confidently edit workflows?” story before expanding node coverage

**Scope**

- bring connection creation into the same undo/redo story if still missing
- verify mixed selection behavior and action enablement states
- make delete/select/center/undo/redo actions reflect actual availability
- tighten context menu logic so options match scene state
- add regression tests for:
  - create connection → undo → redo
  - mixed selection precedence
  - empty-selection command behavior

**Likely files**

- `src/app/MainWindow.cpp`
- `src/qtnodes/QtNodesEditorWidget.cpp`
- `src/qtnodes/QtNodesEditorWidget.hpp`
- `tests/app/MainWindowTests.cpp`

**Acceptance criteria**

- all core structural edits participate in undo/redo consistently
- toolbar/menu/context actions feel state-aware rather than always-on
- no surprising no-op or destructive edge cases in selection handling

### Phase D: Inspector Maintainability And Schema Preparation

**Objective**

Prepare the editor for more node types without letting inspector code become repetitive and brittle.

**Why it goes fourth**

- expanding nodes before this would cause avoidable duplication
- validation and document semantics will already be clearer by this point

**Scope**

- extract repeated field wiring patterns
- define a lightweight property schema or field descriptor layer
- keep `QVariantMap` storage for now, but stop hardcoding every widget path in one place
- ensure validation hints can be attached at the field level where useful

**Likely files**

- `src/inspector/InspectorPanel.cpp`
- `src/inspector/InspectorPanel.hpp`
- maybe new:
  - `src/inspector/InspectorFieldSchema.hpp`
  - `src/inspector/InspectorFieldSchema.cpp`
- `tests/app/MainWindowTests.cpp`

**Acceptance criteria**

- adding one new property field no longer requires copy-pasting a large amount of boilerplate
- validation feedback can point to specific field-level requirements
- the inspector remains easy to retranslate

### Phase E: Node Language Expansion (After Reliability)

**Objective**

Only after the editor is trustworthy, expand the built-in workflow vocabulary.

**Recommended order inside this phase**

1. `memory`
2. `retriever`
3. `template variables`
4. `http request`
5. `json transform`
6. `agent`
7. `chat output`

**Why this order**

- `memory` and `retriever` fit the current AI workflow framing most directly
- `template variables` improves practical authoring without requiring runtime execution
- `http request` and `json transform` increase orchestration value
- `agent` should come later so it lands in a more disciplined schema/validation setup

**Acceptance criteria**

- every new node ships with:
  - definition
  - ports
  - default properties
  - icon
  - inspector support
  - persistence coverage
  - validation rules

### Phase F: Cross-Platform Hardening

**Objective**

Make the project continuously safer to evolve.

**Scope**

- update docs to reflect current implemented reality
- add Windows CI
- test a Qt 5.x line close to 5.13.2
- document runtime/build prerequisites clearly

**Acceptance criteria**

- repository docs no longer misstate completed work
- CI verifies at least one non-macOS environment
- platform regressions become visible earlier than manual testing

---

## Recommended Immediate Next Task

Start with:

`Phase A.1 — centralize node validation result generation and normalize current prompt/llm/tool validation messages`

Why:

- it has high user-facing value
- it is local enough to implement without an immediate architecture split
- it creates the rule backbone needed by later phases

---

## Delivery Strategy

Prefer this cadence:

1. one narrow reliability improvement
2. one focused test block
3. one full build + `ctest`
4. one small push

Avoid combining:

- validation rewrite
- document abstraction
- node expansion
- inspector schema extraction

in a single large patch.

---

## Stop Conditions

Pause and reassess before continuing if any of these become true:

- `QtNodesEditorWidget.cpp` keeps growing and mixes more document logic into scene logic
- `MainWindow.cpp` starts owning document state transitions that should belong elsewhere
- validation rules are duplicated across painter, inspector, and adapter code
- inspector additions require copy-pasting large connect/set/get blocks for every new node type

If two or more of these are true at once, introduce the focused document/schema abstraction before adding more user-facing features.

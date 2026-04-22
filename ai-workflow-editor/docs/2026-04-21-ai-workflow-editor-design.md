# AI Workflow Editor Design

Date: 2026-04-21

## Summary

Build a standalone desktop application inside its own `ai-workflow-editor` project directory and use `QtNodes` from `third_party/nodeeditor` as the canvas layer for a light-themed AI workflow editor. The first version is a visual prototype focused on workflow composition, node configuration, and JSON persistence. It does not execute workflows.

The editor targets mixed AI workflows in the long term, but version one is centered on LLM agent orchestration so the product shape stays coherent while leaving room for RAG and automation nodes later.

## Goals

- Provide a clean desktop workflow editor built with Qt and C++.
- Reuse `QtNodes` for graph visualization, interaction, and connection management.
- Keep workflow semantics in an application-owned domain model rather than coupling project files to `QtNodes` scene internals.
- Ship a fixed set of built-in nodes with an extensible registration structure for future node types.
- Offer a right-side form inspector for editing node properties.
- Support creating, editing, saving, and loading workflows as JSON files.

## Non-Goals

- Executing workflows against real LLMs, tools, or external APIs.
- Runtime logs, debugger views, variable tracing, or breakpoint support.
- Plugin hot-loading or a fully dynamic extension marketplace.
- Complex schema-driven property editing.
- Collaboration, cloud sync, or multi-user features.

## Product Scope

Version one is a visual prototype with four core capabilities:

1. Workflow project file operations.
2. Canvas-based graph editing.
3. Node property editing through a form inspector.
4. Stable JSON persistence with forward-compatible versioning.

The experience should feel like a focused product application rather than a demo utility or generic developer tool.

## User Experience

### Window Structure

The application is a standalone `QMainWindow` with these regions:

- Top toolbar for document and canvas commands.
- Left node library for built-in node categories.
- Center workflow canvas powered by `QtNodes`.
- Right inspector panel for node properties.
- Bottom status bar for lightweight context such as zoom and save state.

### Light Theme Direction

The interface uses a light visual language:

- Warm white or light gray surfaces.
- Slightly darker side panels than the canvas to establish structure.
- White node cards with subtle borders and soft shadows.
- Blue selection and hover accents with restrained contrast.
- A gentle grid background on the canvas.
- Category colors used sparingly on node headers, port accents, or icons.

The target feel is polished, professional, and approachable rather than dark, industrial, or IDE-like.

### Primary Interaction Flow

1. Create or open a workflow project.
2. Add nodes from the left library by drag-and-drop or double-click.
3. Connect nodes on the canvas.
4. Select a node and edit its properties in the right inspector.
5. Save the workflow to `workflow.json`.
6. Reopen the file later and restore node positions, connections, and properties.

### Empty State

A new workflow starts with a canvas hint such as:

`Drag nodes here to build your AI workflow`

This keeps the first-run experience from feeling unfinished.

## Built-In Nodes

Version one includes a fixed node set:

### Flow

- `Start`
- `Condition`
- `Output`

### AI

- `Prompt`
- `LLM`

### Integration

- `Tool`

### Node Semantics

#### Start

- No input ports.
- One output port.
- Represents workflow entry.

#### Prompt

- One input port.
- One output port.
- Stores prompt-related text fields for the future execution layer.

#### LLM

- One input port.
- Two output ports.
- Represents model call configuration.
- A second output is reserved for future error or alternate-path semantics.

#### Tool

- One input port.
- One output port.
- Represents an external action or integration.

#### Condition

- One input port.
- Two output ports named `true` and `false`.
- Represents branching logic.

#### Output

- One input port.
- No output ports.
- Represents a terminal result node.

## Architecture

The recommended architecture is:

`Standalone App + Workflow Domain Model + QtNodes Adapter Layer`

This is the preferred balance between delivery speed and long-term maintainability.

### Why This Architecture

- It keeps business semantics separate from the canvas library.
- It allows the editor to evolve without locking project files to `QtNodes` internals.
- It creates a stable foundation for future execution, validation, and migration work.
- It keeps version one focused without prematurely building a full plugin platform.

## Code Organization

Create a new application under:

`ai-workflow-editor/`

Recommended structure:

```text
ai-workflow-editor/
  CMakeLists.txt
  docs/
  src/
    app/
    domain/
    inspector/
    persistence/
    qtnodes/
    registry/
    validation/
  resources/
```

### Responsibilities

#### `src/app`

- Application entry point.
- Main window.
- Toolbar, dock layout, menu wiring, document lifecycle.

#### `src/domain`

- `WorkflowDocument`
- `WorkflowNodeDefinition`
- `WorkflowNodeInstance`
- `WorkflowEdge`
- Shared data structures independent of the UI library.

#### `src/registry`

- Built-in node registration.
- Category metadata.
- Default field definitions.
- Future extension hooks for adding new node types.

#### `src/qtnodes`

- Adapters between domain nodes and `QtNodes` node models.
- Scene synchronization.
- Node creation and connection restoration.

#### `src/inspector`

- Right-side property widgets.
- Form layout generation for built-in nodes.
- Synchronization between selected node state and form fields.

#### `src/persistence`

- Loading and saving `workflow.json`.
- Schema version handling.

#### `src/validation`

- Edit-time connection rules.
- Lightweight node and graph validation.

## Domain Model

### WorkflowDocument

Represents the full editable project:

- Metadata such as workflow name.
- Node instances.
- Edge list.
- View state like zoom and center position.
- Schema version.

### WorkflowNodeDefinition

Describes a node type:

- Stable type key such as `start` or `llm`.
- Display label.
- Category.
- Port definitions.
- Default config values.
- Property field descriptors for the inspector.

This model supports the required fixed node set while leaving room for future registration-based expansion.

### WorkflowNodeInstance

Represents a concrete node placed on the canvas:

- `id`
- `type`
- `displayName`
- `description`
- `position`
- `config`

### WorkflowEdge

Represents a connection between two node instances:

- `id`
- `sourceNodeId`
- `sourcePort`
- `targetNodeId`
- `targetPort`

Port identifiers should be stable string keys rather than raw numeric indices in persisted data. This makes future migration and schema evolution safer.

## Persistence Format

The editor stores workflows in an application-owned JSON format.

Example:

```json
{
  "version": 1,
  "metadata": {
    "name": "Untitled Workflow"
  },
  "view": {
    "zoom": 1.0,
    "center": { "x": 0, "y": 0 }
  },
  "nodes": [
    {
      "id": "node-start-1",
      "type": "start",
      "displayName": "Start",
      "description": "Workflow entry point",
      "position": { "x": -240, "y": 0 },
      "config": {}
    },
    {
      "id": "node-prompt-1",
      "type": "prompt",
      "displayName": "Draft Prompt",
      "description": "",
      "position": { "x": 20, "y": 0 },
      "config": {
        "systemPrompt": "You are a helpful assistant.",
        "userPromptTemplate": "{{input}}"
      }
    }
  ],
  "edges": [
    {
      "id": "edge-1",
      "sourceNodeId": "node-start-1",
      "sourcePort": "out",
      "targetNodeId": "node-prompt-1",
      "targetPort": "in"
    }
  ]
}
```

### Persistence Rules

- The root object always includes a `version`.
- Node config is interpreted by node type.
- View state is persisted so reopening restores the working context.
- Invalid files should fail gracefully with a user-facing error dialog.

## QtNodes Integration Strategy

`QtNodes` is the canvas and interaction layer, not the source of truth for workflow semantics.

### Core Mapping Rules

1. The left node library is built from `WorkflowNodeDefinition`, not from `QtNodes` internals.
2. Creating a node first creates a `WorkflowNodeInstance`.
3. The adapter layer then instantiates the matching `QtNodes` model for canvas display.
4. Inspector changes update the domain model first, then refresh the corresponding visual node state.
5. Connection creation runs validation before committing a new `WorkflowEdge`.
6. File save and load operate on the domain model, with adapter-driven scene reconstruction.

This keeps the domain model authoritative and the graphics layer replaceable or refactorable.

## Inspector Design

The right inspector is form-based and optimized for reliability over power-user complexity.

### States

- No selection: show an empty-state message.
- Single selection: show the selected node's editable fields.
- Multi-selection: not supported in version one.

### Section Layout

- `General`
- `Configuration`
- Optional read-only port information if useful

### Common Fields

All nodes expose:

- `id` as read-only
- `displayName`
- `description`

### Example Type-Specific Fields

#### Prompt

- `systemPrompt`
- `userPromptTemplate`

#### LLM

- `model`
- `temperature`
- `maxTokens`

#### Tool

- `toolName`
- `toolDescription`
- `inputMapping`

Version one uses standard Qt form widgets such as line edits, combo boxes, spin boxes, and text edits where needed, but avoids a fully dynamic schema engine.

## Edit-Time Validation

The product is not only a drawing tool. It should enforce basic workflow correctness during editing.

### Initial Rules

- `Start` cannot have inputs.
- `Output` cannot have outputs.
- `Condition` must expose `true` and `false` outputs.
- Invalid port combinations should be blocked before the connection is created.
- Any one-to-one port constraints should be enforced consistently.

### User Feedback

- Block invalid connections immediately.
- Show a concise explanation in the status bar or a lightweight message.
- Avoid verbose modal interruptions for routine mistakes.

## Toolbar and Commands

Version one toolbar actions:

- `New`
- `Open`
- `Save`
- `Save As`
- `Undo`
- `Redo`
- `Center`

This keeps the UI compact and product-oriented.

## Error Handling

### File Operations

- Prompt before closing when there are unsaved changes.
- Show clear errors for invalid or unreadable JSON files.
- Report save failures with path and reason when available.

### Resilience

- Never crash because of malformed project data.
- When possible, reject the operation and keep the current document intact.

## Testing Strategy

Version one testing should focus on stable logic rather than brittle visual assertions.

### Priority Test Areas

- Domain object behavior.
- JSON serialization and deserialization.
- Built-in node definition completeness.
- Validation rules for allowable and disallowed connections.

### Lower-Priority GUI Coverage

- Main window smoke test.
- Open and save workflow smoke test.
- Basic selection and inspector update sanity checks when practical.

Pixel-perfect GUI verification is out of scope for version one.

## Implementation Notes

- Follow the product application's own CMake style and link against `third_party/nodeeditor`.
- Reuse `QtNodes` library targets rather than duplicating any canvas logic.
- Preserve the third-party library structure and avoid modifying core library behavior unless the app integration reveals a real gap.
- Favor clear, isolated classes over large all-in-one widgets.

## Future Evolution

This design intentionally leaves room for:

- Real execution engines.
- Runtime logs and debugging panels.
- RAG-specific node types.
- HTTP and automation nodes.
- More advanced field editors.
- Richer validation and graph analysis.
- Plugin-based node registration.

These are deferred so version one can deliver a strong editing foundation first.

## Acceptance Criteria

The first version is successful if:

- The app launches as a standalone Qt desktop application.
- Users can add built-in nodes to a canvas.
- Users can connect valid nodes and are blocked from invalid connections.
- Users can edit node properties in a right-side form inspector.
- Users can save workflows to JSON and load them back accurately.
- The visual design is light-themed and feels like a focused product workspace.

## Selected Approach Record

This design reflects the following validated decisions:

- Product type: mixed AI workflow editor with version one centered on LLM agent orchestration.
- Delivery scope: visual prototype only, no execution engine.
- Node system: fixed built-in nodes with extensible registration structure.
- Inspector model: form-based editing.
- Application shape: standalone app in its own project directory, built on top of `third_party/nodeeditor`.
- Visual direction: light UI theme.

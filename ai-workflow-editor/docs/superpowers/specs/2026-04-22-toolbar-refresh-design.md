# AI Workflow Editor Toolbar Refresh Design

Date: 2026-04-22

## Summary

Refresh the main workbench toolbar so it feels aligned with the existing light desktop editor aesthetic.

The toolbar should remain part of an AI workflow authoring tool, not drift toward chat-app chrome, low-code dashboards, or a generic business admin UI.

## Goals

- Make the toolbar feel lighter, cleaner, and more professional.
- Preserve the current product direction and shallow warm-light palette.
- Improve action hierarchy so primary and secondary actions do not compete equally.
- Keep the existing action set, shortcuts, translations, and editor behavior intact.

## Non-Goals

- No Ribbon-style redesign.
- No new product features.
- No menu-bar redesign.
- No custom painting framework for toolbar controls.
- No changes inside `third_party/nodeeditor`.

## Current Problems

- Every toolbar button reads like an isolated beige card, which makes the whole strip feel heavy.
- Primary actions and utility actions have nearly identical visual weight.
- The language switcher looks like another command button instead of a state/menu entry.
- The toolbar visually competes with the node canvas instead of framing it.

## Recommended Direction

Use a grouped workbench toolbar.

The top bar should read as a light control rail, with grouped tools and restrained emphasis. Most buttons should feel neutral until hover. Only save should carry a modest emphasis state.

## Structure

Arrange toolbar content into four functional groups:

1. File: New, Open, Save
2. Edit: Undo, Redo, Delete
3. View: Select All, Center
4. Status/Menu: Language switcher

Use spacing and separators between groups instead of giving every button a strong background.

## Visual Design

### Toolbar Container

- Keep the existing warm-light family.
- Reduce visual thickness and weight compared with the current toolbar.
- Preserve a subtle bottom boundary so the toolbar remains distinct from the canvas.

### Standard Tool Buttons

- Default state should be near-flat or very lightly filled.
- Borders should be quieter and less card-like.
- Hover should lift the button slightly through a subtle background and border change.
- Pressed should feel firm but not dark or glossy.
- Corners should be smaller than the current rounded-pill look.

### Primary Action

- Save gets a restrained emphasis treatment.
- The emphasis must remain within the current palette and should not read like a web CTA.

### Utility Actions

- Undo, Redo, Delete, Select All, and Center should be visually lighter than file actions.
- They should feel like editor tools, not equal-priority milestones.

### Language Entry

- The language control should look like a compact status/menu entry.
- It should not visually merge with the main command group.

## Implementation Scope

Modify only:

- `src/resources/styles/workbench.qss`
- `src/app/MainWindow.cpp`
- tests only as needed to reflect intentional toolbar grouping or structure changes

Keep stable:

- object names
- action wiring
- shortcuts
- existing localization behavior

## Interaction Requirements

- Hover, pressed, and checked states must remain clear on desktop.
- The toolbar must still work well at the current window size.
- Chinese text should remain readable without buttons becoming oversized again.

## Testing Strategy

- Keep existing toolbar smoke tests passing where behavior is unchanged.
- Adjust tests only if intentional grouping changes affect action ordering.
- Run the full existing build and `ctest` suite after implementation.

## Acceptance Criteria

- The toolbar visually matches the rest of the workbench better than the current version.
- Buttons no longer read as a row of heavy standalone cards.
- Action hierarchy is clearer at a glance.
- Language entry feels distinct from command buttons.
- No product-direction drift and no functional regressions.

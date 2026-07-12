# Project Phoenix Development Roadmap

## Purpose

This document captures the current technical roadmap for Project Phoenix based on the implemented editor shell, scene workflow, and the near-term animation goals.

It is intended to be more accurate than the older high-level milestone image and should be treated as the working roadmap for engineering decisions.

## Current Position

Project Phoenix is currently at the end of `v0.2` and preparing to move into `v0.3`.

The project already includes:

- 3D viewport
- camera navigation
- FBX import
- primitive creation
- outliner
- channel box
- Maya-like dockable window layout
- base shaded viewport plus wireframe overlay
- scene save/open/export workflow
- timeline and playback shell
- early Script Editor
- initial command system for editor actions

## Milestones

### v0.1 Core Editor Foundation

Goal:
- Build the first usable editor shell.

Scope:
- Viewport
- Camera controls
- FBX import
- Primitive creation
- Outliner
- Channel Box
- Dockable workspace layout
- Base shading and selection display

Status:
- Completed

### v0.2 Scene Workflow

Goal:
- Make the editor usable for basic scene work and command-driven workflows.

Scope:
- New / Open / Save / Save As
- Export All / Export Selection
- Archive Scene
- Timeline shell
- Playback shell
- Script Editor window
- Command system for:
  - `select`
  - `poly*`
  - `setAttr`
  - `currentTime`
  - `playbackOptions`
  - `play`
  - `file`
  - `viewFit`
  - `viewSet`
  - `setToolTo`

Status:
- Largely completed

Notes:
- Timeline is functional as an editor shell.
- Real animation data and keyframe evaluation are not finished yet.

### v0.3 Animation Data Foundation

Goal:
- Introduce actual animation data instead of frame-only playback.

Scope:
- Keyframe data model
- Per-object animation tracks
- Set Key
- Auto Key foundation
- Timeline key markers
- Playback that evaluates animated transforms
- Command support for key operations

Exit criteria:
- A user can animate translate / rotate / scale on an object across multiple frames.

Status:
- Next priority

### v0.4 Rigging Foundation

Goal:
- Start character and object rig hierarchy workflows.

Scope:
- Joint / bone objects
- Skeleton hierarchy authoring
- Parenting and hierarchy tools
- Joint orientation basics
- Bind pose representation

Exit criteria:
- A user can create and edit a valid skeleton hierarchy in the editor.

Status:
- Not started

### v0.5 Skinning

Goal:
- Bind meshes to skeletons and deform them in playback.

Scope:
- Skin bind
- Weight storage
- Weight normalization
- Basic weight editing
- Deformation evaluation

Exit criteria:
- A skinned mesh deforms from joint animation in the viewport.

Status:
- Not started

### v0.6 Animation Tools

Goal:
- Improve production usability for animation authoring.

Scope:
- Key editing actions
- Duplicate / offset / mirror animation
- Dope Sheet basics
- Constraints basics
- Better playback controls

Exit criteria:
- A user can edit animation timing and pose data without relying on raw transform edits only.

Status:
- Not started

### v0.7 Graph Editor

Goal:
- Add curve-based animation editing.

Scope:
- Curve display
- Key interpolation
- Tangent editing
- Time/value scaling
- Selection and editing tools

Exit criteria:
- A user can inspect and edit animation curves directly.

Status:
- Not started

### v0.8 Workflow and Extensibility

Goal:
- Make the editor more programmable and extensible.

Scope:
- Stronger Script Editor
- Better command registry
- Undo / redo command stack
- Macro replay
- Plugin extension points

Exit criteria:
- Script-driven workflows become a first-class way to extend the editor.

Status:
- Foundation exists, but not complete

### v0.9 Production Readiness

Goal:
- Stabilize the editor before wider testing.

Scope:
- Performance optimization
- Robust file workflow
- Better preferences and persistence
- UX cleanup
- Crash and regression reduction

Exit criteria:
- Core scene and animation workflows are stable enough for repeated daily use.

Status:
- Not started

### v1.0 Public Beta

Goal:
- Ship a coherent beta focused on animation authoring.

Scope:
- Stable editor shell
- Scene workflow
- Keyframe animation workflow
- Early rigging and skinning support
- Scriptable workflows
- Basic documentation and QA coverage

Exit criteria:
- External users can complete small real animation tasks in the tool.

Status:
- Future milestone

## Recommended Immediate Priorities

The best next engineering sequence is:

1. Add a keyframe data model.
2. Connect object transforms to animation tracks.
3. Show keys on the timeline.
4. Add `Set Key`.
5. Make playback evaluate animation data instead of only changing the frame counter.

## Notes For Planning

- The editor shell is advancing slightly ahead of the original roadmap because tooling such as the Script Editor and command system has already started.
- This is acceptable, because those systems will support animation, automation, undo/redo, and plugin workflows later.
- The main remaining gap before `v0.3` can be considered active is the lack of real animation data and evaluation.

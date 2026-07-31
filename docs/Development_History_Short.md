# Development History Short

| Date / Time | Development Summary |
| --- | --- |
| `2026-07-05` | Initialized the repository and established the first `Editor Shell` foundation, including the viewport, camera, import pipeline, and the `Qt Widgets + OpenGL` stack. |
| `2026-07-12` | Added the scene workflow, timeline/playback shell, command workflow, and the animation data foundation with `Set Key / Auto Key / Delete Key`. |
| `2026-07-13` | Completed `v0.4 Rigging Foundation phase 1`, including `Joint`, hierarchy editing, `Joint Orientation`, `Bind Pose`, and skeleton visualization. |
| `2026-07-13` to `2026-07-22` | Completed the `v0.5 Skinning` foundation and started `v0.6 Animation Tools phase 1` with duplicate/shift/jump key workflows. |
| `2026-07-23` | Extracted document, scene, and history seams; separated timeline UI and script seams; and made the `Controller / FlowController / ViewBuilder / SceneMutation` pattern more explicit. |
| `2026-07-24` | Extracted editor shell seams, renamed the architecture language from `MainWindow` to `EditorShell`, and clarified ownership by shell/module. |
| `2026-07-25` | Extracted `Engine` runtime seams to reduce orchestration inside the shell and establish a clearer playback/runtime coordination layer. |
| `2026-07-26` to `2026-07-31` | Reviewed and synchronized the boundary maps for `Scene`, `Rendering`, `Animation`, and `Rigging`; added dependency/code/roadmap documents to lock the module architecture more clearly. |
| `2026-07-26` to `2026-07-31` | Upgraded viewport object manipulation by strengthening picking/selection sync, completing the `Translate / Rotate / Scale` gizmo flow, improving responsiveness, and separating `interaction / geometry / drag`. |

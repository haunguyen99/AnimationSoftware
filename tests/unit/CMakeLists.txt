find_package(Qt6 REQUIRED COMPONENTS Gui Widgets Test OpenGLWidgets OpenGL)

add_executable(scene_transform_tests
    SceneTransformTests.cpp
    ${CMAKE_SOURCE_DIR}/src/animation/data/CurveInterpolator.cpp
    ${CMAKE_SOURCE_DIR}/src/animation/data/ObjectAnimationState.cpp
    ${CMAKE_SOURCE_DIR}/src/animation/scene/SceneAnimationState.cpp
    ${CMAKE_SOURCE_DIR}/src/rigging/data/ObjectRigState.cpp
    ${CMAKE_SOURCE_DIR}/src/rigging/scene/JointRiggingOps.cpp
    ${CMAKE_SOURCE_DIR}/src/rigging/scene/SkinBindingOps.cpp
    ${CMAKE_SOURCE_DIR}/src/rigging/scene/SkinDeformationOps.cpp
    ${CMAKE_SOURCE_DIR}/src/rigging/scene/SceneRiggingController.cpp
    ${CMAKE_SOURCE_DIR}/src/io/PhoenixSceneDocument.cpp
    ${CMAKE_SOURCE_DIR}/src/scene/Bounds3D.cpp
    ${CMAKE_SOURCE_DIR}/src/scene/SceneObject.cpp
    ${CMAKE_SOURCE_DIR}/src/scene/Scene.cpp
    ${CMAKE_SOURCE_DIR}/src/scene/SceneMath.cpp
)

target_include_directories(scene_transform_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(scene_transform_tests
    PRIVATE
        Qt6::Gui
)

set(PHOENIX_TEST_PATH "C:/Qt/6.11.1/msvc2022_64/bin;E:/Animation Software/vcpkg_installed/x64-windows/debug/bin;E:/Animation Software/vcpkg_installed/x64-windows/bin;$ENV{PATH}")

add_test(
    NAME scene_transform_tests
    COMMAND ${CMAKE_COMMAND} -E env "PATH=${PHOENIX_TEST_PATH}" $<TARGET_FILE:scene_transform_tests>
)

add_executable(editor_ui_tests
    EditorUiTests.cpp
    ${CMAKE_SOURCE_DIR}/apps/editor/src/AnimationTimelinePanel.cpp
    ${CMAKE_SOURCE_DIR}/apps/editor/src/EditorAnimationController.cpp
    ${CMAKE_SOURCE_DIR}/apps/editor/src/EditorAnimationTimelineViewBuilder.cpp
    ${CMAKE_SOURCE_DIR}/apps/editor/src/EditorChannelBoxController.cpp
    ${CMAKE_SOURCE_DIR}/apps/editor/src/EditorCreationController.cpp
    ${CMAKE_SOURCE_DIR}/apps/editor/src/EditorDocumentController.cpp
    ${CMAKE_SOURCE_DIR}/apps/editor/src/EditorFileFlowController.cpp
    ${CMAKE_SOURCE_DIR}/apps/editor/src/EditorInspectorController.cpp
    ${CMAKE_SOURCE_DIR}/apps/editor/src/EditorOutlinerController.cpp
    ${CMAKE_SOURCE_DIR}/apps/editor/src/EditorSceneQueryController.cpp
    ${CMAKE_SOURCE_DIR}/apps/editor/src/EditorSelectionController.cpp
    ${CMAKE_SOURCE_DIR}/apps/editor/src/EditorScriptExecutionController.cpp
    ${CMAKE_SOURCE_DIR}/apps/editor/src/EditorScriptLogController.cpp
    ${CMAKE_SOURCE_DIR}/apps/editor/src/EditorViewportUiController.cpp
    ${CMAKE_SOURCE_DIR}/apps/editor/src/EditorSceneMutationController.cpp
    ${CMAKE_SOURCE_DIR}/apps/editor/src/EditorViewportCommandController.cpp
    ${CMAKE_SOURCE_DIR}/apps/editor/src/GraphEditorPanel.cpp
    ${CMAKE_SOURCE_DIR}/apps/editor/src/KeyframeTimelineWidget.cpp
    ${CMAKE_SOURCE_DIR}/apps/editor/src/RangeSliderPanel.cpp
    ${CMAKE_SOURCE_DIR}/apps/editor/src/ViewportWidget.cpp
    ${CMAKE_SOURCE_DIR}/apps/editor/src/ViewportWorkspaceWidget.cpp
    ${CMAKE_SOURCE_DIR}/src/core/app/EditorShell.cpp
    ${CMAKE_SOURCE_DIR}/src/core/app/EditorShellContexts.cpp
    ${CMAKE_SOURCE_DIR}/src/core/app/EditorMenuBar.cpp
    ${CMAKE_SOURCE_DIR}/src/core/app/EditorTheme.cpp
    ${CMAKE_SOURCE_DIR}/src/core/app/EditorToolBar.cpp
    ${CMAKE_SOURCE_DIR}/src/core/app/WorkspaceManager.cpp
    ${CMAKE_SOURCE_DIR}/src/animation/editor/EditorAnimationFlowController.cpp
    ${CMAKE_SOURCE_DIR}/src/animation/editor/EditorAnimationSceneMutation.cpp
    ${CMAKE_SOURCE_DIR}/src/rigging/editor/EditorRiggingController.cpp
    ${CMAKE_SOURCE_DIR}/src/animation/data/CurveInterpolator.cpp
    ${CMAKE_SOURCE_DIR}/src/animation/data/ObjectAnimationState.cpp
    ${CMAKE_SOURCE_DIR}/src/animation/scene/SceneAnimationState.cpp
    ${CMAKE_SOURCE_DIR}/src/rigging/data/ObjectRigState.cpp
    ${CMAKE_SOURCE_DIR}/src/rigging/scene/JointRiggingOps.cpp
    ${CMAKE_SOURCE_DIR}/src/rigging/scene/SkinBindingOps.cpp
    ${CMAKE_SOURCE_DIR}/src/rigging/scene/SkinDeformationOps.cpp
    ${CMAKE_SOURCE_DIR}/src/rigging/scene/SceneRiggingController.cpp
    ${CMAKE_SOURCE_DIR}/src/engine/animation/EditorAnimationEngineFacade.cpp
    ${CMAKE_SOURCE_DIR}/src/engine/playback/EditorPlaybackController.cpp
    ${CMAKE_SOURCE_DIR}/src/engine/runtime/EditorSceneRuntimeController.cpp
    ${CMAKE_SOURCE_DIR}/src/engine/runtime/EditorViewportSceneController.cpp
    ${CMAKE_SOURCE_DIR}/src/core/commands/ScriptCommandSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/core/history/EditorHistoryController.cpp
    ${CMAKE_SOURCE_DIR}/src/core/settings/EditorPreferences.cpp
    ${CMAKE_SOURCE_DIR}/src/scene/Bounds3D.cpp
    ${CMAKE_SOURCE_DIR}/src/scene/SceneObject.cpp
    ${CMAKE_SOURCE_DIR}/src/scene/Scene.cpp
    ${CMAKE_SOURCE_DIR}/src/scene/SceneMath.cpp
    ${CMAKE_SOURCE_DIR}/src/scene/PrimitiveMeshFactory.cpp
    ${CMAKE_SOURCE_DIR}/src/io/AssimpSceneDocument.cpp
    ${CMAKE_SOURCE_DIR}/src/io/PhoenixSceneDocument.cpp
    ${CMAKE_SOURCE_DIR}/src/io/FbxImportMessages.cpp
    ${CMAKE_SOURCE_DIR}/src/io/FbxImporter.cpp
    ${CMAKE_SOURCE_DIR}/src/io/FbxImportValidation.cpp
    ${CMAKE_SOURCE_DIR}/src/io/FbxSceneBuilder.cpp
    ${CMAKE_SOURCE_DIR}/src/core/logging/LogCategories.cpp
    ${CMAKE_SOURCE_DIR}/src/viewport/EditorCamera.cpp
    ${CMAKE_SOURCE_DIR}/src/viewport/interaction/ViewportInteractionMath.cpp
    ${CMAKE_SOURCE_DIR}/src/viewport/interaction/gizmo/TranslateGizmoInteraction.cpp
    ${CMAKE_SOURCE_DIR}/src/viewport/interaction/gizmo/RotateGizmoInteraction.cpp
    ${CMAKE_SOURCE_DIR}/src/viewport/interaction/gizmo/ScaleGizmoInteraction.cpp
    ${CMAKE_SOURCE_DIR}/src/viewport/gizmo/drag/TranslateGizmoDrag.cpp
    ${CMAKE_SOURCE_DIR}/src/viewport/gizmo/drag/RotateGizmoDrag.cpp
    ${CMAKE_SOURCE_DIR}/src/viewport/gizmo/drag/ScaleGizmoDrag.cpp
    ${CMAKE_SOURCE_DIR}/src/viewport/runtime/ViewportRenderSync.cpp
    ${CMAKE_SOURCE_DIR}/src/rendering/geometry/ViewportOverlayGeometryBuilder.cpp
    ${CMAKE_SOURCE_DIR}/src/rendering/geometry/gizmo/TranslateGizmoGeometry.cpp
    ${CMAKE_SOURCE_DIR}/src/rendering/geometry/gizmo/RotateGizmoGeometry.cpp
    ${CMAKE_SOURCE_DIR}/src/rendering/geometry/gizmo/ScaleGizmoGeometry.cpp
    ${CMAKE_SOURCE_DIR}/src/rendering/scene/ViewportRenderSceneAdapter.cpp
    ${CMAKE_SOURCE_DIR}/src/rendering/ShaderUtils.cpp
    ${CMAKE_SOURCE_DIR}/src/rendering/ViewportRenderer.cpp
)

target_include_directories(editor_ui_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/apps/editor/include
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(editor_ui_tests
    PRIVATE
        assimp::assimp
        Qt6::Gui
        Qt6::Widgets
        Qt6::Test
        Qt6::OpenGLWidgets
        Qt6::OpenGL
)

set_target_properties(editor_ui_tests PROPERTIES AUTOMOC ON)

add_test(
    NAME editor_ui_tests
    COMMAND ${CMAKE_COMMAND} -E env "PATH=${PHOENIX_TEST_PATH}" $<TARGET_FILE:editor_ui_tests>
)

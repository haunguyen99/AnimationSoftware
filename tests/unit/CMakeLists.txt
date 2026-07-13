find_package(Qt6 REQUIRED COMPONENTS Gui Widgets Test OpenGLWidgets OpenGL)

add_executable(scene_transform_tests
    SceneTransformTests.cpp
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
    ${CMAKE_SOURCE_DIR}/apps/editor/src/KeyframeTimelineWidget.cpp
    ${CMAKE_SOURCE_DIR}/apps/editor/src/MainWindow.cpp
    ${CMAKE_SOURCE_DIR}/apps/editor/src/ScriptCommandSystem.cpp
    ${CMAKE_SOURCE_DIR}/apps/editor/src/ViewportWidget.cpp
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
    ${CMAKE_SOURCE_DIR}/src/logging/LogCategories.cpp
    ${CMAKE_SOURCE_DIR}/src/viewport/EditorCamera.cpp
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

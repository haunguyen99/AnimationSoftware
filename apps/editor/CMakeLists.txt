find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets OpenGL Gui)

add_executable(phoenix_editor
    main.cpp
    src/KeyframeTimelineWidget.cpp
    src/MainWindow.cpp
    src/ScriptCommandSystem.cpp
    src/ViewportWidget.cpp
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

target_include_directories(phoenix_editor
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(phoenix_editor
    PRIVATE
        assimp::assimp
        Qt6::Widgets
        Qt6::OpenGLWidgets
        Qt6::OpenGL
        Qt6::Gui
)

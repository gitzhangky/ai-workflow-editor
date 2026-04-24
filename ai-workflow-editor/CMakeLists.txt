cmake_minimum_required(VERSION 3.16)

project(AIWorkflowEditor LANGUAGES CXX)

include(FetchContent)

if(POLICY CMP0169)
  cmake_policy(SET CMP0169 OLD)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(AI_WORKFLOW_EDITOR_BUILD_TESTS "Build AI Workflow Editor tests" ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets Svg LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets Svg LinguistTools)

FetchContent_Declare(
  breeze_stylesheets
  GIT_REPOSITORY https://github.com/Alexhuszagh/BreezeStyleSheets.git
  GIT_TAG 0.0.1
  GIT_SHALLOW TRUE
)
FetchContent_GetProperties(breeze_stylesheets)
if(NOT breeze_stylesheets_POPULATED)
  FetchContent_Populate(breeze_stylesheets)
endif()

set(QT_NODES_DEVELOPER_DEFAULTS OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_DOCS OFF CACHE BOOL "" FORCE)

if(QT_VERSION_MAJOR EQUAL 6)
  set(USE_QT6 ON CACHE BOOL "" FORCE)
else()
  set(USE_QT6 OFF CACHE BOOL "" FORCE)
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/nodeeditor
                 ${CMAKE_CURRENT_BINARY_DIR}/third_party/nodeeditor)

set(AI_WORKFLOW_EDITOR_TS_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/src/i18n/ai_workflow_editor_zh_CN.ts
)

if(QT_VERSION_MAJOR EQUAL 6)
  qt_add_translation(AI_WORKFLOW_EDITOR_QM_FILES ${AI_WORKFLOW_EDITOR_TS_FILES})
else()
  qt5_add_translation(AI_WORKFLOW_EDITOR_QM_FILES ${AI_WORKFLOW_EDITOR_TS_FILES})
endif()

set(AI_WORKFLOW_EDITOR_TRANSLATIONS_DIR ${CMAKE_CURRENT_BINARY_DIR}/i18n)
set(AI_WORKFLOW_EDITOR_COPIED_QM_FILES)
foreach(QM_FILE ${AI_WORKFLOW_EDITOR_QM_FILES})
  get_filename_component(QM_NAME ${QM_FILE} NAME)
  set(COPIED_QM_FILE ${AI_WORKFLOW_EDITOR_TRANSLATIONS_DIR}/${QM_NAME})
  add_custom_command(
    OUTPUT ${COPIED_QM_FILE}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${AI_WORKFLOW_EDITOR_TRANSLATIONS_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${QM_FILE} ${COPIED_QM_FILE}
    DEPENDS ${QM_FILE}
  )
  list(APPEND AI_WORKFLOW_EDITOR_COPIED_QM_FILES ${COPIED_QM_FILE})
endforeach()

add_custom_target(ai_workflow_editor_translations ALL
  DEPENDS ${AI_WORKFLOW_EDITOR_COPIED_QM_FILES}
)

set(AI_WORKFLOW_EDITOR_TRANSLATIONS_QRC ${CMAKE_CURRENT_BINARY_DIR}/app_translations.qrc)
file(WRITE ${AI_WORKFLOW_EDITOR_TRANSLATIONS_QRC} "<RCC>\n  <qresource prefix=\"/app-translations\">\n")
foreach(COPIED_QM_FILE ${AI_WORKFLOW_EDITOR_COPIED_QM_FILES})
  get_filename_component(QM_NAME ${COPIED_QM_FILE} NAME)
  file(APPEND ${AI_WORKFLOW_EDITOR_TRANSLATIONS_QRC}
       "    <file alias=\"${QM_NAME}\">${COPIED_QM_FILE}</file>\n")
endforeach()
file(APPEND ${AI_WORKFLOW_EDITOR_TRANSLATIONS_QRC} "  </qresource>\n</RCC>\n")

add_library(ai_workflow_editor_breeze_resources STATIC
  ${breeze_stylesheets_SOURCE_DIR}/dist/breeze.qrc
)

target_link_libraries(ai_workflow_editor_breeze_resources
  PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Svg
)

add_library(ai_workflow_editor_lib
  src/app/LanguageManager.cpp
  src/app/LightTheme.cpp
  src/app/MainWindow.cpp
  src/app/NodeLibraryListWidget.cpp
  src/domain/WorkflowNodeDefinition.cpp
  src/inspector/InspectorFieldSchema.cpp
  src/inspector/InspectorPanel.cpp
  src/registry/BuiltInNodeRegistry.cpp
  src/qtnodes/EdgeAlignedNodeGeometry.cpp
  src/qtnodes/StaticNodeDelegateModel.cpp
  src/qtnodes/QtNodesEditorWidget.cpp
  src/qtnodes/UndoCommands.cpp
  src/qtnodes/WorkflowGraphModel.cpp
  src/qtnodes/StyledNodePainter.cpp
  src/export/WorkflowExporter.cpp
  src/resources/app_theme.qrc
  ${AI_WORKFLOW_EDITOR_TRANSLATIONS_QRC}
)

target_include_directories(ai_workflow_editor_lib
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(ai_workflow_editor_lib
  PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Svg
    QtNodes
    ai_workflow_editor_breeze_resources
)

add_dependencies(ai_workflow_editor_lib ai_workflow_editor_translations)

add_executable(ai-workflow-editor
  src/main.cpp
)

target_link_libraries(ai-workflow-editor
  PRIVATE
    ai_workflow_editor_lib
)

if(AI_WORKFLOW_EDITOR_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

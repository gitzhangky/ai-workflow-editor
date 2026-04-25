find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test)

add_executable(ai_workflow_editor_domain_tests
  domain/BuiltInNodeRegistryTests.cpp
  ../src/domain/WorkflowNodeDefinition.cpp
  ../src/registry/BuiltInNodeRegistry.cpp
)

target_include_directories(ai_workflow_editor_domain_tests
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(ai_workflow_editor_domain_tests
  PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
)

add_test(NAME ai_workflow_editor_domain_tests COMMAND ai_workflow_editor_domain_tests)

add_executable(ai_workflow_editor_app_tests
  app/MainWindowTests.cpp
)

target_include_directories(ai_workflow_editor_app_tests
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(ai_workflow_editor_app_tests
  PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
    ai_workflow_editor_lib
)

add_test(NAME ai_workflow_editor_app_tests COMMAND ai_workflow_editor_app_tests)

add_executable(ai_workflow_editor_theme_tests
  app/LightThemeTests.cpp
)

target_include_directories(ai_workflow_editor_theme_tests
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(ai_workflow_editor_theme_tests
  PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Test
    ai_workflow_editor_lib
)

add_test(NAME ai_workflow_editor_theme_tests COMMAND ai_workflow_editor_theme_tests)

add_executable(ai_workflow_editor_widget_tests
  app/NodeLibraryListWidgetTests.cpp
)

target_include_directories(ai_workflow_editor_widget_tests
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(ai_workflow_editor_widget_tests
  PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
    ai_workflow_editor_lib
)

add_test(NAME ai_workflow_editor_widget_tests COMMAND ai_workflow_editor_widget_tests)

add_executable(ai_workflow_editor_export_tests
  export/WorkflowExporterTests.cpp
  ../src/export/WorkflowExporter.cpp
)

target_include_directories(ai_workflow_editor_export_tests
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(ai_workflow_editor_export_tests
  PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
)

add_test(NAME ai_workflow_editor_export_tests COMMAND ai_workflow_editor_export_tests)

add_executable(ai_workflow_editor_workflow_tests
  workflow/WorkflowDocumentTests.cpp
  ../src/workflow/WorkflowDocument.cpp
)

target_include_directories(ai_workflow_editor_workflow_tests
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(ai_workflow_editor_workflow_tests
  PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
)

add_test(NAME ai_workflow_editor_workflow_tests COMMAND ai_workflow_editor_workflow_tests)

add_executable(ai_workflow_editor_runtime_tests
  runtime/WorkflowRunnerTests.cpp
)

target_include_directories(ai_workflow_editor_runtime_tests
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(ai_workflow_editor_runtime_tests
  PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
    ai_workflow_editor_lib
)

add_test(NAME ai_workflow_editor_runtime_tests COMMAND ai_workflow_editor_runtime_tests)

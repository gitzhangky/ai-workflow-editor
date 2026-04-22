# AI Workflow Editor Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build the first runnable CMake-based desktop prototype of the AI Workflow Editor with a light-themed main window, `QtNodes` canvas, built-in node palette, and placeholder inspector.

**Architecture:** The product lives in `ai-workflow-editor/` as its own CMake application and links to `QtNodes` from `../third_party/nodeeditor`. Version one keeps workflow semantics in product-owned classes and uses a thin adapter layer to translate built-in node definitions into `QtNodes` node models for display and interaction.

**Tech Stack:** CMake, C++17, Qt Widgets, Qt Test, QtNodes

---

## File Map

### Product Files To Create

- `ai-workflow-editor/CMakeLists.txt`
- `ai-workflow-editor/src/main.cpp`
- `ai-workflow-editor/src/app/MainWindow.hpp`
- `ai-workflow-editor/src/app/MainWindow.cpp`
- `ai-workflow-editor/src/app/LightTheme.hpp`
- `ai-workflow-editor/src/app/LightTheme.cpp`
- `ai-workflow-editor/src/domain/WorkflowNodeDefinition.hpp`
- `ai-workflow-editor/src/domain/WorkflowNodeDefinition.cpp`
- `ai-workflow-editor/src/registry/BuiltInNodeRegistry.hpp`
- `ai-workflow-editor/src/registry/BuiltInNodeRegistry.cpp`
- `ai-workflow-editor/src/qtnodes/StaticNodeData.hpp`
- `ai-workflow-editor/src/qtnodes/StaticNodeDelegateModel.hpp`
- `ai-workflow-editor/src/qtnodes/StaticNodeDelegateModel.cpp`
- `ai-workflow-editor/src/qtnodes/QtNodesEditorWidget.hpp`
- `ai-workflow-editor/src/qtnodes/QtNodesEditorWidget.cpp`
- `ai-workflow-editor/src/inspector/InspectorPanel.hpp`
- `ai-workflow-editor/src/inspector/InspectorPanel.cpp`
- `ai-workflow-editor/tests/CMakeLists.txt`
- `ai-workflow-editor/tests/domain/BuiltInNodeRegistryTests.cpp`

### Existing Third-Party Files To Reuse

- `third_party/nodeeditor/CMakeLists.txt`
- `third_party/nodeeditor/include/QtNodes/NodeDelegateModel`
- `third_party/nodeeditor/include/QtNodes/NodeDelegateModelRegistry`
- `third_party/nodeeditor/include/QtNodes/DataFlowGraphModel`
- `third_party/nodeeditor/include/QtNodes/DataFlowGraphicsScene`
- `third_party/nodeeditor/include/QtNodes/GraphicsView`

## Task 1: Create The Product CMake Skeleton

**Files:**
- Create: `ai-workflow-editor/CMakeLists.txt`
- Create: `ai-workflow-editor/src/main.cpp`

- [ ] **Step 1: Write the failing launch test**

Create `ai-workflow-editor/tests/domain/BuiltInNodeRegistryTests.cpp` with:

```cpp
#include "registry/BuiltInNodeRegistry.hpp"

#include <QtTest/QTest>

class BuiltInNodeRegistryTests : public QObject
{
    Q_OBJECT

private slots:
    void exposesExpectedNodeTypes();
};

void BuiltInNodeRegistryTests::exposesExpectedNodeTypes()
{
    BuiltInNodeRegistry registry;

    const auto definitions = registry.definitions();

    QCOMPARE(definitions.size(), 6);
}

QTEST_APPLESS_MAIN(BuiltInNodeRegistryTests)

#include "BuiltInNodeRegistryTests.moc"
```

- [ ] **Step 2: Run test to verify it fails**

Run:

```bash
cmake -S /Users/zhangkaiyuan/Documents/Codex/2026-04-21-github-qt-c-nodeeditor/ai-workflow-editor -B /Users/zhangkaiyuan/Documents/Codex/2026-04-21-github-qt-c-nodeeditor/ai-workflow-editor/build
cmake --build /Users/zhangkaiyuan/Documents/Codex/2026-04-21-github-qt-c-nodeeditor/ai-workflow-editor/build --target ai_workflow_editor_tests
ctest --test-dir /Users/zhangkaiyuan/Documents/Codex/2026-04-21-github-qt-c-nodeeditor/ai-workflow-editor/build --output-on-failure
```

Expected: configure or build fails because product CMake files and registry classes do not exist yet.

- [ ] **Step 3: Write minimal implementation**

Create `ai-workflow-editor/CMakeLists.txt` with:

```cmake
cmake_minimum_required(VERSION 3.21)

project(AIWorkflowEditor LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(AI_WORKFLOW_EDITOR_BUILD_TESTS "Build AI Workflow Editor tests" ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets Test)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Test)

set(QT_NODES_DEVELOPER_DEFAULTS OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(USE_QT6 ${QT_VERSION_MAJOR} EQUAL 6 CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/nodeeditor
                 ${CMAKE_CURRENT_BINARY_DIR}/third_party/nodeeditor)

add_subdirectory(tests)

add_executable(ai-workflow-editor
  src/main.cpp
)

target_link_libraries(ai-workflow-editor
  PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    QtNodes
)
```

Create `ai-workflow-editor/src/main.cpp` with:

```cpp
#include <QApplication>
#include <QLabel>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QLabel label("AI Workflow Editor bootstrap");
    label.resize(320, 80);
    label.show();

    return app.exec();
}
```

- [ ] **Step 4: Run test to verify it still fails for the right reason**

Run:

```bash
cmake -S /Users/zhangkaiyuan/Documents/Codex/2026-04-21-github-qt-c-nodeeditor/ai-workflow-editor -B /Users/zhangkaiyuan/Documents/Codex/2026-04-21-github-qt-c-nodeeditor/ai-workflow-editor/build
cmake --build /Users/zhangkaiyuan/Documents/Codex/2026-04-21-github-qt-c-nodeeditor/ai-workflow-editor/build --target ai_workflow_editor_tests
```

Expected: build fails because the test target references `BuiltInNodeRegistry` files that still do not exist.

## Task 2: Add Built-In Node Definitions And Registry

**Files:**
- Create: `ai-workflow-editor/src/domain/WorkflowNodeDefinition.hpp`
- Create: `ai-workflow-editor/src/domain/WorkflowNodeDefinition.cpp`
- Create: `ai-workflow-editor/src/registry/BuiltInNodeRegistry.hpp`
- Create: `ai-workflow-editor/src/registry/BuiltInNodeRegistry.cpp`
- Create: `ai-workflow-editor/tests/CMakeLists.txt`
- Modify: `ai-workflow-editor/CMakeLists.txt`
- Test: `ai-workflow-editor/tests/domain/BuiltInNodeRegistryTests.cpp`

- [ ] **Step 1: Write the failing test**

Extend `ai-workflow-editor/tests/domain/BuiltInNodeRegistryTests.cpp` with:

```cpp
void BuiltInNodeRegistryTests::exposesExpectedNodeTypes()
{
    BuiltInNodeRegistry registry;

    const auto definitions = registry.definitions();

    QCOMPARE(definitions.size(), 6);
    QCOMPARE(definitions.at(0).typeKey, QString("start"));
    QCOMPARE(definitions.at(1).typeKey, QString("prompt"));
    QCOMPARE(definitions.at(2).typeKey, QString("llm"));
    QCOMPARE(definitions.at(3).typeKey, QString("tool"));
    QCOMPARE(definitions.at(4).typeKey, QString("condition"));
    QCOMPARE(definitions.at(5).typeKey, QString("output"));
}
```

- [ ] **Step 2: Run test to verify it fails**

Run:

```bash
cmake --build /Users/zhangkaiyuan/Documents/Codex/2026-04-21-github-qt-c-nodeeditor/ai-workflow-editor/build --target ai_workflow_editor_tests
ctest --test-dir /Users/zhangkaiyuan/Documents/Codex/2026-04-21-github-qt-c-nodeeditor/ai-workflow-editor/build --output-on-failure
```

Expected: build fails because registry headers and sources do not exist.

- [ ] **Step 3: Write minimal implementation**

Create `ai-workflow-editor/src/domain/WorkflowNodeDefinition.hpp` with:

```cpp
#pragma once

#include <QString>
#include <QStringList>
#include <QVector>

struct WorkflowPortDefinition
{
    QString id;
    QString label;
};

struct WorkflowNodeDefinition
{
    QString typeKey;
    QString category;
    QString displayName;
    QString description;
    QVector<WorkflowPortDefinition> inputPorts;
    QVector<WorkflowPortDefinition> outputPorts;
};
```

Create `ai-workflow-editor/src/domain/WorkflowNodeDefinition.cpp` with:

```cpp
#include "domain/WorkflowNodeDefinition.hpp"
```

Create `ai-workflow-editor/src/registry/BuiltInNodeRegistry.hpp` with:

```cpp
#pragma once

#include "domain/WorkflowNodeDefinition.hpp"

#include <QVector>

class BuiltInNodeRegistry
{
public:
    QVector<WorkflowNodeDefinition> definitions() const;
};
```

Create `ai-workflow-editor/src/registry/BuiltInNodeRegistry.cpp` with:

```cpp
#include "registry/BuiltInNodeRegistry.hpp"

QVector<WorkflowNodeDefinition> BuiltInNodeRegistry::definitions() const
{
    return {
        {"start", "Flow", "Start", "Workflow entry point", {}, {{"out", "Out"}}},
        {"prompt", "AI", "Prompt", "Prompt template node", {{"in", "In"}}, {{"out", "Out"}}},
        {"llm", "AI", "LLM", "Model invocation node", {{"in", "In"}}, {{"success", "Success"}, {"error", "Error"}}},
        {"tool", "Integration", "Tool", "Tool invocation node", {{"in", "In"}}, {{"out", "Out"}}},
        {"condition", "Flow", "Condition", "Branching node", {{"in", "In"}}, {{"true", "True"}, {"false", "False"}}},
        {"output", "Flow", "Output", "Workflow result node", {{"in", "In"}}, {}}
    };
}
```

Create `ai-workflow-editor/tests/CMakeLists.txt` with:

```cmake
add_executable(ai_workflow_editor_tests
  domain/BuiltInNodeRegistryTests.cpp
  ../src/domain/WorkflowNodeDefinition.cpp
  ../src/registry/BuiltInNodeRegistry.cpp
)

target_include_directories(ai_workflow_editor_tests
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(ai_workflow_editor_tests
  PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
)

add_test(NAME ai_workflow_editor_tests COMMAND ai_workflow_editor_tests)
```

Modify `ai-workflow-editor/CMakeLists.txt` to add:

```cmake
target_include_directories(ai-workflow-editor
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
```

- [ ] **Step 4: Run test to verify it passes**

Run:

```bash
cmake -S /Users/zhangkaiyuan/Documents/Codex/2026-04-21-github-qt-c-nodeeditor/ai-workflow-editor -B /Users/zhangkaiyuan/Documents/Codex/2026-04-21-github-qt-c-nodeeditor/ai-workflow-editor/build
cmake --build /Users/zhangkaiyuan/Documents/Codex/2026-04-21-github-qt-c-nodeeditor/ai-workflow-editor/build --target ai_workflow_editor_tests
ctest --test-dir /Users/zhangkaiyuan/Documents/Codex/2026-04-21-github-qt-c-nodeeditor/ai-workflow-editor/build --output-on-failure
```

Expected: `100% tests passed`.

## Task 3: Add A Thin QtNodes Editor Widget

**Files:**
- Create: `ai-workflow-editor/src/qtnodes/StaticNodeData.hpp`
- Create: `ai-workflow-editor/src/qtnodes/StaticNodeDelegateModel.hpp`
- Create: `ai-workflow-editor/src/qtnodes/StaticNodeDelegateModel.cpp`
- Create: `ai-workflow-editor/src/qtnodes/QtNodesEditorWidget.hpp`
- Create: `ai-workflow-editor/src/qtnodes/QtNodesEditorWidget.cpp`
- Modify: `ai-workflow-editor/CMakeLists.txt`

- [ ] **Step 1: Write the failing test**

Add a second test to `ai-workflow-editor/tests/domain/BuiltInNodeRegistryTests.cpp`:

```cpp
void BuiltInNodeRegistryTests::startNodeHasNoInputsAndOneOutput()
{
    BuiltInNodeRegistry registry;
    const auto definition = registry.definitions().front();

    QCOMPARE(definition.inputPorts.size(), 0);
    QCOMPARE(definition.outputPorts.size(), 1);
    QCOMPARE(definition.outputPorts.front().id, QString("out"));
}
```

- [ ] **Step 2: Run test to verify it fails**

Run:

```bash
cmake --build /Users/zhangkaiyuan/Documents/Codex/2026-04-21-github-qt-c-nodeeditor/ai-workflow-editor/build --target ai_workflow_editor_tests
ctest --test-dir /Users/zhangkaiyuan/Documents/Codex/2026-04-21-github-qt-c-nodeeditor/ai-workflow-editor/build --output-on-failure
```

Expected: test fails to compile until the declaration is added to the test class.

- [ ] **Step 3: Write minimal implementation**

Create `ai-workflow-editor/src/qtnodes/StaticNodeData.hpp` with:

```cpp
#pragma once

#include <QtNodes/NodeData>

class StaticNodeData : public QtNodes::NodeData
{
public:
    QtNodes::NodeDataType type() const override
    {
        return {"workflow", "Workflow"};
    }
};
```

Create `ai-workflow-editor/src/qtnodes/StaticNodeDelegateModel.hpp` with:

```cpp
#pragma once

#include "domain/WorkflowNodeDefinition.hpp"

#include <QtNodes/NodeDelegateModel>

class StaticNodeDelegateModel : public QtNodes::NodeDelegateModel
{
public:
    explicit StaticNodeDelegateModel(WorkflowNodeDefinition definition);

    QString caption() const override;
    bool captionVisible() const override;
    QString name() const override;
    unsigned int nPorts(QtNodes::PortType portType) const override;
    QtNodes::NodeDataType dataType(QtNodes::PortType, QtNodes::PortIndex) const override;
    std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex) override;
    void setInData(std::shared_ptr<QtNodes::NodeData>, QtNodes::PortIndex) override;
    QWidget *embeddedWidget() override;

private:
    WorkflowNodeDefinition _definition;
    std::shared_ptr<QtNodes::NodeData> _staticData;
};
```

Create `ai-workflow-editor/src/qtnodes/StaticNodeDelegateModel.cpp` with:

```cpp
#include "qtnodes/StaticNodeDelegateModel.hpp"
#include "qtnodes/StaticNodeData.hpp"

StaticNodeDelegateModel::StaticNodeDelegateModel(WorkflowNodeDefinition definition)
    : _definition(std::move(definition))
    , _staticData(std::make_shared<StaticNodeData>())
{
}

QString StaticNodeDelegateModel::caption() const { return _definition.displayName; }
bool StaticNodeDelegateModel::captionVisible() const { return true; }
QString StaticNodeDelegateModel::name() const { return _definition.typeKey; }

unsigned int StaticNodeDelegateModel::nPorts(QtNodes::PortType portType) const
{
    return portType == QtNodes::PortType::In ? _definition.inputPorts.size()
                                             : _definition.outputPorts.size();
}

QtNodes::NodeDataType StaticNodeDelegateModel::dataType(QtNodes::PortType, QtNodes::PortIndex) const
{
    return _staticData->type();
}

std::shared_ptr<QtNodes::NodeData> StaticNodeDelegateModel::outData(QtNodes::PortIndex)
{
    return _staticData;
}

void StaticNodeDelegateModel::setInData(std::shared_ptr<QtNodes::NodeData>, QtNodes::PortIndex) {}
QWidget *StaticNodeDelegateModel::embeddedWidget() { return nullptr; }
```

Create `ai-workflow-editor/src/qtnodes/QtNodesEditorWidget.hpp` with:

```cpp
#pragma once

#include "registry/BuiltInNodeRegistry.hpp"

#include <QWidget>

namespace QtNodes
{
class DataFlowGraphicsScene;
class DataFlowGraphModel;
class GraphicsView;
class NodeDelegateModelRegistry;
}

class QtNodesEditorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QtNodesEditorWidget(QWidget *parent = nullptr);

private:
    std::shared_ptr<QtNodes::NodeDelegateModelRegistry> buildRegistry() const;

    BuiltInNodeRegistry _registry;
};
```

Create `ai-workflow-editor/src/qtnodes/QtNodesEditorWidget.cpp` with:

```cpp
#include "qtnodes/QtNodesEditorWidget.hpp"
#include "qtnodes/StaticNodeDelegateModel.hpp"

#include <QtNodes/DataFlowGraphModel>
#include <QtNodes/DataFlowGraphicsScene>
#include <QtNodes/GraphicsView>
#include <QtNodes/NodeDelegateModelRegistry>

#include <QVBoxLayout>

QtNodesEditorWidget::QtNodesEditorWidget(QWidget *parent)
    : QWidget(parent)
{
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    auto registry = buildRegistry();
    auto model = new QtNodes::DataFlowGraphModel(registry, this);
    auto scene = new QtNodes::DataFlowGraphicsScene(*model, this);
    auto view = new QtNodes::GraphicsView(scene);

    layout->addWidget(view);
}

std::shared_ptr<QtNodes::NodeDelegateModelRegistry> QtNodesEditorWidget::buildRegistry() const
{
    auto registry = std::make_shared<QtNodes::NodeDelegateModelRegistry>();
    for (const auto &definition : _registry.definitions()) {
        const auto category = definition.category.toStdString();
        registry->registerModel<StaticNodeDelegateModel>(category, [definition]() {
            return std::make_unique<StaticNodeDelegateModel>(definition);
        });
    }
    return registry;
}
```

Modify `ai-workflow-editor/CMakeLists.txt` so the app target includes these new source files.

- [ ] **Step 4: Run test to verify existing tests still pass**

Run:

```bash
cmake --build /Users/zhangkaiyuan/Documents/Codex/2026-04-21-github-qt-c-nodeeditor/ai-workflow-editor/build --target ai_workflow_editor_tests
ctest --test-dir /Users/zhangkaiyuan/Documents/Codex/2026-04-21-github-qt-c-nodeeditor/ai-workflow-editor/build --output-on-failure
```

Expected: tests remain green while the editor widget compiles.

## Task 4: Build The Main Window Shell

**Files:**
- Create: `ai-workflow-editor/src/app/MainWindow.hpp`
- Create: `ai-workflow-editor/src/app/MainWindow.cpp`
- Create: `ai-workflow-editor/src/app/LightTheme.hpp`
- Create: `ai-workflow-editor/src/app/LightTheme.cpp`
- Create: `ai-workflow-editor/src/inspector/InspectorPanel.hpp`
- Create: `ai-workflow-editor/src/inspector/InspectorPanel.cpp`
- Modify: `ai-workflow-editor/src/main.cpp`
- Modify: `ai-workflow-editor/CMakeLists.txt`

- [ ] **Step 1: Write the failing test**

Add a third test to `ai-workflow-editor/tests/domain/BuiltInNodeRegistryTests.cpp`:

```cpp
void BuiltInNodeRegistryTests::conditionNodeHasTrueAndFalsePorts()
{
    BuiltInNodeRegistry registry;
    const auto definition = registry.definitions().at(4);

    QCOMPARE(definition.outputPorts.size(), 2);
    QCOMPARE(definition.outputPorts.at(0).id, QString("true"));
    QCOMPARE(definition.outputPorts.at(1).id, QString("false"));
}
```

- [ ] **Step 2: Run test to verify it fails**

Run:

```bash
cmake --build /Users/zhangkaiyuan/Documents/Codex/2026-04-21-github-qt-c-nodeeditor/ai-workflow-editor/build --target ai_workflow_editor_tests
ctest --test-dir /Users/zhangkaiyuan/Documents/Codex/2026-04-21-github-qt-c-nodeeditor/ai-workflow-editor/build --output-on-failure
```

Expected: test fails until the test class declaration is updated and the registry definition order is still intact.

- [ ] **Step 3: Write minimal implementation**

Create `ai-workflow-editor/src/inspector/InspectorPanel.hpp` with:

```cpp
#pragma once

#include <QWidget>

class QLabel;
class QFormLayout;

class InspectorPanel : public QWidget
{
    Q_OBJECT

public:
    explicit InspectorPanel(QWidget *parent = nullptr);

private:
    QLabel *_titleLabel;
    QFormLayout *_formLayout;
};
```

Create `ai-workflow-editor/src/inspector/InspectorPanel.cpp` with:

```cpp
#include "inspector/InspectorPanel.hpp"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QVBoxLayout>

InspectorPanel::InspectorPanel(QWidget *parent)
    : QWidget(parent)
{
    auto layout = new QVBoxLayout(this);
    _titleLabel = new QLabel("Select a node to edit its configuration", this);
    layout->addWidget(_titleLabel);

    auto formHost = new QWidget(this);
    _formLayout = new QFormLayout(formHost);
    _formLayout->addRow("Name", new QLineEdit(formHost));
    _formLayout->addRow("Description", new QTextEdit(formHost));

    layout->addWidget(formHost);
    layout->addStretch();
}
```

Create `ai-workflow-editor/src/app/LightTheme.hpp` with:

```cpp
#pragma once

class QApplication;

namespace LightTheme
{
void apply(QApplication &app);
}
```

Create `ai-workflow-editor/src/app/LightTheme.cpp` with:

```cpp
#include "app/LightTheme.hpp"

#include <QApplication>
#include <QPalette>

namespace LightTheme
{
void apply(QApplication &app)
{
    QPalette palette;
    palette.setColor(QPalette::Window, QColor("#f4f1eb"));
    palette.setColor(QPalette::Base, QColor("#ffffff"));
    palette.setColor(QPalette::AlternateBase, QColor("#ebe5dc"));
    palette.setColor(QPalette::Text, QColor("#2d241f"));
    palette.setColor(QPalette::Button, QColor("#ebe5dc"));
    palette.setColor(QPalette::ButtonText, QColor("#2d241f"));
    palette.setColor(QPalette::Highlight, QColor("#4b84d9"));
    palette.setColor(QPalette::HighlightedText, QColor("#ffffff"));
    app.setPalette(palette);
}
}
```

Create `ai-workflow-editor/src/app/MainWindow.hpp` with:

```cpp
#pragma once

#include <QMainWindow>

class QListWidget;
class QtNodesEditorWidget;
class InspectorPanel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    QListWidget *createNodeLibrary();

    QtNodesEditorWidget *_editorWidget;
    InspectorPanel *_inspectorPanel;
};
```

Create `ai-workflow-editor/src/app/MainWindow.cpp` with:

```cpp
#include "app/MainWindow.hpp"

#include "inspector/InspectorPanel.hpp"
#include "qtnodes/QtNodesEditorWidget.hpp"

#include <QDockWidget>
#include <QListWidget>
#include <QStatusBar>
#include <QToolBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("AI Workflow Editor");
    resize(1440, 920);

    auto toolbar = addToolBar("Main");
    toolbar->addAction("New");
    toolbar->addAction("Open");
    toolbar->addAction("Save");
    toolbar->addSeparator();
    toolbar->addAction("Undo");
    toolbar->addAction("Redo");
    toolbar->addAction("Center");

    auto nodeDock = new QDockWidget("Node Library", this);
    nodeDock->setWidget(createNodeLibrary());
    addDockWidget(Qt::LeftDockWidgetArea, nodeDock);

    auto inspectorDock = new QDockWidget("Inspector", this);
    _inspectorPanel = new InspectorPanel(this);
    inspectorDock->setWidget(_inspectorPanel);
    addDockWidget(Qt::RightDockWidgetArea, inspectorDock);

    _editorWidget = new QtNodesEditorWidget(this);
    setCentralWidget(_editorWidget);

    statusBar()->showMessage("Ready");
}

QListWidget *MainWindow::createNodeLibrary()
{
    auto *list = new QListWidget(this);
    list->addItem("Start");
    list->addItem("Prompt");
    list->addItem("LLM");
    list->addItem("Tool");
    list->addItem("Condition");
    list->addItem("Output");
    return list;
}
```

Replace `ai-workflow-editor/src/main.cpp` with:

```cpp
#include "app/LightTheme.hpp"
#include "app/MainWindow.hpp"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    LightTheme::apply(app);

    MainWindow window;
    window.show();

    return app.exec();
}
```

Modify `ai-workflow-editor/CMakeLists.txt` so the app target includes all new source files.

- [ ] **Step 4: Run test and app verification**

Run:

```bash
cmake --build /Users/zhangkaiyuan/Documents/Codex/2026-04-21-github-qt-c-nodeeditor/ai-workflow-editor/build --target ai_workflow_editor_tests ai-workflow-editor
ctest --test-dir /Users/zhangkaiyuan/Documents/Codex/2026-04-21-github-qt-c-nodeeditor/ai-workflow-editor/build --output-on-failure
```

Expected: tests pass and the application target builds successfully.

## Task 5: Verify The First Runnable Slice

**Files:**
- Modify as needed based on build output.

- [ ] **Step 1: Run the built app**

Run:

```bash
/Users/zhangkaiyuan/Documents/Codex/2026-04-21-github-qt-c-nodeeditor/ai-workflow-editor/build/ai-workflow-editor
```

Expected: a light-themed desktop window opens with a toolbar, node library, QtNodes canvas, and inspector panel.

- [ ] **Step 2: Fix any build or launch issues minimally**

Only apply the smallest code changes required by compiler or runtime feedback.

- [ ] **Step 3: Re-run full verification**

Run:

```bash
cmake --build /Users/zhangkaiyuan/Documents/Codex/2026-04-21-github-qt-c-nodeeditor/ai-workflow-editor/build
ctest --test-dir /Users/zhangkaiyuan/Documents/Codex/2026-04-21-github-qt-c-nodeeditor/ai-workflow-editor/build --output-on-failure
```

Expected: build succeeds and tests pass after any launch fixes.

## Self-Review

- Spec coverage: this plan covers the CMake project skeleton, light-themed shell, built-in node definitions, QtNodes integration, node library, inspector placeholder, and basic verification for the first runnable prototype.
- Placeholder scan: all tasks use explicit file paths and concrete commands.
- Type consistency: `WorkflowNodeDefinition`, `BuiltInNodeRegistry`, `StaticNodeDelegateModel`, `QtNodesEditorWidget`, `InspectorPanel`, and `MainWindow` names are consistent across tasks.

## Execution Handoff

This plan is saved to `ai-workflow-editor/docs/plans/2026-04-21-ai-workflow-editor-implementation-plan.md`.

The user already requested that development begin immediately, so execution should continue inline in this session using this plan as the source of truth.

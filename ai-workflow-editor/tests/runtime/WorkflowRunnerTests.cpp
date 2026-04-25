#include "runtime/WorkflowRunner.hpp"

#include <QJsonArray>
#include <QJsonObject>
#include <QtTest/QtTest>

namespace
{
QJsonObject node(int id, QString const &type, QString const &name, QJsonObject const &properties = {})
{
    return QJsonObject{{"id", id},
                       {"type", type},
                       {"displayName", name},
                       {"description", QString()},
                       {"properties", properties},
                       {"position", QJsonObject{{"x", id * 100.0}, {"y", 0.0}}}};
}

QJsonObject connection(int outNodeId, int outPortIndex, int inNodeId, int inPortIndex)
{
    return QJsonObject{{"outNodeId", outNodeId},
                       {"outPortIndex", outPortIndex},
                       {"inNodeId", inNodeId},
                       {"inPortIndex", inPortIndex}};
}
}

class WorkflowRunnerTests : public QObject
{
    Q_OBJECT

private slots:
    void runsPromptLlmOutputWorkflowWithTrace();
    void rendersStringBackedJsonTransformProperties();
};

void WorkflowRunnerTests::runsPromptLlmOutputWorkflowWithTrace()
{
    QJsonObject workflow;
    workflow["version"] = 2;
    workflow["nodes"] = QJsonArray{node(1, "start", "Start"),
                                   node(2,
                                        "prompt",
                                        "Prompt",
                                        QJsonObject{{"userPromptTemplate", "Summarize: {{input}}"}}),
                                   node(3, "llm", "LLM", QJsonObject{{"modelName", "mock-model"}}),
                                   node(4, "output", "Output")};
    workflow["connections"] = QJsonArray{connection(1, 0, 2, 0), connection(2, 0, 3, 0), connection(3, 0, 4, 0)};

    const auto result = WorkflowRunner::run(workflow, "A long document");

    QVERIFY2(result.success, qPrintable(result.errorMessage));
    QVERIFY(result.finalOutput.contains("Mock LLM response"));
    QVERIFY(result.finalOutput.contains("mock-model"));
    QCOMPARE(result.steps.size(), 4);
    QCOMPARE(result.steps.at(1).output, QString("Summarize: A long document"));
    QCOMPARE(result.steps.last().typeKey, QString("output"));
    QCOMPARE(result.steps.last().output, result.finalOutput);
}

void WorkflowRunnerTests::rendersStringBackedJsonTransformProperties()
{
    QJsonObject workflow;
    workflow["version"] = 2;
    workflow["nodes"] = QJsonArray{node(1, "start", "Start"),
                                   node(2,
                                        "jsonTransform",
                                        "JSON Transform",
                                        QJsonObject{{"transformJson", "{\"summary\":\"{{input}}\"}"}}),
                                   node(3, "output", "Output")};
    workflow["connections"] = QJsonArray{connection(1, 0, 2, 0), connection(2, 0, 3, 0)};

    const auto result = WorkflowRunner::run(workflow, "Important input");

    QVERIFY2(result.success, qPrintable(result.errorMessage));
    QCOMPARE(result.steps.at(1).output, QString("{\"summary\":\"Important input\"}"));
    QCOMPARE(result.finalOutput, QString("{\"summary\":\"Important input\"}"));
}

QTEST_MAIN(WorkflowRunnerTests)
#include "WorkflowRunnerTests.moc"

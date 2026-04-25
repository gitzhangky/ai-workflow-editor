#include "workflow/WorkflowDocument.hpp"

#include <QJsonArray>
#include <QJsonObject>
#include <QtTest/QtTest>

class WorkflowDocumentTests : public QObject
{
    Q_OBJECT

private slots:
    void parsesAndSerializesVersionedWorkflowRecords();
};

void WorkflowDocumentTests::parsesAndSerializesVersionedWorkflowRecords()
{
    QJsonObject node;
    node["id"] = 7;
    node["type"] = "prompt";
    node["displayName"] = "Summarizer";
    node["description"] = "Create a short summary";
    node["properties"] = QJsonObject{{"userPromptTemplate", "Summarize {{input}}"}};
    node["position"] = QJsonObject{{"x", 120.5}, {"y", -32.0}};

    QJsonObject outputNode;
    outputNode["id"] = 9;
    outputNode["type"] = "output";
    outputNode["displayName"] = "Output";
    outputNode["description"] = "Return the final response";
    outputNode["properties"] = QJsonObject{};
    outputNode["position"] = QJsonObject{{"x", 320.0}, {"y", -32.0}};

    QJsonObject connection;
    connection["outNodeId"] = 7;
    connection["outPortIndex"] = 0;
    connection["inNodeId"] = 9;
    connection["inPortIndex"] = 0;

    QJsonObject root;
    root["version"] = 2;
    root["nodes"] = QJsonArray{node, outputNode};
    root["connections"] = QJsonArray{connection};

    QString error;
    const auto document = WorkflowDocument::fromJson(root, &error);
    QVERIFY2(document.has_value(), qPrintable(error));

    QCOMPARE(document->version(), 2);
    QCOMPARE(document->nodes().size(), 2);
    QCOMPARE(document->connections().size(), 1);

    const auto parsedNode = document->nodes().front();
    QCOMPARE(parsedNode.serializedId, 7);
    QCOMPARE(parsedNode.typeKey, QString("prompt"));
    QCOMPARE(parsedNode.displayName, QString("Summarizer"));
    QCOMPARE(parsedNode.description, QString("Create a short summary"));
    QCOMPARE(parsedNode.properties.value("userPromptTemplate").toString(), QString("Summarize {{input}}"));
    QCOMPARE(parsedNode.position, QPointF(120.5, -32.0));

    const auto parsedConnection = document->connections().front();
    QCOMPARE(parsedConnection.outSerializedNodeId, 7);
    QCOMPARE(parsedConnection.outPortIndex, 0);
    QCOMPARE(parsedConnection.inSerializedNodeId, 9);
    QCOMPARE(parsedConnection.inPortIndex, 0);

    const auto serialized = document->toJson();
    QCOMPARE(serialized["version"].toInt(), 2);
    QCOMPARE(serialized["nodes"].toArray().first().toObject()["displayName"].toString(), QString("Summarizer"));
    QCOMPARE(serialized["connections"].toArray().first().toObject()["inNodeId"].toInt(), 9);
}

QTEST_MAIN(WorkflowDocumentTests)
#include "WorkflowDocumentTests.moc"

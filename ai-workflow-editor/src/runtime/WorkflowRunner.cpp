#include "runtime/WorkflowRunner.hpp"

#include "workflow/WorkflowDocument.hpp"

#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QQueue>
#include <QSet>

namespace
{
QString renderTemplate(QString text, QString const &input)
{
    text.replace(QStringLiteral("{{input}}"), input);
    return text;
}

QString compactJson(QVariantMap const &properties, QString const &key)
{
    const QVariant value = properties.value(key);
    if (value.canConvert<QVariantMap>())
        return QString::fromUtf8(QJsonDocument(QJsonObject::fromVariantMap(value.toMap())).toJson(QJsonDocument::Compact));

    const QString text = value.toString().trimmed();
    const auto document = QJsonDocument::fromJson(text.toUtf8());
    if (document.isObject() || document.isArray())
        return QString::fromUtf8(document.toJson(QJsonDocument::Compact));
    return text;
}

QString executeNode(WorkflowDocument::NodeRecord const &node, QString const &input)
{
    const auto type = node.typeKey;
    if (type == QStringLiteral("start"))
        return input;

    if (type == QStringLiteral("prompt")) {
        const QString userPrompt = node.properties.value(QStringLiteral("userPromptTemplate")).toString();
        return renderTemplate(userPrompt.isEmpty() ? input : userPrompt, input);
    }

    if (type == QStringLiteral("llm")) {
        const QString modelName = node.properties.value(QStringLiteral("modelName"), QStringLiteral("mock-llm")).toString();
        return QStringLiteral("Mock LLM response (%1): %2").arg(modelName, input);
    }

    if (type == QStringLiteral("agent")) {
        const QString modelName = node.properties.value(QStringLiteral("modelName"), QStringLiteral("mock-agent")).toString();
        return QStringLiteral("Mock Agent response (%1): %2").arg(modelName, input);
    }

    if (type == QStringLiteral("templateVariables"))
        return compactJson(node.properties, QStringLiteral("variablesJson"));

    if (type == QStringLiteral("jsonTransform"))
        return renderTemplate(compactJson(node.properties, QStringLiteral("transformJson")), input);

    if (type == QStringLiteral("httpRequest")) {
        const QString method = node.properties.value(QStringLiteral("method"), QStringLiteral("GET")).toString();
        const QString url = node.properties.value(QStringLiteral("url")).toString();
        return QStringLiteral("Mock HTTP %1 %2").arg(method, url);
    }

    if (type == QStringLiteral("retriever")) {
        const QString key = node.properties.value(QStringLiteral("retrieverKey"), QStringLiteral("mock-retriever")).toString();
        return QStringLiteral("Mock retrieved context (%1): %2").arg(key, input);
    }

    if (type == QStringLiteral("memory")) {
        const QString key = node.properties.value(QStringLiteral("memoryKey"), QStringLiteral("mock-memory")).toString();
        return QStringLiteral("Mock memory value (%1): %2").arg(key, input);
    }

    if (type == QStringLiteral("tool")) {
        const QString toolName = node.properties.value(QStringLiteral("toolName"), QStringLiteral("mock-tool")).toString();
        return QStringLiteral("Mock tool result (%1): %2").arg(toolName, input);
    }

    if (type == QStringLiteral("chatOutput")) {
        const QString messageTemplate = node.properties.value(QStringLiteral("messageTemplate")).toString();
        return renderTemplate(messageTemplate.isEmpty() ? input : messageTemplate, input);
    }

    if (type == QStringLiteral("condition"))
        return input;

    if (type == QStringLiteral("output"))
        return input;

    return input;
}
}

WorkflowRunner::Result WorkflowRunner::run(QJsonObject const &workflow, QString const &inputText)
{
    QString parseError;
    const auto document = WorkflowDocument::fromJson(workflow, &parseError);
    if (!document)
        return {false, {}, parseError, {}};

    QMap<int, WorkflowDocument::NodeRecord> nodesById;
    QMap<int, QVector<WorkflowDocument::ConnectionRecord>> outgoing;
    QMap<int, int> incomingCount;
    for (auto const &node : document->nodes()) {
        nodesById.insert(node.serializedId, node);
        incomingCount.insert(node.serializedId, 0);
    }

    for (auto const &connection : document->connections()) {
        outgoing[connection.outSerializedNodeId].push_back(connection);
        incomingCount[connection.inSerializedNodeId] = incomingCount.value(connection.inSerializedNodeId) + 1;
    }

    QQueue<int> queue;
    for (auto it = incomingCount.cbegin(); it != incomingCount.cend(); ++it) {
        if (it.value() == 0)
            queue.enqueue(it.key());
    }

    QMap<int, QString> nodeOutputs;
    QSet<int> visited;
    Result result;
    result.success = true;
    while (!queue.isEmpty()) {
        const int nodeId = queue.dequeue();
        if (visited.contains(nodeId))
            continue;
        visited.insert(nodeId);

        const auto node = nodesById.value(nodeId);
        QString nodeInput = inputText;
        for (auto const &connection : document->connections()) {
            if (connection.inSerializedNodeId == nodeId) {
                nodeInput = nodeOutputs.value(connection.outSerializedNodeId);
                break;
            }
        }

        const QString nodeOutput = executeNode(node, nodeInput);
        nodeOutputs.insert(nodeId, nodeOutput);
        result.finalOutput = nodeOutput;
        result.steps.push_back(Step{node.serializedId, node.typeKey, node.displayName, nodeInput, nodeOutput, QStringLiteral("ok")});

        for (auto const &connection : outgoing.value(nodeId)) {
            const int nextNodeId = connection.inSerializedNodeId;
            incomingCount[nextNodeId] = incomingCount.value(nextNodeId) - 1;
            if (incomingCount.value(nextNodeId) <= 0)
                queue.enqueue(nextNodeId);
        }
    }

    if (visited.size() != nodesById.size()) {
        result.success = false;
        result.errorMessage = QStringLiteral("Workflow contains nodes that cannot be reached during preview.");
    }

    return result;
}

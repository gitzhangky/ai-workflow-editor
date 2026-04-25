#include "workflow/WorkflowDocument.hpp"

#include <QJsonArray>
#include <QJsonValue>
#include <QSet>

#include <limits>

namespace
{
void setError(QString *errorMessage, QString const &message)
{
    if (errorMessage != nullptr)
        *errorMessage = message;
}
}

WorkflowDocument::WorkflowDocument(QVector<NodeRecord> nodes, QVector<ConnectionRecord> connections, int version)
    : _version(version)
    , _nodes(std::move(nodes))
    , _connections(std::move(connections))
{
}

std::optional<WorkflowDocument> WorkflowDocument::fromJson(QJsonObject const &root, QString *errorMessage)
{
    const int version = root.value(QStringLiteral("version")).toInt(1);
    if (version < 1 || version > CurrentVersion) {
        setError(errorMessage, QStringLiteral("Unsupported workflow file version."));
        return std::nullopt;
    }

    const auto nodesValue = root.value(QStringLiteral("nodes"));
    const auto connectionsValue = root.value(QStringLiteral("connections"));
    if (!nodesValue.isArray() || !connectionsValue.isArray()) {
        setError(errorMessage, QStringLiteral("Workflow file must contain nodes and connections arrays."));
        return std::nullopt;
    }

    QVector<NodeRecord> nodes;
    QSet<int> serializedIds;
    for (auto const &nodeValue : nodesValue.toArray()) {
        if (!nodeValue.isObject()) {
            setError(errorMessage, QStringLiteral("Workflow node entry must be an object."));
            return std::nullopt;
        }

        const auto nodeObject = nodeValue.toObject();
        const int serializedId =
            nodeObject.value(QStringLiteral("id")).toInt(std::numeric_limits<int>::min());
        if (serializedId == std::numeric_limits<int>::min() || serializedIds.contains(serializedId)) {
            setError(errorMessage, QStringLiteral("Workflow node id is missing or duplicated."));
            return std::nullopt;
        }

        const QString typeKey = nodeObject.value(QStringLiteral("type")).toString();
        if (typeKey.trimmed().isEmpty()) {
            setError(errorMessage, QStringLiteral("Workflow node type is missing."));
            return std::nullopt;
        }

        const auto positionObject = nodeObject.value(QStringLiteral("position")).toObject();
        NodeRecord node;
        node.serializedId = serializedId;
        node.typeKey = typeKey;
        node.displayName = nodeObject.value(QStringLiteral("displayName")).toString();
        node.description = nodeObject.value(QStringLiteral("description")).toString();
        node.properties = nodeObject.value(QStringLiteral("properties")).toObject().toVariantMap();
        node.position =
            QPointF(positionObject.value(QStringLiteral("x")).toDouble(), positionObject.value(QStringLiteral("y")).toDouble());

        serializedIds.insert(serializedId);
        nodes.push_back(std::move(node));
    }

    QVector<ConnectionRecord> connections;
    for (auto const &connectionValue : connectionsValue.toArray()) {
        if (!connectionValue.isObject()) {
            setError(errorMessage, QStringLiteral("Workflow connection entry must be an object."));
            return std::nullopt;
        }

        const auto connectionObject = connectionValue.toObject();
        ConnectionRecord connection;
        connection.outSerializedNodeId =
            connectionObject.value(QStringLiteral("outNodeId")).toInt(std::numeric_limits<int>::min());
        connection.outPortIndex =
            connectionObject.value(QStringLiteral("outPortIndex")).toInt(std::numeric_limits<int>::min());
        connection.inSerializedNodeId =
            connectionObject.value(QStringLiteral("inNodeId")).toInt(std::numeric_limits<int>::min());
        connection.inPortIndex =
            connectionObject.value(QStringLiteral("inPortIndex")).toInt(std::numeric_limits<int>::min());

        if (connection.outSerializedNodeId == std::numeric_limits<int>::min()
            || connection.outPortIndex == std::numeric_limits<int>::min()
            || connection.inSerializedNodeId == std::numeric_limits<int>::min()
            || connection.inPortIndex == std::numeric_limits<int>::min()) {
            setError(errorMessage, QStringLiteral("Workflow connection endpoint is incomplete."));
            return std::nullopt;
        }

        if (!serializedIds.contains(connection.outSerializedNodeId)
            || !serializedIds.contains(connection.inSerializedNodeId)) {
            setError(errorMessage, QStringLiteral("Workflow connection references a missing node."));
            return std::nullopt;
        }

        connections.push_back(connection);
    }

    return WorkflowDocument(std::move(nodes), std::move(connections), version);
}

int WorkflowDocument::version() const
{
    return _version;
}

QVector<WorkflowDocument::NodeRecord> const &WorkflowDocument::nodes() const
{
    return _nodes;
}

QVector<WorkflowDocument::ConnectionRecord> const &WorkflowDocument::connections() const
{
    return _connections;
}

QJsonObject WorkflowDocument::toJson() const
{
    QJsonObject root;
    root[QStringLiteral("version")] = CurrentVersion;

    QJsonArray nodes;
    for (auto const &nodeRecord : _nodes) {
        QJsonObject node;
        node[QStringLiteral("id")] = nodeRecord.serializedId;
        node[QStringLiteral("type")] = nodeRecord.typeKey;
        node[QStringLiteral("displayName")] = nodeRecord.displayName;
        node[QStringLiteral("description")] = nodeRecord.description;
        node[QStringLiteral("properties")] = QJsonObject::fromVariantMap(nodeRecord.properties);
        node[QStringLiteral("position")] =
            QJsonObject{{QStringLiteral("x"), nodeRecord.position.x()}, {QStringLiteral("y"), nodeRecord.position.y()}};
        nodes.push_back(node);
    }
    root[QStringLiteral("nodes")] = nodes;

    QJsonArray connections;
    for (auto const &connectionRecord : _connections) {
        QJsonObject connection;
        connection[QStringLiteral("outNodeId")] = connectionRecord.outSerializedNodeId;
        connection[QStringLiteral("outPortIndex")] = connectionRecord.outPortIndex;
        connection[QStringLiteral("inNodeId")] = connectionRecord.inSerializedNodeId;
        connection[QStringLiteral("inPortIndex")] = connectionRecord.inPortIndex;
        connections.push_back(connection);
    }
    root[QStringLiteral("connections")] = connections;

    return root;
}

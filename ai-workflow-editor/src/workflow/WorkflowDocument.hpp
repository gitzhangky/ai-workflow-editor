#pragma once

#include <QJsonObject>
#include <QPointF>
#include <QString>
#include <QVariantMap>
#include <QVector>

#include <optional>

class WorkflowDocument
{
public:
    struct NodeRecord
    {
        int serializedId = 0;
        QString typeKey;
        QString displayName;
        QString description;
        QVariantMap properties;
        QPointF position;
    };

    struct ConnectionRecord
    {
        int outSerializedNodeId = 0;
        int outPortIndex = 0;
        int inSerializedNodeId = 0;
        int inPortIndex = 0;
    };

    static constexpr int CurrentVersion = 2;

    WorkflowDocument() = default;
    WorkflowDocument(QVector<NodeRecord> nodes, QVector<ConnectionRecord> connections, int version = CurrentVersion);

    static std::optional<WorkflowDocument> fromJson(QJsonObject const &root, QString *errorMessage = nullptr);

    int version() const;
    QVector<NodeRecord> const &nodes() const;
    QVector<ConnectionRecord> const &connections() const;
    QJsonObject toJson() const;

private:
    int _version = CurrentVersion;
    QVector<NodeRecord> _nodes;
    QVector<ConnectionRecord> _connections;
};

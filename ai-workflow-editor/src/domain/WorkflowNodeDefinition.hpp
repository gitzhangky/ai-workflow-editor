#pragma once

#include <QString>
#include <QVariantMap>
#include <QVector>

struct WorkflowPortDefinition
{
    QString id;
    QString label;
    QString dataTypeId;
};

struct WorkflowNodeDefinition
{
    QString typeKey;
    QString categoryKey;
    QString categoryDisplayName;
    QString displayName;
    QString description;
    QVariantMap defaultProperties;
    QVector<WorkflowPortDefinition> inputPorts;
    QVector<WorkflowPortDefinition> outputPorts;
};

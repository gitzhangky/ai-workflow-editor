#pragma once

#include <QString>
#include <QVariantMap>
#include <QVector>

struct WorkflowPortDefinition
{
    QString id;
    QString label;
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

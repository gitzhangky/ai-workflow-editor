#pragma once

#include "domain/WorkflowNodeDefinition.hpp"

#include <QCoreApplication>
#include <QString>
#include <QVector>

class BuiltInNodeRegistry
{
    Q_DECLARE_TR_FUNCTIONS(BuiltInNodeRegistry)

public:
    BuiltInNodeRegistry();

    void retranslate();
    const QVector<WorkflowNodeDefinition> &definitions() const;
    const WorkflowNodeDefinition *find(QString const &typeKey) const;

private:
    QVector<WorkflowNodeDefinition> _definitions;
};

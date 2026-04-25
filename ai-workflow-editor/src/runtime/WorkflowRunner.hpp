#pragma once

#include <QJsonObject>
#include <QString>
#include <QVector>

class WorkflowRunner
{
public:
    struct Step
    {
        int nodeId = 0;
        QString typeKey;
        QString displayName;
        QString input;
        QString output;
        QString status;
    };

    struct Result
    {
        bool success = false;
        QString finalOutput;
        QString errorMessage;
        QVector<Step> steps;
    };

    static Result run(QJsonObject const &workflow, QString const &inputText);
};

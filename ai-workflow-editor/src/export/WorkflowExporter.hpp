#pragma once

#include <QJsonObject>
#include <QString>

class WorkflowExporter
{
public:
    enum class Format
    {
        PythonLangChain,
        PythonScript,
        PythonLangGraph,
        PythonCrewAI
    };

    struct Result
    {
        bool success = false;
        QString code;
        QString errorMessage;
    };

    static Result exportWorkflow(QJsonObject const &workflow, Format format);
    static Result exportFromFile(QString const &inputPath, Format format);
    static bool exportToFile(QJsonObject const &workflow, Format format, QString const &outputPath);

    static QString formatDisplayName(Format format);
    static QString formatFileFilter(Format format);
    static QString formatFileExtension(Format format);

private:
    struct NodeInfo
    {
        int id = 0;
        QString typeKey;
        QString displayName;
        QString variableName;
        QJsonObject properties;
        QStringList upstreamVars;
    };

    struct ConnectionInfo
    {
        int outNodeId = 0;
        int outPortIndex = 0;
        int inNodeId = 0;
        int inPortIndex = 0;
    };

    static Result generatePythonLangChain(QJsonObject const &workflow);
    static Result generatePythonScript(QJsonObject const &workflow);
    static Result generatePythonLangGraph(QJsonObject const &workflow);
    static Result generatePythonCrewAI(QJsonObject const &workflow);

    static QString sanitizeVariableName(QString const &displayName);
    static QList<NodeInfo> parseNodes(QJsonObject const &workflow);
    static QList<ConnectionInfo> parseConnections(QJsonObject const &workflow);
    static QList<NodeInfo> topologicalSort(QList<NodeInfo> const &nodes,
                                           QList<ConnectionInfo> const &connections);
    static void resolveUpstreamVariables(QList<NodeInfo> &sortedNodes,
                                          QList<ConnectionInfo> const &connections);
};

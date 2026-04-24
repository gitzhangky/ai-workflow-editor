#include "export/WorkflowExporter.hpp"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMap>
#include <QQueue>
#include <QRegularExpression>
#include <QSet>
#include <QStringList>

WorkflowExporter::Result WorkflowExporter::exportWorkflow(QJsonObject const &workflow, Format format)
{
    switch (format) {
    case Format::PythonLangChain:
        return generatePythonLangChain(workflow);
    case Format::PythonScript:
        return generatePythonScript(workflow);
    }
    return {false, {}, QStringLiteral("Unknown export format")};
}

WorkflowExporter::Result WorkflowExporter::exportFromFile(QString const &inputPath, Format format)
{
    QFile file(inputPath);
    if (!file.open(QIODevice::ReadOnly))
        return {false, {}, QStringLiteral("Cannot open file: %1").arg(inputPath)};

    const auto doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject())
        return {false, {}, QStringLiteral("Invalid workflow JSON")};

    return exportWorkflow(doc.object(), format);
}

bool WorkflowExporter::exportToFile(QJsonObject const &workflow, Format format, QString const &outputPath)
{
    const auto result = exportWorkflow(workflow, format);
    if (!result.success)
        return false;

    QFile file(outputPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;

    file.write(result.code.toUtf8());
    return true;
}

QString WorkflowExporter::formatDisplayName(Format format)
{
    switch (format) {
    case Format::PythonLangChain:
        return QStringLiteral("Python (LangChain)");
    case Format::PythonScript:
        return QStringLiteral("Python Script");
    }
    return {};
}

QString WorkflowExporter::formatFileFilter(Format format)
{
    switch (format) {
    case Format::PythonLangChain:
        return QStringLiteral("Python Files (*.py)");
    case Format::PythonScript:
        return QStringLiteral("Python Files (*.py)");
    }
    return {};
}

QString WorkflowExporter::formatFileExtension(Format format)
{
    return QStringLiteral("py");
}

QString WorkflowExporter::sanitizeVariableName(QString const &displayName)
{
    QString result = displayName.toLower().trimmed();
    result.replace(QRegularExpression(QStringLiteral("[^a-z0-9_]")), QStringLiteral("_"));
    result.replace(QRegularExpression(QStringLiteral("_+")), QStringLiteral("_"));
    result.remove(QRegularExpression(QStringLiteral("^_+|_+$")));
    if (result.isEmpty())
        result = QStringLiteral("node");
    if (result.at(0).isDigit())
        result.prepend(QStringLiteral("n_"));
    return result;
}

QList<WorkflowExporter::NodeInfo> WorkflowExporter::parseNodes(QJsonObject const &workflow)
{
    QList<NodeInfo> nodes;
    const auto nodesArray = workflow[QStringLiteral("nodes")].toArray();
    QMap<QString, int> varNameCounts;

    for (auto const &nodeValue : nodesArray) {
        const auto obj = nodeValue.toObject();
        NodeInfo info;
        info.id = obj[QStringLiteral("id")].toInt();
        info.typeKey = obj[QStringLiteral("type")].toString();
        info.displayName = obj[QStringLiteral("displayName")].toString();
        info.properties = obj[QStringLiteral("properties")].toObject();

        QString baseName = sanitizeVariableName(info.displayName);
        int count = varNameCounts.value(baseName, 0);
        varNameCounts[baseName] = count + 1;
        info.variableName = count > 0
                                ? QStringLiteral("%1_%2").arg(baseName).arg(count)
                                : baseName;

        nodes.append(info);
    }
    return nodes;
}

QList<WorkflowExporter::ConnectionInfo> WorkflowExporter::parseConnections(QJsonObject const &workflow)
{
    QList<ConnectionInfo> connections;
    const auto connectionsArray = workflow[QStringLiteral("connections")].toArray();

    for (auto const &connValue : connectionsArray) {
        const auto obj = connValue.toObject();
        ConnectionInfo info;
        info.outNodeId = obj[QStringLiteral("outNodeId")].toInt();
        info.outPortIndex = obj[QStringLiteral("outPortIndex")].toInt();
        info.inNodeId = obj[QStringLiteral("inNodeId")].toInt();
        info.inPortIndex = obj[QStringLiteral("inPortIndex")].toInt();
        connections.append(info);
    }
    return connections;
}

QList<WorkflowExporter::NodeInfo> WorkflowExporter::topologicalSort(
    QList<NodeInfo> const &nodes, QList<ConnectionInfo> const &connections)
{
    QMap<int, int> inDegree;
    QMap<int, QList<int>> adjacency;
    QMap<int, NodeInfo const *> nodeById;

    for (auto const &node : nodes) {
        inDegree[node.id] = 0;
        nodeById[node.id] = &node;
    }

    for (auto const &conn : connections) {
        adjacency[conn.outNodeId].append(conn.inNodeId);
        inDegree[conn.inNodeId] = inDegree.value(conn.inNodeId, 0) + 1;
    }

    QQueue<int> queue;
    for (auto it = inDegree.constBegin(); it != inDegree.constEnd(); ++it) {
        if (it.value() == 0)
            queue.enqueue(it.key());
    }

    QList<NodeInfo> sorted;
    while (!queue.isEmpty()) {
        const int current = queue.dequeue();
        if (nodeById.contains(current))
            sorted.append(*nodeById[current]);

        for (int neighbor : adjacency.value(current)) {
            inDegree[neighbor]--;
            if (inDegree[neighbor] == 0)
                queue.enqueue(neighbor);
        }
    }

    if (sorted.size() < nodes.size()) {
        for (auto const &node : nodes) {
            bool found = false;
            for (auto const &s : sorted) {
                if (s.id == node.id) {
                    found = true;
                    break;
                }
            }
            if (!found)
                sorted.append(node);
        }
    }

    return sorted;
}

void WorkflowExporter::resolveUpstreamVariables(
    QList<NodeInfo> &sortedNodes, QList<ConnectionInfo> const &connections)
{
    QMap<int, QString> varNameById;
    for (auto const &node : sortedNodes)
        varNameById[node.id] = node.variableName;

    for (auto &node : sortedNodes) {
        for (auto const &conn : connections) {
            if (conn.inNodeId == node.id) {
                const QString upstream = varNameById.value(conn.outNodeId);
                if (!upstream.isEmpty() && !node.upstreamVars.contains(upstream))
                    node.upstreamVars.append(upstream);
            }
        }
    }
}

static QString escapeString(QString const &s)
{
    QString escaped = s;
    escaped.replace(QStringLiteral("\\"), QStringLiteral("\\\\"));
    escaped.replace(QStringLiteral("\""), QStringLiteral("\\\""));
    escaped.replace(QStringLiteral("\n"), QStringLiteral("\\n"));
    return escaped;
}

static QString multilineString(QString const &s)
{
    if (s.contains(QStringLiteral("\n")) || s.contains(QStringLiteral("\""))) {
        QString escaped = s;
        escaped.replace(QStringLiteral("\\"), QStringLiteral("\\\\"));
        escaped.replace(QStringLiteral("\"\"\""), QStringLiteral("\\\"\\\"\\\""));
        return QStringLiteral("\"\"\"%1\"\"\"").arg(escaped);
    }
    return QStringLiteral("\"%1\"").arg(escapeString(s));
}

WorkflowExporter::Result WorkflowExporter::generatePythonLangChain(QJsonObject const &workflow)
{
    auto nodes = parseNodes(workflow);
    auto connections = parseConnections(workflow);

    if (nodes.isEmpty())
        return {false, {}, QStringLiteral("Workflow has no nodes")};

    auto sorted = topologicalSort(nodes, connections);
    resolveUpstreamVariables(sorted, connections);

    QStringList imports;
    imports << QStringLiteral("# Auto-generated by AI Workflow Editor")
            << QStringLiteral("# Export format: Python (LangChain)")
            << QStringLiteral("");

    QSet<QString> importedModules;
    QStringList body;

    for (auto const &node : sorted) {
        const QString &type = node.typeKey;

        if (type == QStringLiteral("start")) {
            body << QStringLiteral("# --- Workflow Entry Point ---");
            body << QStringLiteral("%1 = {}  # workflow input data").arg(node.variableName);
            body << QStringLiteral("");
            continue;
        }

        if (type == QStringLiteral("prompt")) {
            if (!importedModules.contains(QStringLiteral("prompts"))) {
                imports << QStringLiteral("from langchain_core.prompts import ChatPromptTemplate");
                importedModules.insert(QStringLiteral("prompts"));
            }
            const QString systemPrompt = node.properties[QStringLiteral("systemPrompt")].toString();
            const QString userTemplate = node.properties[QStringLiteral("userPromptTemplate")].toString();

            body << QStringLiteral("# %1").arg(node.displayName);
            body << QStringLiteral("%1 = ChatPromptTemplate.from_messages([").arg(node.variableName);
            if (!systemPrompt.isEmpty())
                body << QStringLiteral("    (\"system\", %1),").arg(multilineString(systemPrompt));
            body << QStringLiteral("    (\"human\", %1),").arg(
                multilineString(userTemplate.isEmpty() ? QStringLiteral("{input}") : userTemplate));
            body << QStringLiteral("])");
            body << QStringLiteral("");
            continue;
        }

        if (type == QStringLiteral("llm")) {
            if (!importedModules.contains(QStringLiteral("llm"))) {
                imports << QStringLiteral("from langchain_openai import ChatOpenAI");
                importedModules.insert(QStringLiteral("llm"));
            }
            const QString model = node.properties[QStringLiteral("modelName")].toString();
            const double temperature = node.properties[QStringLiteral("temperature")].toDouble(0.2);
            const int maxTokens = node.properties[QStringLiteral("maxTokens")].toInt(1024);

            body << QStringLiteral("# %1").arg(node.displayName);
            body << QStringLiteral("%1 = ChatOpenAI(").arg(node.variableName);
            body << QStringLiteral("    model=\"%1\",").arg(escapeString(model.isEmpty() ? QStringLiteral("gpt-4") : model));
            body << QStringLiteral("    temperature=%1,").arg(temperature);
            body << QStringLiteral("    max_tokens=%1,").arg(maxTokens);
            body << QStringLiteral(")");
            body << QStringLiteral("");
            continue;
        }

        if (type == QStringLiteral("agent")) {
            if (!importedModules.contains(QStringLiteral("agents"))) {
                imports << QStringLiteral("from langchain.agents import AgentExecutor, create_openai_functions_agent");
                importedModules.insert(QStringLiteral("agents"));
            }
            if (!importedModules.contains(QStringLiteral("llm"))) {
                imports << QStringLiteral("from langchain_openai import ChatOpenAI");
                importedModules.insert(QStringLiteral("llm"));
            }
            const QString instructions = node.properties[QStringLiteral("agentInstructions")].toString();
            const QString model = node.properties[QStringLiteral("modelName")].toString();
            const int maxIter = node.properties[QStringLiteral("maxIterations")].toInt(6);

            body << QStringLiteral("# %1").arg(node.displayName);
            if (!model.isEmpty()) {
                body << QStringLiteral("%1_llm = ChatOpenAI(model=\"%2\")")
                            .arg(node.variableName, escapeString(model));
            }
            body << QStringLiteral("%1_tools = []  # TODO: add agent tools").arg(node.variableName);
            if (!instructions.isEmpty()) {
                body << QStringLiteral("%1_instructions = %2")
                            .arg(node.variableName, multilineString(instructions));
            }
            body << QStringLiteral("%1 = AgentExecutor(").arg(node.variableName);
            body << QStringLiteral("    agent=create_openai_functions_agent(%1_llm, %1_tools, prompt=%1_instructions),")
                        .arg(node.variableName);
            body << QStringLiteral("    tools=%1_tools,").arg(node.variableName);
            body << QStringLiteral("    max_iterations=%1,").arg(maxIter);
            body << QStringLiteral(")");
            body << QStringLiteral("");
            continue;
        }

        if (type == QStringLiteral("memory")) {
            if (!importedModules.contains(QStringLiteral("memory"))) {
                imports << QStringLiteral("from langchain.memory import ConversationBufferMemory");
                importedModules.insert(QStringLiteral("memory"));
            }
            const QString memoryKey = node.properties[QStringLiteral("memoryKey")].toString();

            body << QStringLiteral("# %1").arg(node.displayName);
            body << QStringLiteral("%1 = ConversationBufferMemory(").arg(node.variableName);
            if (!memoryKey.isEmpty())
                body << QStringLiteral("    memory_key=\"%1\",").arg(escapeString(memoryKey));
            body << QStringLiteral(")");
            body << QStringLiteral("");
            continue;
        }

        if (type == QStringLiteral("retriever")) {
            if (!importedModules.contains(QStringLiteral("retriever"))) {
                imports << QStringLiteral("# TODO: import your retriever implementation");
                importedModules.insert(QStringLiteral("retriever"));
            }
            const QString key = node.properties[QStringLiteral("retrieverKey")].toString();

            body << QStringLiteral("# %1").arg(node.displayName);
            body << QStringLiteral("%1 = None  # TODO: initialize retriever (key: \"%2\")")
                        .arg(node.variableName, escapeString(key));
            body << QStringLiteral("");
            continue;
        }

        if (type == QStringLiteral("templateVariables")) {
            const QString varsJson = node.properties[QStringLiteral("variablesJson")].toString();

            body << QStringLiteral("# %1").arg(node.displayName);
            body << QStringLiteral("%1 = %2")
                        .arg(node.variableName,
                             varsJson.isEmpty() ? QStringLiteral("{}") : varsJson);
            body << QStringLiteral("");
            continue;
        }

        if (type == QStringLiteral("httpRequest")) {
            if (!importedModules.contains(QStringLiteral("requests"))) {
                imports << QStringLiteral("import requests");
                importedModules.insert(QStringLiteral("requests"));
            }
            const QString method = node.properties[QStringLiteral("method")].toString();
            const QString url = node.properties[QStringLiteral("url")].toString();
            const QString headers = node.properties[QStringLiteral("headersJson")].toString();
            const QString bodyTemplate = node.properties[QStringLiteral("bodyTemplate")].toString();
            const int timeout = node.properties[QStringLiteral("timeoutMs")].toInt(30000);

            body << QStringLiteral("# %1").arg(node.displayName);
            body << QStringLiteral("def %1(**kwargs):").arg(node.variableName);
            body << QStringLiteral("    response = requests.%1(").arg(method.toLower().isEmpty() ? QStringLiteral("get") : method.toLower());
            body << QStringLiteral("        url=\"%1\",").arg(escapeString(url));
            if (headers != QStringLiteral("{}") && !headers.isEmpty())
                body << QStringLiteral("        headers=%1,").arg(headers);
            if (!bodyTemplate.isEmpty())
                body << QStringLiteral("        data=%1,").arg(multilineString(bodyTemplate));
            body << QStringLiteral("        timeout=%1,").arg(timeout / 1000.0);
            body << QStringLiteral("    )");
            body << QStringLiteral("    response.raise_for_status()");
            body << QStringLiteral("    return response.json()");
            body << QStringLiteral("");
            continue;
        }

        if (type == QStringLiteral("jsonTransform")) {
            const QString transform = node.properties[QStringLiteral("transformJson")].toString();

            body << QStringLiteral("# %1").arg(node.displayName);
            body << QStringLiteral("def %1(data):").arg(node.variableName);
            body << QStringLiteral("    transform = %1").arg(
                transform.isEmpty() ? QStringLiteral("{}") : transform);
            body << QStringLiteral("    # TODO: apply transform to data");
            body << QStringLiteral("    return data");
            body << QStringLiteral("");
            continue;
        }

        if (type == QStringLiteral("tool")) {
            if (!importedModules.contains(QStringLiteral("tools"))) {
                imports << QStringLiteral("from langchain_core.tools import tool");
                importedModules.insert(QStringLiteral("tools"));
            }
            const QString toolName = node.properties[QStringLiteral("toolName")].toString();

            body << QStringLiteral("# %1").arg(node.displayName);
            body << QStringLiteral("@tool");
            body << QStringLiteral("def %1(input_data: str) -> str:").arg(
                node.variableName.isEmpty() ? sanitizeVariableName(toolName) : node.variableName);
            body << QStringLiteral("    \"\"\"Tool: %1\"\"\"").arg(escapeString(toolName));
            body << QStringLiteral("    # TODO: implement tool logic");
            body << QStringLiteral("    return input_data");
            body << QStringLiteral("");
            continue;
        }

        if (type == QStringLiteral("condition")) {
            body << QStringLiteral("# %1").arg(node.displayName);
            body << QStringLiteral("def %1(data):").arg(node.variableName);
            body << QStringLiteral("    # TODO: implement condition logic");
            body << QStringLiteral("    return True  # True branch / False branch");
            body << QStringLiteral("");
            continue;
        }

        if (type == QStringLiteral("chatOutput")) {
            const QString role = node.properties[QStringLiteral("messageRole")].toString();
            const QString tmpl = node.properties[QStringLiteral("messageTemplate")].toString();

            body << QStringLiteral("# %1").arg(node.displayName);
            body << QStringLiteral("def %1(content):").arg(node.variableName);
            body << QStringLiteral("    return {\"role\": \"%1\", \"content\": content}")
                        .arg(escapeString(role.isEmpty() ? QStringLiteral("assistant") : role));
            body << QStringLiteral("");
            continue;
        }

        if (type == QStringLiteral("output")) {
            body << QStringLiteral("# %1 — Workflow Output").arg(node.displayName);
            if (!node.upstreamVars.isEmpty()) {
                body << QStringLiteral("result = %1").arg(node.upstreamVars.first());
            } else {
                body << QStringLiteral("result = None  # TODO: connect upstream node");
            }
            body << QStringLiteral("");
            continue;
        }

        body << QStringLiteral("# %1 (type: %2) — unsupported, skipped").arg(node.displayName, type);
        body << QStringLiteral("");
    }

    // Build chain assembly from connections
    QStringList chainParts;
    bool hasPromptLlmChain = false;
    for (int i = 0; i < sorted.size(); ++i) {
        if (sorted[i].typeKey == QStringLiteral("prompt")) {
            for (auto const &conn : connections) {
                if (conn.outNodeId == sorted[i].id) {
                    for (auto const &downstream : sorted) {
                        if (downstream.id == conn.inNodeId && downstream.typeKey == QStringLiteral("llm")) {
                            chainParts << QStringLiteral("%1_chain = %1 | %2")
                                              .arg(sorted[i].variableName, downstream.variableName);
                            hasPromptLlmChain = true;
                        }
                    }
                }
            }
        }
    }

    if (hasPromptLlmChain) {
        body << QStringLiteral("# --- Chain Assembly ---");
        for (auto const &part : chainParts)
            body << part;
        body << QStringLiteral("");
    }

    // Main execution block
    body << QStringLiteral("");
    body << QStringLiteral("if __name__ == \"__main__\":");
    if (hasPromptLlmChain) {
        body << QStringLiteral("    # Example: invoke the chain");
        body << QStringLiteral("    # response = %1_chain.invoke({\"input\": \"Hello\"})").arg(
            sorted.first().typeKey == QStringLiteral("start") && sorted.size() > 1
                ? sorted[1].variableName
                : sorted.first().variableName);
        body << QStringLiteral("    # print(response)");
    }
    body << QStringLiteral("    pass");
    body << QStringLiteral("");

    imports << QStringLiteral("");

    const QString code = imports.join(QStringLiteral("\n")) + QStringLiteral("\n")
                         + body.join(QStringLiteral("\n"));

    return {true, code, {}};
}

WorkflowExporter::Result WorkflowExporter::generatePythonScript(QJsonObject const &workflow)
{
    auto nodes = parseNodes(workflow);
    auto connections = parseConnections(workflow);

    if (nodes.isEmpty())
        return {false, {}, QStringLiteral("Workflow has no nodes")};

    auto sorted = topologicalSort(nodes, connections);
    resolveUpstreamVariables(sorted, connections);

    QStringList lines;
    lines << QStringLiteral("#!/usr/bin/env python3");
    lines << QStringLiteral("# Auto-generated by AI Workflow Editor");
    lines << QStringLiteral("# Export format: Python Script");
    lines << QStringLiteral("");
    lines << QStringLiteral("import json");
    lines << QStringLiteral("");
    lines << QStringLiteral("");

    for (auto const &node : sorted) {
        const QString &type = node.typeKey;

        if (type == QStringLiteral("start")) {
            lines << QStringLiteral("def %1():").arg(node.variableName);
            lines << QStringLiteral("    \"\"\"Workflow entry point\"\"\"");
            lines << QStringLiteral("    return {}");
            lines << QStringLiteral("");
            lines << QStringLiteral("");
            continue;
        }

        QString inputParam = node.upstreamVars.isEmpty()
                                 ? QStringLiteral("data")
                                 : QStringLiteral("data");

        if (type == QStringLiteral("prompt")) {
            const QString systemPrompt = node.properties[QStringLiteral("systemPrompt")].toString();
            const QString userTemplate = node.properties[QStringLiteral("userPromptTemplate")].toString();

            lines << QStringLiteral("def %1(%2):").arg(node.variableName, inputParam);
            lines << QStringLiteral("    system_prompt = %1").arg(multilineString(systemPrompt));
            lines << QStringLiteral("    user_template = %1").arg(
                multilineString(userTemplate.isEmpty() ? QStringLiteral("{input}") : userTemplate));
            lines << QStringLiteral("    return {\"system\": system_prompt, \"user\": user_template.format(**%1)}").arg(inputParam);
            lines << QStringLiteral("");
            lines << QStringLiteral("");
            continue;
        }

        if (type == QStringLiteral("llm")) {
            const QString model = node.properties[QStringLiteral("modelName")].toString();
            const double temperature = node.properties[QStringLiteral("temperature")].toDouble(0.2);

            lines << QStringLiteral("def %1(%2):").arg(node.variableName, inputParam);
            lines << QStringLiteral("    # TODO: implement LLM call (model: %1, temperature: %2)").arg(
                model.isEmpty() ? QStringLiteral("default-llm") : model, QString::number(temperature));
            lines << QStringLiteral("    raise NotImplementedError(\"LLM invocation not implemented\")");
            lines << QStringLiteral("");
            lines << QStringLiteral("");
            continue;
        }

        if (type == QStringLiteral("condition")) {
            lines << QStringLiteral("def %1(%2):").arg(node.variableName, inputParam);
            lines << QStringLiteral("    # TODO: implement condition");
            lines << QStringLiteral("    return True");
            lines << QStringLiteral("");
            lines << QStringLiteral("");
            continue;
        }

        if (type == QStringLiteral("httpRequest")) {
            const QString method = node.properties[QStringLiteral("method")].toString();
            const QString url = node.properties[QStringLiteral("url")].toString();

            lines << QStringLiteral("def %1(%2):").arg(node.variableName, inputParam);
            lines << QStringLiteral("    import requests");
            lines << QStringLiteral("    response = requests.%1(\"%2\")").arg(
                method.toLower().isEmpty() ? QStringLiteral("get") : method.toLower(),
                escapeString(url));
            lines << QStringLiteral("    return response.json()");
            lines << QStringLiteral("");
            lines << QStringLiteral("");
            continue;
        }

        if (type == QStringLiteral("output") || type == QStringLiteral("chatOutput")) {
            lines << QStringLiteral("def %1(%2):").arg(node.variableName, inputParam);
            lines << QStringLiteral("    return %1").arg(inputParam);
            lines << QStringLiteral("");
            lines << QStringLiteral("");
            continue;
        }

        lines << QStringLiteral("def %1(%2):").arg(node.variableName, inputParam);
        lines << QStringLiteral("    # TODO: implement %1 (%2)").arg(node.displayName, type);
        lines << QStringLiteral("    return %1").arg(inputParam);
        lines << QStringLiteral("");
        lines << QStringLiteral("");
    }

    // Generate pipeline function
    lines << QStringLiteral("def run_workflow():");
    for (int i = 0; i < sorted.size(); ++i) {
        if (i == 0 && sorted[i].typeKey == QStringLiteral("start")) {
            lines << QStringLiteral("    data = %1()").arg(sorted[i].variableName);
        } else if (sorted[i].typeKey == QStringLiteral("condition")) {
            lines << QStringLiteral("    if %1(data):").arg(sorted[i].variableName);
            lines << QStringLiteral("        pass  # True branch");
            lines << QStringLiteral("    else:");
            lines << QStringLiteral("        pass  # False branch");
        } else {
            lines << QStringLiteral("    data = %1(data)").arg(sorted[i].variableName);
        }
    }
    lines << QStringLiteral("    return data");
    lines << QStringLiteral("");
    lines << QStringLiteral("");
    lines << QStringLiteral("if __name__ == \"__main__\":");
    lines << QStringLiteral("    result = run_workflow()");
    lines << QStringLiteral("    print(json.dumps(result, indent=2, default=str))");
    lines << QStringLiteral("");

    return {true, lines.join(QStringLiteral("\n")), {}};
}

#include "registry/BuiltInNodeRegistry.hpp"

BuiltInNodeRegistry::BuiltInNodeRegistry()
{
    retranslate();
}

void BuiltInNodeRegistry::retranslate()
{
    _definitions = {
        {"start",
         "flow",
         tr("Flow"),
         tr("Start"),
         tr("Workflow entry point"),
         {},
         {},
         {{"out", tr("Out")}}},
        {"prompt",
         "ai",
         tr("AI"),
         tr("Prompt"),
         tr("Prompt template node"),
         {{"systemPrompt", QString()}, {"userPromptTemplate", QString()}},
         {{"in", tr("In")}},
         {{"out", tr("Out")}}},
        {"llm",
         "ai",
         tr("AI"),
         tr("LLM"),
         tr("Model invocation node"),
         {{"modelName", QStringLiteral("default-llm")}, {"temperature", 0.2}, {"maxTokens", 1024}},
         {{"in", tr("In")}},
         {{"success", tr("Success")}, {"error", tr("Error")}}},
        {"agent",
         "ai",
         tr("AI"),
         tr("Agent"),
         tr("Agent orchestration node"),
         {{"agentInstructions", QString()}, {"modelName", QString()}, {"maxIterations", 6}},
         {{"in", tr("In")}},
         {{"success", tr("Success")}, {"error", tr("Error")}}},
        {"memory",
         "ai",
         tr("AI"),
         tr("Memory"),
         tr("Workflow memory node"),
         {{"memoryKey", QString()}},
         {{"in", tr("In")}},
         {{"out", tr("Out")}}},
        {"retriever",
         "ai",
         tr("AI"),
         tr("Retriever"),
         tr("Workflow retriever node"),
         {{"retrieverKey", QString()}},
         {{"in", tr("In")}},
         {{"out", tr("Out")}}},
        {"templateVariables",
         "ai",
         tr("AI"),
         tr("Template Variables"),
         tr("Template variables node"),
         {{"variablesJson", QStringLiteral("{}")}},
         {{"in", tr("In")}},
         {{"out", tr("Out")}}},
        {"httpRequest",
         "integration",
         tr("Integration"),
         tr("HTTP Request"),
         tr("HTTP request node"),
         {{"method", QStringLiteral("GET")},
          {"url", QString()},
          {"headersJson", QStringLiteral("{}")},
          {"bodyTemplate", QString()},
          {"timeoutMs", 30000}},
         {{"in", tr("In")}},
         {{"success", tr("Success")}, {"error", tr("Error")}}},
        {"jsonTransform",
         "integration",
         tr("Integration"),
         tr("JSON Transform"),
         tr("JSON transform node"),
         {{"transformJson", QStringLiteral("{}")}},
         {{"in", tr("In")}},
         {{"out", tr("Out")}}},
        {"tool",
         "integration",
         tr("Integration"),
         tr("Tool"),
         tr("Tool invocation node"),
         {{"toolName", QString()}, {"timeoutMs", 10000}, {"inputMapping", QStringLiteral("{}")}},
         {{"in", tr("In")}},
         {{"out", tr("Out")}}},
        {"condition",
         "flow",
         tr("Flow"),
         tr("Condition"),
         tr("Branching node"),
         {},
         {{"in", tr("In")}},
         {{"true", tr("True")}, {"false", tr("False")}}},
        {"chatOutput",
         "flow",
         tr("Flow"),
         tr("Chat Output"),
         tr("Chat output node"),
         {{"messageRole", QStringLiteral("assistant")}, {"messageTemplate", QString()}},
         {{"in", tr("In")}},
         {}},
        {"output",
         "flow",
         tr("Flow"),
         tr("Output"),
         tr("Workflow result node"),
         {},
         {{"in", tr("In")}},
         {}}
    };
}

const QVector<WorkflowNodeDefinition> &BuiltInNodeRegistry::definitions() const
{
    return _definitions;
}

const WorkflowNodeDefinition *BuiltInNodeRegistry::find(QString const &typeKey) const
{
    for (auto const &definition : _definitions) {
        if (definition.typeKey == typeKey)
            return &definition;
    }

    return nullptr;
}

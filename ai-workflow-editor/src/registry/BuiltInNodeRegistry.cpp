#include "registry/BuiltInNodeRegistry.hpp"

#include "domain/PortDataTypes.hpp"

BuiltInNodeRegistry::BuiltInNodeRegistry()
{
    retranslate();
}

void BuiltInNodeRegistry::retranslate()
{
    const auto F = PortDataTypes::flow();
    const auto T = PortDataTypes::text();
    const auto C = PortDataTypes::completion();
    const auto E = PortDataTypes::error();
    const auto H = PortDataTypes::httpResponse();

    _definitions = {
        {"start",
         "flow",
         tr("Flow"),
         tr("Start"),
         tr("Workflow entry point"),
         {},
         {},
         {{"out", tr("Out"), F}}},
        {"prompt",
         "ai",
         tr("AI"),
         tr("Prompt"),
         tr("Prompt template node"),
         {{"systemPrompt", QString()}, {"userPromptTemplate", QString()}},
         {{"in", tr("In"), F}},
         {{"out", tr("Out"), T}}},
        {"llm",
         "ai",
         tr("AI"),
         tr("LLM"),
         tr("Model invocation node"),
         {{"modelName", QStringLiteral("default-llm")}, {"temperature", 0.2}, {"maxTokens", 1024}},
         {{"in", tr("In"), T}},
         {{"success", tr("Success"), C}, {"error", tr("Error"), E}}},
        {"agent",
         "ai",
         tr("AI"),
         tr("Agent"),
         tr("Agent orchestration node"),
         {{"agentInstructions", QString()}, {"modelName", QString()}, {"maxIterations", 6}},
         {{"in", tr("In"), T}},
         {{"success", tr("Success"), C}, {"error", tr("Error"), E}}},
        {"memory",
         "ai",
         tr("AI"),
         tr("Memory"),
         tr("Workflow memory node"),
         {{"memoryKey", QString()}},
         {{"in", tr("In"), F}},
         {{"out", tr("Out"), F}}},
        {"retriever",
         "ai",
         tr("AI"),
         tr("Retriever"),
         tr("Workflow retriever node"),
         {{"retrieverKey", QString()}},
         {{"in", tr("In"), F}},
         {{"out", tr("Out"), F}}},
        {"templateVariables",
         "ai",
         tr("AI"),
         tr("Template Variables"),
         tr("Template variables node"),
         {{"variablesJson", QStringLiteral("{}")}},
         {{"in", tr("In"), F}},
         {{"out", tr("Out"), F}}},
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
         {{"in", tr("In"), F}},
         {{"success", tr("Success"), H}, {"error", tr("Error"), E}}},
        {"jsonTransform",
         "integration",
         tr("Integration"),
         tr("JSON Transform"),
         tr("JSON transform node"),
         {{"transformJson", QStringLiteral("{}")}},
         {{"in", tr("In"), F}},
         {{"out", tr("Out"), F}}},
        {"tool",
         "integration",
         tr("Integration"),
         tr("Tool"),
         tr("Tool invocation node"),
         {{"toolName", QString()}, {"timeoutMs", 10000}, {"inputMapping", QStringLiteral("{}")}},
         {{"in", tr("In"), F}},
         {{"out", tr("Out"), F}}},
        {"condition",
         "flow",
         tr("Flow"),
         tr("Condition"),
         tr("Branching node"),
         {},
         {{"in", tr("In"), F}},
         {{"true", tr("True"), F}, {"false", tr("False"), F}}},
        {"chatOutput",
         "flow",
         tr("Flow"),
         tr("Chat Output"),
         tr("Chat output node"),
         {{"messageRole", QStringLiteral("assistant")}, {"messageTemplate", QString()}},
         {{"in", tr("In"), F}},
         {}},
        {"output",
         "flow",
         tr("Flow"),
         tr("Output"),
         tr("Workflow result node"),
         {},
         {{"in", tr("In"), F}},
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

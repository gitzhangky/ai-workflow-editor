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

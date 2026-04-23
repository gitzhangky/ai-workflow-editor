#include "registry/BuiltInNodeRegistry.hpp"

#include <QtTest/QTest>

class BuiltInNodeRegistryTests : public QObject
{
    Q_OBJECT

private slots:
    void exposesExpectedNodeTypes();
    void findsStartNodeWithExpectedPorts();
    void findsConditionNodeWithBranchPorts();
    void findsMemoryNodeWithExpectedDefaults();
    void findsRetrieverNodeWithExpectedDefaults();
    void findsTemplateVariablesNodeWithExpectedDefaults();
    void findsHttpRequestNodeWithExpectedDefaults();
};

void BuiltInNodeRegistryTests::exposesExpectedNodeTypes()
{
    BuiltInNodeRegistry registry;

    const auto definitions = registry.definitions();

    QCOMPARE(definitions.size(), 10);
    QCOMPARE(definitions.at(0).typeKey, QString("start"));
    QCOMPARE(definitions.at(1).typeKey, QString("prompt"));
    QCOMPARE(definitions.at(2).typeKey, QString("llm"));
    QCOMPARE(definitions.at(3).typeKey, QString("memory"));
    QCOMPARE(definitions.at(4).typeKey, QString("retriever"));
    QCOMPARE(definitions.at(5).typeKey, QString("templateVariables"));
    QCOMPARE(definitions.at(6).typeKey, QString("httpRequest"));
    QCOMPARE(definitions.at(7).typeKey, QString("tool"));
    QCOMPARE(definitions.at(8).typeKey, QString("condition"));
    QCOMPARE(definitions.at(9).typeKey, QString("output"));
}

void BuiltInNodeRegistryTests::findsStartNodeWithExpectedPorts()
{
    BuiltInNodeRegistry registry;

    const auto *definition = registry.find("start");

    QVERIFY(definition != nullptr);
    QCOMPARE(definition->inputPorts.size(), 0);
    QCOMPARE(definition->outputPorts.size(), 1);
    QCOMPARE(definition->outputPorts.front().id, QString("out"));
}

void BuiltInNodeRegistryTests::findsConditionNodeWithBranchPorts()
{
    BuiltInNodeRegistry registry;

    const auto *definition = registry.find("condition");

    QVERIFY(definition != nullptr);
    QCOMPARE(definition->outputPorts.size(), 2);
    QCOMPARE(definition->outputPorts.at(0).id, QString("true"));
    QCOMPARE(definition->outputPorts.at(1).id, QString("false"));
}

void BuiltInNodeRegistryTests::findsMemoryNodeWithExpectedDefaults()
{
    BuiltInNodeRegistry registry;

    const auto *definition = registry.find("memory");

    QVERIFY(definition != nullptr);
    QCOMPARE(definition->categoryKey, QString("ai"));
    QCOMPARE(definition->defaultProperties.value("memoryKey").toString(), QString());
    QCOMPARE(definition->inputPorts.size(), 1);
    QCOMPARE(definition->outputPorts.size(), 1);
    QCOMPARE(definition->inputPorts.front().id, QString("in"));
    QCOMPARE(definition->outputPorts.front().id, QString("out"));
}

void BuiltInNodeRegistryTests::findsRetrieverNodeWithExpectedDefaults()
{
    BuiltInNodeRegistry registry;

    const auto *definition = registry.find("retriever");

    QVERIFY(definition != nullptr);
    QCOMPARE(definition->categoryKey, QString("ai"));
    QCOMPARE(definition->defaultProperties.value("retrieverKey").toString(), QString());
    QCOMPARE(definition->inputPorts.size(), 1);
    QCOMPARE(definition->outputPorts.size(), 1);
    QCOMPARE(definition->inputPorts.front().id, QString("in"));
    QCOMPARE(definition->outputPorts.front().id, QString("out"));
}

void BuiltInNodeRegistryTests::findsTemplateVariablesNodeWithExpectedDefaults()
{
    BuiltInNodeRegistry registry;

    const auto *definition = registry.find("templateVariables");

    QVERIFY(definition != nullptr);
    QCOMPARE(definition->categoryKey, QString("ai"));
    QCOMPARE(definition->defaultProperties.value("variablesJson").toString(), QString("{}"));
    QCOMPARE(definition->inputPorts.size(), 1);
    QCOMPARE(definition->outputPorts.size(), 1);
    QCOMPARE(definition->inputPorts.front().id, QString("in"));
    QCOMPARE(definition->outputPorts.front().id, QString("out"));
}

void BuiltInNodeRegistryTests::findsHttpRequestNodeWithExpectedDefaults()
{
    BuiltInNodeRegistry registry;

    const auto *definition = registry.find("httpRequest");

    QVERIFY(definition != nullptr);
    QCOMPARE(definition->categoryKey, QString("integration"));
    QCOMPARE(definition->defaultProperties.value("method").toString(), QString("GET"));
    QCOMPARE(definition->defaultProperties.value("url").toString(), QString());
    QCOMPARE(definition->defaultProperties.value("headersJson").toString(), QString("{}"));
    QCOMPARE(definition->defaultProperties.value("bodyTemplate").toString(), QString());
    QCOMPARE(definition->defaultProperties.value("timeoutMs").toInt(), 30000);
    QCOMPARE(definition->inputPorts.size(), 1);
    QCOMPARE(definition->outputPorts.size(), 2);
    QCOMPARE(definition->inputPorts.front().id, QString("in"));
    QCOMPARE(definition->outputPorts.at(0).id, QString("success"));
    QCOMPARE(definition->outputPorts.at(1).id, QString("error"));
}

QTEST_APPLESS_MAIN(BuiltInNodeRegistryTests)

#include "BuiltInNodeRegistryTests.moc"

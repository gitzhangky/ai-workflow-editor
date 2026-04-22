#include "registry/BuiltInNodeRegistry.hpp"

#include <QtTest/QTest>

class BuiltInNodeRegistryTests : public QObject
{
    Q_OBJECT

private slots:
    void exposesExpectedNodeTypes();
    void findsStartNodeWithExpectedPorts();
    void findsConditionNodeWithBranchPorts();
};

void BuiltInNodeRegistryTests::exposesExpectedNodeTypes()
{
    BuiltInNodeRegistry registry;

    const auto definitions = registry.definitions();

    QCOMPARE(definitions.size(), 6);
    QCOMPARE(definitions.at(0).typeKey, QString("start"));
    QCOMPARE(definitions.at(1).typeKey, QString("prompt"));
    QCOMPARE(definitions.at(2).typeKey, QString("llm"));
    QCOMPARE(definitions.at(3).typeKey, QString("tool"));
    QCOMPARE(definitions.at(4).typeKey, QString("condition"));
    QCOMPARE(definitions.at(5).typeKey, QString("output"));
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

QTEST_APPLESS_MAIN(BuiltInNodeRegistryTests)

#include "BuiltInNodeRegistryTests.moc"

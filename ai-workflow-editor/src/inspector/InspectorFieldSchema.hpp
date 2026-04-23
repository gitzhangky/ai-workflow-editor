#pragma once

#include <QVariant>

#include <vector>

struct InspectorFieldSchema
{
    QString typeKey;
    QString propertyKey;
    QString labelObjectName;
    QString widgetObjectName;
    QString labelText;
    QVariant defaultValue;
    QString placeholderText;
    QString helpText;
};

struct InspectorSectionSchema
{
    QString typeKey;
    QString sectionObjectName;
    QString displayName;
    QString sectionTitle;
    QString summaryText;
    bool showsEmptyState = false;
    QString emptyStateText;
};

std::vector<InspectorFieldSchema> builtInInspectorFieldSchemas();
std::vector<InspectorSectionSchema> builtInInspectorSectionSchemas();

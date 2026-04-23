#pragma once

#include <QVariant>

#include <vector>

struct InspectorFieldSchema
{
    QString typeKey;
    QString propertyKey;
    QString labelText;
    QVariant defaultValue;
    QString placeholderText;
    QString helpText;
};

std::vector<InspectorFieldSchema> builtInInspectorFieldSchemas();

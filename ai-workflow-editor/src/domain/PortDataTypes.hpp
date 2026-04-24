#pragma once

#include <QSet>
#include <QString>
#include <QStringLiteral>

namespace PortDataTypes
{

inline QString flow() { return QStringLiteral("flow"); }
inline QString text() { return QStringLiteral("text"); }
inline QString completion() { return QStringLiteral("completion"); }
inline QString error() { return QStringLiteral("error"); }
inline QString httpResponse() { return QStringLiteral("http_response"); }

inline bool areCompatible(QString const &outputTypeId, QString const &inputTypeId)
{
    if (outputTypeId == inputTypeId)
        return true;

    // "flow" inputs accept any output type
    if (inputTypeId == flow())
        return true;

    return false;
}

}

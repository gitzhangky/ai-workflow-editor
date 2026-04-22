#include "app/LightTheme.hpp"

#include <QApplication>
#include <QColor>
#include <QFile>
#include <QResource>
#include <QTextStream>

void ensureThemeResourcesInitialized()
{
    static bool initialized = []() {
        Q_INIT_RESOURCE(breeze);
        Q_INIT_RESOURCE(app_theme);
        return true;
    }();

    Q_UNUSED(initialized);
}

namespace
{
QString readResourceText(QString const &resourcePath)
{
    QFile file(resourcePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};

    QTextStream stream(&file);
    return stream.readAll();
}
}

namespace LightTheme
{
QPalette buildPalette()
{
    QPalette palette;
    palette.setColor(QPalette::Window, QColor("#f4f1eb"));
    palette.setColor(QPalette::WindowText, QColor("#2d241f"));
    palette.setColor(QPalette::Base, QColor("#ffffff"));
    palette.setColor(QPalette::AlternateBase, QColor("#ebe5dc"));
    palette.setColor(QPalette::Text, QColor("#2d241f"));
    palette.setColor(QPalette::Button, QColor("#ebe5dc"));
    palette.setColor(QPalette::ButtonText, QColor("#2d241f"));
    palette.setColor(QPalette::Highlight, QColor("#4b84d9"));
    palette.setColor(QPalette::HighlightedText, QColor("#ffffff"));
    return palette;
}

QString buildStyleSheet()
{
    ensureThemeResourcesInitialized();

    const auto breezeStyleSheet = readResourceText(QStringLiteral(":/light/stylesheet.qss"));
    const auto workbenchStyleSheet = readResourceText(QStringLiteral(":/app-theme/styles/workbench.qss"));

    QString styleSheet = breezeStyleSheet;
    if (!styleSheet.isEmpty() && !workbenchStyleSheet.isEmpty())
        styleSheet.append(QStringLiteral("\n\n"));
    styleSheet.append(workbenchStyleSheet);
    return styleSheet;
}

void apply(QApplication &application)
{
    application.setStyle(QStringLiteral("Fusion"));
    application.setPalette(buildPalette());
    application.setStyleSheet(buildStyleSheet());
}
}

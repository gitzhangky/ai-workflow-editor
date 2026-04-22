#pragma once

#include <QPalette>
#include <QString>

class QApplication;

namespace LightTheme
{
QPalette buildPalette();
QString buildStyleSheet();
void apply(QApplication &application);
}

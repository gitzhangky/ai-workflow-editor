#include "app/LightTheme.hpp"

#include <QApplication>
#include <QColor>
#include <QPalette>
#include <QtTest/QTest>

class LightThemeTests : public QObject
{
    Q_OBJECT

private slots:
    void buildsExpectedPalette();
    void buildsWorkbenchStyleSheet();
    void appliesWorkbenchStyleSheetToApplication();
};

void LightThemeTests::buildsExpectedPalette()
{
    const auto palette = LightTheme::buildPalette();

    QCOMPARE(palette.color(QPalette::Window), QColor("#f4f1eb"));
    QCOMPARE(palette.color(QPalette::Base), QColor("#ffffff"));
    QCOMPARE(palette.color(QPalette::Highlight), QColor("#4b84d9"));
}

void LightThemeTests::buildsWorkbenchStyleSheet()
{
    const auto styleSheet = LightTheme::buildStyleSheet();

    QVERIFY(!styleSheet.isEmpty());
    QVERIFY(styleSheet.contains("QTabBar::close-button"));
    QVERIFY(styleSheet.contains("QToolBar#primaryToolBar"));
    QVERIFY(styleSheet.contains("QDockWidget#nodeLibraryDock"));
    QVERIFY(styleSheet.contains("QDockWidget#inspectorDock"));
    QVERIFY(styleSheet.contains("QListWidget#nodeLibraryList"));
    QVERIFY(!styleSheet.contains("QListWidget#nodeLibraryList::item"));
    QVERIFY(styleSheet.contains("QStatusBar"));
}

void LightThemeTests::appliesWorkbenchStyleSheetToApplication()
{
    auto *application = qobject_cast<QApplication *>(QCoreApplication::instance());
    QVERIFY(application != nullptr);

    application->setStyleSheet(QString());
    LightTheme::apply(*application);

    QVERIFY(!application->styleSheet().isEmpty());
    QVERIFY(application->styleSheet().contains("QToolBar#primaryToolBar"));
}

QTEST_MAIN(LightThemeTests)

#include "LightThemeTests.moc"

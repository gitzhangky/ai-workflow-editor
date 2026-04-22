#include "app/LanguageManager.hpp"
#include "app/LightTheme.hpp"
#include "app/MainWindow.hpp"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("Codex"));
    QCoreApplication::setApplicationName(QStringLiteral("AIWorkflowEditor"));

    LanguageManager languageManager;
    LightTheme::apply(app);

    MainWindow window(&languageManager);
    window.show();

    return app.exec();
}

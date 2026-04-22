#include "app/LanguageManager.hpp"

#include <QCoreApplication>
#include <QLatin1String>
#include <QSettings>
#include <QTranslator>

void ensureTranslationResourcesInitialized()
{
    static bool initialized = []() {
        Q_INIT_RESOURCE(app_translations);
        return true;
    }();

    Q_UNUSED(initialized);
}

namespace
{
constexpr auto SettingsKey = "ui/language";
constexpr auto ChineseLocale = "zh_CN";
constexpr auto EnglishLocale = "en_US";
}

LanguageManager::LanguageManager(QObject *parent)
    : QObject(parent)
    , _translator(new QTranslator(this))
    , _currentLanguage(Language::English)
{
    ensureTranslationResourcesInitialized();
    setLanguage(preferredLanguage());
}

LanguageManager::~LanguageManager() = default;

LanguageManager::Language LanguageManager::currentLanguage() const
{
    return _currentLanguage;
}

bool LanguageManager::setLanguage(Language language)
{
    auto *application = QCoreApplication::instance();
    if (application == nullptr)
        return false;

    application->removeTranslator(_translator);

    if (language == Language::Chinese) {
        if (!_translator->load(translationFilePath(language)))
            return false;

        application->installTranslator(_translator);
    }

    _currentLanguage = language;
    persistLanguage(language);
    Q_EMIT languageChanged(language);
    return true;
}

LanguageManager::Language LanguageManager::preferredLanguage() const
{
    QSettings settings;
    const auto storedLanguage = settings.value(QLatin1String(SettingsKey), QLatin1String(ChineseLocale)).toString();
    return storedLanguage == QLatin1String(EnglishLocale) ? Language::English : Language::Chinese;
}

QString LanguageManager::translationFilePath(Language language) const
{
    if (language != Language::Chinese)
        return {};

    return QStringLiteral(":/app-translations/ai_workflow_editor_zh_CN.qm");
}

void LanguageManager::persistLanguage(Language language) const
{
    QSettings settings;
    settings.setValue(QLatin1String(SettingsKey),
                      language == Language::Chinese ? QLatin1String(ChineseLocale)
                                                    : QLatin1String(EnglishLocale));
}

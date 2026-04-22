#pragma once

#include <QObject>
#include <QString>

class QTranslator;

class LanguageManager : public QObject
{
    Q_OBJECT

public:
    enum class Language
    {
        Chinese,
        English
    };
    Q_ENUM(Language)

    explicit LanguageManager(QObject *parent = nullptr);
    ~LanguageManager() override;

    Language currentLanguage() const;
    bool setLanguage(Language language);

Q_SIGNALS:
    void languageChanged(Language language);

private:
    Language preferredLanguage() const;
    QString translationFilePath(Language language) const;
    void persistLanguage(Language language) const;

    QTranslator *_translator;
    Language _currentLanguage;
};

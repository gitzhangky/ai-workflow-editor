#pragma once

#include <QWidget>

class QTextBrowser;

class HelpDocumentWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HelpDocumentWidget(QWidget *parent = nullptr);

    void retranslateUi();

private:
    QString buildHelpContent() const;
    void registerIllustrationResources(bool chinese);

    QTextBrowser *_browser;
};

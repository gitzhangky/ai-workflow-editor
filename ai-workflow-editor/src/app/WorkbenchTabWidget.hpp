#pragma once

#include <QTabWidget>

class WorkbenchTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    explicit WorkbenchTabWidget(QWidget *parent = nullptr);

    int addClosableTab(QWidget *widget, QString const &label);
    void activateOrAddTab(QWidget *widget, QString const &label);

private:
    void handleTabCloseRequested(int index);
};

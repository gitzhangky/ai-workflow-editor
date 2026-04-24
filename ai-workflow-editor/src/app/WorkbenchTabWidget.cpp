#include "app/WorkbenchTabWidget.hpp"

#include <QTabBar>

WorkbenchTabWidget::WorkbenchTabWidget(QWidget *parent)
    : QTabWidget(parent)
{
    setObjectName("workbenchTabWidget");
    setTabsClosable(true);
    setMovable(false);
    setDocumentMode(true);

    connect(this, &QTabWidget::tabCloseRequested, this, &WorkbenchTabWidget::handleTabCloseRequested);
}

int WorkbenchTabWidget::addClosableTab(QWidget *widget, QString const &label)
{
    const int index = addTab(widget, label);
    setCurrentIndex(index);
    return index;
}

void WorkbenchTabWidget::activateOrAddTab(QWidget *widget, QString const &label)
{
    const int existingIndex = indexOf(widget);
    if (existingIndex >= 0) {
        setCurrentIndex(existingIndex);
        return;
    }

    addClosableTab(widget, label);
}

void WorkbenchTabWidget::handleTabCloseRequested(int index)
{
    if (index <= 0)
        return;

    QWidget *w = widget(index);
    removeTab(index);
    w->deleteLater();
}

#pragma once

#include <QJsonObject>
#include <QWidget>

class QPushButton;
class QTableWidget;
class QTextEdit;

class RunPreviewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RunPreviewWidget(QWidget *parent = nullptr);

    void setWorkflow(QJsonObject const &workflow);
    void retranslateUi();

private:
    void runPreview();

    QJsonObject _workflow;
    QTextEdit *_inputEdit;
    QPushButton *_runButton;
    QTableWidget *_traceTable;
    QTextEdit *_outputEdit;
};

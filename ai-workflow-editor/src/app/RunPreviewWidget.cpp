#include "app/RunPreviewWidget.hpp"

#include "runtime/WorkflowRunner.hpp"

#include <QAbstractItemView>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QTextEdit>
#include <QVBoxLayout>

RunPreviewWidget::RunPreviewWidget(QWidget *parent)
    : QWidget(parent)
    , _inputEdit(new QTextEdit(this))
    , _runButton(new QPushButton(this))
    , _traceTable(new QTableWidget(this))
    , _outputEdit(new QTextEdit(this))
{
    setObjectName("runPreviewWidget");

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(18, 16, 18, 18);
    layout->setSpacing(10);

    auto *inputLabel = new QLabel(this);
    inputLabel->setObjectName("runPreviewInputLabel");
    layout->addWidget(inputLabel);

    _inputEdit->setObjectName("runPreviewInputEdit");
    _inputEdit->setMinimumHeight(82);
    layout->addWidget(_inputEdit);

    _runButton->setObjectName("runPreviewRunButton");
    layout->addWidget(_runButton);

    _traceTable->setObjectName("runPreviewTraceTable");
    _traceTable->setColumnCount(5);
    _traceTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _traceTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    _traceTable->setSelectionMode(QAbstractItemView::SingleSelection);
    _traceTable->setAlternatingRowColors(true);
    _traceTable->setShowGrid(false);
    _traceTable->verticalHeader()->hide();
    _traceTable->horizontalHeader()->setStretchLastSection(true);
    _traceTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    _traceTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    _traceTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    layout->addWidget(_traceTable, 1);

    auto *outputLabel = new QLabel(this);
    outputLabel->setObjectName("runPreviewOutputLabel");
    layout->addWidget(outputLabel);

    _outputEdit->setObjectName("runPreviewOutputEdit");
    _outputEdit->setReadOnly(true);
    _outputEdit->setMinimumHeight(96);
    layout->addWidget(_outputEdit);

    connect(_runButton, &QPushButton::clicked, this, &RunPreviewWidget::runPreview);
    retranslateUi();
}

void RunPreviewWidget::setWorkflow(QJsonObject const &workflow)
{
    _workflow = workflow;
}

void RunPreviewWidget::retranslateUi()
{
    if (auto *inputLabel = findChild<QLabel *>("runPreviewInputLabel"); inputLabel != nullptr)
        inputLabel->setText(tr("Test Input"));
    if (auto *outputLabel = findChild<QLabel *>("runPreviewOutputLabel"); outputLabel != nullptr)
        outputLabel->setText(tr("Final Output"));

    _inputEdit->setPlaceholderText(tr("Enter a sample input for this workflow."));
    if (_inputEdit->toPlainText().isEmpty())
        _inputEdit->setPlainText(tr("Hello from Run Preview"));
    _runButton->setText(tr("Run Preview"));
    _traceTable->setHorizontalHeaderLabels({tr("Node"), tr("Type"), tr("Status"), tr("Input"), tr("Output")});
}

void RunPreviewWidget::runPreview()
{
    const auto result = WorkflowRunner::run(_workflow, _inputEdit->toPlainText());
    _traceTable->setRowCount(result.steps.size());
    for (int row = 0; row < result.steps.size(); ++row) {
        const auto &step = result.steps.at(row);
        _traceTable->setItem(row, 0, new QTableWidgetItem(step.displayName));
        _traceTable->setItem(row, 1, new QTableWidgetItem(step.typeKey));
        _traceTable->setItem(row, 2, new QTableWidgetItem(step.status));
        _traceTable->setItem(row, 3, new QTableWidgetItem(step.input));
        _traceTable->setItem(row, 4, new QTableWidgetItem(step.output));
    }
    _traceTable->resizeRowsToContents();

    _outputEdit->setPlainText(result.success ? result.finalOutput : result.errorMessage);
}

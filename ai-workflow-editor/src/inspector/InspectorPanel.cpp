#include "inspector/InspectorPanel.hpp"

#include <QDoubleSpinBox>
#include <QEvent>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTextEdit>
#include <QVBoxLayout>

namespace
{
QWidget *createSectionWidget(QWidget *parent, QVBoxLayout *rootLayout)
{
    auto *section = new QWidget(parent);
    auto *sectionLayout = new QFormLayout(section);
    sectionLayout->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    sectionLayout->setFormAlignment(Qt::AlignTop | Qt::AlignLeft);
    sectionLayout->setHorizontalSpacing(10);
    sectionLayout->setVerticalSpacing(10);
    sectionLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->addWidget(section);
    return section;
}
}

InspectorPanel::InspectorPanel(QWidget *parent)
    : QWidget(parent)
    , _currentTypeKey()
    , _hintLabel(new QLabel(this))
    , _typeBadgeLabel(new QLabel(this))
    , _typeSummaryLabel(new QLabel(this))
    , _sectionTitleLabel(new QLabel(this))
    , _emptyStateLabel(new QLabel(this))
    , _displayNameLabel(new QLabel(this))
    , _descriptionLabel(new QLabel(this))
    , _displayNameEdit(new QLineEdit(this))
    , _descriptionEdit(new QTextEdit(this))
    , _promptSection(nullptr)
    , _promptSystemLabel(new QLabel(this))
    , _promptSystemEdit(new QTextEdit(this))
    , _promptUserTemplateLabel(new QLabel(this))
    , _promptUserTemplateEdit(new QTextEdit(this))
    , _llmSection(nullptr)
    , _llmModelNameLabel(new QLabel(this))
    , _llmModelNameEdit(new QLineEdit(this))
    , _llmTemperatureLabel(new QLabel(this))
    , _llmTemperatureSpin(new QDoubleSpinBox(this))
    , _llmMaxTokensLabel(new QLabel(this))
    , _llmMaxTokensSpin(new QSpinBox(this))
    , _toolSection(nullptr)
    , _toolNameLabel(new QLabel(this))
    , _toolNameEdit(new QLineEdit(this))
    , _toolTimeoutLabel(new QLabel(this))
    , _toolTimeoutSpin(new QSpinBox(this))
    , _toolInputMappingLabel(new QLabel(this))
    , _toolInputMappingEdit(new QTextEdit(this))
{
    setObjectName("inspectorPanel");

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(14);

    _hintLabel->setObjectName("inspectorHintLabel");
    layout->addWidget(_hintLabel);
    _typeBadgeLabel->setObjectName("inspectorTypeBadgeLabel");
    _typeSummaryLabel->setObjectName("inspectorTypeSummaryLabel");
    _sectionTitleLabel->setObjectName("inspectorSectionTitleLabel");
    _emptyStateLabel->setObjectName("inspectorEmptyStateLabel");
    layout->addWidget(_typeBadgeLabel);
    layout->addWidget(_typeSummaryLabel);
    layout->addWidget(_sectionTitleLabel);

    auto *baseFormLayout = new QFormLayout();
    baseFormLayout->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    baseFormLayout->setFormAlignment(Qt::AlignTop | Qt::AlignLeft);
    baseFormLayout->setHorizontalSpacing(10);
    baseFormLayout->setVerticalSpacing(10);
    _displayNameEdit->setObjectName("inspectorDisplayNameEdit");
    _descriptionEdit->setObjectName("inspectorDescriptionEdit");
    baseFormLayout->addRow(_displayNameLabel, _displayNameEdit);
    baseFormLayout->addRow(_descriptionLabel, _descriptionEdit);
    layout->addLayout(baseFormLayout);

    _promptSection = createSectionWidget(this, layout);
    _promptSection->setObjectName("inspectorPromptSection");
    _promptSystemEdit->setObjectName("inspectorPromptSystemEdit");
    _promptUserTemplateEdit->setObjectName("inspectorPromptUserTemplateEdit");
    auto *promptFormLayout = qobject_cast<QFormLayout *>(_promptSection->layout());
    promptFormLayout->addRow(_promptSystemLabel, _promptSystemEdit);
    promptFormLayout->addRow(_promptUserTemplateLabel, _promptUserTemplateEdit);

    _llmSection = createSectionWidget(this, layout);
    _llmSection->setObjectName("inspectorLlmSection");
    _llmModelNameEdit->setObjectName("inspectorLlmModelNameEdit");
    _llmTemperatureSpin->setObjectName("inspectorLlmTemperatureSpin");
    _llmTemperatureSpin->setRange(0.0, 2.0);
    _llmTemperatureSpin->setDecimals(2);
    _llmTemperatureSpin->setSingleStep(0.05);
    _llmMaxTokensSpin->setObjectName("inspectorLlmMaxTokensSpin");
    _llmMaxTokensSpin->setRange(1, 200000);
    auto *llmFormLayout = qobject_cast<QFormLayout *>(_llmSection->layout());
    llmFormLayout->addRow(_llmModelNameLabel, _llmModelNameEdit);
    llmFormLayout->addRow(_llmTemperatureLabel, _llmTemperatureSpin);
    llmFormLayout->addRow(_llmMaxTokensLabel, _llmMaxTokensSpin);

    _toolSection = createSectionWidget(this, layout);
    _toolSection->setObjectName("inspectorToolSection");
    _toolNameEdit->setObjectName("inspectorToolNameEdit");
    _toolTimeoutSpin->setObjectName("inspectorToolTimeoutSpin");
    _toolTimeoutSpin->setRange(0, 600000);
    _toolTimeoutSpin->setSingleStep(1000);
    _toolInputMappingEdit->setObjectName("inspectorToolInputMappingEdit");
    auto *toolFormLayout = qobject_cast<QFormLayout *>(_toolSection->layout());
    toolFormLayout->addRow(_toolNameLabel, _toolNameEdit);
    toolFormLayout->addRow(_toolTimeoutLabel, _toolTimeoutSpin);
    toolFormLayout->addRow(_toolInputMappingLabel, _toolInputMappingEdit);

    layout->addWidget(_emptyStateLabel);
    layout->addStretch();

    connect(_displayNameEdit, &QLineEdit::textChanged, this, &InspectorPanel::displayNameEdited);
    connect(_descriptionEdit, &QTextEdit::textChanged, this, [this]() {
        Q_EMIT descriptionEdited(_descriptionEdit->toPlainText());
    });
    connect(_promptSystemEdit, &QTextEdit::textChanged, this, [this]() {
        Q_EMIT propertyEdited(QStringLiteral("systemPrompt"), _promptSystemEdit->toPlainText());
    });
    connect(_promptUserTemplateEdit, &QTextEdit::textChanged, this, [this]() {
        Q_EMIT propertyEdited(QStringLiteral("userPromptTemplate"), _promptUserTemplateEdit->toPlainText());
    });
    connect(_llmModelNameEdit, &QLineEdit::textChanged, this, [this](QString const &value) {
        Q_EMIT propertyEdited(QStringLiteral("modelName"), value);
    });
    connect(_llmTemperatureSpin,
            qOverload<double>(&QDoubleSpinBox::valueChanged),
            this,
            [this](double value) { Q_EMIT propertyEdited(QStringLiteral("temperature"), value); });
    connect(_llmMaxTokensSpin,
            qOverload<int>(&QSpinBox::valueChanged),
            this,
            [this](int value) { Q_EMIT propertyEdited(QStringLiteral("maxTokens"), value); });
    connect(_toolNameEdit, &QLineEdit::textChanged, this, [this](QString const &value) {
        Q_EMIT propertyEdited(QStringLiteral("toolName"), value);
    });
    connect(_toolTimeoutSpin,
            qOverload<int>(&QSpinBox::valueChanged),
            this,
            [this](int value) { Q_EMIT propertyEdited(QStringLiteral("timeoutMs"), value); });
    connect(_toolInputMappingEdit, &QTextEdit::textChanged, this, [this]() {
        Q_EMIT propertyEdited(QStringLiteral("inputMapping"), _toolInputMappingEdit->toPlainText());
    });

    retranslateUi();
    clearSelection();
}

void InspectorPanel::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();

    QWidget::changeEvent(event);
}

void InspectorPanel::clearSelection()
{
    _currentTypeKey.clear();

    QSignalBlocker displayNameBlocker(_displayNameEdit);
    QSignalBlocker descriptionBlocker(_descriptionEdit);
    QSignalBlocker promptSystemBlocker(_promptSystemEdit);
    QSignalBlocker promptUserBlocker(_promptUserTemplateEdit);
    QSignalBlocker llmModelBlocker(_llmModelNameEdit);
    QSignalBlocker llmTemperatureBlocker(_llmTemperatureSpin);
    QSignalBlocker llmMaxTokensBlocker(_llmMaxTokensSpin);
    QSignalBlocker toolNameBlocker(_toolNameEdit);
    QSignalBlocker toolTimeoutBlocker(_toolTimeoutSpin);
    QSignalBlocker toolInputBlocker(_toolInputMappingEdit);

    _displayNameEdit->clear();
    _descriptionEdit->clear();
    _promptSystemEdit->clear();
    _promptUserTemplateEdit->clear();
    _llmModelNameEdit->clear();
    _llmTemperatureSpin->setValue(0.0);
    _llmMaxTokensSpin->setValue(1);
    _toolNameEdit->clear();
    _toolTimeoutSpin->setValue(0);
    _toolInputMappingEdit->clear();
    _typeBadgeLabel->clear();
    _typeSummaryLabel->clear();
    _sectionTitleLabel->clear();

    for (auto *widget : {static_cast<QWidget *>(_displayNameEdit),
                         static_cast<QWidget *>(_descriptionEdit),
                         static_cast<QWidget *>(_promptSystemEdit),
                         static_cast<QWidget *>(_promptUserTemplateEdit),
                         static_cast<QWidget *>(_llmModelNameEdit),
                         static_cast<QWidget *>(_llmTemperatureSpin),
                         static_cast<QWidget *>(_llmMaxTokensSpin),
                         static_cast<QWidget *>(_toolNameEdit),
                         static_cast<QWidget *>(_toolTimeoutSpin),
                         static_cast<QWidget *>(_toolInputMappingEdit)}) {
        widget->setEnabled(false);
    }

    setTypeSpecificSectionVisible(QString());
}

    void InspectorPanel::setSelectedNode(QString const &typeKey,
                                     QString const &displayName,
                                     QString const &description,
                                     QVariantMap const &properties)
{
    _currentTypeKey = typeKey;

    QSignalBlocker displayNameBlocker(_displayNameEdit);
    QSignalBlocker descriptionBlocker(_descriptionEdit);
    QSignalBlocker promptSystemBlocker(_promptSystemEdit);
    QSignalBlocker promptUserBlocker(_promptUserTemplateEdit);
    QSignalBlocker llmModelBlocker(_llmModelNameEdit);
    QSignalBlocker llmTemperatureBlocker(_llmTemperatureSpin);
    QSignalBlocker llmMaxTokensBlocker(_llmMaxTokensSpin);
    QSignalBlocker toolNameBlocker(_toolNameEdit);
    QSignalBlocker toolTimeoutBlocker(_toolTimeoutSpin);
    QSignalBlocker toolInputBlocker(_toolInputMappingEdit);

    _displayNameEdit->setEnabled(true);
    _descriptionEdit->setEnabled(true);
    _displayNameEdit->setText(displayName);
    _descriptionEdit->setPlainText(description);

    _promptSystemEdit->setPlainText(properties.value(QStringLiteral("systemPrompt")).toString());
    _promptUserTemplateEdit->setPlainText(properties.value(QStringLiteral("userPromptTemplate")).toString());
    _llmModelNameEdit->setText(properties.value(QStringLiteral("modelName")).toString());
    _llmTemperatureSpin->setValue(properties.value(QStringLiteral("temperature")).toDouble());
    _llmMaxTokensSpin->setValue(properties.value(QStringLiteral("maxTokens"), 1).toInt());
    _toolNameEdit->setText(properties.value(QStringLiteral("toolName")).toString());
    _toolTimeoutSpin->setValue(properties.value(QStringLiteral("timeoutMs")).toInt());
    _toolInputMappingEdit->setPlainText(properties.value(QStringLiteral("inputMapping")).toString());
    _typeBadgeLabel->setText(typeDisplayName(typeKey));
    _typeSummaryLabel->setText(typeSummary(typeKey));
    _sectionTitleLabel->setText(sectionTitle(typeKey));

    setTypeSpecificSectionVisible(typeKey);
}

void InspectorPanel::retranslateUi()
{
    _hintLabel->setText(tr("Select a node to edit its configuration"));
    _displayNameLabel->setText(tr("Name"));
    _descriptionLabel->setText(tr("Description"));
    _promptSystemLabel->setText(tr("System Prompt"));
    _promptUserTemplateLabel->setText(tr("User Prompt Template"));
    _llmModelNameLabel->setText(tr("Model Name"));
    _llmTemperatureLabel->setText(tr("Temperature"));
    _llmMaxTokensLabel->setText(tr("Max Tokens"));
    _toolNameLabel->setText(tr("Tool Name"));
    _toolTimeoutLabel->setText(tr("Timeout (ms)"));
    _toolInputMappingLabel->setText(tr("Input Mapping"));
    _emptyStateLabel->setText(tr("This node has no advanced settings."));

    if (!_currentTypeKey.isEmpty()) {
        _typeBadgeLabel->setText(typeDisplayName(_currentTypeKey));
        _typeSummaryLabel->setText(typeSummary(_currentTypeKey));
        _sectionTitleLabel->setText(sectionTitle(_currentTypeKey));
    }
}

void InspectorPanel::setTypeSpecificSectionVisible(QString const &typeKey)
{
    const bool isPrompt = typeKey == QStringLiteral("prompt");
    const bool isLlm = typeKey == QStringLiteral("llm");
    const bool isTool = typeKey == QStringLiteral("tool");

    _promptSection->setVisible(isPrompt);
    _llmSection->setVisible(isLlm);
    _toolSection->setVisible(isTool);
    _emptyStateLabel->setVisible(!typeKey.isEmpty() && !isPrompt && !isLlm && !isTool);

    for (auto *widget : {static_cast<QWidget *>(_promptSystemEdit), static_cast<QWidget *>(_promptUserTemplateEdit)}) {
        widget->setEnabled(isPrompt);
    }

    for (auto *widget : {static_cast<QWidget *>(_llmModelNameEdit),
                         static_cast<QWidget *>(_llmTemperatureSpin),
                         static_cast<QWidget *>(_llmMaxTokensSpin)}) {
        widget->setEnabled(isLlm);
    }

    for (auto *widget : {static_cast<QWidget *>(_toolNameEdit),
                         static_cast<QWidget *>(_toolTimeoutSpin),
                         static_cast<QWidget *>(_toolInputMappingEdit)}) {
        widget->setEnabled(isTool);
    }
}

QString InspectorPanel::typeDisplayName(QString const &typeKey) const
{
    if (typeKey == QStringLiteral("prompt"))
        return tr("Prompt");
    if (typeKey == QStringLiteral("llm"))
        return tr("LLM");
    if (typeKey == QStringLiteral("tool"))
        return tr("Tool");
    if (typeKey == QStringLiteral("start"))
        return tr("Start");
    if (typeKey == QStringLiteral("condition"))
        return tr("Condition");
    if (typeKey == QStringLiteral("output"))
        return tr("Output");

    return QString();
}

QString InspectorPanel::typeSummary(QString const &typeKey) const
{
    if (typeKey == QStringLiteral("prompt"))
        return tr("Edit the prompt template and runtime text for this node.");
    if (typeKey == QStringLiteral("llm"))
        return tr("Configure the model and generation parameters for this node.");
    if (typeKey == QStringLiteral("tool"))
        return tr("Configure the tool call, timeout, and input mapping.");
    if (typeKey == QStringLiteral("start"))
        return tr("This is the workflow entry point.");
    if (typeKey == QStringLiteral("condition"))
        return tr("This node controls workflow branching.");
    if (typeKey == QStringLiteral("output"))
        return tr("This node represents the workflow result.");

    return QString();
}

QString InspectorPanel::sectionTitle(QString const &typeKey) const
{
    if (typeKey == QStringLiteral("prompt"))
        return tr("Prompt Settings");
    if (typeKey == QStringLiteral("llm"))
        return tr("Model Settings");
    if (typeKey == QStringLiteral("tool"))
        return tr("Tool Settings");

    return tr("General Settings");
}

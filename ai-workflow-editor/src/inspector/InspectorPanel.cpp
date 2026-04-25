#include "inspector/InspectorPanel.hpp"

#include <QDoubleSpinBox>
#include <QEvent>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QStyle>
#include <QTextEdit>
#include <QVBoxLayout>

#include <memory>

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

void setWidgetValue(QWidget *widget, QVariant const &value)
{
    if (auto *lineEdit = qobject_cast<QLineEdit *>(widget); lineEdit != nullptr) {
        if (lineEdit->text() != value.toString())
            lineEdit->setText(value.toString());
        return;
    }
    if (auto *textEdit = qobject_cast<QTextEdit *>(widget); textEdit != nullptr) {
        if (textEdit->toPlainText() != value.toString())
            textEdit->setPlainText(value.toString());
        return;
    }
    if (auto *doubleSpinBox = qobject_cast<QDoubleSpinBox *>(widget); doubleSpinBox != nullptr) {
        doubleSpinBox->setValue(value.toDouble());
        return;
    }
    if (auto *spinBox = qobject_cast<QSpinBox *>(widget); spinBox != nullptr)
        spinBox->setValue(value.toInt());
}

void setWidgetPlaceholderText(QWidget *widget, QString const &text)
{
    if (auto *lineEdit = qobject_cast<QLineEdit *>(widget); lineEdit != nullptr) {
        lineEdit->setPlaceholderText(text);
        return;
    }

    if (auto *textEdit = qobject_cast<QTextEdit *>(widget); textEdit != nullptr)
        textEdit->setPlaceholderText(text);
}

void refreshWidgetStyle(QWidget *widget)
{
    if (widget == nullptr)
        return;

    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
    widget->update();
}

}

InspectorPanel::InspectorPanel(QWidget *parent)
    : QWidget(parent)
    , _currentTypeKey()
    , _validationState()
    , _validationPropertyKey()
    , _hintLabel(new QLabel(this))
    , _typeBadgeLabel(new QLabel(this))
    , _typeSummaryLabel(new QLabel(this))
    , _sectionTitleLabel(new QLabel(this))
    , _validationLabel(new QLabel(this))
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
    , _agentSection(nullptr)
    , _agentInstructionsLabel(new QLabel(this))
    , _agentInstructionsEdit(new QTextEdit(this))
    , _agentModelNameLabel(new QLabel(this))
    , _agentModelNameEdit(new QLineEdit(this))
    , _agentMaxIterationsLabel(new QLabel(this))
    , _agentMaxIterationsSpin(new QSpinBox(this))
    , _chatOutputSection(nullptr)
    , _chatOutputRoleLabel(new QLabel(this))
    , _chatOutputRoleEdit(new QLineEdit(this))
    , _chatOutputTemplateLabel(new QLabel(this))
    , _chatOutputTemplateEdit(new QTextEdit(this))
    , _memorySection(nullptr)
    , _memoryKeyLabel(new QLabel(this))
    , _memoryKeyEdit(new QLineEdit(this))
    , _retrieverSection(nullptr)
    , _retrieverKeyLabel(new QLabel(this))
    , _retrieverKeyEdit(new QLineEdit(this))
    , _templateVariablesSection(nullptr)
    , _templateVariablesLabel(new QLabel(this))
    , _templateVariablesEdit(new QTextEdit(this))
    , _httpRequestSection(nullptr)
    , _httpRequestMethodLabel(new QLabel(this))
    , _httpRequestMethodEdit(new QLineEdit(this))
    , _httpRequestUrlLabel(new QLabel(this))
    , _httpRequestUrlEdit(new QLineEdit(this))
    , _httpRequestHeadersLabel(new QLabel(this))
    , _httpRequestHeadersEdit(new QTextEdit(this))
    , _httpRequestBodyLabel(new QLabel(this))
    , _httpRequestBodyEdit(new QTextEdit(this))
    , _httpRequestTimeoutLabel(new QLabel(this))
    , _httpRequestTimeoutSpin(new QSpinBox(this))
    , _jsonTransformSection(nullptr)
    , _jsonTransformLabel(new QLabel(this))
    , _jsonTransformEdit(new QTextEdit(this))
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
    _validationLabel->setObjectName("inspectorValidationLabel");
    _validationLabel->setWordWrap(true);
    _emptyStateLabel->setObjectName("inspectorEmptyStateLabel");
    layout->addWidget(_typeBadgeLabel);
    layout->addWidget(_typeSummaryLabel);
    layout->addWidget(_sectionTitleLabel);
    layout->addWidget(_validationLabel);

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
    _promptSystemLabel->setObjectName("inspectorPromptSystemLabel");
    _promptSystemEdit->setObjectName("inspectorPromptSystemEdit");
    _promptUserTemplateLabel->setObjectName("inspectorPromptUserTemplateLabel");
    _promptUserTemplateEdit->setObjectName("inspectorPromptUserTemplateEdit");
    auto *promptFormLayout = qobject_cast<QFormLayout *>(_promptSection->layout());
    promptFormLayout->addRow(_promptSystemLabel, _promptSystemEdit);
    promptFormLayout->addRow(_promptUserTemplateLabel, _promptUserTemplateEdit);

    _llmSection = createSectionWidget(this, layout);
    _llmSection->setObjectName("inspectorLlmSection");
    _llmModelNameLabel->setObjectName("inspectorLlmModelNameLabel");
    _llmModelNameEdit->setObjectName("inspectorLlmModelNameEdit");
    _llmTemperatureLabel->setObjectName("inspectorLlmTemperatureLabel");
    _llmTemperatureSpin->setObjectName("inspectorLlmTemperatureSpin");
    _llmTemperatureSpin->setRange(0.0, 2.0);
    _llmTemperatureSpin->setDecimals(2);
    _llmTemperatureSpin->setSingleStep(0.05);
    _llmMaxTokensLabel->setObjectName("inspectorLlmMaxTokensLabel");
    _llmMaxTokensSpin->setObjectName("inspectorLlmMaxTokensSpin");
    _llmMaxTokensSpin->setRange(1, 200000);
    auto *llmFormLayout = qobject_cast<QFormLayout *>(_llmSection->layout());
    llmFormLayout->addRow(_llmModelNameLabel, _llmModelNameEdit);
    llmFormLayout->addRow(_llmTemperatureLabel, _llmTemperatureSpin);
    llmFormLayout->addRow(_llmMaxTokensLabel, _llmMaxTokensSpin);

    _agentSection = createSectionWidget(this, layout);
    _agentSection->setObjectName("inspectorAgentSection");
    _agentInstructionsLabel->setObjectName("inspectorAgentInstructionsLabel");
    _agentInstructionsEdit->setObjectName("inspectorAgentInstructionsEdit");
    _agentModelNameLabel->setObjectName("inspectorAgentModelNameLabel");
    _agentModelNameEdit->setObjectName("inspectorAgentModelNameEdit");
    _agentMaxIterationsLabel->setObjectName("inspectorAgentMaxIterationsLabel");
    _agentMaxIterationsSpin->setObjectName("inspectorAgentMaxIterationsSpin");
    _agentMaxIterationsSpin->setRange(1, 50);
    auto *agentFormLayout = qobject_cast<QFormLayout *>(_agentSection->layout());
    agentFormLayout->addRow(_agentInstructionsLabel, _agentInstructionsEdit);
    agentFormLayout->addRow(_agentModelNameLabel, _agentModelNameEdit);
    agentFormLayout->addRow(_agentMaxIterationsLabel, _agentMaxIterationsSpin);

    _chatOutputSection = createSectionWidget(this, layout);
    _chatOutputSection->setObjectName("inspectorChatOutputSection");
    _chatOutputRoleLabel->setObjectName("inspectorChatOutputRoleLabel");
    _chatOutputRoleEdit->setObjectName("inspectorChatOutputRoleEdit");
    _chatOutputTemplateLabel->setObjectName("inspectorChatOutputTemplateLabel");
    _chatOutputTemplateEdit->setObjectName("inspectorChatOutputTemplateEdit");
    auto *chatOutputFormLayout = qobject_cast<QFormLayout *>(_chatOutputSection->layout());
    chatOutputFormLayout->addRow(_chatOutputRoleLabel, _chatOutputRoleEdit);
    chatOutputFormLayout->addRow(_chatOutputTemplateLabel, _chatOutputTemplateEdit);

    _memorySection = createSectionWidget(this, layout);
    _memorySection->setObjectName("inspectorMemorySection");
    _memoryKeyLabel->setObjectName("inspectorMemoryKeyLabel");
    _memoryKeyEdit->setObjectName("inspectorMemoryKeyEdit");
    auto *memoryFormLayout = qobject_cast<QFormLayout *>(_memorySection->layout());
    memoryFormLayout->addRow(_memoryKeyLabel, _memoryKeyEdit);

    _retrieverSection = createSectionWidget(this, layout);
    _retrieverSection->setObjectName("inspectorRetrieverSection");
    _retrieverKeyLabel->setObjectName("inspectorRetrieverKeyLabel");
    _retrieverKeyEdit->setObjectName("inspectorRetrieverKeyEdit");
    auto *retrieverFormLayout = qobject_cast<QFormLayout *>(_retrieverSection->layout());
    retrieverFormLayout->addRow(_retrieverKeyLabel, _retrieverKeyEdit);

    _templateVariablesSection = createSectionWidget(this, layout);
    _templateVariablesSection->setObjectName("inspectorTemplateVariablesSection");
    _templateVariablesLabel->setObjectName("inspectorTemplateVariablesLabel");
    _templateVariablesEdit->setObjectName("inspectorTemplateVariablesEdit");
    auto *templateVariablesFormLayout = qobject_cast<QFormLayout *>(_templateVariablesSection->layout());
    templateVariablesFormLayout->addRow(_templateVariablesLabel, _templateVariablesEdit);

    _httpRequestSection = createSectionWidget(this, layout);
    _httpRequestSection->setObjectName("inspectorHttpRequestSection");
    _httpRequestMethodLabel->setObjectName("inspectorHttpRequestMethodLabel");
    _httpRequestMethodEdit->setObjectName("inspectorHttpRequestMethodEdit");
    _httpRequestUrlLabel->setObjectName("inspectorHttpRequestUrlLabel");
    _httpRequestUrlEdit->setObjectName("inspectorHttpRequestUrlEdit");
    _httpRequestHeadersLabel->setObjectName("inspectorHttpRequestHeadersLabel");
    _httpRequestHeadersEdit->setObjectName("inspectorHttpRequestHeadersEdit");
    _httpRequestBodyLabel->setObjectName("inspectorHttpRequestBodyLabel");
    _httpRequestBodyEdit->setObjectName("inspectorHttpRequestBodyEdit");
    _httpRequestTimeoutLabel->setObjectName("inspectorHttpRequestTimeoutLabel");
    _httpRequestTimeoutSpin->setObjectName("inspectorHttpRequestTimeoutSpin");
    _httpRequestTimeoutSpin->setRange(0, 600000);
    _httpRequestTimeoutSpin->setSingleStep(1000);
    auto *httpRequestFormLayout = qobject_cast<QFormLayout *>(_httpRequestSection->layout());
    httpRequestFormLayout->addRow(_httpRequestMethodLabel, _httpRequestMethodEdit);
    httpRequestFormLayout->addRow(_httpRequestUrlLabel, _httpRequestUrlEdit);
    httpRequestFormLayout->addRow(_httpRequestHeadersLabel, _httpRequestHeadersEdit);
    httpRequestFormLayout->addRow(_httpRequestBodyLabel, _httpRequestBodyEdit);
    httpRequestFormLayout->addRow(_httpRequestTimeoutLabel, _httpRequestTimeoutSpin);

    _jsonTransformSection = createSectionWidget(this, layout);
    _jsonTransformSection->setObjectName("inspectorJsonTransformSection");
    _jsonTransformLabel->setObjectName("inspectorJsonTransformLabel");
    _jsonTransformEdit->setObjectName("inspectorJsonTransformEdit");
    auto *jsonTransformFormLayout = qobject_cast<QFormLayout *>(_jsonTransformSection->layout());
    jsonTransformFormLayout->addRow(_jsonTransformLabel, _jsonTransformEdit);

    _toolSection = createSectionWidget(this, layout);
    _toolSection->setObjectName("inspectorToolSection");
    _toolNameLabel->setObjectName("inspectorToolNameLabel");
    _toolNameEdit->setObjectName("inspectorToolNameEdit");
    _toolTimeoutLabel->setObjectName("inspectorToolTimeoutLabel");
    _toolTimeoutSpin->setObjectName("inspectorToolTimeoutSpin");
    _toolTimeoutSpin->setRange(0, 600000);
    _toolTimeoutSpin->setSingleStep(1000);
    _toolInputMappingLabel->setObjectName("inspectorToolInputMappingLabel");
    _toolInputMappingEdit->setObjectName("inspectorToolInputMappingEdit");
    auto *toolFormLayout = qobject_cast<QFormLayout *>(_toolSection->layout());
    toolFormLayout->addRow(_toolNameLabel, _toolNameEdit);
    toolFormLayout->addRow(_toolTimeoutLabel, _toolTimeoutSpin);
    toolFormLayout->addRow(_toolInputMappingLabel, _toolInputMappingEdit);

    initializePropertyFieldBindings();

    layout->addWidget(_emptyStateLabel);
    layout->addStretch();

    connect(_displayNameEdit, &QLineEdit::textChanged, this, &InspectorPanel::displayNameEdited);
    connect(_descriptionEdit, &QTextEdit::textChanged, this, [this]() {
        Q_EMIT descriptionEdited(_descriptionEdit->toPlainText());
    });
    connectPropertyFieldSignals();

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
    std::vector<std::unique_ptr<QSignalBlocker>> propertyFieldBlockers;
    propertyFieldBlockers.reserve(_propertyFields.size());
    for (auto const &binding : _propertyFields)
        propertyFieldBlockers.push_back(std::make_unique<QSignalBlocker>(binding.widget));

    _displayNameEdit->clear();
    _descriptionEdit->clear();
    clearPropertyFieldValues();
    _typeBadgeLabel->clear();
    _typeSummaryLabel->clear();
    _sectionTitleLabel->clear();
    _validationState.clear();
    _validationPropertyKey.clear();
    _validationLabel->clear();
    _validationLabel->setVisible(false);
    _validationLabel->setProperty("severity", QString());
    _validationLabel->style()->unpolish(_validationLabel);
    _validationLabel->style()->polish(_validationLabel);
    clearPropertyFieldValidationState();

    for (auto *widget : {static_cast<QWidget *>(_displayNameEdit),
                         static_cast<QWidget *>(_descriptionEdit)}) {
        widget->setEnabled(false);
    }
    setPropertyFieldEnabledForType(QString());

    setTypeSpecificSectionVisible(QString());
}

void InspectorPanel::setValidationFeedback(QString const &state, QString const &message, QString const &propertyKey)
{
    _validationState = state;
    _validationPropertyKey = propertyKey;
    _validationLabel->setText(message);
    clearPropertyFieldValidationState();
    applyPropertyFieldValidationState(state, propertyKey, message);
    updateValidationLabel();
}

void InspectorPanel::setSelectedNode(QString const &typeKey,
                                     QString const &displayName,
                                     QString const &description,
                                     QVariantMap const &properties)
{
    _currentTypeKey = typeKey;

    QSignalBlocker displayNameBlocker(_displayNameEdit);
    QSignalBlocker descriptionBlocker(_descriptionEdit);
    std::vector<std::unique_ptr<QSignalBlocker>> propertyFieldBlockers;
    propertyFieldBlockers.reserve(_propertyFields.size());
    for (auto const &binding : _propertyFields)
        propertyFieldBlockers.push_back(std::make_unique<QSignalBlocker>(binding.widget));

    _displayNameEdit->setEnabled(true);
    _descriptionEdit->setEnabled(true);
    if (_displayNameEdit->text() != displayName)
        _displayNameEdit->setText(displayName);
    if (_descriptionEdit->toPlainText() != description)
        _descriptionEdit->setPlainText(description);

    applyPropertyFieldValues(properties);
    clearPropertyFieldValidationState();
    _typeBadgeLabel->setText(typeDisplayName(typeKey));
    _typeSummaryLabel->setText(typeSummary(typeKey));
    _sectionTitleLabel->setText(sectionTitle(typeKey));

    setTypeSpecificSectionVisible(typeKey);
}

void InspectorPanel::initializePropertyFieldBindings()
{
    _propertyFields.clear();
    for (auto const &schema : builtInInspectorFieldSchemas()) {
        auto *label = findChild<QLabel *>(schema.labelObjectName);
        auto *widget = findChild<QWidget *>(schema.widgetObjectName);
        Q_ASSERT(label != nullptr);
        Q_ASSERT(widget != nullptr);
        _propertyFields.push_back({schema, label, widget});
    }

    for (auto const &binding : _propertyFields) {
        if (binding.label != nullptr)
            binding.label->setProperty("inspectorPropertyKey", binding.schema.propertyKey);
        binding.widget->setProperty("inspectorPropertyKey", binding.schema.propertyKey);
        if (binding.label != nullptr)
            binding.label->setProperty("inspectorTypeKey", binding.schema.typeKey);
        binding.widget->setProperty("inspectorTypeKey", binding.schema.typeKey);
        binding.widget->setProperty("inspectorPlaceholderTextSource", binding.schema.placeholderText);
        binding.widget->setProperty("inspectorHelpTextSource", binding.schema.helpText);
        if (binding.label != nullptr) {
            binding.label->setProperty("inspectorPlaceholderTextSource", binding.schema.placeholderText);
            binding.label->setProperty("inspectorHelpTextSource", binding.schema.helpText);
        }
    }
}

void InspectorPanel::connectPropertyFieldSignals()
{
    for (auto const &binding : _propertyFields) {
        if (auto *lineEdit = qobject_cast<QLineEdit *>(binding.widget); lineEdit != nullptr) {
            connect(lineEdit, &QLineEdit::textChanged, this, [this, key = binding.schema.propertyKey](QString const &value) {
                Q_EMIT propertyEdited(key, value);
            });
            continue;
        }

        if (auto *textEdit = qobject_cast<QTextEdit *>(binding.widget); textEdit != nullptr) {
            connect(textEdit, &QTextEdit::textChanged, this, [this, textEdit, key = binding.schema.propertyKey]() {
                Q_EMIT propertyEdited(key, textEdit->toPlainText());
            });
            continue;
        }

        if (auto *doubleSpinBox = qobject_cast<QDoubleSpinBox *>(binding.widget); doubleSpinBox != nullptr) {
            connect(doubleSpinBox,
                    qOverload<double>(&QDoubleSpinBox::valueChanged),
                    this,
                    [this, key = binding.schema.propertyKey](double value) { Q_EMIT propertyEdited(key, value); });
            continue;
        }

        if (auto *spinBox = qobject_cast<QSpinBox *>(binding.widget); spinBox != nullptr) {
            connect(spinBox,
                    qOverload<int>(&QSpinBox::valueChanged),
                    this,
                    [this, key = binding.schema.propertyKey](int value) { Q_EMIT propertyEdited(key, value); });
        }
    }
}

void InspectorPanel::clearPropertyFieldValues()
{
    for (auto const &binding : _propertyFields)
        setWidgetValue(binding.widget, binding.schema.defaultValue);
}

void InspectorPanel::applyPropertyFieldValues(QVariantMap const &properties)
{
    for (auto const &binding : _propertyFields) {
        const QVariant value = properties.contains(binding.schema.propertyKey) ? properties.value(binding.schema.propertyKey)
                                                                               : binding.schema.defaultValue;
        setWidgetValue(binding.widget, value);
    }
}

void InspectorPanel::applyPropertyFieldHints()
{
    for (auto const &binding : _propertyFields) {
        const QString placeholderText =
            binding.schema.placeholderText.isEmpty() ? QString() : tr(binding.schema.placeholderText.toUtf8().constData());
        const QString helpText =
            binding.schema.helpText.isEmpty() ? QString() : tr(binding.schema.helpText.toUtf8().constData());

        setWidgetPlaceholderText(binding.widget, placeholderText);
        binding.widget->setToolTip(helpText);
        if (binding.label != nullptr)
            binding.label->setToolTip(helpText);
    }
}

void InspectorPanel::setPropertyFieldEnabledForType(QString const &typeKey)
{
    for (auto const &binding : _propertyFields)
        binding.widget->setEnabled(binding.schema.typeKey == typeKey);
}

void InspectorPanel::clearPropertyFieldValidationState()
{
    for (auto const &binding : _propertyFields) {
        if (binding.label != nullptr) {
            binding.label->setProperty("validationState", QString());
            binding.label->setProperty("validationMessage", QString());
            refreshWidgetStyle(binding.label);
        }

        binding.widget->setProperty("validationState", QString());
        binding.widget->setProperty("validationMessage", QString());
        refreshWidgetStyle(binding.widget);
    }
}

void InspectorPanel::applyPropertyFieldValidationState(QString const &state,
                                                       QString const &propertyKey,
                                                       QString const &message)
{
    if ((state != QStringLiteral("warning") && state != QStringLiteral("error")) || propertyKey.trimmed().isEmpty())
        return;

    for (auto const &binding : _propertyFields) {
        if (binding.schema.propertyKey != propertyKey || binding.schema.typeKey != _currentTypeKey)
            continue;

        if (binding.label != nullptr) {
            binding.label->setProperty("validationState", state);
            binding.label->setProperty("validationMessage", message);
            refreshWidgetStyle(binding.label);
        }

        binding.widget->setProperty("validationState", state);
        binding.widget->setProperty("validationMessage", message);
        refreshWidgetStyle(binding.widget);
        return;
    }
}

void InspectorPanel::retranslateUi()
{
    _hintLabel->setText(tr("Select a node to edit its configuration"));
    _displayNameLabel->setText(tr("Name"));
    _descriptionLabel->setText(tr("Description"));
    for (auto const &binding : _propertyFields) {
        if (binding.label != nullptr)
            binding.label->setText(tr(binding.schema.labelText.toUtf8().constData()));
    }
    applyPropertyFieldHints();
    _emptyStateLabel->setText(tr("This node has no advanced settings."));

    if (!_currentTypeKey.isEmpty()) {
        _typeBadgeLabel->setText(typeDisplayName(_currentTypeKey));
        _typeSummaryLabel->setText(typeSummary(_currentTypeKey));
        _sectionTitleLabel->setText(sectionTitle(_currentTypeKey));
    }

    updateValidationLabel();
    clearPropertyFieldValidationState();
    applyPropertyFieldValidationState(_validationState, _validationPropertyKey, _validationLabel->text());
}

void InspectorPanel::updateValidationLabel()
{
    const bool showValidation =
        (_validationState == QStringLiteral("warning") || _validationState == QStringLiteral("error"))
        && !_validationLabel->text().trimmed().isEmpty();

    _validationLabel->setProperty("severity", showValidation ? _validationState : QString());
    _validationLabel->style()->unpolish(_validationLabel);
    _validationLabel->style()->polish(_validationLabel);
    _validationLabel->setVisible(showValidation);
}

void InspectorPanel::setTypeSpecificSectionVisible(QString const &typeKey)
{
    bool showsEmptyState = false;
    QString emptyStateText;
    for (auto const &sectionSchema : builtInInspectorSectionSchemas()) {
        auto *section = findChild<QWidget *>(sectionSchema.sectionObjectName);
        if (section == nullptr && !sectionSchema.sectionObjectName.isEmpty())
            continue;

        const bool isVisible = sectionSchema.typeKey == typeKey;
        if (section != nullptr)
            section->setVisible(isVisible);

        if (isVisible) {
            showsEmptyState = sectionSchema.showsEmptyState;
            emptyStateText = sectionSchema.emptyStateText;
        }
    }

    _emptyStateLabel->setText(emptyStateText.isEmpty() ? tr("This node has no advanced settings.")
                                                       : tr(emptyStateText.toUtf8().constData()));
    _emptyStateLabel->setVisible(!typeKey.isEmpty() && showsEmptyState);
    setPropertyFieldEnabledForType(typeKey);
}

QString InspectorPanel::typeDisplayName(QString const &typeKey) const
{
    for (auto const &schema : builtInInspectorSectionSchemas()) {
        if (schema.typeKey == typeKey)
            return tr(schema.displayName.toUtf8().constData());
    }

    return QString();
}

QString InspectorPanel::typeSummary(QString const &typeKey) const
{
    for (auto const &schema : builtInInspectorSectionSchemas()) {
        if (schema.typeKey == typeKey)
            return tr(schema.summaryText.toUtf8().constData());
    }

    return QString();
}

QString InspectorPanel::sectionTitle(QString const &typeKey) const
{
    for (auto const &schema : builtInInspectorSectionSchemas()) {
        if (schema.typeKey == typeKey)
            return tr(schema.sectionTitle.toUtf8().constData());
    }

    return tr("General Settings");
}

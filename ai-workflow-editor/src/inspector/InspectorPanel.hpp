#pragma once

#include "inspector/InspectorFieldSchema.hpp"

#include <QVariant>
#include <QWidget>

#include <vector>

class QDoubleSpinBox;
class QEvent;
class QLabel;
class QLineEdit;
class QSpinBox;
class QTextEdit;
class QWidget;

class InspectorPanel : public QWidget
{
    Q_OBJECT

public:
    explicit InspectorPanel(QWidget *parent = nullptr);

    void clearSelection();
    void setValidationFeedback(QString const &state, QString const &message, QString const &propertyKey = QString());
    void setSelectedNode(QString const &typeKey,
                         QString const &displayName,
                         QString const &description,
                         QVariantMap const &properties);

protected:
    void changeEvent(QEvent *event) override;

Q_SIGNALS:
    void displayNameEdited(QString const &displayName);
    void descriptionEdited(QString const &description);
    void propertyEdited(QString const &propertyKey, QVariant const &value);

private:
    struct PropertyFieldBinding
    {
        InspectorFieldSchema schema;
        QLabel *label;
        QWidget *widget;
    };

    void initializePropertyFieldBindings();
    void connectPropertyFieldSignals();
    void clearPropertyFieldValues();
    void applyPropertyFieldValues(QVariantMap const &properties);
    void applyPropertyFieldHints();
    void setPropertyFieldEnabledForType(QString const &typeKey);
    void clearPropertyFieldValidationState();
    void applyPropertyFieldValidationState(QString const &state, QString const &propertyKey, QString const &message);
    void retranslateUi();
    void updateValidationLabel();
    void setTypeSpecificSectionVisible(QString const &typeKey);
    QString typeDisplayName(QString const &typeKey) const;
    QString typeSummary(QString const &typeKey) const;
    QString sectionTitle(QString const &typeKey) const;

    QString _currentTypeKey;
    QString _validationState;
    QString _validationPropertyKey;
    QLabel *_hintLabel;
    QLabel *_typeBadgeLabel;
    QLabel *_typeSummaryLabel;
    QLabel *_sectionTitleLabel;
    QLabel *_validationLabel;
    QLabel *_emptyStateLabel;
    QLabel *_displayNameLabel;
    QLabel *_descriptionLabel;
    QLineEdit *_displayNameEdit;
    QTextEdit *_descriptionEdit;

    QWidget *_promptSection;
    QLabel *_promptSystemLabel;
    QTextEdit *_promptSystemEdit;
    QLabel *_promptUserTemplateLabel;
    QTextEdit *_promptUserTemplateEdit;

    QWidget *_llmSection;
    QLabel *_llmModelNameLabel;
    QLineEdit *_llmModelNameEdit;
    QLabel *_llmTemperatureLabel;
    QDoubleSpinBox *_llmTemperatureSpin;
    QLabel *_llmMaxTokensLabel;
    QSpinBox *_llmMaxTokensSpin;

    QWidget *_memorySection;
    QLabel *_memoryKeyLabel;
    QLineEdit *_memoryKeyEdit;

    QWidget *_retrieverSection;
    QLabel *_retrieverKeyLabel;
    QLineEdit *_retrieverKeyEdit;

    QWidget *_templateVariablesSection;
    QLabel *_templateVariablesLabel;
    QTextEdit *_templateVariablesEdit;

    QWidget *_httpRequestSection;
    QLabel *_httpRequestMethodLabel;
    QLineEdit *_httpRequestMethodEdit;
    QLabel *_httpRequestUrlLabel;
    QLineEdit *_httpRequestUrlEdit;
    QLabel *_httpRequestHeadersLabel;
    QTextEdit *_httpRequestHeadersEdit;
    QLabel *_httpRequestBodyLabel;
    QTextEdit *_httpRequestBodyEdit;
    QLabel *_httpRequestTimeoutLabel;
    QSpinBox *_httpRequestTimeoutSpin;

    QWidget *_toolSection;
    QLabel *_toolNameLabel;
    QLineEdit *_toolNameEdit;
    QLabel *_toolTimeoutLabel;
    QSpinBox *_toolTimeoutSpin;
    QLabel *_toolInputMappingLabel;
    QTextEdit *_toolInputMappingEdit;
    std::vector<PropertyFieldBinding> _propertyFields;
};

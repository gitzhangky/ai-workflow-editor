#pragma once

#include "domain/WorkflowNodeDefinition.hpp"

#include <QtNodes/NodeDelegateModel>

class StaticNodeDelegateModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    explicit StaticNodeDelegateModel(WorkflowNodeDefinition definition);

    QString name() const override;
    QString caption() const override;
    bool captionVisible() const override;
    unsigned int nPorts(QtNodes::PortType portType) const override;
    QString portCaption(QtNodes::PortType portType, QtNodes::PortIndex portIndex) const override;
    bool portCaptionVisible(QtNodes::PortType portType, QtNodes::PortIndex portIndex) const override;
    QtNodes::NodeDataType dataType(QtNodes::PortType portType, QtNodes::PortIndex portIndex) const override;
    std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex portIndex) override;
    void setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex portIndex) override;
    QWidget *embeddedWidget() override;

    void setDisplayName(QString const &displayName);

private:
    WorkflowNodeDefinition _definition;
    std::shared_ptr<QtNodes::NodeData> _data;
    QString _displayName;
};

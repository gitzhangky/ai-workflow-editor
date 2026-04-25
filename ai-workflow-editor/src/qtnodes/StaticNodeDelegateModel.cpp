#include "qtnodes/StaticNodeDelegateModel.hpp"

#include "domain/PortDataTypes.hpp"
#include "qtnodes/StaticNodeData.hpp"

StaticNodeDelegateModel::StaticNodeDelegateModel(WorkflowNodeDefinition definition)
    : _definition(std::move(definition))
    , _data(std::make_shared<StaticNodeData>())
    , _displayName(_definition.displayName)
{
}

QString StaticNodeDelegateModel::name() const
{
    return _definition.typeKey;
}

QString StaticNodeDelegateModel::caption() const
{
    return _displayName;
}

bool StaticNodeDelegateModel::captionVisible() const
{
    return true;
}

unsigned int StaticNodeDelegateModel::nPorts(QtNodes::PortType portType) const
{
    if (portType == QtNodes::PortType::In)
        return static_cast<unsigned int>(_definition.inputPorts.size());

    return static_cast<unsigned int>(_definition.outputPorts.size());
}

QString StaticNodeDelegateModel::portCaption(QtNodes::PortType portType, QtNodes::PortIndex portIndex) const
{
    auto const &ports = (portType == QtNodes::PortType::In) ? _definition.inputPorts
                                                            : _definition.outputPorts;
    const int idx = static_cast<int>(portIndex);
    if (idx >= 0 && idx < ports.size())
        return ports[idx].label;
    return {};
}

bool StaticNodeDelegateModel::portCaptionVisible(QtNodes::PortType, QtNodes::PortIndex) const
{
    return true;
}

QtNodes::NodeDataType StaticNodeDelegateModel::dataType(QtNodes::PortType portType, QtNodes::PortIndex portIndex) const
{
    auto const &ports = (portType == QtNodes::PortType::In) ? _definition.inputPorts
                                                            : _definition.outputPorts;
    const int index = static_cast<int>(portIndex);
    if (index >= 0 && index < ports.size() && !ports[index].dataTypeId.isEmpty())
        return {ports[index].dataTypeId, ports[index].label};

    return {PortDataTypes::flow(), QStringLiteral("Flow")};
}

std::shared_ptr<QtNodes::NodeData> StaticNodeDelegateModel::outData(QtNodes::PortIndex)
{
    return _data;
}

void StaticNodeDelegateModel::setInData(std::shared_ptr<QtNodes::NodeData>, QtNodes::PortIndex) {}

QWidget *StaticNodeDelegateModel::embeddedWidget()
{
    return nullptr;
}

void StaticNodeDelegateModel::setDisplayName(QString const &displayName)
{
    _displayName = displayName;
    Q_EMIT requestNodeUpdate();
}

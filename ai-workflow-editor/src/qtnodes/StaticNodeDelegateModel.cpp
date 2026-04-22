#include "qtnodes/StaticNodeDelegateModel.hpp"

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

QtNodes::NodeDataType StaticNodeDelegateModel::dataType(QtNodes::PortType, QtNodes::PortIndex) const
{
    return _data->type();
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

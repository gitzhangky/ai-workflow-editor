#include "qtnodes/WorkflowGraphModel.hpp"

#include "domain/PortDataTypes.hpp"

#include <QtNodes/internal/ConnectionIdUtils.hpp>

#include <stack>

bool WorkflowGraphModel::connectionPossible(QtNodes::ConnectionId const connectionId) const
{
    using namespace QtNodes;

    if (!nodeExists(connectionId.outNodeId) || !nodeExists(connectionId.inNodeId))
        return false;

    auto checkPortBounds = [&](PortType const portType) {
        NodeId const nodeId = getNodeId(portType, connectionId);
        auto portCountRole = (portType == PortType::Out) ? NodeRole::OutPortCount
                                                         : NodeRole::InPortCount;
        std::size_t const portCount = nodeData(nodeId, portCountRole).toUInt();
        return getPortIndex(portType, connectionId) < portCount;
    };

    if (!checkPortBounds(PortType::Out) || !checkPortBounds(PortType::In))
        return false;

    auto getDataType = [&](PortType const portType) {
        return portData(getNodeId(portType, connectionId),
                        portType,
                        getPortIndex(portType, connectionId),
                        PortRole::DataType)
            .value<NodeDataType>();
    };

    auto portVacant = [&](PortType const portType) {
        NodeId const nodeId = getNodeId(portType, connectionId);
        PortIndex const portIndex = getPortIndex(portType, connectionId);
        auto const connected = connections(nodeId, portType, portIndex);
        auto policy = portData(nodeId, portType, portIndex, PortRole::ConnectionPolicyRole)
                          .value<ConnectionPolicy>();
        return connected.empty() || (policy == ConnectionPolicy::Many);
    };

    const auto outType = getDataType(PortType::Out);
    const auto inType = getDataType(PortType::In);

    if (!PortDataTypes::areCompatible(outType.id, inType.id))
        return false;

    if (!portVacant(PortType::Out) || !portVacant(PortType::In))
        return false;

    auto hasLoops = [this, &connectionId]() -> bool {
        std::stack<NodeId> filo;
        filo.push(connectionId.inNodeId);

        while (!filo.empty()) {
            auto id = filo.top();
            filo.pop();

            if (id == connectionId.outNodeId)
                return true;

            std::size_t const nOutPorts = nodeData(id, NodeRole::OutPortCount).toUInt();
            for (PortIndex index = 0; index < nOutPorts; ++index) {
                auto const &outConnectionIds = connections(id, PortType::Out, index);
                for (auto cid : outConnectionIds)
                    filo.push(cid.inNodeId);
            }
        }
        return false;
    };

    return loopsEnabled() || !hasLoops();
}

#pragma once

#include <QtNodes/DataFlowGraphModel>

namespace QtNodes { class NodeDelegateModelRegistry; }

class WorkflowGraphModel : public QtNodes::DataFlowGraphModel
{
    Q_OBJECT

public:
    using DataFlowGraphModel::DataFlowGraphModel;

    bool connectionPossible(QtNodes::ConnectionId const connectionId) const override;
};

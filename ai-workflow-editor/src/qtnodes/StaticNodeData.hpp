#pragma once

#include <QtNodes/NodeData>

class StaticNodeData : public QtNodes::NodeData
{
public:
    QtNodes::NodeDataType type() const override
    {
        return {"workflow", "Workflow"};
    }
};

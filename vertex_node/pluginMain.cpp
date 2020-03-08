#include <maya\MFnPlugin.h>
#include "vertexNode.h"

MStatus initializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin fn_plugin(obj, "", "2018", "any");

    status = fn_plugin.registerNode(VertexNode::name, 
        VertexNode::id, 
        &VertexNode::creator, 
        &VertexNode::initialize);
    if (!status)
    {
        status.perror("registerNode");
        return status;
    }
    return status;
}

MStatus uninitializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin fn_plugin(obj);

    fn_plugin.deregisterNode(VertexNode::id);
    if (!status)
    {
        status.perror("deregisterNode");
        return status;
    }

    return status;


}
#include <maya\MFnPlugin.h>
#include "typedAttrNode.h"

MStatus initializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin fn_plugin(obj, "", "2018", "any");

    status = fn_plugin.registerNode(TypedNode::name, TypedNode::id, &TypedNode::creator, &TypedNode::initialize);
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

    fn_plugin.deregisterNode(TypedNode::id);
    if (!status)
    {
        status.perror("deregisterNode");
        return status;
    }

    return status;


}
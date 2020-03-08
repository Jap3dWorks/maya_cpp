#include <maya\MFnPlugin.h>
#include "dynNode.h"


MStatus initializePlugin(MObject object)
{
    MStatus status;
    MFnPlugin fn_plugin(object, "", "2018", "Any");
    status = fn_plugin.registerNode(dynNode::name, dynNode::id, &dynNode::creator, &dynNode::initialize, MPxNode::kDependNode);
    
    if (!status)
    {
        status.perror("registerNode");
        return status;
    }

    return status;

}

MStatus uninitializePlugin(MObject object)
{
    MStatus status;
    MFnPlugin fn_plugin(object);
    status = fn_plugin.deregisterNode(dynNode::id);
    if (!status)
    {
        status.perror("deregisterNode");
        return status;
    }
    return status;

}
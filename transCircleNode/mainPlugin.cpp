#include <maya\MFnPlugin.h>
#include "setUpTransCircle.h"
#include "transCircleNode.h"

MStatus initializePlugin(MObject obj)
{
    MStatus state;
    MFnPlugin fn_plugin(obj, "", "2018", "any");

    state = fn_plugin.registerCommand(SetUpTransCircle::name, &SetUpTransCircle::creator);

    state = fn_plugin.registerNode(TransCircle::name, TransCircle::id,
        &TransCircle::creator, &TransCircle::initialize, MPxNode::kDependNode, nullptr);

    if (!state)
    {
        state.perror("registerNode");
        return state;
    }

    return state;
}

MStatus uninitializePlugin(MObject obj)
{
    MStatus state;
    MFnPlugin fn_plugin(obj);

    state = fn_plugin.deregisterCommand(SetUpTransCircle::name);

    state = fn_plugin.deregisterNode(TransCircle::id);

    if (!state)
    {
        state.perror("deregisterNode");
        return state;
    }

    return state;

}
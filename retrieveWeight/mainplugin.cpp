#include "retrieveWeightCmd.h"
#include <maya\MFnPlugin.h>


MStatus initializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin fn_plugin(obj, "", "2018", "Any");

    status = fn_plugin.registerCommand(retrieveWeightCmd::name, retrieveWeightCmd::creator);
    if (!status)
    {
        status.perror("registerCommand");
        return status;
    }

    return status;
}

MStatus uninitializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin fn_plugin(obj);

    status = fn_plugin.deregisterCommand(retrieveWeightCmd::name);
    if (!status)
    {
        status.perror("deregisterCommand");
        return status;
    }

    return status;
}
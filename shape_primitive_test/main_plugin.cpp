#include "cube_prim.h"
#include <maya/MFnPlugin.h>


MStatus initializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin fn_plugin(obj);

    // register CubePrim
    status = fn_plugin.registerNode(
        CubePrim::name,
        CubePrim::id,
        &CubePrim::creator,
        &CubePrim::initialize);

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

    fn_plugin.deregisterNode(CubePrim::id);
    if (!status)
    {
        status.perror("deregisterNode");
        return status;
    }

    return status;


}
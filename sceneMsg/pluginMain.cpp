#include <maya\MFnPlugin.h>
#include "sceneMsgCmd.h"

MStatus initializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin plugin(obj, "", "2018", "Any");

    status = plugin.registerCommand(SceneMsgCmd::name, &SceneMsgCmd::creator);

    return status;
}

MStatus uninitializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin plugin;

    status = plugin.deregisterCommand(SceneMsgCmd::name);
    MCallbackIdArray tempIds = SceneMsgCmd::IDs;
    if (tempIds.length() != 0)
    {
        status = MSceneMessage::removeCallbacks(tempIds);
    }

    return status;
}
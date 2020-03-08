#include "sceneMsgCmd.h"

MString SceneMsgCmd::name(MString("sceneMsg"));
MCallbackIdArray SceneMsgCmd::IDs;

SceneMsgCmd::SceneMsgCmd()
{
    // clean callback array
    IDs.clear();
}

MStatus SceneMsgCmd::doIt(const MArgList& args)
{
    return redoIt();
}

MStatus SceneMsgCmd::redoIt()
{
    MStatus stat = MS::kSuccess;

    MCallbackId openCallBackId = MSceneMessage::addCallback(MSceneMessage::kBeforeOpen, (MMessage::MBasicFunction)openCallBack, NULL, &stat);
    IDs.append(openCallBackId);

    MCallbackId newCallBackId = MSceneMessage::addCallback(MSceneMessage::kAfterNew, (MMessage::MBasicFunction)newCallback, NULL, &stat);
    IDs.append(newCallBackId);

    MCallbackId saveCheckCallbackId = MSceneMessage::addCheckCallback(MSceneMessage::kBeforeSaveCheck, (MMessage::MCheckFunction)saveCheckCallback, NULL, &stat);
    IDs.append(saveCheckCallbackId);

    return stat;
}

MStatus SceneMsgCmd::undoIt()
{
    MStatus stat = MS::kSuccess;
    if (IDs.length() != 0)
    {
        stat = MSceneMessage::removeCallbacks(IDs);
    }
    return stat;
}

// -message callbacks
void openCallBack(void* clienData)
{
    std::cout << "The callback registered for MSceneMessage::kBeforeOpen is executed.\n";
}

void newCallback(void* clienData)
{
    std::cout << "The callback registered for MSceneMessage::kAfterNew is executed.\n";
}

void saveCheckCallback(bool *retCode, void* clientData)
{
    std::cout << "The callback registered for MSceneMessage::kBeforesaveCheck is executed.\n";

    // abort operation setting recode to false.
    *retCode = false;
    std::cout << "Abort current operations.\n";
}
#ifndef SCENEMSGCMD_H
#define SCENEMSGCMD_H

#include <maya\MPxCommand.h>
#include <maya\MCallbackIdArray.h>
#include <maya\MSceneMessage.h>

class SceneMsgCmd : public MPxCommand
{
public:
    SceneMsgCmd();
    virtual ~SceneMsgCmd() {}

    virtual MStatus doIt(const MArgList&) override;
    virtual MStatus undoIt() override;
    virtual MStatus redoIt() override;

    virtual bool isUndoable() const override
    {
        return true;
    }

    static void* creator()
    {
        return new SceneMsgCmd();
    }

    static MString name;

public:
    static MCallbackIdArray IDs;

};

// message callbacks
void openCallBack(void* clientData);
void newCallback(void* clientData);
void saveCheckCallback(bool* retCode, void* clientData);

#endif // ! SCENEMSGCMD_H

#ifndef SETUPTRANSCIRCLE_H
#define SETUPTRANSCIRCLE_H

#include <maya\MPxCommand.h>
#include <maya\MDGModifier.h>

class MArgList;

class SetUpTransCircle : public MPxCommand
{
public:
    SetUpTransCircle() {}
    virtual ~SetUpTransCircle() {}

    virtual MStatus doIt(const MArgList&) override;
    virtual MStatus undoIt() override;
    virtual MStatus redoIt() override;

    virtual bool isUndoable() const override
    {
        return true;
    }

    static void* creator()
    {
        return new SetUpTransCircle();
    }

    static MString name;

private:
    MDGModifier dg_mod;
};


#endif // !SETUPTRANSCIRCLE_H

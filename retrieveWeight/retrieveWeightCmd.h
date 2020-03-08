#ifndef RETRIEVEWEIGHTCMD_H
#define RETRIEVEWEIGHTCMD_H

#include <maya\MPxCommand.h>

class retrieveWeightCmd : public MPxCommand
{
public:
    retrieveWeightCmd() {}
    virtual ~retrieveWeightCmd() {}

    virtual MStatus doIt(const MArgList&) override;

    static void* creator()
    {
        return new retrieveWeightCmd();
    }

    static MString name;

};

#endif // !RETRIEVEWEIGHTCMD_H

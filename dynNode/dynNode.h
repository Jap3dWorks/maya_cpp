#ifndef DYNNODE_H
#define DYNNODE_H

#include <maya/MPxNode.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnStringData.h>
#include <maya/MTypeId.h> 
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MPlugArray.h>
#include <maya/MFnDependencyNode.h>

class dynNode : public MPxNode
{
public:
    dynNode() {}

    virtual ~dynNode() {}

    virtual MStatus compute(const MPlug&, MDataBlock&) override;

    static void* creator();

    static MStatus initialize();

    virtual void postConstructor() override;

    virtual MStatus setDependentsDirty(const MPlug&, MPlugArray&) override;

public:
    static MObject input;
    static MObject output;

    MObject dynAttr;

    static MTypeId id;
    static MString name;
};


#endif
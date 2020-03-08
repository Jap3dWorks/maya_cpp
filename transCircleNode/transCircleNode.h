#ifndef TRANSCIRCLENODE_H
#define TRANSCIRCLENODE_H

#include <maya\MPxNode.h>
#include <maya\MString.h>
#include <maya\MTypeId.h>
#include <maya\MPlug.h>

#include <maya/MFnNumericAttribute.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnTypedAttribute.h>

#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>

#include <string.h>
#include <maya/MIOStream.h>
#include <math.h>

// default maya proxy class
class TransCircle : public MPxNode
{
public:
    TransCircle(){}
    virtual ~TransCircle() {}

    virtual MStatus compute(const MPlug&, MDataBlock&) override;

    static MStatus initialize();
    static void* creator()
    {
        return new TransCircle();
    }

public:
    static MObject input;
    static MObject inputTranslateX;
    static MObject inputTranslateY;
    static MObject inputTranslateZ;
    static MObject inputTranslate;  // compound attribute

    static MObject output;
    static MObject outputTranslateX;
    static MObject outputTranslateY;
    static MObject outputTranslateZ;
    static MObject outputTranslate;  // compound attribute

    static MObject frames;
    static MObject scale;  // radius of circle

    static MTypeId id;
    static MString name;
};

#endif // !TRANSCIRCLENODE_H

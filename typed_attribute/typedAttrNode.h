#ifndef TYPEDATTRNODE_H
#define TYPEDATTRNODE_H

//- Include Maya necessary headers for our class
#include <maya/MPxNode.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnStringData.h>
#include <maya/MTypeId.h> 
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>

class TypedNode : public MPxNode
{
public:
    TypedNode(){}
    virtual ~TypedNode() {}

    virtual MStatus compute(const MPlug&, MDataBlock&) override;

    static void* creator()
    {
        return new TypedNode();
    }

    static MStatus initialize();

public:
    static MObject input;
    static MObject output;
    static MObject decString;

    static MTypeId id;
    static MString name;

};



#endif // TYPEDATTRNODE_H
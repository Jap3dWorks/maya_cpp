#ifndef VERTEX_NODE_H
#define VERTEX_NODE_H

#include <string>

#include <maya/MPxGeometryFilter.h>
#include <maya/MItGeometry.h>
#include <maya/MPxLocatorNode.h>

#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnMatrixData.h>
#include <maya/MFnMeshData.h>

#include <maya/MDGContext.h>
#include <maya/MEvaluationNode.h>

#include <maya/MString.h>
#include <maya/MPxNode.h>

#include <maya/MTypeId.h>
#include <maya/MPlug.h>

#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>



class VertexNode : public MPxNode
{
public:
    VertexNode() {}
    virtual ~VertexNode() override {}

    static void* creator();
    static MStatus initialize();

    MStatus compute(const MPlug& plug, MDataBlock& data) override;

    MStatus setDependentsDirty(const MPlug& plug, MPlugArray& affectedPlugs);

    MStatus preEvaluation(const MDGContext& context, const MEvaluationNode& evaluationNode);

    bool isPassiveOutput(const MPlug& plug) const;

private:
    bool _dirty=false;

public:
    // attributes
    static MObject aInputMesh;

    static MObject aPoint;
    static MObject aPointX;
    static MObject aPointY;
    static MObject aPointZ;


    static MObject aVector;
    static MObject aVectorX;
    static MObject aVectorY;
    static MObject aVectorZ;
    
    static MObject aOutput;  // array, one float per vertex

    // node data
    static MTypeId id;
    static MString name;

};


#endif // !VERTEX_NODE_H
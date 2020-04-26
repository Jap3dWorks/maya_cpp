#include "vertexNode.h"
#include <maya/MMeshIntersector.h>
#include <maya/MFnMesh.h>
#include <maya/MPointArray.h>
#include <maya/MArrayDataBuilder.h>
#include <maya/MIOStream.h>
#include <maya/MGlobal.h>
#include <string.h>

#define MAKE_INPUT(attr)                    \
    CHECK_MSTATUS(attr.setKeyable(true));   \
    CHECK_MSTATUS(attr.setStorable(true));  \
    CHECK_MSTATUS(attr.setReadable(true));  \
    CHECK_MSTATUS(attr.setWritable(true));

#define MAKE_OUTPUT(attr)                   \
    CHECK_MSTATUS(attr.setKeyable(false));  \
    CHECK_MSTATUS(attr.setStorable(false)); \
    CHECK_MSTATUS(attr.setReadable(true));  \
    CHECK_MSTATUS(attr.setWritable(false));

static float* get_vertex_weight(
    const float3& vertex1, const float3& vertex2, const float3& vertex3, const float3& point);
static float dot(const float3 vector1, const float3 vector2);
static float* cross_product(const float3 vector1, const float3 vector2);
static float vector_magnitude(const float3 vector);

MTypeId VertexNode::id(0x8104E);
MString VertexNode::name("vertexNode");

// attributes
MObject VertexNode::aInputMesh;
MObject VertexNode::aOutput;
MObject VertexNode::aPoint;
MObject VertexNode::aPointX;
MObject VertexNode::aPointY;
MObject VertexNode::aPointZ;
MObject VertexNode::aVector;
MObject VertexNode::aVectorX;
MObject VertexNode::aVectorY;
MObject VertexNode::aVectorZ;

void* VertexNode::creator()
{
    return new VertexNode;
}

MStatus VertexNode::initialize()
{
    MStatus state;

    MFnTypedAttribute mAttr;
    MFnNumericAttribute nAttr;

    aInputMesh = mAttr.create("inputMesh", "im", MFnMeshData::kMesh);
    mAttr.setStorable(true);
    addAttribute(aInputMesh);

    aPointX = nAttr.create("pointX", "pX", MFnNumericData::kFloat);
    aPointY = nAttr.create("pointY", "pY", MFnNumericData::kFloat);
    aPointZ = nAttr.create("pointZ", "pZ", MFnNumericData::kFloat);
    aPoint = nAttr.create("point", "p", aPointX, aPointY, aPointZ);
    MAKE_INPUT(nAttr);
    addAttribute(aPoint);

    aVectorX = nAttr.create("vectorX", "vX", MFnNumericData::kFloat);
    aVectorY = nAttr.create("vectorY", "vY", MFnNumericData::kFloat);
    aVectorZ = nAttr.create("vectorZ", "vZ", MFnNumericData::kFloat);
    aVector = nAttr.create("vector", "v", aVectorX, aVectorY, aVectorZ);
    MAKE_INPUT(nAttr);
    addAttribute(aVector);

    // output
    aOutput = nAttr.create("output", "o", MFnNumericData::kFloat);
    nAttr.setArray(true);
    nAttr.setUsesArrayDataBuilder(true);
    nAttr.setWritable(false);
    nAttr.setStorable(false);
    addAttribute(aOutput);

    // attribute affects
    CHECK_MSTATUS(attributeAffects(aInputMesh, aOutput));
    CHECK_MSTATUS(attributeAffects(aPointX, aOutput));
    CHECK_MSTATUS(attributeAffects(aPointY, aOutput));
    CHECK_MSTATUS(attributeAffects(aPointZ, aOutput));
    CHECK_MSTATUS(attributeAffects(aPoint, aOutput));
    CHECK_MSTATUS(attributeAffects(aVectorX, aOutput));
    CHECK_MSTATUS(attributeAffects(aVectorY, aOutput));
    CHECK_MSTATUS(attributeAffects(aVectorZ, aOutput));
    CHECK_MSTATUS(attributeAffects(aVector, aOutput));

    return MS::kSuccess;
}

MStatus VertexNode::setDependentsDirty(const MPlug& plug, MPlugArray& affectedPlugs)
{
    if (plug == aPoint || plug == aPointX || plug == aPointY || plug == aPointZ ||
        plug == aVector || plug == aVectorX || plug == aVectorY || plug == aVectorZ ||
        plug == aInputMesh) _dirty = true;

    return MPxNode::setDependentsDirty(plug, affectedPlugs);
}

MStatus VertexNode::preEvaluation(const MDGContext& context, const MEvaluationNode& evaluationNode)
{
    MStatus status;

    if (!context.isNormal())
    {
        return MS::kFailure;
    }

    if ((evaluationNode.dirtyPlugExists(aInputMesh, &status) && status) ||
        (evaluationNode.dirtyPlugExists(aPoint, &status) && status)     ||
        (evaluationNode.dirtyPlugExists(aPointX, &status) && status)    ||
        (evaluationNode.dirtyPlugExists(aPointY, &status) && status)    ||
        (evaluationNode.dirtyPlugExists(aPointZ, &status) && status)    ||
        (evaluationNode.dirtyPlugExists(aVector, &status) && status)    ||
        (evaluationNode.dirtyPlugExists(aVectorX, &status) && status)   ||
        (evaluationNode.dirtyPlugExists(aVectorY, &status) && status)   ||
        (evaluationNode.dirtyPlugExists(aVectorZ, &status) && status)) _dirty = true;

    return MS::kSuccess;
}

bool VertexNode::isPassiveOutput(const MPlug& plug) const
{
    if (plug == aOutput)
    {
        return true;
    }

    return MPxNode::isPassiveOutput(plug);
}

MStatus VertexNode::compute(const MPlug& plug, MDataBlock& data)
{
    MStatus status = MS::kUnknownParameter;
    if (plug != aOutput)
    {
        return  status;
    }

    // get inputs
    const MDataHandle& mesh_handle = data.inputValue(aInputMesh);
    if (mesh_handle.type() != MFnData::kMesh)
    {
        return MS::kInvalidParameter;
    }

    const MObject mesh_shape = mesh_handle.asMesh();

    MFnMesh fnMesh(mesh_shape);

    const float3& position = data.inputValue(aPoint).asFloat3();
    const float3& vector = data.inputValue(aVector).asFloat3();

    MArrayDataHandle output_array = data.outputArrayValue(aOutput);
    MArrayDataBuilder output_builder = output_array.builder(&status);
    CHECK_MSTATUS(status);

    MFloatPoint ray_position(position[0], position[1], position[2]);
    MFloatVector ray_direction(vector[0], vector[1], vector[2]);

    // output array builder
    unsigned int num_verts = fnMesh.numVertices();

    for (unsigned int i = 0; i < num_verts; ++i)
    {
        MDataHandle outHandle = output_builder.addElement(i);
        outHandle.set(0.f);
    }

    MIntArray faceIds;
    MIntArray triIds;
    MMeshIsectAccelParams mmAccelParams = fnMesh.autoUniformGridParams();
    MFloatPoint hitPoint;
    float hitRayParam;
    int hitFace, hitTriangle;
    float bary1, bary2;

    bool intersects = fnMesh.closestIntersection(
        ray_position, 
        ray_direction, 
        nullptr, 
        nullptr,
        true, 
        MSpace::kWorld,
        99.f,
        false,
        &mmAccelParams,
        hitPoint,
        &hitRayParam,
        &hitFace,
        &hitTriangle,
        &bary1,
        &bary2,
        1e-6,
        &status);

    if (intersects)
    {
        // get vertex ids
        int3 vertex_id;
        fnMesh.getPolygonTriangleVertices(hitFace, hitTriangle, vertex_id);

        // get points position
        MPoint point1;
        fnMesh.getPoint(vertex_id[0], point1, MSpace::kWorld);
        MPoint point2;
        fnMesh.getPoint(vertex_id[1], point2, MSpace::kWorld);
        MPoint point3;
        fnMesh.getPoint(vertex_id[2], point3, MSpace::kWorld);

        float3 float_point1{ point1.x, point1.y, point1.z };
        float3 float_point2{ point2.x, point2.y, point2.z };
        float3 float_point3{ point3.x, point3.y, point3.z };
        float3 float_point{ (float)hitPoint.x, (float)hitPoint.y, (float)hitPoint.z };

        float* vector_weight = get_vertex_weight(
        float_point1,
        float_point2,
        float_point3,
        float_point);

        for (unsigned int i = 0; i < 3; i++)
        {
            MDataHandle data_handle = output_builder.addElement(vertex_id[i]);
            data_handle.set(vector_weight[i]);
        }
    }
    else
    {
        //MGlobal::displayInfo("No intersection found\n");
    }

    output_array.set(output_builder);

    output_array.setAllClean();
    return MS::kSuccess;
}


static float* get_vertex_weight(
    const float3& vertex1, const float3& vertex2, const float3& vertex3, const float3& point)
{
    // barycentric coordinates to extract the area of the inverse try
    float area_v1, area_v2, area_v3;

    const float* vertex_array[3]{ vertex1, vertex2, vertex3 };

    float3 vertex_areas;

    for (unsigned int i = 0; i < 3; ++i)
    {
        float3 v0 = {
            vertex_array[(i + 1) % 3][0] - vertex_array[i][0],
            vertex_array[(i + 1) % 3][1] - vertex_array[i][1],
            vertex_array[(i + 1) % 3][2] - vertex_array[i][2],
        };

        float3 v1 = {
            vertex_array[(i + 2) % 3][0] - vertex_array[i][0],
            vertex_array[(i + 2) % 3][1] - vertex_array[i][1],
            vertex_array[(i + 2) % 3][2] - vertex_array[i][2],
        };

        float3 v2 = {
            point[0] - vertex_array[i][0],
            point[1] - vertex_array[i][1],
            point[2] - vertex_array[i][2]
        };

        float dot00 = dot(v0, v0);
        float dot01 = dot(v0, v1);
        float dot02 = dot(v0, v2);
        float dot11 = dot(v1, v1);
        float dot12 = dot(v1, v2);

        // calc barycentric
        float inv_denom = 1 / (dot00 * dot11 - dot01 * dot01);
        float u = 1 - ((dot11 * dot02 - dot01 * dot12) * inv_denom);
        float v = 1 - ((dot00 * dot12 - dot01 * dot02) * inv_denom);

        std::cout << u << ", " << v << std::endl;
        vertex_areas[i] = vector_magnitude(cross_product(
            float3{ v0[0] * u, v0[1] * u, v0[2] * u },
            float3{ v1[0] * v, v1[1] * v, v1[2] * v }));
    }

    float total_area = vertex_areas[0] + vertex_areas[1] + vertex_areas[2];

    static float3 vertex_weight;
    for (unsigned int i = 0; i < 3; ++i)
    {
        // normalize areas and get inverse
        vertex_weight[i] = vertex_areas[i] / total_area;
    }

    std::cout << vertex_weight[0] + vertex_weight[1] + vertex_weight[2] << std::endl;

    return vertex_weight;
}


static float dot(const float3 vector1, const float3 vector2)
{
    return vector1[0] * vector2[0] + vector1[1] * vector2[1] + vector1[2] * vector2[2];
}


static float* cross_product(const float3 vector1, const float3 vector2)
{
    static float3 cross;
    for (unsigned int i = 0; i < 3; ++i)
    {
        cross[i] = vector1[(i + 1) % 3] * vector2[(i + 2) % 3] - vector1[(i + 2) % 3] * vector2[(i + 1) % 3];
    }

    return cross;
}


static float vector_magnitude(const float3 vector)
{
    return sqrt(vector[0] * vector[0] + vector[1] * vector[1] + vector[2] * vector[2]);
}
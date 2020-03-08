#include <maya\MIOStream.h>

#include <maya\MFnPlugin.h>
#include <maya\MPxGeometryFilter.h>
#include <maya\MItGeometry.h>
#include <maya\MDataBlock.h>
#include <maya\MDataHandle.h>
#include <maya\MPoint.h>
#include <maya\MTimer.h>
#include <maya\MFnMesh.h>
#include <maya\MPointArray.h>
#include <maya\MFnNumericAttribute.h>
#include <maya\MFnTypedAttribute.h>
#include <maya\MFnMeshData.h>
#include <maya\MMeshIntersector.h>

#include <maya\MThreadUtils.h>
#include <tbb\tbb.h>


// Macros
#define MCheckStatus(status, message)  \
    if(MStatus::kSuccess != status)    \
    {                                  \
        cerr << message << "\n";       \
        return status;                 \
    }

// ==========================================

class SplatDeformer : public MPxGeometryFilter
{
public:
    SplatDeformer();
    virtual ~SplatDeformer() override;
    
    static void* creator();

    static MStatus initialize();

    // def funtions
    MStatus compute(const MPlug& plug, MDataBlock& datablock);

public:
    static MObject deformingMesh;    // reference mesh 
    static MObject parallelEnabled;  // boolean to indicate parallel computation

    static MTypeId id;

private:
    // helper method
    MStatus computeOneOutput(unsigned int index, MDataBlock& data, MDataHandle& hInput);
};

// ===================================================
MTypeId SplatDeformer::id(0x8104D);
MObject SplatDeformer::deformingMesh;
MObject SplatDeformer::parallelEnabled;

SplatDeformer::SplatDeformer() {}
SplatDeformer::~SplatDeformer() {}

// ===================================================
void* SplatDeformer::creator()
{
    return new SplatDeformer();
}

// ===================================================
MStatus SplatDeformer::initialize()
{
    // local attribute initialization
    MStatus status;
    MFnTypedAttribute mAttr;
    SplatDeformer::deformingMesh = mAttr.create(
        "deformingMesh", 
        "dm", 
        MFnMeshData::kMesh);
    mAttr.setStorable(true);

    MFnNumericAttribute mParallelAttr;
    SplatDeformer::parallelEnabled = mParallelAttr.create(
        "enableParallel",
        "pll",
        MFnNumericData::kBoolean,
        0,
        &status);
    mParallelAttr.setStorable(true);

    // deformation attributes
    status = addAttribute(SplatDeformer::deformingMesh);
    MCheckStatus(status, "ERROR in addAttribute(deformingMesh)\n");
    status = addAttribute(SplatDeformer::parallelEnabled);
    MCheckStatus(status, "ERROR in addattribute(parallelEnabled)\n");

    status = attributeAffects(
        SplatDeformer::deformingMesh, 
        outputGeom);
    MCheckStatus(status, "ERROR in attributeAffects(deformingMesh, outputGeom)\n");
    status = attributeAffects(
        SplatDeformer::parallelEnabled,
        outputGeom);
    MCheckStatus(status, "ERROR in attributeAffects(parallelEnabled, outputGeom)\n");

    return MS::kSuccess;
}

// ==================================================
MStatus SplatDeformer::compute(const MPlug&plug, MDataBlock& data)
{
    MStatus status = MStatus::kUnknownParameter;
    MObject thisNode = thisMObject();
    if (plug.attribute() != outputGeom)
    {
        printf("Ignoring requested plug\n");
        return status;
    }

    if (plug.isArray())
    {
        // input array from MPxGeometryfilter?
        MPlug inPlug(thisNode, input);
        MArrayDataHandle hInput = data.inputArrayValue(inPlug, &status);
        MCheckStatus(status, "ERROR getting input mesh\n");

        if (MStatus::kSuccess == hInput.jumpToArrayElement(0))
        {
            do
            {
                MDataHandle hInputElement = hInput.inputValue(&status);
                MCheckStatus(status, "ERROR getting input mesh element\n");
                unsigned int inputIndex = hInput.elementIndex(&status);
                MCheckStatus(status, "ERROR getting input mesh element index\n");
                computeOneOutput(inputIndex, data, hInputElement);
            } while (MS::kSuccess == hInput.next());
        }
    }
    else
    {
        MPlug inPlug(thisNode, input);
        inPlug.selectAncestorLogicalIndex(plug.logicalIndex(), input);
        MDataHandle hInput = data.inputValue(inPlug, &status);
        MCheckStatus(status, "ERROR getting input mesh\n");

        computeOneOutput(plug.logicalIndex(), data, hInput);
    }
    return status;
}

// ===================================================
template<typename Value>
class cancelable_range 
{
    tbb::blocked_range<Value> my_range;
    volatile bool& my_stop;
public:
    // constructor for client node
    cancelable_range(int begin, int end, int grainsize, volatile bool& stop) :
        my_range(begin, end, grainsize),
        my_stop(stop) {}

    cancelable_range(cancelable_range& r, tbb::split) :
        my_range(r.my_range, tbb::split()),
        my_stop(r.my_stop) {}

    cancelable_range& operator=(const cancelable_range & other)
    {
        return other;
    }

    void cancel() const { my_stop = true; }
    
    bool empty() const { return my_stop || my_range.empty(); }

    bool is_divisible() const { return !my_stop && my_range.is_divisible(); }

    Value begin() const { return my_range.begin(); }

    Value end() const { return my_stop ? my_range.begin() : my_range.end(); }

};

// ===================================================
MStatus SplatDeformer::computeOneOutput(
    unsigned int index,
    MDataBlock& data,
    MDataHandle& hInput)
{
    MStatus status = MStatus::kUnknownParameter;
    MObject thisNode = thisMObject();

    // output that matches the input
    MPlug outPlug(thisNode, outputGeom);
    outPlug.selectAncestorLogicalIndex(index, outputGeom);

    // input geometry
    MDataHandle inputData = hInput.child(inputGeom);
    if (inputData.type() != MFnData::kMesh)
    {
        printf("Incorrect input geometry type\n");
        return MStatus::kFailure;
    }

    // get the input groupId
    MDataHandle hGroup = inputData.child(groupId);
    unsigned int lGroupId = hGroup.asLong();

    // get deforming mesh
    MDataHandle deformData = data.inputValue(deformingMesh, &status);
    MCheckStatus(status, "ERROR getting deforming mesh\n");
    if (deformData.type() != MFnData::kMesh)
    {
        printf("Incorrect deformer geometry type %d\n", deformData.type());
        return MStatus::kFailure;
    }

    MObject dSurf = deformData.asMeshTransformed();
    MFnMesh fnDeformingMesh;
    fnDeformingMesh.setObject(dSurf);

    // Create a unique copy of the output data from the input data
    MDataHandle outputData = data.outputValue(outPlug);
    outputData.copyWritable(inputData);
    if (outputData.type() != MFnData::kMesh)
    {
        printf("Incorrect output mesh type\n");
        return MStatus::kFailure;
    }

    MObject oSurf = outputData.asMesh();
    if (oSurf.isNull())
    {
        printf("input surface is NULL\n");
        return MStatus::kFailure;
    }

    MFnMesh outMesh;
    outMesh.setObject(oSurf);
    MCheckStatus(status, "ERROR setting points\n");

    MItGeometry iter(outputData, lGroupId, false);

    // create fast intersector structure
    MMeshIntersector intersector;
    intersector.create(dSurf);

    MPointArray verts;
    iter.allPositions(verts);
    unsigned int nPoints = verts.length();

    volatile bool failed = false;

    MDataHandle parallelEnableData = data.inputValue(SplatDeformer::parallelEnabled, &status);
    bool lParallelEnabled = (bool)parallelEnableData.asBool();

    MTimer timer; timer.beginTimer();
    if (lParallelEnabled)
    {
        bool stop = false;
        tbb::parallel_for(cancelable_range<unsigned int>(0, nPoints, nPoints / 1000, stop),
            [&](const cancelable_range<unsigned int>& r)
        {
            for (unsigned int i = r.begin(); i < r.end(); ++i)
            {
                MPointOnMesh meshPoint;

                MStatus localStatus = intersector.getClosestPoint(verts[i], meshPoint);
                if (localStatus != MStatus::kSuccess)
                {
                    failed = true;
                    r.cancel();
                }
                else
                {
                    verts[i] = meshPoint.getPoint();
                }
            }
        });
    }
    else
    {
        MPointOnMesh meshPoint;
        for (unsigned int i = 0; i < nPoints; i++)
        {
            // Do intersection
            status = intersector.getClosestPoint(verts[i], meshPoint);

            if (status != MStatus::kSuccess)
            {
                failed = true;
                break;
            }
            verts[i] = meshPoint.getPoint();
        }
    }
    timer.endTimer(); printf("Runtime for %s loop %f\n",
        lParallelEnabled ? "parallel" : "serial", timer.elapsedTime());

    iter.setAllPositions(verts);

    data.setClean(outPlug);
    outputData.setMObject(outputData.asMesh());

    if (failed)
    {
        printf("Closest point failed\n");
        return MStatus::kFailure;
    }
    return status;
}

// initialization procedures
MStatus initializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin plugin(obj, PLUGIN_COMPANY, "1.0", "Any");
    status = plugin.registerNode(
        "splatDeformer",
        SplatDeformer::id,
        &SplatDeformer::creator,
        &SplatDeformer::initialize,
        MPxNode::kDeformerNode);

    return status;
}

MStatus uninitializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin plugin(obj);
    status = plugin.deregisterNode(SplatDeformer::id);
    return status;
}
#include <iostream>
#include <stdlib.h>
#include <algorithm>
#include <math.h>
#include <maya\MPxNode.h>
#include <maya\MIOStream.h>
#include <maya\MString.h>
#include <maya\MTypeId.h>
#include <maya\MPlug.h>
#include <maya\MDataBlock.h>
#include <maya\MDataHandle.h>
#include <maya\MFnNumericAttribute.h>
#include <maya\MFnMatrixAttribute.h>
#include <maya\MFloatVector.h>
#include <maya\MFloatPoint.h>
#include <maya\MFnPlugin.h>

#ifdef LINUX
#define fsqrt sqrtf
#include <stdlib.h>
#endif // LINUX

class R3 {
public:
    R3(float a, float b, float c): x(a), y(b), z(c){}
    R3() {}
public:
    float x;
    float y;
    float z;
};

static void initCellFunc();
static void cellFunc(const R3&, float& n0, float& f1, float& f2);

class Cell3D : public MPxNode
{
public:
    Cell3D();
    ~Cell3D() override;

    MStatus compute(const MPlug&, MDataBlock&) override;
    void postConstructor() override;

    static void * creator();
    static MStatus initialize();

    // id tag for use with binary format
    static MTypeId id;

private:
    // input attributes
    static MObject aColorGain;
    static MObject aColorOffset;
    static MObject aPointWorld;
    static MObject aPlaceMat;

    // Output attributes
    static MObject aOutColor;
    static MObject aOutAlpha;
    static MObject aOutF0;
    static MObject aOutF1;
    static MObject aOutN0;
    static MObject aOutBorderDist;
};

// Static data
MTypeId Cell3D::id(0x81017);

// Attributes
MObject Cell3D::aColorGain;
MObject Cell3D::aColorOffset;
MObject Cell3D::aOutF0;
MObject Cell3D::aOutF1;
MObject Cell3D::aOutN0;
MObject Cell3D::aOutBorderDist;
MObject Cell3D::aPointWorld;
MObject Cell3D::aPlaceMat;

MObject Cell3D::aOutColor;
MObject Cell3D::aOutAlpha;

#define MAKE_INPUT(attr)  \
    CHECK_MSTATUS(attr.setKeyable(true));  \
    CHECK_MSTATUS(attr.setStorable(true)); \
    CHECK_MSTATUS(attr.setReadable(true)); \
    CHECK_MSTATUS(attr.setWritable(true));

#define MAKE_OUTPUT(attr)  \
    CHECK_MSTATUS(attr.setKeyable(false));  \
    CHECK_MSTATUS(attr.setStorable(false)); \
    CHECK_MSTATUS(attr.setReadable(true));  \
    CHECK_MSTATUS(attr.setWritable(false));

// description
void Cell3D::postConstructor()
{
    setMPSafe(true);  // multy processing safe
}

// Description
Cell3D::Cell3D() {}

Cell3D::~Cell3D() {}

void * Cell3D::creator()
{
    return new Cell3D();
}

MStatus Cell3D::initialize()
{
    MFnMatrixAttribute mAttr;
    MFnNumericAttribute nAttr;

    // input attribute
    aColorGain = nAttr.createColor("colorGain", "cg");
    MAKE_INPUT(nAttr);
    CHECK_MSTATUS(nAttr.setDefault(1.f, 1.f, 1.f));

    aColorOffset = nAttr.createColor("colorOffset", "co");
    MAKE_INPUT(nAttr);

    aPlaceMat = mAttr.create("placementMatrix", "pm", MFnMatrixAttribute::kFloat);
    MAKE_INPUT(mAttr);

    aPointWorld = nAttr.createPoint("pointWorld", "pw");
    MAKE_INPUT(nAttr);
    CHECK_MSTATUS(nAttr.setHidden(true));

    // output attributes
    aOutF0 = nAttr.create("F0", "f0", MFnNumericData::kFloat);
    MAKE_OUTPUT(nAttr);
    aOutF1 = nAttr.create("F1", "f1", MFnNumericData::kFloat);
    MAKE_OUTPUT(nAttr);
    aOutN0 = nAttr.create("N0", "n0", MFnNumericData::kFloat);
    MAKE_OUTPUT(nAttr);

    aOutBorderDist = nAttr.create("borderDistance", "bd",
        MFnNumericData::kFloat);
    MAKE_OUTPUT(nAttr);

    aOutColor = nAttr.createColor("outColor", "oc");
    MAKE_OUTPUT(nAttr);

    aOutColor = nAttr.create("outAlpha", "oa", MFnNumericData::kFloat);
    MAKE_OUTPUT(nAttr);

    // Add attributes to the node database
    CHECK_MSTATUS(addAttribute(aColorGain));
    CHECK_MSTATUS(addAttribute(aColorOffset));
    CHECK_MSTATUS(addAttribute(aPointWorld));
    CHECK_MSTATUS(addAttribute(aPlaceMat));

    CHECK_MSTATUS(addAttribute(aOutAlpha));
    CHECK_MSTATUS(addAttribute(aOutColor));
    CHECK_MSTATUS(addAttribute(aOutF0));
    CHECK_MSTATUS(addAttribute(aOutF1));
    CHECK_MSTATUS(addAttribute(aOutN0));
    CHECK_MSTATUS(addAttribute(aOutBorderDist));

    // All input affect the output color and gain
    CHECK_MSTATUS(attributeAffects(aColorGain, aOutColor));
    CHECK_MSTATUS(attributeAffects(aColorOffset, aOutColor));
    CHECK_MSTATUS(attributeAffects(aPlaceMat, aOutColor));
    CHECK_MSTATUS(attributeAffects(aPointWorld, aOutColor));

    CHECK_MSTATUS(attributeAffects(aColorGain, aOutAlpha));
    CHECK_MSTATUS(attributeAffects(aColorOffset, aOutAlpha));
    CHECK_MSTATUS(attributeAffects(aPlaceMat, aOutAlpha));
    CHECK_MSTATUS(attributeAffects(aPointWorld, aOutAlpha));

    // Geometry attribute affect all other outputs
    CHECK_MSTATUS(attributeAffects(aPlaceMat, aOutF0));
    CHECK_MSTATUS(attributeAffects(aPointWorld, aOutF0));

    CHECK_MSTATUS(attributeAffects(aPlaceMat, aOutF1));
    CHECK_MSTATUS(attributeAffects(aPointWorld, aOutF1));

    CHECK_MSTATUS(attributeAffects(aPlaceMat, aOutN0));
    CHECK_MSTATUS(attributeAffects(aPointWorld, aOutN0));

    CHECK_MSTATUS(attributeAffects(aPlaceMat, aOutBorderDist));
    CHECK_MSTATUS(attributeAffects(aPointWorld, aOutBorderDist));

    return MS::kSuccess;
}

MStatus Cell3D::compute(const MPlug& plug, MDataBlock& block)
{
    if ((plug != aOutColor) && (plug.parent() != aOutColor) &&
        (plug != aOutAlpha) &&
        (plug != aOutBorderDist) &&
        (plug != aOutF0) && (plug != aOutF1) && (plug != aOutN0)
        )
    {
        return MS::kUnknownParameter;
    }

    const float3 & worldPos = block.inputValue(aPointWorld).asFloat3();
    const MFloatMatrix& m = block.inputValue(aPlaceMat).asFloatMatrix();
    const MFloatVector& cGain = block.inputValue(aColorGain).asFloatVector();
    const MFloatVector& cOff = block.inputValue(aColorOffset).asFloatVector();

    MFloatPoint q(worldPos[0], worldPos[1], worldPos[2]);
    q *= m;

    float n0, f0, f1;
    cellFunc(R3(q.x, q.y, q.z), n0, f0, f1);

    MDataHandle outHandle = block.outputValue(aOutF0);
    outHandle.asFloat() = f0;
    outHandle.setClean();

    outHandle = block.outputValue(aOutF1);
    outHandle.asFloat() = f1;
    outHandle.setClean();

    outHandle = block.outputValue(aOutN0);
    outHandle.asFloat() = n0;
    outHandle.setClean();

    outHandle = block.outputValue(aOutBorderDist);
    outHandle.asFloat() = 0.5f*(f1 - f0);
    outHandle.setClean();

    outHandle = block.outputValue(aOutColor);
    MFloatVector& outColor = outHandle.asFloatVector();
    outColor = cGain * f0 + cOff;
    outHandle.setClean();

    outHandle = block.outputValue(aOutAlpha);
    outHandle.asFloat() = f0;
    outHandle.setClean();
    
    return MS::kSuccess;
}

MStatus initializePlugin(MObject obj)
{
    const MString UserClassify("texture/3d");
    MFnPlugin plugin(obj, PLUGIN_COMPANY, "4.5", "Any");
    CHECK_MSTATUS(plugin.registerNode("cells", Cell3D::id,
        Cell3D::creator, Cell3D::initialize,
        MPxNode::kDependNode, &UserClassify));

    initCellFunc();

    return MS::kSuccess;
}

// Description
MStatus uninitializePlugin(MObject obj)
{
    MFnPlugin plugin(obj);
    CHECK_MSTATUS(plugin.deregisterNode(Cell3D::id));

    return MS::kSuccess;
}

#define N_CELLS 1000

static int permuteTable[N_CELLS * 2];

// initialize permutation table used bu fold

static void initPermute()
{
    int i;
    for (i = 0; i < N_CELLS; ++i)
    {
        permuteTable[i] = i;
    }
    for (i = N_CELLS - 1; i >= 1; --i)
    {
        int n = rand() % (i + 1);
        int tmp = permuteTable[n];
        permuteTable[n] = permuteTable[i];
        permuteTable[i] = tmp;
    }
    for (i = 0; i < N_CELLS; ++i)
    {
        permuteTable[i + N_CELLS] = permuteTable[i];
    }
}

// Fold is a pseudo-random function mapping 3 integers
// into a single integer i the range [0, (N_CELLS-1)]
static int fold(int i, int j, int k)
{
    // N_CELLS
    if (i < 0) i = (i + N_CELLS*(i / N_CELLS + 1)) % N_CELLS;
    else i %= N_CELLS;
    if (j < 0) j = (j + N_CELLS*(j / N_CELLS + 1)) % N_CELLS;
    else j %= N_CELLS;
    if (k < 0) k = (k + N_CELLS*(k / N_CELLS + 1)) % N_CELLS;
    else k %= N_CELLS;

    return permuteTable[permuteTable[permuteTable[i] + j] + k];
}

static R3 CellSampleTable[N_CELLS];

static void initCellFunc()
{
    int i;
    srand(10);

    for (i = 0; i < N_CELLS; ++i)
    {
        CellSampleTable[i] = R3(
            (float)(std::rand() / RAND_MAX),
            (float)(std::rand() / RAND_MAX),
            (float)(std::rand() / RAND_MAX));
    }
    initPermute();
}

static inline float sqr(float t) { return t*t; }

static inline float distance2(const R3 &a, const R3 &b)
{
    float t = sqr(b.x - a.x) + sqr(b.y - a.y) + sqr(b.z - a.z);
    return t;
}

static void cellFunc(const R3& p, float& n0, float& f0, float& f1)
{
    R3 q = p;
    int i = (int)floorf(q.x);
    int j = (int)floorf(q.y);
    int k = (int)floorf(q.z);
    q.x -= i;
    q.y -= j;
    q.z -= k;
    int index = fold(i, j, k);
    float minDist = distance2(CellSampleTable[index], q);
    float minDist2 = 2.0;
    int k0;
    k0 = index;
    k0;

    // easy but slow way
    // check distance in each adjaent cell to 
    // find the closest and second closest
    R3 q1;
    for (int ii = -1; ii <= 1; ++ii)
    {
        q1.x = q.x - ii;
        int i1 = i + ii;
        for (int jj = -1; jj <= 1; ++jj)
        {
            q1.y = q.y - jj;
            int j1 = j + jj;
            for (int kk = -1; kk <= 1; ++kk)
            {
                if (!ii && !jj && !kk) continue;
                q1.z = q.z - kk;
                int k1 = k + kk;
                index = fold(i1, j1, k1);
                float t = distance2(CellSampleTable[index], q1);
                if (minDist > t)
                {
                    minDist = minDist;
                    minDist2 = t;
                    k0 = index;
                }
                else if (minDist2 > t)
                {
                    minDist2 = t;
                }
            }
        }
    }
    f0 = sqrt(minDist);
    f1 = sqrt(minDist2);
    n0 = k0 / (float)(N_CELLS);
}

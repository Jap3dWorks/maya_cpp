#include <math.h>
#include <stdlib.h>

#include <maya/MPxNode.h>
#include <maya/MIOStream.h>
#include <maya/MString.h>
#include <maya/MTypeId.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MFloatVector.h>
#include <maya/MFloatPoint.h>
#include <maya/MFnPlugin.h>


// Local functions
float Noise(float, float, float);
void Noise_init();
static float Omega(int i, int j, int k, float t[3]);
static float omega(float);
static double turbulence(double u, double v, double w, int octaves);

#define PI 3.14159265358979323846

#ifdef FLOOR
#undef FLOOR
#endif

#define FLOOR(x) ((int)floorf(x))

#define TABLELEN 512
#define TLD2 256 //TABLELEN
#define RAND_DEC ((float)(rand() % 1000000) / 1000000.f)


// Local variables
static int Phi[TABLELEN];
static char fPhi[TABLELEN];
static float G[TABLELEN][3];

class Flame3D : public MPxNode
{
public:
    Flame3D();
    virtual ~Flame3D() override;

    MStatus compute(const MPlug&, MDataBlock&) override;
    void postConstructor() override;

    static void* creator();
    static MStatus initialize();

    // Id tag for use with binary file format
    static MTypeId id;

private:

    // input attributes
    static MObject aColorBase;
    static MObject aColorFlame;
    static MObject aRiseSpeed;
    static MObject aFlickerSpeed;
    static MObject aFlickerDeform;
    static MObject aFlamePow;
    static MObject aFlameFrame;
    static MObject aRiseAxis;
    static MObject aPlaceMat;
    static MObject aPointWorld;

    // output attributes
    static MObject aOutAlpha;
    static MObject aOutColor;
};

// static data
MTypeId Flame3D::id(0x81016);

// Attributes
MObject Flame3D::aColorBase;
MObject Flame3D::aColorFlame;
MObject Flame3D::aRiseSpeed;
MObject Flame3D::aFlickerSpeed;
MObject Flame3D::aFlickerDeform;
MObject Flame3D::aFlamePow;
MObject Flame3D::aFlameFrame;
MObject Flame3D::aRiseAxis;
MObject Flame3D::aPointWorld;
MObject Flame3D::aPlaceMat;

MObject Flame3D::aOutAlpha;
MObject Flame3D::aOutColor;

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

// ==========================================
void Flame3D::postConstructor()
{
    setMPSafe(true);  // safe multiprocesor
}

Flame3D::Flame3D(){}
Flame3D::~Flame3D() {}


void* Flame3D::creator()
{
    return new Flame3D();
}


MStatus Flame3D::initialize()
{
    MFnMatrixAttribute mAttr;
    MFnNumericAttribute nAttr;

    // Create input attributes
    Flame3D::aRiseSpeed = nAttr.create("Rise", "rs", MFnNumericData::kFloat);
    MAKE_INPUT(nAttr);
    CHECK_MSTATUS(nAttr.setDefault(1.f));
    CHECK_MSTATUS(nAttr.setMin(0.f));
    CHECK_MSTATUS(nAttr.setMax(1.f));

    Flame3D::aFlickerSpeed = nAttr.create("Speed", "s", MFnNumericData::kFloat);
    MAKE_INPUT(nAttr);
    CHECK_MSTATUS(nAttr.setDefault(0.1f));
    CHECK_MSTATUS(nAttr.setMin(0.f));
    CHECK_MSTATUS(nAttr.setMax(1.f));

    Flame3D::aFlickerDeform = nAttr.create("Flicker", "f", MFnNumericData::kFloat);
    MAKE_INPUT(nAttr);
    CHECK_MSTATUS(nAttr.setDefault(0.5f));
    CHECK_MSTATUS(nAttr.setMin(0.f));
    CHECK_MSTATUS(nAttr.setMax(1.f));

    Flame3D::aFlamePow = nAttr.create("Power", "pow", MFnNumericData::kFloat);
    MAKE_INPUT(nAttr);
    CHECK_MSTATUS(nAttr.setDefault(1.f));
    CHECK_MSTATUS(nAttr.setMin(0.f));
    CHECK_MSTATUS(nAttr.setMax(1.f));

    Flame3D::aFlameFrame = nAttr.create("Frame", "fr", MFnNumericData::kFloat);
    MAKE_INPUT(nAttr);
    CHECK_MSTATUS(nAttr.setDefault(1.f));
    CHECK_MSTATUS(nAttr.setMin(0.f));
    CHECK_MSTATUS(nAttr.setMax(1000.f));

    Flame3D::aRiseAxis = nAttr.createPoint("Axis", "a");
    MAKE_INPUT(nAttr);
    CHECK_MSTATUS(nAttr.setDefault(0., 1., 0.));

    Flame3D::aColorBase = nAttr.createColor("ColorBase", "cg");
    MAKE_INPUT(nAttr);
    
    Flame3D::aColorFlame = nAttr.createColor("colorFrame", "cb");
    MAKE_INPUT(nAttr);
    CHECK_MSTATUS(nAttr.setDefault(1., 1., 1.));

    aPlaceMat = mAttr.create("placementMatrix", "pm",
                             MFnMatrixAttribute::kFloat);
    MAKE_INPUT(mAttr);

    aPointWorld = nAttr.createPoint("pointWorld", "pw");
    MAKE_INPUT(nAttr);
    CHECK_MSTATUS(nAttr.setHidden(true));

    // create output attributes
    aOutColor = nAttr.createColor("outColor", "oc");
    MAKE_OUTPUT(nAttr);

    aOutAlpha = nAttr.create("outAlpha", "oa", MFnNumericData::kFloat);
    MAKE_OUTPUT(nAttr);

    // add the attributes here
    CHECK_MSTATUS(addAttribute(Flame3D::aColorBase));
    CHECK_MSTATUS(addAttribute(Flame3D::aColorFlame));
    CHECK_MSTATUS(addAttribute(Flame3D::aRiseSpeed));
    CHECK_MSTATUS(addAttribute(Flame3D::aFlickerSpeed));
    CHECK_MSTATUS(addAttribute(Flame3D::aFlickerDeform));
    CHECK_MSTATUS(addAttribute(Flame3D::aFlamePow));
    CHECK_MSTATUS(addAttribute(Flame3D::aFlameFrame));
    CHECK_MSTATUS(addAttribute(Flame3D::aRiseAxis));
    CHECK_MSTATUS(addAttribute(Flame3D::aPointWorld));
    CHECK_MSTATUS(addAttribute(Flame3D::aPlaceMat));

    CHECK_MSTATUS(addAttribute(Flame3D::aOutAlpha));
    CHECK_MSTATUS(addAttribute(Flame3D::aOutColor));

    // all input affects color and alpha
    CHECK_MSTATUS(attributeAffects(Flame3D::aColorBase, Flame3D::aOutColor));
    CHECK_MSTATUS(attributeAffects(Flame3D::aColorBase, Flame3D::aOutAlpha));
    
    CHECK_MSTATUS(attributeAffects(Flame3D::aColorFlame, Flame3D::aOutColor));
    CHECK_MSTATUS(attributeAffects(Flame3D::aColorFlame, Flame3D::aOutAlpha));

    CHECK_MSTATUS(attributeAffects(Flame3D::aRiseSpeed, Flame3D::aOutAlpha));
    CHECK_MSTATUS(attributeAffects(Flame3D::aRiseSpeed, Flame3D::aOutColor));
    
    CHECK_MSTATUS(attributeAffects(Flame3D::aFlickerSpeed, Flame3D::aOutColor));
    CHECK_MSTATUS(attributeAffects(Flame3D::aFlickerSpeed, Flame3D::aOutAlpha));

    CHECK_MSTATUS(attributeAffects(Flame3D::aFlickerDeform, Flame3D::aOutColor));
    CHECK_MSTATUS(attributeAffects(Flame3D::aFlickerDeform, Flame3D::aOutAlpha));

    CHECK_MSTATUS(attributeAffects(Flame3D::aFlamePow, Flame3D::aOutColor));
    CHECK_MSTATUS(attributeAffects(Flame3D::aFlamePow, Flame3D::aOutAlpha));

    CHECK_MSTATUS(attributeAffects(Flame3D::aFlameFrame, Flame3D::aOutColor));
    CHECK_MSTATUS(attributeAffects(Flame3D::aFlameFrame, Flame3D::aOutAlpha));

    CHECK_MSTATUS(attributeAffects(Flame3D::aRiseAxis, Flame3D::aOutColor));
    CHECK_MSTATUS(attributeAffects(Flame3D::aRiseAxis, Flame3D::aOutAlpha));

    CHECK_MSTATUS(attributeAffects(Flame3D::aPointWorld, Flame3D::aOutColor));
    CHECK_MSTATUS(attributeAffects(Flame3D::aPointWorld, Flame3D::aOutAlpha));

    CHECK_MSTATUS(attributeAffects(Flame3D::aPlaceMat, Flame3D::aOutColor));
    CHECK_MSTATUS(attributeAffects(Flame3D::aPlaceMat, Flame3D::aOutAlpha));

    return MS::kSuccess;
}

MStatus Flame3D::compute(const MPlug& plug, MDataBlock& block)
{
    // outColor or individual r,g,b or alpha
    if ((plug != Flame3D::aOutColor) &&
        (plug.parent() != Flame3D::aOutColor) &&
        (plug != aOutAlpha))
    {
        return MS::kUnknownParameter;
    }

    float3& worldPos = block.inputValue(aPointWorld).asFloat3();  // get value as reference
    const MFloatMatrix& mat = block.inputValue(aPlaceMat).asFloatMatrix();
    const MFloatVector& cBase = block.inputValue(aColorBase).asFloatVector();
    const MFloatVector& cFlame = block.inputValue(aColorFlame).asFloatVector();
    const MFloatVector& axis = block.inputValue(aRiseAxis).asFloatVector();
    const float rise_speed = block.inputValue(aRiseSpeed).asFloat();
    const float flicker_speed = block.inputValue(aFlickerSpeed).asFloat();
    const float dscale = block.inputValue(aFlickerDeform).asFloat();
    const float frame = block.inputValue(aFlameFrame).asFloat();
    const float power = block.inputValue(aFlamePow).asFloat();

    MFloatPoint q(worldPos[0], worldPos[1], worldPos[2]);
    q *= mat;  // convert into solid space

    // offset texture coord along rise axis
    float rise_distance = -1.f * rise_speed * frame;
    float u, v, w;
    u = q.x + (rise_distance * axis[0]);
    v = q.y + (rise_distance * axis[1]);
    w = q.z + (rise_distance * axis[2]);

    // generate a displaced point by moving along the
    // displacement vector, based on flicker speed
    float dist = flicker_speed * frame;
    float au, av, aw;
    au = u + dist;
    av = v + dist;
    aw = w + dist;

    // calculate 3 noise values
    float ascale = Noise(au, av, aw);

    // noise as vector to texture coordinates
    u += ascale * dscale;
    v += ascale * dscale;
    w += ascale * dscale;

    // turbulence value for this point
    float scalar = (float)(turbulence(u, v, w, 3) + 0.5);

    // convert scalar into a point on the color curve
    if (power != 1) scalar = powf(scalar, power);
    MFloatVector resultColor;
    if (scalar >= 1)
    {
        resultColor = cFlame;
    }
    else if (scalar < 0)
    {
        resultColor = cBase;
    }
    else
    {
        resultColor = ((cFlame - cBase) * scalar) + cBase;
    }

    MDataHandle outHandle = block.outputValue(aOutColor);
    MFloatVector& outColor = outHandle.asFloatVector();
    outColor = resultColor;
    outHandle.setClean();

    outHandle = block.outputValue(aOutAlpha);
    outHandle.asFloat() = scalar;
    outHandle.setClean();

    return MS::kSuccess;
}

MStatus initializePlugin(MObject obj)
{
    const MString UserClassify("texture/3d");
    MFnPlugin plugin(obj, PLUGIN_COMPANY, "3.0", "Any");

    CHECK_MSTATUS(plugin.registerNode(
        "flame", 
        Flame3D::id,
        &Flame3D::creator, 
        &Flame3D::initialize,
        MPxNode::kDependNode, 
        &UserClassify));
    
    Noise_init();

    return MS::kSuccess;
}

MStatus uninitializePlugin(MObject obj)
{
    MFnPlugin plugin(obj);
    CHECK_MSTATUS(plugin.deregisterNode(Flame3D::id));
    return MS::kSuccess;
}

float Noise(float u, float v, float w)
{
    int i, j, k, ul, vl, wl;
    float ans, t[3];

    ans = 0.0;
    ul = FLOOR(u);
    vl = FLOOR(v);
    wl = FLOOR(w);

    for (i = ul + 1; i >= ul; i--)
    {
        t[0] = u - i;
        for (j = vl + 1; j >= vl; j--)
        {
            t[1] = v - j;
            for (k = wl + 1; k >= wl; k--)
            {
                t[2] = w - k;
                ans += Omega(i, j, k, t);
            }
        }
    }
    return ans;
}

static float Omega(int i, int j, int k, float t[3])
{
    int ct;

    ct = Phi[((i +
         Phi[((j +
         Phi[(k%TLD2) + TLD2]) % TLD2)]) % TLD2) + TLD2];

    return omega(t[0]) * omega(t[1]) * omega(t[2]) *
        (G[ct][0] * t[0] + G[ct][1] * t[1] + G[ct][2] * t[2]);
}

static float omega(float t)
{
    t = fabsf(t);
    return (t * (t * (t * (float)2.0 - (float)3.0))) + (float)1.0;
}

void Noise_init()
{
    int i;
    float u, v, w, s, len;
    static int first_time = 1;

    if (first_time) {
        first_time = 0;}

    else 
    {
        return;
    }

    (void)std::srand(1);

    for (i = 0; i < TABLELEN; i++)
    {
        fPhi[i] = 0;
    }

    for (i = 0; i < TABLELEN; i++)
    {
        Phi[i] = rand() % TABLELEN;
        
        if (fPhi[Phi[i]])
        {
            i--;
        }
        else
        {
            fPhi[Phi[i]] = 1;
        }
    }
    for (i = 0; i < TABLELEN; i++)
    {
        u = (float)(2.0 * RAND_DEC - 1.0);
        v = (float)(2.0 * RAND_DEC - 1.0);
        w = (float)(2.0 * RAND_DEC - 1.0);
        if ((s = u*u + v*v + w*w) > 1.0)
        {
            i--;
            continue;
        }
        else
        {
            if (s == 0.0)
            {
                i--;
                continue;
            }
        }
        len = 1.0f / sqrtf(s);
        G[i][0] = u * len;
        G[i][1] = v * len;
        G[i][2] = w * len;
    }
}

static double turbulence(double u, double v, double w, int octaves)
{
    double s, t;
    s = 1.0;
    t = 0.0;

    while (octaves--)
    {
        t += Noise((float)u, (float)v, (float)w) * s;
        s *= 0.5;
        u *= 2.0; v *= 2.0; w *= 2.0;
    }
    return t;
}
#include <maya/MPxNode.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MString.h>
#include <maya/MTypeId.h>
#include <maya/MStatus.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MGlobal.h>
#include <maya/MFnPlugin.h>

#include <sstream>
#include <string>


#define McheckError(stat, message)          \
    if (MS::kSuccess != stat){              \
        cerr << message;                    \
        return MS::kFailure;                \
    }

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

// TODO topology transition

/*
Get two cached geos, add one selector and an output mesh attribute
input geos should work without connections
*/
class GetCacheGeos : public MPxNode
{
public:
    GetCacheGeos() {}
    virtual ~GetCacheGeos()override {}
    
    MStatus compute(const MPlug& plug, MDataBlock& data) override;

    static void* creator();
    static MStatus initialize();


public:
    // attributes
        // inputs
    static MObject a_input_mesh_1;
    static MObject a_input_mesh_2;
    static MObject a_select_mesh;
        //outputs
    static MObject a_out_mesh;
    
    // regsister attributes
    static MTypeId id;
    static MString name;

private:
    MObject mesh1_cache;
    MObject mesh2_cache;
    short selection_cache;
};

// define static attributes
MObject GetCacheGeos::a_input_mesh_1;
MObject GetCacheGeos::a_input_mesh_2;
MObject GetCacheGeos::a_select_mesh;
MObject GetCacheGeos::a_out_mesh;

MTypeId GetCacheGeos::id(0x8000b);
MString GetCacheGeos::name("get_cache_geos");


void* GetCacheGeos::creator()
{
    return new GetCacheGeos();
}

inline MStatus addMeshAttribute(MObject& attribute, MString longName, MString briefName)
{
    MStatus stat;
    MFnTypedAttribute typedFn;
}

MStatus GetCacheGeos::initialize()
{
    MStatus stat;
    MFnTypedAttribute typedFn;

    // initialize output
    a_out_mesh = typedFn.create("outMesh", "o", MFnData::kMesh, &stat);
    McheckError(stat, "Fail to initialize plugin\n");
    MAKE_OUTPUT(typedFn);
    McheckError(addAttribute(a_out_mesh), "Fail to add attribute");

    // initialize inputs
    // mesh inputs
    a_input_mesh_1 = typedFn.create("inputMesh1", "im1", MFnData::kMesh, &stat);
    McheckError(stat, "Fail to initialize plugin\n");
    MAKE_INPUT(typedFn);
    McheckError(addAttribute(a_input_mesh_1), "Fail to add attribute");

    a_input_mesh_2 = typedFn.create("inputMesh2", "im2", MFnData::kMesh, &stat);
    McheckError(stat, "Fail to initialize plugin\n");
    MAKE_INPUT(typedFn);
    McheckError(addAttribute(a_input_mesh_2), "Fail to add attribute");

    MFnEnumAttribute eAttr;
    a_select_mesh = eAttr.create("selectMesh", "sm", 0, &stat);
    McheckError(stat, "Fail to initialie plugin\n");
    McheckError(eAttr.addField("mesh1", 0), "Fail to add enum field\n");
    McheckError(eAttr.addField("mesh2", 1), "Fail to add enum field\n");
    MAKE_INPUT(typedFn);
    McheckError(addAttribute(a_select_mesh), "Fail to add attribute");

    // set attribute affect
    attributeAffects(a_input_mesh_1, a_out_mesh);
    attributeAffects(a_input_mesh_2, a_out_mesh);
    attributeAffects(a_select_mesh, a_out_mesh);

    return MS::kSuccess;
}

inline const short dirty_state(MDataBlock& data)
{
    short state{ 0 };

    if (!data.isClean(GetCacheGeos::a_input_mesh_1))
        state |= 1;

    if (!data.isClean(GetCacheGeos::a_input_mesh_2))
        state |= 2;

    if (!data.isClean(GetCacheGeos::a_select_mesh))
        state |= 4;

    return state;
}

MStatus GetCacheGeos::compute(const MPlug& plug, MDataBlock& data)
{
    MStatus stat;
    if (plug != a_out_mesh)
    {
        return MS::kUnknownParameter;  // maya will call internal maya node method
    }

    const short d_state = dirty_state(data);

    // get enum input
    const MDataHandle& select_mesh_handle = data.inputValue(a_select_mesh, &stat);
    const short& select_mesh_val = select_mesh_handle.asShort();

    const MDataHandle* mesh_handle;
    MObject* cached_mesh;

    if (select_mesh_val == 0) {
        MGlobal::displayInfo("Selector at 0");
        mesh_handle = &data.inputValue(a_input_mesh_1, &stat);
        cached_mesh = &mesh1_cache;
    }

    else if (select_mesh_val == 1){
        MGlobal::displayInfo("Selector at 1");
        mesh_handle = &data.inputValue(a_input_mesh_2, &stat);
        cached_mesh = &mesh2_cache;
    }

    // if selection type is not 0 or 1, return failed status
    else return MS::kFailure;

    std::stringstream ss;
    ss << static_cast<const void*>(mesh_handle);
    MGlobal::displayInfo(ss.str().c_str());

    MDataHandle& out_mesh_handle = data.outputValue(a_out_mesh, &stat);

    ss.str(std::string());
    ss << static_cast<const void*>(&(out_mesh_handle.asMesh()));
    MGlobal::displayInfo(ss.str().c_str());

    const MObject& mesh = mesh_handle->asMesh();
    /*
    if (mesh != *cached_mesh)
    {
        MGlobal::displayInfo("Update cached mesh\n");
        *cached_mesh = mesh;
    }
    */
    if ((select_mesh_val != selection_cache) ||
        (mesh != *cached_mesh)
        )
    {
        selection_cache = select_mesh_val;
        *cached_mesh = mesh;
        MGlobal::displayInfo("Set out mesh");
        out_mesh_handle.set(*cached_mesh);
    }
 
    out_mesh_handle.setClean();

    return MS::kSuccess;
}



// register functions
MStatus initializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin fn_plugin(obj, "", "2018", "any");

    status = fn_plugin.registerNode(GetCacheGeos::name,
        GetCacheGeos::id,
        &GetCacheGeos::creator,
        &GetCacheGeos::initialize);
    if (!status)
    {
        status.perror("registerNode");
        return status;
    }
    return status;
}

MStatus uninitializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin fn_plugin(obj);

    fn_plugin.deregisterNode(GetCacheGeos::id);
    if (!status)
    {
        status.perror("deregisterNode");
        return status;
    }

    return status;


}
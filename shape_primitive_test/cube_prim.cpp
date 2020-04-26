#include "cube_prim.h"
#include <maya/MPoint.h>

#define McheckErr(stat, msg)    \
    if (MS::kSuccess != stat){  \
        cerr << msg << "\n";    \
        return stat;            \
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

static const float _basic_cube_vtx[8][3] =
{
    {-1.f,-1.f,-1.f},
    { 1.f,-1.f,-1.f },
    {1.f, -1.f, 1.f},
    {-1.f, -1.f, 1.f},
    {-1.f, 1.f, -1.f},
    { -1.f, 1.f, 1.f },
    { 1.f, 1.f, 1.f },
    { 1.f, 1.f, -1.f }
};


MTypeId CubePrim::id(0x8000b);

MObject CubePrim::size;
MObject CubePrim::size_x;
MObject CubePrim::size_y;
MObject CubePrim::size_z;

MObject CubePrim::subdivision;
MObject CubePrim::subdivision_x;
MObject CubePrim::subdivision_y;
MObject CubePrim::subdivision_z;

MObject CubePrim::outMesh;

void* CubePrim::creator()
{
    return new CubePrim;
}

MStatus CubePrim::initialize()
{
    MStatus stat;
    MFnTypedAttribute typedFn;
    MFnNumericAttribute uAttr;

    outMesh = typedFn.create("outMesh", "o", MFnData::kMesh, &stat);
    McheckErr(stat, "ERROR creating outMesh attribute");
    MAKE_OUTPUT(typedFn);
    stat = addAttribute(outMesh);
    McheckErr(stat, "ERROR adding attribute");

    // size 
    size_x = uAttr.create("sizeX", "sx", MFnNumericData::kDouble, 1.0, &stat);
    McheckErr(stat, "ERROR creating sizeX attribute");
    size_y = uAttr.create("sizeY", "sy", MFnNumericData::kDouble, 1.0, &stat);
    McheckErr(stat, "ERROR creating sizeY attribute");
    size_z = uAttr.create("sizeZ", "sz", MFnNumericData::kDouble, 1.0, &stat);
    McheckErr(stat, "ERROR creating sizeZ attribute");
    size = uAttr.create("size", "s", size_x, size_y, size_z, &stat);
    McheckErr(stat, "ERROR creating size attribute");
    MAKE_INPUT(uAttr);
    stat = addAttribute(size);
    McheckErr(stat, "ERROR adding size attributes");

    // subdivision
    subdivision_x = uAttr.create("subdivisionX", "sdx", MFnNumericData::kInt, 0, &stat);
    McheckErr(stat, "ERROR creating subdivisionX attribute");
    subdivision_y = uAttr.create("subdivisionY", "sdy", MFnNumericData::kInt, 0, &stat);
    McheckErr(stat, "ERROR creating subdivisionY attribute");
    subdivision_x = uAttr.create("subdivisionZ", "sdz", MFnNumericData::kInt, 0, &stat);
    McheckErr(stat, "ERROR creating subdivisionZ attribute");
    subdivision = uAttr.create("subdivision", "sd", subdivision_x, subdivision_y, subdivision_z, &stat);
    McheckErr(stat, "ERROR creating subdivision attribute");
    stat = addAttribute(subdivision);
    McheckErr(stat, "ERROR adding subdivision attributes");
    
    // affects
    McheckErr(attributeAffects(size_x, outMesh), "ERROR sizeX affects outMesh");
    McheckErr(attributeAffects(size_y, outMesh), "ERROR sizeY affects outMesh");
    McheckErr(attributeAffects(size_z, outMesh), "ERROR sizeZ affects outMesh");
    McheckErr(attributeAffects(size, outMesh), "ERROR size affects outMesh");
    McheckErr(attributeAffects(subdivision_x, outMesh), "ERROR subdivisionX affects outMesh");
    McheckErr(attributeAffects(subdivision_y, outMesh), "ERROR subdivisionY affects outMesh");
    McheckErr(attributeAffects(subdivision_z, outMesh), "ERROR subdivisionZ affects outMesh");
    McheckErr(attributeAffects(subdivision, outMesh), "ERROR subdivision affects outMesh");

    return MS::kSuccess;
}

template<typename T>
static inline void fill_attr(T& _attr, const T& other, int size)
{
    for (int i = 0; i < size; ++i)
    {    _attr[i] = other[i];    }
}

MStatus CubePrim::compute(const MPlug& plug, MDataBlock& data)
{

    if (plug != outMesh) return MS::kUnknownParameter;
    
    MStatus stat;

    // store attributes in class attributes
    const double3& size_val = data.inputValue(size).asDouble3;
    const int3& subdivision_val = data.inputValue(subdivision).asInt3;

    fill_attr(_size, size_val, 3);
    fill_attr(_subdivision, subdivision_val, 3);

    // output value
    MDataHandle output_h = data.outputValue(outMesh, &stat);
    McheckErr(stat, "ERROR getting output poly handle");
    MObject& mesh = output_h.asMesh();

    // data block
    if (_redo_topology || mesh.isNull())
    {
        MFnMeshData mesh_data;
        MObject mesh_parent = mesh_data.create(&stat);
        McheckErr(stat, "ERROR creating mesh output parent");

        // build complete topology data
        _build_cube_data();

        MFnMesh fnMesh;
        mesh = fnMesh.create(
            _num_vertices,
            _num_polygons,
            _vertex_array,
            _poly_counts,
            _polygon_connects,
            mesh_parent,
            &stat);

        output_h.set(mesh_parent);
    }
    else
    {
        // move vertices

    }

    data.setClean(plug);
    return MS::kSuccess;
}

void CubePrim::_build_cube_data()
{
    // num faces
    _num_polygons = (_subdivision[1] + 1) * (_subdivision[2] + 1) * 2 +
        (_subdivision[0] + 1) * (_subdivision[2] + 1) * 2 +
        (_subdivision[0] + 1) * (_subdivision[1] + 1) * 2;

    // num vertex
    _num_vertices = 8 +
        4 * _subdivision[0] + 4 * _subdivision[1] + 4 * _subdivision[2] +  // edges vertex
        _subdivision[0] * _subdivision[1] * 2 +  // xy vertex
        _subdivision[1] * _subdivision[2] * 2 + // yz vertex
        _subdivision[2] * _subdivision[0] * 2; // zx vertex

    // polygon counts
    _poly_counts.clear();
    for (unsigned int i = 0; i < _num_polygons; ++i)
    {
        _poly_counts.append(4);
    }

    _build_vertex_array();


    _build_connection_array();

}

void CubePrim::_build_vertex_array()
// vert positions
{
    _vertex_array.clear();

    int face_vertex = 4 + _subdivision[0] * 2 + _subdivision[2] * 2 + _subdivision[0] * _subdivision[2];

    double x_initial_val = - _size[0] / 2.0;
    double y_initial_val = - _size[1] / 2.0;
    double z_initial_val = - _size[2] / 2.0;

    double x_increments = _size[0] / (_subdivision[0] + 1);
    double y_increments = _size[1] / (_subdivision[1] + 1);
    double z_increments = _size[2] / (_subdivision[2] + 1);

    // bottom and top face vertex
    for (unsigned int i = 0; i < 2; ++i) 
    {
        for (unsigned int j = 0; j < face_vertex; ++j)
        {
            _vertex_array.append(
                MPoint(x_initial_val + (x_increments * (j % (_subdivision[0] + 2))),
                       y_initial_val + (_size[1] * j),
                       z_initial_val + (z_increments * (j / (_subdivision[0] + 2)))
                ));
        }
    }
    
    // middle vertex
    // top face vertex
    for (unsigned int i = 0; i < _subdivision[1]; ++i)
    {
        double y_floor_val = y_initial_val + (y_increments * (i + 1));

        for (unsigned int j = 0; j < _subdivision[0] + 2; ++j) {
            _vertex_array.append(MPoint(
                x_initial_val + (x_increments * j),
                y_floor_val,
                z_initial_val));
        }

        for (unsigned int j = 0; j < _subdivision[2] * 2; ++j)
        {
            _vertex_array.append(
                MPoint(x_initial_val + (_size[0] * (i % 2)),
                    y_floor_val,
                    z_initial_val + (z_increments * (i / 2))
                ));
        }

        for (unsigned int j = 0; j < _subdivision[0] + 2; ++j) {
            _vertex_array.append(MPoint(
                x_initial_val + (x_increments * j),
                y_floor_val,
                - z_initial_val));
        }
    }
}

void CubePrim::_build_connection_array()
{
    _polygon_connects.clear();
    // build bottom and top connections
    int xz_f_count = (_subdivision[0] + 1) * (_subdivision[2] + 1);
    for (unsigned int f = 0; f < 2; ++f)
    {
        int fvtx_offset = f * ((_subdivision[0] + 2) * (_subdivision[2] + 2));

        for (unsigned int i = 0; i < xz_f_count; ++i)
        {
            int indx0 = (i % _subdivision[0] + 1) + (i / (_subdivision[0] + 1)*(_subdivision[0] + 2)) + fvtx_offset;
            int indx1 = indx0 + 1;
            int indx2 = indx0 + _subdivision[0] + 2;
            int indx3 = indx0 + _subdivision[0] + 3;

            if (f == 0) {
                _polygon_connects.append(indx0);
                _polygon_connects.append(indx1);
                _polygon_connects.append(indx2);
                _polygon_connects.append(indx3);
            }
            else
            {
                _polygon_connects.append(indx3);
                _polygon_connects.append(indx2);
                _polygon_connects.append(indx1);
                _polygon_connects.append(indx0);
            }
        }
    }

    // xy faces

    int yx_f_count = (_subdivision[0] + 1) * (_subdivision[1] + 1);
    for (unsigned int f = 0; f < 2; ++f)
    {
        int fvtx_offset = f * ((_subdivision[0] + 2) * (_subdivision[2] + 2));

        for (unsigned int i = 0; i < xz_f_count; ++i)
        {
            int indx0 = (i % _subdivision[0] + 1) + (i / (_subdivision[0] + 1)*(_subdivision[0] + 2)) + fvtx_offset;
            int indx1 = indx0 + 1;
            int indx2 = indx0 + _subdivision[0] + 2;
            int indx3 = indx0 + _subdivision[0] + 3;

            if (f == 0) {
                _polygon_connects.append(indx0);
                _polygon_connects.append(indx1);
                _polygon_connects.append(indx2);
                _polygon_connects.append(indx3);
            }
            else
            {
                _polygon_connects.append(indx3);
                _polygon_connects.append(indx2);
                _polygon_connects.append(indx1);
                _polygon_connects.append(indx0);
            }
        }
    }

    // yz faces

}
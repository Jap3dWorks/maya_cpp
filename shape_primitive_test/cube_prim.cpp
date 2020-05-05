#include "cube_prim.h"
#include <maya/MPoint.h>
#include <algorithm>

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

    int xz_inner_vtx = _subdivision[2] * _subdivision[0];
    int xz_outhrt_vtx = 4 + _subdivision[0] * 2 + _subdivision[2] * 2;

    int face_vertex = 4 + _subdivision[0] * 2 + _subdivision[2] * 2 + _subdivision[0] * _subdivision[2];

    double x_initial_val = - _size[0] / 2.0;
    double y_initial_val = - _size[1] / 2.0;
    double z_initial_val = - _size[2] / 2.0;

    double x_increments = _size[0] / (_subdivision[0] + 1);
    double y_increments = _size[1] / (_subdivision[1] + 1);
    double z_increments = _size[2] / (_subdivision[2] + 1);

    unsigned int i, j;

    // bottom face grid points
    _fill_grid_points(_vertex_array, _subdivision[0], _subdivision[2], x_increments, z_increments, 
        (-_size[0]/2.f) + x_increments, (-_size[2]/2.f) + z_increments, -_size[1]/2.f);
    // side points
    _fill_side_points(_vertex_array, _subdivision[0], _subdivision[2], _subdivision[1], 
        x_increments, z_increments, y_increments, -(_size[0] / 2.f), -(_size[2] / 2.f), -(_size[1] / 2.f));
    // top face grid points
    _fill_grid_points(_vertex_array, _subdivision[0], _subdivision[2], x_increments, z_increments,
        (-_size[0] / 2.f) + x_increments, (-_size[2] / 2.f) + z_increments, _size[1] / 2.f);
}

// TODO:: return status
static void _fill_side_points(
    MFloatPointArray& point_array, const int& x_sub, const int& z_sub, const int& y_sub,
    const float& x_incr, const float& z_incr, const float& y_incr, const float& x_offset, 
    const float& z_offset, const float& y_offset)
{
    float x_max_value = (x_incr * (x_sub + 1)) + x_offset;
    float z_max_value = (z_incr * (z_sub + 1)) + z_offset;

    unsigned int i = 0, j = 0;
    for (j = 0; j <= y_sub+1; ++j)
    {
        float y_current_value = (y_incr * j) + y_offset;

        // x row
        for (i = 0; i <= x_sub; ++i)
        {
            point_array.append(MPoint(
                (x_incr * i) + x_offset,
                y_current_value,
                z_offset));
        }

        // z row
        for (i = 0; i <= z_sub; ++i)
        {
            point_array.append(MPoint(
                x_incr * (x_sub + 1),
                y_current_value,
                (z_incr * i) + z_offset));
        }

        // -x row
        for (i = 0; i <= x_sub; ++i)
        {
            point_array.append(MPoint(
                x_max_value - (x_incr * i),
                y_current_value,
                z_max_value
            ));
        }

        // -z row
        for (i = 0; i <= z_sub; ++i)
        {
            point_array.append(MPoint(
                x_offset,
                y_current_value,
                z_max_value - (z_incr * i)
            ));
        }
    }
}

// TODO:: return status
static void _fill_grid_points(MFloatPointArray& point_array, const int& x_sub, const int& z_sub,
    const float& x_incr, const float& z_incr, 
    const float& x_offset, const float& z_offset, const float& y_offset,
    bool** visited_points, int2 pos, int2 dir)
{
    if (visited_points[pos[0]][pos[1]])
    {
        return;
    }

    // add point
    point_array.append(MPoint(
        (x_incr * pos[0]) + x_offset,
        y_offset,
        (z_incr * pos[1]) + z_offset));

    int next_x = pos[0] + dir[0];
    int next_z = pos[1] + dir[1];

    // if next point is visited yet or it is out of margins, change direction
    if (visited_points[next_x][next_z] ||
        next_x > x_sub - 1 || next_x < 0 ||
        next_z > z_sub - 1 || next_z < 0)
        // change direction
        if (dir[0] > 0 && dir[1] == 0) {
            dir[0] = 0; dir[1] == 1;
        }
        else if (dir[0] == 0 && dir[1] > 0) {
            dir[0] = -1; dir[1] = 0;
        }
        else if (dir[0] < 0 && dir[1] == 0) {
            dir[0] = 0; dir[1] = -1;
        }
        else {
            dir[0] = 1; dir[1] = 0;
        }
    }

    // set current point as visited
    visited_points[pos[0]][pos[1]] = true;

    // update pos
    pos[0] += dir[0];
    pos[1] += dir[1];

    _fill_grid_points(point_array, x_sub, z_sub, x_incr, z_incr, 
        x_offset, z_offset, y_offset,
        visited_points, pos, dir);
}

/*Overload implementation to intialize default arguments*/
static MStatus _fill_grid_points(MFloatPointArray& point_array, const int& x_sub, const int& z_sub,
    const float& x_incr, const float& z_incr,
    const float& x_offset, const float& z_offset, const float& y_offset)
{
    int2 pos, dir;
    pos[0] = pos[1] = 0;
    dir[0] = 1; dir[1] = 0; // check first direction take in account subdivisions

    if (!(x_sub * z_sub))
    {
        return MS::kSuccess;
    }

    MStatus stat = MS::kSuccess;
    // create points int array
    bool** visited_points = new bool*[x_sub];
    for (unsigned int i = 0; i < z_sub; ++i) visited_points[i] = new bool[z_sub] {false};
    
    try
    { 
        _fill_grid_points(point_array, x_sub, z_sub, x_incr, z_incr, x_offset, z_offset, y_offset, visited_points, pos, dir);
    }
    catch (...)
    {
        std::cout << "ERROR at calculate grid points positions";
        stat = MS::kFailure;
    }

    // set free visited points pointers
    for (unsigned int i = 0; i < z_sub; ++i)
    {
        delete[] visited_points[i];
    }

    delete[] visited_points;

    return stat;
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
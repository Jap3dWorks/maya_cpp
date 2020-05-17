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

void CubePrim::_clear_buffers()
{
    if (!_buffers_initialized) return;

    unsigned int i;
    // bottom buffer
    for (const auto& buffer : { bottom_grid_id, upper_grid_id })
    {
        for (i = 0; i < _sub_z + 2; ++i) 
        {
            delete[] buffer[i];
            buffer[i] = nullptr;
        };
        delete[] buffer;
    }
    
    // front, back, left, right buffers
    for (const auto& buffer : { front_grid_id, back_grid_id,
                                left_grid_id, right_grid_id })
    {
        for (i = 0; i < _sub_y + 2; ++i) 
        {
            delete[] buffer[i];
            buffer[i] = nullptr;
        };
        delete[] buffer;
    }

    // assing nullptr to each buffer
    bottom_grid_id = nullptr;
    front_grid_id = nullptr;
    back_grid_id = nullptr;
    left_grid_id = nullptr;
    right_grid_id = nullptr;
    upper_grid_id = nullptr;

    _buffers_initialized = false;
}

void CubePrim::_build_buffers()
{
    if (_buffers_initialized) return;

    unsigned int i = 0;
    
    // bottom buffer
    bottom_grid_id = new unsigned int*[2 + _sub_z];
    for (i = 0; i < _sub_z + 2 ; ++i)
    {
        bottom_grid_id[i] = new unsigned int[_sub_x + 2];
    }
    // upper buffer
    upper_grid_id = new unsigned int*[2 + _sub_z];
    for (i = 0; i < _sub_z + 2; i++)
    {
        upper_grid_id[i] = new unsigned int[_sub_x + 2];
    }

    // --side buffers--
    // front buffer
    front_grid_id = new unsigned int*[2 + _sub_y];
    for (i = 0; i < _sub_y + 2; ++i)
    {
        front_grid_id[i] = new unsigned int[2 + _sub_x];
    }
    // back buffer
    back_grid_id = new unsigned int*[2 + _sub_y];
    for (i = 0; i < _sub_y + 2; ++i)
    {
        back_grid_id[i] = new unsigned int[2 + _sub_x];
    }
    // left buffer
    left_grid_id = new unsigned int*[2 + _sub_y];
    for (i = 0; i < _sub_y + 2; ++i)
    {
        left_grid_id[i] = new unsigned int[2 + _sub_z];
    }
    // right buffer
    right_grid_id = new unsigned int*[2 + _sub_y];
    for (i = 0; i < _sub_y + 2; ++i)
    {
        right_grid_id[i] = new unsigned int[2 + _sub_z];
    }

    _buffers_initialized = true;
}

void CubePrim::_build_cube_data()
{
    // num faces
    _num_polygons = (_sub_y + 1) * (_sub_z + 1) * 2 +
        (_sub_x + 1) * (_sub_z + 1) * 2 +
        (_sub_x + 1) * (_sub_y + 1) * 2;

    // num vertex
    _num_vertices = 8 +
        4 * _sub_x + 4 * _sub_y + 4 * _sub_z +  // edges vertex
        _sub_x * _sub_y * 2 +
        _sub_y * _sub_z * 2 +
        _sub_z * _sub_x * 2;

    // polygon counts
    _poly_counts.clear();
    for (unsigned int i = 0; i < _num_polygons; ++i)
    {
        _poly_counts.append(4);
    }

    _build_vertex_array();

    _build_connection_array();
}

// vertex array functions
// ----------------------
void CubePrim::_build_vertex_array()
// vert positions
{
    _vertex_array.clear();
    _clear_buffers();
    _build_buffers();

    int xz_inner_vtx = _sub_z * _sub_x;
    int xz_outhrt_vtx = 4 + _sub_x * 2 + _sub_z * 2;

    int face_vertex = 4 + _sub_x * 2 + _sub_z * 2 + _sub_x * _sub_z;

    double x_initial_val = - _size[0] / 2.0;
    double y_initial_val = - _size[1] / 2.0;
    double z_initial_val = - _size[2] / 2.0;

    double x_increments = _size[0] / (_sub_x + 1);
    double y_increments = _size[1] / (_sub_y + 1);
    double z_increments = _size[2] / (_sub_z + 1);

    double x_offset = (-_size[0] / 2.f) + x_increments;
    double y_offset = (-_size[1] / 2.f) + y_increments;
    double z_offset = (-_size[2] / 2.f) + z_increments;

    unsigned int i, j;

    // bottom grid points
    _fill_grid_points(_sub_x, _sub_z, 0,
        x_increments, z_increments, y_increments,
        x_offset, z_offset, -_size[1] / 2.f,
        bottom_grid_id, _vertex_array.length());

    // back grid points
    _fill_grid_points(_sub_x, 0, _sub_y,
        x_increments, z_increments, y_increments,
        x_offset, -_size[2] / 2.f, y_offset,
        back_grid_id, _vertex_array.length());

    // right grid points
    _fill_grid_points(0, _sub_z, _sub_y,
        x_increments, z_increments, y_increments,
        -_size[0] / 2.f, z_offset, y_offset,
        right_grid_id, _vertex_array.length());

    // upper grid points
    _fill_grid_points(_sub_x, _sub_z, 0,
        x_increments, z_increments, y_increments,
        x_offset, z_offset, _size[1] / 2.f,
        upper_grid_id, _vertex_array.length());

    // front grid points
    _fill_grid_points(_sub_x, 0, _sub_y,
        x_increments, z_increments, y_increments,
        x_offset, _size[2] / 2.f, y_offset,
        front_grid_id, _vertex_array.length());

    // left grid points
    _fill_grid_points(0, _sub_z, _sub_y,
        x_increments, z_increments, y_increments,
        _size[0] / 2.f, z_offset, y_offset,
        left_grid_id, _vertex_array.length());


    // fill edge points

    // side points
    _fill_edge_points(x_increments, z_increments, y_increments, 
        -(_size[0] / 2.f), -(_size[2] / 2.f), -(_size[1] / 2.f));
    
}

// TODO: build vertex positions and fill id buffers, theese index are shared
void CubePrim::_fill_edge_points(
    const float& x_incr, const float& z_incr, const float& y_incr, 
    const float& x_offset, const float& z_offset, const float& y_offset)
{
    float x_max_value = (x_incr * (_sub_x + 1)) + x_offset;
    float z_max_value = (z_incr * (_sub_z + 1)) + z_offset;

    unsigned int i = 0, j = 0;
    for (j = 0; j <= _sub_y+1; ++j)
    {
        float y_current_value = (y_incr * j) + y_offset;

        // x row
        for (i = 0; i <= _sub_x; ++i)
        {
            _vertex_array.append(MPoint(
                (x_incr * i) + x_offset,
                y_current_value,
                z_offset));
        }

        // z row
        for (i = 0; i <= _sub_z; ++i)
        {
            _vertex_array.append(MPoint(
                x_incr * (_sub_x + 1),
                y_current_value,
                (z_incr * i) + z_offset));
        }

        // -x row
        for (i = 0; i <= _sub_x; ++i)
        {
            _vertex_array.append(MPoint(
                x_max_value - (x_incr * i),
                y_current_value,
                z_max_value
            ));
        }

        // -z row
        for (i = 0; i <= _sub_z; ++i)
        {
            _vertex_array.append(MPoint(
                x_offset,
                y_current_value,
                z_max_value - (z_incr * i)
            ));
        }
    }
}

/**/
void CubePrim::_fill_grid_points(
    const float& x_sub, const float& z_sub, const float& y_sub,
    const float& x_incr, const float& z_incr, const float& y_incr,
    const float& x_offset, const float& z_offset, const float& y_offset,
    unsigned int**& id_buffer, int id_buffer_offset = 0)
{
    unsigned int i, j, k;

    if (x_sub && z_sub) {
        for (j = 0; j < z_sub; ++j)
        {
            for (i = 0; i < x_sub; ++i)
            {
                _vertex_array.append(MPoint(
                    (x_incr * i) + x_offset,
                    y_offset,
                    (z_incr * j) + z_offset));

                // fill the buffer id
                id_buffer[j + 1][i + 1] = (x_sub * j) + i + id_buffer_offset;
            }
        }
    }

    else if (z_sub && y_sub)
    {
        for (k = 0; k < y_sub; ++k)
        {
            for (j = 0; j < z_sub; ++j)
            {
                _vertex_array.append(MPoint(
                    x_offset,
                    (y_incr * k) + y_offset,
                    (z_incr * j) + z_offset));

                // fill the buffer id
                id_buffer[k + 1][j + 1] = (z_sub * k) + j + id_buffer_offset;

            }
        }
    }

    else if (x_sub && y_sub)
    {
        for (k = 0; k < y_sub; ++k)
        {
            for (i = 0; i < x_sub; ++i)
            {
                _vertex_array.append(MPoint(
                    (x_incr * i) + x_offset,
                    (y_incr * k) + y_offset,
                    z_offset));

                // fill the buffer id
                id_buffer[k + 1][i + 1] = (x_sub * k) + i + id_buffer_offset;
            }
        }
    }

}


// Build topology
// --------------
void CubePrim::_build_topological_data()
{
    _topological_data.clear();

    const int xz_grid_size = _subdivision[0] * _subdivision[2];

    const int& x_sub = _subdivision[0];
    const int& y_sub = _subdivision[1];
    const int& z_sub = _subdivision[2];

    const int vertex_floor = _subdivision[0] * 2 + _subdivision[2] * 2 + 4;
    unsigned int vertex_floor_offset;

    auto _create_face = [&vertex_floor](const int& id)->std::vector<int>
    {
        std::vector<int>face_ids;
        face_ids.push_back(id );
        face_ids.push_back(id + 1);
        face_ids.push_back(id + 1 - vertex_floor);
        face_ids.push_back(id - vertex_floor);
        
        return face_ids;
    };

    unsigned int i, j, k;
    for (i = 1; i <= y_sub + 1; ++i) // for each y subdivision
    {
        // always start at 1rst floor
        vertex_floor_offset = vertex_floor * (i) + xz_grid_size;

        // construct x direction faces
        for (j = 0; j < x_sub + 1; ++j)
        {
            _topological_data.push_back(
                _create_face(j + vertex_floor_offset));
        }

        // construct z direction faces
        for (j = 0; j < z_sub + 1; ++j)
        {
            _topological_data.push_back(
                _create_face(j + x_sub + vertex_floor_offset));
        }

        // construct -x direction faces
        for (j = 0; j < x_sub + 1; ++j)
        {
            _topological_data.push_back(
                _create_face(j + x_sub + z_sub + vertex_floor_offset));
        }

        // construct -x direction faces
        for (j = 0; j < x_sub + 1; ++j)
        {
            _topological_data.push_back(
                _create_face(j + 2*x_sub + z_sub + vertex_floor_offset));
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
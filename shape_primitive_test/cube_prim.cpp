#include "cube_prim.h"
#include <maya/MPoint.h>
#include <algorithm>
#include <cmath>
#include <math.h>
#include <maya/MGlobal.h>

#define DEBUG_LEVEL 3

#if DEBUG_LEVEL == 3
#define __DEBUG__
#endif

#ifdef __DEBUG__
#define DEBUG(message)  \
    std::cout << message << std::endl;\
    MGlobal::displayInfo(message);
#else // __DEBUG__
#define DEBUG(message)
#endif



#define McheckErr(status, msg)    \
    if (MS::kSuccess != status){  \
        cerr << msg << "\n";    \
        return status;            \
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

MTypeId CubePrim::id(0x8000b);
MString CubePrim::name("cubePrim");

MObject CubePrim::size;
MObject CubePrim::size_x;
MObject CubePrim::size_y;
MObject CubePrim::size_z;

MObject CubePrim::subdivision;
MObject CubePrim::subdivision_x;
MObject CubePrim::subdivision_y;
MObject CubePrim::subdivision_z;

MObject CubePrim::outMesh;

/*constructor set redo_topology as true to bild an initial cube*/
CubePrim::CubePrim(): _redo_topology(true) {}

CubePrim::~CubePrim() {
    DEBUG("Call destructor")
    _clear_buffers();
}

void* CubePrim::creator()
{
    return new CubePrim();
}

MStatus CubePrim::initialize()
{
    MStatus status;
    MFnTypedAttribute typedFn;
    MFnNumericAttribute uAttr;

    outMesh = typedFn.create("outMesh", "o", MFnData::kMesh, MObject::kNullObj, &status);
    McheckErr(status, "ERROR creating outMesh attribute");

    MAKE_OUTPUT(typedFn);

    // size 
    size_x = uAttr.create("sizeX", "six", MFnNumericData::kDouble, 1.0, &status);
    McheckErr(status, "ERROR creating sizeX attribute");
    size_y = uAttr.create("sizeY", "siy", MFnNumericData::kDouble, 1.0, &status);
    McheckErr(status, "ERROR creating sizeY attribute");
    size_z = uAttr.create("sizeZ", "siz", MFnNumericData::kDouble, 1.0, &status);
    McheckErr(status, "ERROR creating sizeZ attribute");
    size = uAttr.create("size", "si", size_x, size_y, size_z, &status);
    McheckErr(status, "ERROR creating size attribute");
    
    MAKE_INPUT(uAttr);
    
    // subdivision
    subdivision_x = uAttr.create("subdivisionX", "sdx", MFnNumericData::kInt, 0, &status);
    McheckErr(status, "ERROR creating subdivisionX attribute");
    McheckErr(uAttr.setMin(0), "ERROR setting min subdivisionX value");

    subdivision_y = uAttr.create("subdivisionY", "sdy", MFnNumericData::kInt, 0, &status);
    McheckErr(status, "ERROR creating subdivisionY attribute");
    McheckErr(uAttr.setMin(0), "ERROR setting min subdivisionY value");

    subdivision_z = uAttr.create("subdivisionZ", "sdz", MFnNumericData::kInt, 0, &status);
    McheckErr(status, "ERROR creating subdivisionZ attribute");
    McheckErr(uAttr.setMin(0), "ERROR setting min subdivisionZ value");

    subdivision = uAttr.create("subdivision", "sd", subdivision_x, subdivision_y, subdivision_z, &status);
    McheckErr(status, "ERROR creating subdivision attribute");
    
    MAKE_INPUT(uAttr);

    // add attributes
    status = addAttribute(outMesh);
    McheckErr(status, "ERROR adding attribute");
    status = addAttribute(size);
    McheckErr(status, "ERROR adding size attributes");
    status = addAttribute(subdivision);
    McheckErr(status, "ERROR adding subdivision attributes");
    
    // affects
    CHECK_MSTATUS(attributeAffects(size_x, outMesh));
    CHECK_MSTATUS(attributeAffects(size_y, outMesh));
    CHECK_MSTATUS(attributeAffects(size_z, outMesh));
    CHECK_MSTATUS(attributeAffects(size, outMesh));
    CHECK_MSTATUS(attributeAffects(subdivision_x, outMesh));
    CHECK_MSTATUS(attributeAffects(subdivision_y, outMesh));
    CHECK_MSTATUS(attributeAffects(subdivision_z, outMesh));
    CHECK_MSTATUS(attributeAffects(subdivision, outMesh));

    return MS::kSuccess;
}

MStatus CubePrim::compute(const MPlug& plug, MDataBlock& data)
{
    if (plug != outMesh) return MS::kUnknownParameter;
    
    MStatus status;

    // store attributes in class attributes
    const double3& i_size = data.inputValue(size).asDouble3();
    const int3& i_subdivision = data.inputValue(subdivision).asInt3();

    // fill class attributes
    _size_x = i_size[0];
    _size_y = i_size[1];
    _size_z = i_size[2];

    // if new input subdivision is different from stored subdivision,
    // we need to rebuild topology
    if ((!_redo_topology) && 
        (_sub_x != i_subdivision[0] ||
         _sub_y != i_subdivision[1] ||
         _sub_z != i_subdivision[2] ))
    {
        _redo_topology = true;
    }

    // save new subdivision values
    _sub_x = i_subdivision[0];
    _sub_y = i_subdivision[1];
    _sub_z = i_subdivision[2];

    _incr_x = _size_x / (_sub_x + 1);
    _incr_y = _size_y / (_sub_y + 1);
    _incr_z = _size_z / (_sub_z + 1);

    _offset_x = _size_x / 2.0;
    _offset_y = _size_y / 2.0;
    _offset_z = _size_z / 2.0;

    // output value
    MDataHandle output_h = data.outputValue(outMesh, &status);
    McheckErr(status, "ERROR getting output poly handle");
    MObject& mesh = output_h.asMesh();

    // data block
    if (_redo_topology || mesh.isNull())
    {
        MFnMeshData mesh_data;
        MObject mesh_parent = mesh_data.create(&status);
        McheckErr(status, "ERROR creating mesh output parent");

        // build complete topology data
        build_cube();

        MFnMesh fnMesh;
        mesh = fnMesh.create(
            m_vertex_array.length(),
            m_poly_counts.length(),
            m_vertex_array,
            m_poly_counts,
            m_polygon_connects,
            mesh_parent,
            &status);

        output_h.set(mesh_parent);
    }
    else
    {
        MFnMesh fnMesh(mesh);
        MFloatPointArray vertex_array;
        // get vertices
        fnMesh.getPoints(vertex_array);
        // move vertices
        reposition_vertices(vertex_array);

        // return adjusted vertex to mesh internal data
        fnMesh.setPoints(vertex_array);
    }

    data.setClean(plug);
    return MS::kSuccess;
}

void CubePrim::build_cube()
{
    // num faces
    unsigned int _num_polygons = (_sub_y + 1) * (_sub_z + 1) * 2 +
                                 (_sub_x + 1) * (_sub_z + 1) * 2 +
                                 (_sub_x + 1) * (_sub_y + 1) * 2;

    // polygon counts
    m_poly_counts.clear();
    for (unsigned int i = 0; i < _num_polygons; ++i)
    {
        m_poly_counts.append(4);
    }
    _build_buffers();
    _build_vertex();
    _build_topology_connection();
}

void CubePrim::_clear_buffers()
{
    unsigned int i;
    // bottom buffer and upper buffer front, back, left, right buffers
    for (const auto& buffer : { bottom_buffer_id, upper_buffer_id,
                                front_buffer_id, back_buffer_id,
                                left_buffer_id, right_buffer_id })
    {
        for (i = 0; i < _size_buffer; ++i)
        {
            delete[] buffer[i];
            buffer[i] = nullptr;
        };
        delete[] buffer;
    }
    
    // assing nullptr to each buffer
    bottom_buffer_id = nullptr;
    upper_buffer_id = nullptr;
    front_buffer_id = nullptr;
    back_buffer_id = nullptr;
    left_buffer_id = nullptr;
    right_buffer_id = nullptr;
}

/*Returns the new size buffer to contain the need size*/
static int get_new_buffer_size(unsigned int current, unsigned int need)
{
    float numerator = static_cast<float>(need - current);
    float denominator = static_cast<float>(current);

    float increments = log((numerator / denominator) + 1.f) / log(2.f);
    
    if (increments > 0)
    {
        return current * pow(2, std::ceil(increments));
    }
    else
    {
        return current;
    }
}

void CubePrim::_build_buffers()
{
    // scale buffers only if needed
    unsigned int max_needed = std::max(2 + _sub_x, std::max(2 + _sub_y, 2 + _sub_z));
    if (max_needed > _size_buffer)
    {
        _clear_buffers();

        // new buffer size
        _size_buffer = get_new_buffer_size(_size_buffer, max_needed);
        
        _initialize_buffers = true;
    }

    if (_initialize_buffers)
    {
        unsigned int i = 0;

        // bottom buffer
        bottom_buffer_id = new unsigned int*[_size_buffer];
        for (i = 0; i < _size_buffer; ++i)
        {
            bottom_buffer_id[i] = new unsigned int[_size_buffer];
        }
        // upper buffer
        upper_buffer_id = new unsigned int*[_size_buffer];
        for (i = 0; i < _size_buffer; i++)
        {
            upper_buffer_id[i] = new unsigned int[_size_buffer];
        }

        // --side buffers--
        // front buffer
        front_buffer_id = new unsigned int*[_size_buffer];
        for (i = 0; i < _size_buffer; ++i)
        {
            front_buffer_id[i] = new unsigned int[_size_buffer];
        }
        // back buffer
        back_buffer_id = new unsigned int*[_size_buffer];
        for (i = 0; i < _size_buffer; ++i)
        {
            back_buffer_id[i] = new unsigned int[_size_buffer];
        }
        // left buffer
        left_buffer_id = new unsigned int*[_size_buffer];
        for (i = 0; i < _size_buffer; ++i)
        {
            left_buffer_id[i] = new unsigned int[_size_buffer];
        }
        // right buffer
        right_buffer_id = new unsigned int*[_size_buffer];
        for (i = 0; i < _size_buffer; ++i)
        {
            right_buffer_id[i] = new unsigned int[_size_buffer];
        }

        _initialize_buffers = false;
    }
}

// vertex array functions
// ----------------------
void CubePrim::_build_vertex()
// vert positions
{
    // reset internal data
    m_vertex_array.clear();

    // bottom grid points
    _fill_grid_vertex(
        _sub_x, _sub_z, 0,
        _incr_x, _incr_z, _incr_y,
        -_offset_x, -_offset_z, -_offset_y,
        bottom_buffer_id, m_vertex_array.length());

    // back grid points
    _fill_grid_vertex(
        _sub_x, 0, _sub_y,
        _incr_x, _incr_z, _incr_y,
        - _offset_x, -_offset_z, -_offset_y,
        back_buffer_id, m_vertex_array.length());

    // right grid points
    _fill_grid_vertex(
        0, _sub_z, _sub_y,
        _incr_x, _incr_z, _incr_y,
        -_offset_x, -_offset_z, -_offset_y,
        right_buffer_id, m_vertex_array.length());

    // upper grid points
    _fill_grid_vertex(
        _sub_x, _sub_z, 0,
        _incr_x, _incr_z, _incr_y,
        -_offset_x, -_offset_z, _offset_y,
        upper_buffer_id, m_vertex_array.length());

    // front grid points
    _fill_grid_vertex(
        _sub_x, 0, _sub_y,
        _incr_x, _incr_z, _incr_y,
        -_offset_x, _offset_z, -_offset_y,
        front_buffer_id, m_vertex_array.length());

    // left grid points
    _fill_grid_vertex(
        0, _sub_z, _sub_y,
        _incr_x, _incr_z, _incr_y,
        _offset_x, -_offset_z, -_offset_y,
        left_buffer_id, m_vertex_array.length());

    // fill edge points
    _fill_edge_vertex(_incr_x, _incr_z, _incr_y,
        _offset_x, _offset_z, _offset_y);
}

/*Fills shared vertex positions array and each id buffer*/
void CubePrim::_fill_edge_vertex(
    const float& x_incr, const float& z_incr, const float& y_incr, 
    const float& x_offset, const float& z_offset, const float& y_offset)
{
    unsigned int id = m_vertex_array.length();
    // 3 faces shared points
    // -x, -y, -z -> min point
    m_vertex_array.append(MPoint(
        -x_offset, -y_offset, -z_offset));
    bottom_buffer_id[0][0] = id;
    back_buffer_id[0][0] = id;
    right_buffer_id[0][0] = id;
    // x , -y, -z 
    m_vertex_array.append(MPoint(
        x_offset, -y_offset, -z_offset));
    bottom_buffer_id[0][_sub_x + 1] = ++id;
    back_buffer_id[0][_sub_x + 1] = id;
    left_buffer_id[0][0] = id;
    // x , -y, z
    m_vertex_array.append(MPoint(
        x_offset, -y_offset, z_offset));
    bottom_buffer_id[_sub_z + 1][_sub_x + 1] = ++id;
    front_buffer_id[0][_sub_x + 1] = id;
    left_buffer_id[0][_sub_z + 1] = id;
    // -x, -y, z
    m_vertex_array.append(MPoint(
        -x_offset, -y_offset, z_offset));
    bottom_buffer_id[_sub_z + 1][0] = ++id;
    front_buffer_id[0][0] = id;
    right_buffer_id[0][_sub_z + 1] = id;
    // -x, y, -z
    m_vertex_array.append(MPoint(
        -x_offset, y_offset, -z_offset));
    upper_buffer_id[0][0] = ++id;
    back_buffer_id[_sub_y + 1][0] = id;
    right_buffer_id[_sub_y + 1][0] = id;
    // x, y, -z
    m_vertex_array.append(MPoint(
        x_offset, y_offset, -z_offset));
    upper_buffer_id[0][_sub_x + 1] = ++id;
    back_buffer_id[_sub_y + 1][_sub_x + 1] = id;
    left_buffer_id[_sub_y + 1][0] = id;
    //x, y, z -> max point
    m_vertex_array.append(MPoint(
        x_offset, y_offset, z_offset));
    upper_buffer_id[_sub_z + 1][_sub_x + 1] = ++id;
    front_buffer_id[_sub_y + 1][_sub_x + 1] = id;
    left_buffer_id[_sub_y + 1][_sub_z + 1] = id;
    // -x , y, z
    m_vertex_array.append(MPoint(
        -x_offset, y_offset, z_offset));
    upper_buffer_id[_sub_z + 1][0] = ++id;
    front_buffer_id[_sub_y + 1][0] = id;
    right_buffer_id[_sub_y + 1][_sub_z + 1] = id;

    // bottom back
    float x_max_value = (x_incr * (_sub_x + 1)) + x_offset;
    float z_max_value = (z_incr * (_sub_z + 1)) + z_offset;
    unsigned int i = 0, j = 0, k=0;
    // 1 bottom back
    for (i = 0; i < _sub_x; ++i)
    {
        m_vertex_array.append(MPoint(
            x_incr + (x_incr * i) - x_offset,
            -y_offset,
            -z_offset
        ));
        bottom_buffer_id[0][i + 1] = ++id;
        back_buffer_id[0][i + 1] = id;
    }
    // 2 bottom left
    for (j = 0; j < _sub_z; ++j)
    {
        m_vertex_array.append(MPoint(
            x_offset,
            -y_offset,
            z_incr + (z_incr * j) - z_offset
        ));
        bottom_buffer_id[j + 1][_sub_x + 1] = ++id;
        left_buffer_id[0][j + 1] = id;
    }
    // 3 bottom front
    for (i = 0; i < _sub_x; ++i)
    {
        m_vertex_array.append(MPoint(
            x_incr + (x_incr * i) - x_offset,
            -y_offset,
            z_offset
        ));
        bottom_buffer_id[_sub_z + 1][i + 1] = ++id;
        front_buffer_id[0][i + 1] = id;
    }
    // 4 bottom right
    for (j = 0; j < _sub_z; ++j)
    {
        m_vertex_array.append(MPoint(
            - x_offset,
            -y_offset,
            z_incr + (z_incr * j) - z_offset
        ));
        bottom_buffer_id[j + 1][0] = ++id;
        right_buffer_id[0][j + 1] = id;
    }
    // upper edges
    // 5 upper back
    for (i = 0; i < _sub_x; ++i)
    {
        m_vertex_array.append(MPoint(
            x_incr + (x_incr * i) - x_offset,
            y_offset,
            -z_offset
        ));
        upper_buffer_id[0][i + 1] = ++id;
        back_buffer_id[_sub_y + 1][i + 1] = id;
    }
    // 6 upper left
    for (j = 0; j < _sub_z; ++j)
    {
        m_vertex_array.append(MPoint(
            x_offset,
            y_offset,
            z_incr + (z_incr * j) - z_offset
        ));
        upper_buffer_id[j + 1][_sub_x + 1] = ++id;
        left_buffer_id[_sub_y + 1][j + 1] = id;
    }
    // 7 upper front
    for (i = 0; i < _sub_x; ++i)
    {
        m_vertex_array.append(MPoint(
            x_incr + (x_incr * i) - x_offset,
            y_offset,
            z_offset
        ));
        upper_buffer_id[_sub_z + 1][i + 1] = ++id;
        front_buffer_id[_sub_y + 1][i + 1] = id;
    }
    // 8 upper right
    for (j = 0; j < _sub_z; ++j)
    {
        m_vertex_array.append(MPoint(
            -x_offset,
            y_offset,
            z_incr + (z_incr * j) - z_offset
        ));
        upper_buffer_id[j + 1][0] = ++id;
        right_buffer_id[_sub_y + 1][j + 1] = id;
    }

    // 9 right back
    for (k = 0; k < _sub_y; ++k)
    {
        m_vertex_array.append(MPoint(
            -x_offset,
            y_incr + (y_incr * k) - y_offset,
            - z_offset
        ));
        right_buffer_id[k + 1][0] = ++id;
        back_buffer_id[k + 1][0] = id;
    }
    // 10 back left
    for (k = 0; k < _sub_y; ++k)
    {
        m_vertex_array.append(MPoint(
            x_offset,
            y_incr + (y_incr * k) - y_offset,
            -z_offset
        ));
        left_buffer_id[k + 1][0] = ++id;
        back_buffer_id[k + 1][_sub_x + 1] = id;
    }
    // 11 left front
    for (k = 0; k < _sub_y; ++k)
    {
        m_vertex_array.append(MPoint(
            x_offset,
            y_incr + (y_incr * k) - y_offset,
            z_offset
        ));
        left_buffer_id[k + 1][_sub_z + 1] = ++id;
        front_buffer_id[k + 1][_sub_x + 1] = id;
    }
    // 12 front right
    for (k = 0; k < _sub_y; ++k)
    {
        m_vertex_array.append(MPoint(
            -x_offset,
            y_incr + (y_incr * k) - y_offset,
            z_offset
        ));
        right_buffer_id[k + 1][_sub_z + 1] = ++id;
        front_buffer_id[k + 1][0] = id;
    }
}

/*Fill the unique vertex positions array and the input id buffers*/
void CubePrim::_fill_grid_vertex(
    const float& x_sub, const float& z_sub, const float& y_sub,
    const float& x_incr, const float& z_incr, const float& y_incr,
    const float& x_offset, const float& z_offset, const float& y_offset,
    unsigned int**& id_buffer, int id_buffer_offset)
{
    unsigned int i, j, k;

    if (x_sub && z_sub) {
        for (j = 1; j <= z_sub; ++j)
        {
            for (i = 1; i <= x_sub; ++i)
            {
                m_vertex_array.append(MPoint(
                    (x_incr * i) + x_offset,
                    y_offset,
                    (z_incr * j) + z_offset));

                // fill the buffer id
                id_buffer[j][i] = (x_sub * (j-1)) + i - 1 + id_buffer_offset;
            }
        }
    }

    else if (z_sub && y_sub)
    {
        for (k = 1; k <= y_sub; ++k)
        {
            for (j = 1; j <= z_sub; ++j)
            {
                m_vertex_array.append(MPoint(
                    x_offset,
                    (y_incr * k) + y_offset,
                    (z_incr * j) + z_offset));

                // fill the buffer id
                id_buffer[k][j] = (z_sub * (k - 1)) + j - 1 + id_buffer_offset;
            }
        }
    }

    else if (x_sub && y_sub)
    {
        for (k = 1; k <= y_sub; ++k)
        {
            for (i = 1; i <= x_sub; ++i)
            {
                m_vertex_array.append(MPoint(
                    (x_incr * i) + x_offset,
                    (y_incr * k) + y_offset,
                    z_offset));

                // fill the buffer id
                id_buffer[k][i] = (x_sub * (k - 1)) + i - 1 + id_buffer_offset;
            }
        }
    }

}

// build topology connection buffer
// --------------------------------
/*First run _build_vertex method to fill all the buffers id*/
void CubePrim::_build_topology_connection()
{
    if (_initialize_buffers) return;
    
    // clear previous connection data
    m_polygon_connects.clear();

    unsigned int i, j, k;

    auto add_face = [this](unsigned int length, unsigned int height, unsigned int**& buffer)
    {
        m_polygon_connects.append(buffer[height][length]);
        m_polygon_connects.append(buffer[height][length+1]);
        m_polygon_connects.append(buffer[height+1][length+1]);
        m_polygon_connects.append(buffer[height+1][length]);

    };

    // bottom face topology
    for (j = 0; j < _sub_z + 1; ++j)
    {
        for (i = 0; i < _sub_x + 1; ++i)
        {
            add_face(i, j, bottom_buffer_id);
        }
    }
    // front face topology
    for (k = 0; k < _sub_y + 1; ++k)
    {
        for (i = 0; i < _sub_x + 1; ++i)
        {
            add_face(i, k, front_buffer_id);
        }
    }
    // right face topology
    for (k = 0; k < _sub_y + 1; ++k)
    {
        for (j = 0; j < _sub_z + 1; ++j)
        {
            add_face(j, k, right_buffer_id);
        }
    }

    // reverse add face
    auto add_face_reverse = [this](unsigned int length, unsigned int height, unsigned int**& buffer)
    {
        m_polygon_connects.append(buffer[height][length]);
        m_polygon_connects.append(buffer[height + 1][length]);
        m_polygon_connects.append(buffer[height + 1][length + 1]);
        m_polygon_connects.append(buffer[height][length + 1]);
    };
    // upper face topology
    for (j = 0; j < _sub_z + 1; ++j)
    {
        for (i = 0; i < _sub_x + 1; ++i)
        {
            add_face_reverse(i, j, upper_buffer_id);
        }
    }
    // back face topology
    for (k = 0; k < _sub_y + 1; ++k)
    {
        for (i = 0; i < _sub_x + 1; ++i)
        {
            add_face_reverse(i, k, back_buffer_id);
        }
    }
    // left face topology
    for (k = 0; k < _sub_y + 1; ++k)
    {
        for (j = 0; j < _sub_z + 1; ++j)
        {
            add_face_reverse(j, k, left_buffer_id);
        }
    }
}

/*Move verte to it's new position, but not recalculate the topology*/
void CubePrim::reposition_vertices(MFloatPointArray& vertex_array)
{
     const double offset_y = _size_y / 2;
     const double offset_z = _size_z / 2;
     const double offset_x = _size_x / 2;

    // back buffer indices
    unsigned int i, j, k; // i = x, j = z, k = y
    for (k = 0; k < _sub_y + 2; ++k)
    {
        for (i = 0; i < _sub_x + 1; ++i)
        {
            unsigned int vertex = back_buffer_id[k][i];
            vertex_array[vertex].x = (_incr_x * i) - offset_x;
            vertex_array[vertex].y = (_incr_y * k) - offset_y;
            vertex_array[vertex].z = -offset_z;
        }
    }

    // left buffer indices
    for (k = 0; k < _sub_y + 2; ++k)
    {
        for (j = 0; j < _sub_z + 1; ++j)
        {
            unsigned int vertex = left_buffer_id[k][j];
            vertex_array[vertex].x =  offset_x;
            vertex_array[vertex].y = (_incr_y * k) - offset_y;
            vertex_array[vertex].z = (_incr_z * j) - offset_z;
        }
    }

    // front buffer indices
    for (k = 0; k < _sub_y + 2; ++k)
    {
        for (i = 1; i <= _sub_x + 1; ++i)
        {
            unsigned int vertex = front_buffer_id[k][i];
            vertex_array[vertex].x = (_incr_x * i) - offset_x;
            vertex_array[vertex].y = (_incr_y * k) - offset_y;
            vertex_array[vertex].z = offset_z;
        }
    }

    // right buffer indices
    for (k = 0; k < _sub_y + 2; ++k)
    {
        for (j = 1; j <= _sub_z + 1; ++j)
        {
            unsigned int vertex = right_buffer_id[k][j];
            vertex_array[vertex].x = - offset_x;
            vertex_array[vertex].y = (_incr_y * k) - offset_y;
            vertex_array[vertex].z = (_incr_z * j) - offset_z;
        }
    }

    // bottom buffer
    for (j = 1; j <= _sub_z; ++j)
    {
        for (i = 1; i <= _sub_x; ++i)
        {
            unsigned int vertex = bottom_buffer_id[j][i];
            vertex_array[vertex].x = (_incr_x * i) - offset_x;
            vertex_array[vertex].y = - offset_y;
            vertex_array[vertex].z = (_incr_z * j) - offset_z;
        }
    }

    // bottom buffer
    for (j = 1; j <= _sub_z; ++j)
    {
        for (i = 1; i <= _sub_x; ++i)
        {
            unsigned int vertex = upper_buffer_id[j][i];
            vertex_array[vertex].x = (_incr_x * i) - offset_x;
            vertex_array[vertex].y = offset_y;
            vertex_array[vertex].z = (_incr_z * j) - offset_z;
        }
    }

}

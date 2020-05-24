// http://blog.andreaskahler.com/2009/06/creating-icosphere-mesh-in-code.html
// https://help.autodesk.com/view/MAYAUL/2018/ENU/?guid=__cpp_ref_class_m_fn_mesh_html
// https://help.autodesk.com/view/MAYAUL/2018/ENU/?guid=__cpp_ref_class_m_fn_mesh_data_html

// https://help.autodesk.com/view/MAYAUL/2018/ENU/?guid=__cpp_ref_shell_node_2shell_node_8cpp_example_html
// https://community.khronos.org/t/how-create-cube-with-subdivisions/71603/3

// https://rosettacode.org/wiki/Catmull%E2%80%93Clark_subdivision_surface
// http://paulbourke.net/geometry/polygonise/

#ifndef CUBE_PRIM_H
#define CUBE_PRIM_H

#include <maya/MObject.h>
#include <maya/MPxNode.h>
#include <maya/MStatus.h>
#include <maya/MStatus.h>

#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>

#include <maya/MPlug.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnNumericAttribute.h>

#include <maya/MIntArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MFloatPointArray.h>

#include <maya/MIOStream.h>
#include <math.h>
#include <maya/MTypeId.h>

#include <vector>

#define MIN_BUFFER_SIZE 64;

class CubePrim : public MPxNode
{

public:
    CubePrim();
    virtual ~CubePrim() override;

    static void* creator();
    static MStatus initialize();
    MStatus compute(const MPlug& plug, MDataBlock& data) override;

public:
    // attributes
    static MTypeId id;
    static MString name;

    static MObject size;
    static MObject size_x;
    static MObject size_y;
    static MObject size_z;

    static MObject subdivision;
    static MObject subdivision_x;
    static MObject subdivision_y;
    static MObject subdivision_z;

    static MObject outMesh;

    static MString uvSetName;

private:
    // attributes
    // copy of size and subdivision input attributes
    double _size_x;
    double _size_y;
    double _size_z;

    unsigned int _sub_x;
    unsigned int _sub_y;
    unsigned int _sub_z;

    // distance between two vertex in the same axis
    double _incr_x;
    double _incr_y;
    double _incr_z;

    // max distance of each axis from center
    double _offset_x;
    double _offset_y;
    double _offset_z;

    // mesh construction attributes
    MFloatPointArray m_vertex_array; // vertex position array
    MIntArray m_poly_counts; // poly vertex array, also used for uvs counts
    MIntArray m_polygon_connects;

    // id face buffers
    unsigned int _size_buffer = MIN_BUFFER_SIZE;
    unsigned int** bottom_buffer_id = nullptr;
    unsigned int** upper_buffer_id = nullptr;
    unsigned int** front_buffer_id = nullptr;
    unsigned int** back_buffer_id = nullptr;
    unsigned int** left_buffer_id = nullptr;
    unsigned int** right_buffer_id = nullptr;

    bool _initialize_buffers = true;

    // chanfer grids

    // uvs
    MIntArray m_uvs_ids;


    // methods
    void build_cube();
    void _build_vertex();
    void _build_topology_connection();

    void _fill_edge_vertex(const float& x_incr, const float& z_incr,
        const float& y_incr, const float& x_offset,
        const float& z_offset, const float& y_offset);

    void _fill_grid_vertex(
        const float& x_sub, const float& z_sub, const float& y_sub,
        const float& x_incr, const float& z_incr, const float& y_incr,
        const float& x_offset, const float& z_offset, const float& y_offset,
        unsigned int**& id_buffer, int id_buffer_offset = 0);

    void _build_uv_coords(MFnMesh& fn_mesh);

    // buffer operations
    void _clear_buffers();
    void _build_buffers();

    // TODO check if redo topology, only if subdvision changes
    bool _redo_topology = true;

    void reposition_vertices(MFloatPointArray& vertex_array);
};

// ---template functions---

struct _AddUvPoly
{
    template<typename T, typename U, typename V>
    inline void operator()(MIntArray& uvIds, T&& row_length, U&& l_pos, V&& h_pos)
    {
        uvIds.append(l_pos);
        uvIds.append(l_pos + 1);
        uvIds.append((row_length * (h_pos + 1)) + l_pos + 1);
        uvIds.append((row_length * (h_pos + 1)) + l_pos);
    }
};

struct _AddUvPolyReversed
{
    template<typename T, typename U, typename V>
    inline void operator()(MIntArray& uvIds, T&& row_length, U&& l_pos, V&& h_pos)
    {
        uvIds.append(l_pos);
        uvIds.append(l_pos + 1);
        uvIds.append((row_length * (h_pos + 1)) + l_pos + 1);
        uvIds.append((row_length * (h_pos + 1)) + l_pos);
    }
};

template<typename F>
inline int add_uv_face(MIntArray& uvIds,
    const unsigned int& sub_length,
    const unsigned int& sub_height,
    const unsigned int& offset,
    F&& function)
{
    unsigned int i, j;
    for (j = 0; j < sub_height + 1; ++j)
    {
        for (i = 0; i < sub_length + 1; ++i)
        {
            function(
                uvIds,
                sub_length + 2,
                i + offset,
                j);
        }
    }
    return (sub_height + 2) * (sub_length + 2);
}

template <typename T>
inline void add_uv_value(MFloatArray& uList, MFloatArray& vList,
    T&& u_subdivision, T&& v_subdivision)
{
    float u_increment = 1.f / (float)(u_subdivision + 1);
    float v_increment = 1.f / (float)(v_subdivision + 1);
    unsigned int i, j;
    for (j = 0; j < v_subdivision + 2; ++j)
    {
        for (i = 0; i < u_subdivision + 2; ++i)
        {
            uList.append(u_increment * i);
            vList.append(v_increment * j);
        }
    }
}


#endif // !CUBE_PRIM_H


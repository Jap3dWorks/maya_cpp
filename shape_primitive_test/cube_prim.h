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

class CubePrim : public MPxNode
{

public:
    CubePrim() {}
    virtual ~CubePrim() override {}

    static void* creator();
    static MStatus initialize();
    MStatus compute(const MPlug& plug, MDataBlock& data) override;

public:
    // attributes
    static MTypeId id;

    static MObject size;
    static MObject size_x;
    static MObject size_y;
    static MObject size_z;

    static MObject subdivision;
    static MObject subdivision_x;
    static MObject subdivision_y;
    static MObject subdivision_z;

    static MObject outMesh;

private:
    // attributes
    // copy of size and subdivision input attributes
    double3 _size;
    int3 _subdivision;
    int _sub_x;
    int _sub_y;
    int _sub_z;

    // mesh construction attributes
    int _num_vertices;
    int _num_polygons;
    MFloatPointArray _vertex_array;
    MIntArray _poly_counts;
    MIntArray _polygon_connects;

    // topological data buffers
    std::vector<std::vector<int>> _topological_data;

    unsigned int** bottom_grid_id;
    unsigned int** front_grid_id;
    unsigned int** back_grid_id;
    unsigned int** left_grid_id;
    unsigned int** right_grid_id;
    unsigned int** upper_grid_id;

    // chanfer grids


    // methods
    void _build_cube_data();
    void _build_vertex_array();
    void _build_topological_data();
    void _build_connection_array();

    void _fill_edge_points(const float& x_incr, const float& z_incr,
        const float& y_incr, const float& x_offset,
        const float& z_offset, const float& y_offset);

    void _fill_grid_points(
        const float& x_sub, const float& z_sub, const float& y_sub,
        const float& x_incr, const float& z_incr, const float& y_incr,
        const float& x_offset, const float& z_offset, const float& y_offset,
        unsigned int**& id_buffer, int id_buffer_offset = 0);

    // buffer operations
    void _clear_buffers();
    void _build_buffers();
    bool _buffers_initialized = false;


    // TODO check if redo topology, only if subdvision changes
    bool _redo_topology = true;
};


#endif // !CUBE_PRIM_H


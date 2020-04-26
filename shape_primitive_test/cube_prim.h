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

    // mesh construction attributes
    int _num_vertices;
    int _num_polygons;
    MFloatPointArray _vertex_array;
    MIntArray _poly_counts;
    MIntArray _polygon_connects;


    // topology attributes


    // methods
    void _build_cube_data();
    void _build_vertex_array();
    void _build_connection_array();

    // TODO check if redo topology, only if subdvision changes
    bool _redo_topology = true;
};


#endif // !CUBE_PRIM_H


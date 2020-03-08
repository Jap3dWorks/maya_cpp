#include "setUpTransCircle.h"
#include "transCircleNode.h"

#include <maya\MGlobal.h>
#include <maya\MArgDatabase.h>
#include <maya\MFnDagNode.h>
#include <maya\MFnCamera.h>
#include <maya\MFnLight.h>
#include <maya\MColor.h>
#include <maya\MDagPath.h>
#include <maya\MFnNurbsSurface.h>
#include <maya\MFnMesh.h>
#include <maya\MSelectionList.h>
#include <maya\MDagModifier.h>


MString SetUpTransCircle::name("setUpTransCircle");


static MStatus create_sphere(std::string name, float radius = 1.f);


MStatus SetUpTransCircle::doIt(const MArgList& arglist)
{
    MStatus stat = MS::kSuccess;

    MString tranCircleType("transCircle");
    MObject transCircleObj = dg_mod.createNode(tranCircleType, &stat);
    
    dg_mod.renameNode(transCircleObj, MString("circleNode1"));

    //MString mel_cmd("sphere -n \" sphere1\" -r 1; sphere -n \"sphere2\" -r 2;");
    //dg_mod.commandToExecute(mel_cmd);
    // create nurbsSphere with pure api cpp
    MObject testNode = dg_mod.createNode(MString("makeNurbSphere"), &stat);
    MFnDependencyNode fn_nurbsSphere(testNode);
    MPlug radius_plug = fn_nurbsSphere.findPlug("radius");
    radius_plug.setFloat(1.f);

    // force to execute actions
    dg_mod.doIt();

    stat = create_sphere("sphere1");
    stat = create_sphere("sphere2");

    MDagModifier dag_mod;
    MObject nurbsTransform = dag_mod.createNode("nurbsSurface");
    dag_mod.doIt();
    MPlug output_plug = fn_nurbsSphere.findPlug("outputSurface");

    MFnDagNode fn_transform(nurbsTransform);
    MFnDagNode fn_nurbs(fn_transform.child(0));
    MPlug createPlug = fn_nurbs.findPlug("create");

    dag_mod.connect(output_plug, createPlug);
    dag_mod.doIt();

    MSelectionList selList;
    //MGlobal::getSelectionListByName();
    MGlobal::getSelectionListByName(MString("sphere1"), selList);
    MObject sphereDep;
    selList.getDependNode(0, sphereDep);
    selList.clear();

    MGlobal::getSelectionListByName(MString("sphere2"), selList);
    MObject sphereTwoDep;
    selList.getDependNode(0, sphereDep);
    selList.clear();

    MFnDependencyNode fn_sphere(sphereDep, &stat);
    MFnDependencyNode fn_sphereTwo(sphereTwoDep, &stat);
    MFnDependencyNode fn_transCircleNode(transCircleObj, &stat);

    MObject inputTransAttr = fn_transCircleNode.attribute(MString("inputTranslate"), &stat);
    MObject sphereTwoTranslateAttr = fn_sphereTwo.attribute(MString("translate"), &stat);
    dg_mod.connect(sphereTwoDep, sphereTwoTranslateAttr, transCircleObj, inputTransAttr);

    // Connect outputTranslate
    MObject outputTransAttr = fn_transCircleNode.attribute(MString("outputTranslate"), &stat);
    MObject sphereOneTranslateAttr = fn_sphere.attribute(MString("translate"), &stat);
    dg_mod.connect(transCircleObj, outputTransAttr, sphereDep, sphereOneTranslateAttr);

    // connect time1
    MObject time_node;
    MGlobal::getSelectionListByName(MString("time1"), selList);
    selList.getDependNode(0, time_node);
    selList.clear();

    MFnDependencyNode fn_time_node(time_node);
    MObject timeAttr = fn_time_node.attribute(MString("outTime"), &stat);
    MObject inputAttr = fn_transCircleNode.attribute(MString("input"), &stat);
    dg_mod.connect(time_node, timeAttr, transCircleObj, inputAttr);

    stat = dg_mod.doIt();
    return stat;
}

MStatus create_sphere(std::string name, float radius)
{
    MStatus stat;
    
    MDGModifier dgmod;
    MObject testNode = dgmod.createNode(MString("makeNurbSphere"), &stat);
    dgmod.renameNode(testNode, MString((name + "Sphere").c_str()));
    stat = dgmod.doIt();
    
    MFnDependencyNode fn_nurbsSphere(testNode);
    MPlug radius_plug = fn_nurbsSphere.findPlug("radius");
    radius_plug.setFloat(radius);

    MDagModifier dag_mod;
    MObject nurbsTransform = dag_mod.createNode("nurbsSurface");
    stat = dag_mod.doIt();

    MPlug output_plug = fn_nurbsSphere.findPlug("outputSurface");

    MFnDagNode fn_transform(nurbsTransform);
    MFnDagNode fn_nurbs(fn_transform.child(0));
    MPlug createPlug = fn_nurbs.findPlug("create");

    dag_mod.connect(output_plug, createPlug);
    dag_mod.renameNode(nurbsTransform, name.c_str());
    dag_mod.renameNode(fn_transform.child(0), (name + "Shape").c_str());

    stat = dag_mod.doIt();

    return MS::kSuccess;
}

MStatus SetUpTransCircle::undoIt()
{
    return dg_mod.undoIt();
}

MStatus SetUpTransCircle::redoIt()
{
    return dg_mod.doIt();
}
#ifndef NODES_H
#define NODES_H

#include <maya\MGlobal.h>
#include <maya\MPxNode.h>
#include <maya\MPxTransformationMatrix.h>
#include <maya\MPxManipContainer.h>

#include <maya\MFnUnitAttribute.h>
#include <maya\MFnNumericAttribute.h>
#include <maya\MFnMatrixAttribute.h>

#include <maya\MFloatMatrix.h>
#include <maya\MAngle.h>
#include <maya\MPxTransform.h>

#include <cmath>  // c++ math lib


/*    DG    */
static const int NODE_TRIG_SIN_ID = 0x0012a23f;
static const char* NODE_TRIG_SIN_NAME = "trig_sin";

static const int NODE_TRIG_COS_ID = 0x0012a23e;
static const char* NODE_TRIG_COS_NAME = "trig_cos";


/*    DAG    */
static const int MATRIX_STATICHR_ID = 0x0012a23d;
static const int MATRIX_AIM_ID = 0x0012a23c;

static const int NODE_STATICHR_ID = 0X0012a23b;
static const char* NODE_STATICHR_NAME = "static_hrc";

static const int NODE_AIM_ID = 0x0012a23a;
static const char* NODE_AIM_NAME = "aim_transform";


/*    DGNODES    */
/*      sin      */
class SinNode : public MPxNode 
{
public:
    SinNode();
    virtual ~SinNode();

    virtual MStatus compute(const MPlug& plug, MDataBlock& datablock) override;

    static void* creator();  // this
    static MStatus initialize();

    static MTypeId id;
    static MString name;

    static MObject operand_smob;
    static MObject result_smob;
};


class CosNode : public MPxNode
{
public:
    CosNode();
    virtual ~CosNode();

    virtual MStatus compute(const MPlug& mplug, MDataBlock& datablock) override;

    static void* creator();
    static MStatus initialize();

    static MTypeId id;
    static MString name;

    static MObject operand_smob;
    static MObject result_smob;
};


/*     DAG_NODES     */
/*  STATIC           */
/*   static matrix   */
class StaticMatrix : public MPxTransformationMatrix
{
public:
    StaticMatrix();

    static void* creator();

    virtual MMatrix asMatrix() const override;
    virtual MMatrix asMatrix(double) const override;

    static MTypeId id;
    static MString name;

protected:
    typedef MPxTransformationMatrix ParentClass;  // this do not has functionality
};


/*   static node   */
class StaticHrc : public MPxTransform
{
public:
    static MTypeId id;
    static MString name;

    StaticHrc();
    StaticHrc(MPxTransformationMatrix*);
    virtual ~StaticHrc();

    static void* creator();
    static MStatus initialize();
    void postConstructor();

    MPxTransformationMatrix* createTransformationMatrix();

    virtual MStatus compute(const MPlug& mplug, MDataBlock& datablock) override;

protected:
    typedef MPxTransform ParentClass;
};


/*  AIM           */
/*   aim_matrix   */
class AimMatrix : public MPxTransformationMatrix
{
public:
    AimMatrix();
    virtual MMatrix asMatrix() const override;
    virtual MMatrix asMatrix(double percent) const override;

    static void* creator();
    
    MFloatMatrix inverse_parent_space;
    MFloatVector position;
    MFloatVector aim;
    MFloatVector up;

    static MTypeId id;

protected:
    typedef MPxTransformationMatrix ParentClass;
private:
    MMatrix matrix_from_internals() const;
};


/*     aim node     */
class AimTransform : public MPxTransform
{
public:
    // factory
    static MTypeId id;
    static MString name;

    // additional
    static MObject inverse_parent_space_smob;
    static MObject driver_position_smob;
    static MObject driver_at_smob;
    static MObject driver_up_smob;

    // methods
public:
    // factory
    AimTransform();
    AimTransform(MPxTransformationMatrix*);

    virtual ~AimTransform();
    static void* creator();
    static MStatus initialize();

    MStatus validateAndSetValue(const MPlug&, const MDataHandle&);

    MPxTransformationMatrix* createTransformationMatrix();

    virtual MStatus compute(const MPlug&, MDataBlock&);

protected:
    typedef MPxTransform ParentClass;
};


class TrigManip : public MPxManipulatorNode
{
public:
    static MTypeId id;
    static MString name;

    TrigManip();
    virtual ~TrigManip();

    virtual void postConstructor();

    virtual void draw(M3dView& view, const MDagPath& path,
        M3dView::DisplayStyle disp_style, M3dView::DisplayStatus disp_status);

    void preDrawUI(const M3dView& view);
    void drawUI(const M3dView& view);

    virtual MStatus doPress(M3dView& view);
    virtual MStatus doDrag(M3dView& view);
    virtual MStatus doRelease(M3dView& view);

    virtual MStatus connectToDependNode(const MObject &dependNode);

private:
    MPoint centre_point;
    MPoint end_point;

    MVector manip_plane_normal;
};

#endif // !NODES_H

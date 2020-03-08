#include "nodes.h"

MTypeId TrigManip::id(0x81047);
MString TrigManip::name("trigManipulator");

TrigManip::TrigManip()
{
    centre_point = MPoint(0.0, 0.0, 0.0, 1.0);
    end_point = MPoint(0.0, 0.0, 0.0, 1.0);

    manip_plane_normal = MVector(0.0, 0.0, -1.0);
}

TrigManip::~TrigManip(){}

void TrigManip::postConstructor(){}

void TrigManip::draw(M3dView& view, const MDagPath& path,
    M3dView::DisplayStyle disp_style, M3dView::DisplayStatus disp_status)
{
    static MGLFunctionTable* gGLFT = 0;
}

void TrigManip::preDrawUI(const M3dView& view) {}

void TrigManip::drawUI(const M3dView& view)
{
}

MStatus TrigManip::doPress(M3dView& view)
{
    return MS::kUnknownParameter;
}

MStatus TrigManip::doDrag(M3dView& view)
{
    return MS::kUnknownParameter;
}

MStatus TrigManip::doRelease(M3dView& view)
{
    return MS::kUnknownParameter;
}

MStatus TrigManip::connectToDependNode(const MObject& dependNode)
{
    return MStatus();
}


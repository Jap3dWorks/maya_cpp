#include "arrowLocatorManipNode.h"

#include <maya\MPlug.h>
#include <maya\MDataBlock.h>
#include <maya\MDataHandle.h>
#include <maya\MFnDependencyNode.h>
#include <maya\MFnDiscManip.h>
#include <maya\MAngle.h>

#include <maya\MGlobal.h>

MTypeId ArrowLocatorManip::id(0x00003);
MString ArrowLocatorManip::name("arrowLocatorManip");
MDagPath ArrowLocatorManip::fDiscManip;

MDagPath ArrowLocatorManip::fNodePath;

ArrowLocatorManip::ArrowLocatorManip() {}
ArrowLocatorManip::~ArrowLocatorManip() {}

void* ArrowLocatorManip::creator()
{
    return new ArrowLocatorManip();
}


MStatus ArrowLocatorManip::initialize()
{
    MStatus status;
    status = MPxManipContainer::initialize();
    return status;
}

MStatus ArrowLocatorManip::createChildren()
{
    MStatus status;

    MString manipName("angleManip");
    MString angleName("yRotation");
    ArrowLocatorManip::fDiscManip = addDiscManip(manipName, angleName);

    // initialize angle and starting point
    MPoint startPoint(0.0, 0.0, 0.0);
    MAngle startAngle(0.0, MAngle::kDegrees);
    MFnDiscManip fnDisc(ArrowLocatorManip::fDiscManip, &status);
    fnDisc.setCenterPoint(startPoint);
    fnDisc.setAngle(startAngle);

    return status;
}

MStatus ArrowLocatorManip::connectToDependNode(const MObject& dependNode)
{
    MStatus status = MS::kSuccess;

    MFnDependencyNode  fnDepNode(dependNode);
    MPlug rotationPlug = fnDepNode.findPlug(MString("windDirection"), &status);

    // connect wind dir manip with the node
    MFnDiscManip fnDisc(ArrowLocatorManip::fDiscManip, &status);
    status = fnDisc.connectToAnglePlug(rotationPlug);

    MFnDagNode fnDagNode(dependNode);
    fnDagNode.getPath(fNodePath);
    unsigned int centerPointIndex = fnDisc.centerIndex(&status);
    addPlugToManipConversionCallback(
        centerPointIndex,
        // convert from pointer ->&
        (plugToManipConversionCallback)&ArrowLocatorManip::centerPointCallback);
    
    // mandatory functions
    status = finishAddingManips();
    status = MPxManipContainer::connectToDependNode(dependNode);

    return status;
}

MManipData ArrowLocatorManip::centerPointCallback(unsigned int manipIndex)
{
    MStatus status;

    MObject parentTransform = ArrowLocatorManip::fNodePath.transform(&status);
    
    MDagPath transformPath;
    MDagPath::getAPathTo(parentTransform, transformPath); //mobject->dagPath

    // retrieve world space translation
    MFnTransform fnTrans(transformPath, &status);
    MVector translation = fnTrans.getTranslation(MSpace::kWorld, &status);

    MFnNumericData numData;
    MObject numDataValue = numData.create(MFnNumericData::k3Double, &status);
    status = numData.setData3Double(translation.x,
                                    translation.y,
                                    translation.z);
    
    MManipData manipData(numDataValue);
    return manipData;
}

void ArrowLocatorManip::draw(M3dView &view,
    const MDagPath& path,
    M3dView::DisplayStyle style,
    M3dView::DisplayStatus status)
{
    MPxManipContainer::draw(view, path, style, status);
}

// viewport 2.0 overrides
void ArrowLocatorManip::preDrawUI(const M3dView& view)
{
    // add code to prepare specific drawing
}

void ArrowLocatorManip::drawUI(
    MHWRender::MUIDrawManager& drawManager,
    const MHWRender::MFrameContext& frameContext) const
{
    // expecific draw here, this manipulator doesn't have a specific draw
}

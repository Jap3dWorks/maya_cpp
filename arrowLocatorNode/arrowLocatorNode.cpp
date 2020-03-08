#include "arrowLocatorNode.h"
#include "arrowLocatorManipNode.h"

#include <maya\MPlug.h>
#include <maya\MDataBlock.h>
#include <maya\MDataHandle.h>

#include <maya\MGlobal.h>
#include <maya\MFnDagNode.h>
#include <maya\MFnUnitAttribute.h>
#include <maya\MAngle.h>
#include <maya\MColor.h>
#include <maya\MPxManipContainer.h>

#include <maya\MFnPlugin.h>

MTypeId ArrowLocator::id(0x80002);
MString ArrowLocator::name("arrowLocator");

MString ArrowLocator::drawDbClassification("drawdb/geometry/arrowLocator");
MString ArrowLocator::drawRegistrantId("arrowLocatorNodePlugin");

MObject ArrowLocator::windDirection;

// array of points to draw in opengl
static float arrow[][3] = { { 2.f,  0.f,  0.f},
                            {-3.f,  0.f,  2.f},
                            {-2.f,  0.f,  0.f},
                            {-3.f,  0.f, -2.f} };

// indices into the arrow array
static GLuint triangleIndices[] = { 0,1,2,0,2,3 };

ArrowLocator::ArrowLocator() {}
ArrowLocator::~ArrowLocator() {}

MStatus ArrowLocator::compute(const MPlug& plug, MDataBlock& data)
{
    return MS::kSuccess;
}

MStatus ArrowLocator::initialize()
{
    // handle units, like angle time or distance
    MFnUnitAttribute uAttr;
    MStatus status;

    ArrowLocator::windDirection = uAttr.create(MString("windDirection"), "wd", MAngle(0.f, MAngle::kRadians), &status);
    uAttr.setStorable(true);
    uAttr.setWritable(true);
    uAttr.setReadable(true);
    uAttr.setKeyable(true);
    uAttr.setMin(0.f);
    uAttr.setMax(2 * PI);
    uAttr.setDefault(MAngle(0.f, MAngle::kRadians));
    
    addAttribute(ArrowLocator::windDirection);

    MPxManipContainer::addToManipConnectTable(ArrowLocator::id);

    return MS::kSuccess;
}

void ArrowLocator::draw(M3dView& view, const MDagPath& path, M3dView::DisplayStyle style, M3dView::DisplayStatus status)
{
    // reference to this node
    MObject thisNode = thisMObject();

    MPlug wdPlug(thisNode, ArrowLocator::windDirection);
    MAngle angle;
    wdPlug.getValue(angle);

    // start drawing
    view.beginGL();

    // draw style shaded
    if ((style == M3dView::kFlatShaded) || (style == M3dView::kGouraudShaded))
    {
        glPushAttrib(GL_CURRENT_BIT);
        if (status == M3dView::kActive)
        {
            view.setDrawColor(13, M3dView::kActiveColors);
        }
        else
        {
            view.setDrawColor(13, M3dView::kDormantColors);
        }

        glPushMatrix();
        glRotated(-angle.asDegrees(), 0.0, 1.0, 0.0);
        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(arrow[0][0], arrow[0][1], arrow[0][2]);
        glVertex3f(arrow[1][0], arrow[1][1], arrow[1][2]);
        glVertex3f(arrow[2][0], arrow[2][1], arrow[2][2]);
        glEnd();

        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(arrow[2][0], arrow[2][1], arrow[2][2]);
        glVertex3f(arrow[3][0], arrow[3][1], arrow[3][2]);
        glVertex3f(arrow[0][0], arrow[0][1], arrow[0][2]);
        glEnd();
        glPopMatrix();

        glPopAttrib();
    }

    // Draw the outline
    glPushMatrix();
    glRotated(-angle.asDegrees(), 0.0, 1.0, 0.0);
    glBegin(GL_LINE_STRIP);
    glVertex3f(arrow[0][0], arrow[0][1], arrow[0][2]);
    glVertex3f(arrow[1][0], arrow[1][1], arrow[1][2]);
    glVertex3f(arrow[2][0], arrow[2][1], arrow[2][2]);
    glEnd();

    glBegin(GL_LINE_STRIP);
    glVertex3f(arrow[2][0], arrow[2][1], arrow[2][2]);
    glVertex3f(arrow[3][0], arrow[3][1], arrow[3][2]);
    glVertex3f(arrow[0][0], arrow[0][1], arrow[0][2]);
    glEnd();
    glPopMatrix();

    view.endGL();
}

MBoundingBox ArrowLocator::boundingBox() const
{
    MPoint upLeftCorner(-3.0, 0.0, -2.0);
    MPoint downRightCorner(2.0, 0.0, 2.0);
    MBoundingBox boundingArea(upLeftCorner, downRightCorner);
    return boundingArea;
}

void ArrowLocator::getRotationAngle(ArrowLocatorData* data)
{
    // retrieve value of the angle attrobibute of the node
    MStatus status;

    MObject node = thisMObject();
    MPlug wdPlug(node, ArrowLocator::windDirection);
    MAngle angle;
    wdPlug.getValue(angle);

    data->rotateAngle = angle;
}

class ArrowLocatorOverride : public MHWRender::MPxDrawOverride
{
public:
    static MHWRender::MPxDrawOverride* Creator(const MObject& obj)
    {
        return new ArrowLocatorOverride(obj);
    }

    virtual ~ArrowLocatorOverride();
    virtual MHWRender::DrawAPI supportedDrawAPIs() const;

    virtual bool isBounded(
        const MDagPath& objPath,
        const MDagPath& cameraPath) const override;

    virtual MBoundingBox boundingBox(
        const MDagPath& objPath,
        const MDagPath& cameraPath) const override;

    virtual MUserData* prepareForDraw(
        const MDagPath& objPath,
        const MDagPath& cameraPath,
        const MHWRender::MFrameContext& frameContext,
        MUserData* oldData) override;

    static void draw(
        const MHWRender::MDrawContext& context,
        const MUserData* data);

private:
    ArrowLocatorOverride(const MObject& obj);
};

ArrowLocatorOverride::ArrowLocatorOverride(const MObject& obj) :
    MHWRender::MPxDrawOverride(obj, ArrowLocatorOverride::draw)
{}

ArrowLocatorOverride::~ArrowLocatorOverride()
{}

MHWRender::DrawAPI ArrowLocatorOverride::supportedDrawAPIs() const
{
    return MHWRender::kOpenGL;
}

bool ArrowLocatorOverride::isBounded(
    const MDagPath& objPath,
    const MDagPath& cameraPath) const
{
    return true;
}

MBoundingBox ArrowLocatorOverride::boundingBox(
    const MDagPath& objPath,
    const MDagPath& cameraPath) const
{
    MStatus status;
    MFnDependencyNode node(objPath.node(), &status);
    // maybe is better a static_cast
    ArrowLocator* locatorNode = dynamic_cast<ArrowLocator*>(node.userNode());
    return locatorNode->boundingBox();
}

MUserData* ArrowLocatorOverride::prepareForDraw(
    const MDagPath& objPath,
    const MDagPath& cameraPath,
    const MHWRender::MFrameContext& frameContext,
    MUserData* oldData)
{
    // get the node
    MStatus status;
    MFnDependencyNode node(objPath.node(), &status);
    if (!status) return NULL;

    ArrowLocator* locatorNode = dynamic_cast<ArrowLocator*>(node.userNode());
    if (!locatorNode) return NULL;

    // prerpare data for draw callback
    ArrowLocatorData* data = dynamic_cast<ArrowLocatorData*>(oldData);
    if (!data)
    {
        data = new ArrowLocatorData();
    }

    // compute data and cache it
    locatorNode->getRotationAngle(data);

    return data;
}

void ArrowLocatorOverride::draw(
    const MHWRender::MDrawContext& context,
    const MUserData* data)
{
    MAngle rotationAngle;

    float color[3] = { 0.f, 1.f, 0.f };

    // data
    MStatus status;
    MHWRender::MStateManager* stateMgr = context.getStateManager();
    const ArrowLocatorData* locatorData = 
        dynamic_cast<const ArrowLocatorData*>(data);
    if (!stateMgr || !locatorData) return;
    
    if (locatorData)
    {
        rotationAngle = locatorData->rotateAngle;
    }

    // matrices
    const MMatrix transform = 
        context.getMatrix(MHWRender::MFrameContext::kWorldViewMtx, &status);
    if (status != MStatus::kSuccess) return;

    const MMatrix projection =
        context.getMatrix(MHWRender::MFrameContext::kProjectionMtx, &status);
    if (status != MStatus::kSuccess) return;

    // get render
    MHWRender::MRenderer *theRenderer = MHWRender::MRenderer::theRenderer();
    if (!theRenderer)
        return;
    if (theRenderer->drawAPIIsOpenGL())
    {
        glPushAttrib(GL_CURRENT_BIT);
        glColor4fv(color);

        glPushMatrix();
        glRotated(-rotationAngle.asDegrees(), 0.f, 1.f, 0.f);
        glBegin(GL_LINE_STRIP);

        glVertex3f(arrow[0][0], arrow[0][1], arrow[0][2]);
        glVertex3f(arrow[1][0], arrow[1][1], arrow[1][2]);
        glVertex3f(arrow[2][0], arrow[2][1], arrow[2][2]);
        glEnd();

        glBegin(GL_LINE_STRIP);
        glVertex3f(arrow[2][0], arrow[2][1], arrow[2][2]);
        glVertex3f(arrow[3][0], arrow[3][1], arrow[3][2]);
        glVertex3f(arrow[0][0], arrow[0][1], arrow[0][2]);
        glEnd();
        glPopMatrix();
    }
}

MStatus initializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin plugin(obj, "", "2018", "Any");

    status = plugin.registerNode(ArrowLocator::name, ArrowLocator::id,
        &ArrowLocator::creator, &ArrowLocator::initialize, MPxNode::kLocatorNode,
        &ArrowLocator::drawDbClassification);

    status = MHWRender::MDrawRegistry::registerDrawOverrideCreator(
        ArrowLocator::drawDbClassification,
        ArrowLocator::drawRegistrantId,
        &ArrowLocatorOverride::Creator);

    if (!status)
    {
        status.perror("registerNode");
        return status;
    }

    //- Register custom manipulator for your custom locator node.
    //- In order to make it work, you need to name your custom manipulator 
    //- after your custom node type name, also in your custom node's initialize()
    //- function, you need to call MPxManipContainer::addToManipConnectTable()

    // register manipulator
    status = plugin.registerNode(ArrowLocatorManip::name, ArrowLocatorManip::id,
        &ArrowLocatorManip::creator, &ArrowLocatorManip::initialize,
        MPxNode::kDependNode);

    if (!status)
    {
        status.perror("registerNode");
        return status;
    }

    return status;
}

MStatus uninitializePlugin(MObject obj)
{
    MStatus status;

    MFnPlugin plugin(obj);

    status = plugin.deregisterNode(ArrowLocatorManip::id);
    if (!status)
    {
        status.perror("deregisterNode");
        return status;
    }

    status = plugin.deregisterNode(ArrowLocator::id);
    MHWRender::MDrawRegistry::deregisterDrawOverrideCreator(
        ArrowLocator::drawDbClassification,
        ArrowLocator::drawRegistrantId);

    if (!status)
    {
        status.perror("deregisterNode");
        return status;
    }

    return status;

}
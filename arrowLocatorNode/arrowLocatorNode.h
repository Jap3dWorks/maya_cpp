#ifndef ARROWLOCATORNODE_H
#define ARROWLOCATORNODE_H

#include <maya\MPxLocatorNode.h>
#include <maya\MFnNumericAttribute.h>
#include <maya\MTypeId.h>
#include <maya\M3dView.h>
#include <maya\MDagPath.h>

// viewport 2.0 includes
#include <maya\MDrawRegistry.h>
#include <maya\MPxDrawOverride.h>
#include <maya\MUserData.h>
#include <maya\MDrawContext.h>
#include <maya\MGlobal.h>
#include <maya\MSelectionList.h>
#include <maya\MViewport2Renderer.h>
#include <maya\MHWGeometryUtilities.h>
#include <maya\MStateManager.h>
#include <maya\MShaderManager.h>
#include <maya\MAngle.h>

// macros to use to rotate the locator
#define PI 3.1426926538979
#define toDegree(rot) rot*(180.0/PI)


class ArrowLocatorData : public MUserData
{
public:
    MAngle rotateAngle;

    ArrowLocatorData() : MUserData(false) {}  // don't delete after use
    virtual ~ArrowLocatorData() {}
};

class ArrowLocator : public MPxLocatorNode
{
public:
    ArrowLocator();
    virtual ~ArrowLocator();

    virtual MStatus compute(const MPlug& plug, MDataBlock& data) override;

    virtual void draw(M3dView& view, const MDagPath& path, M3dView::DisplayStyle style, M3dView::DisplayStatus status);
    virtual MBoundingBox boundingBox() const;
    virtual bool isBounded() const {
        return true;
    }

    void getRotationAngle(ArrowLocatorData* data);
    
    static MStatus initialize();

    static void* creator()
    {
        return new ArrowLocator();
    }

public:
    static MTypeId id;
    static MString name;

    static MString drawDbClassification;
    static MString drawRegistrantId;
    static MObject windDirection;

};


#endif

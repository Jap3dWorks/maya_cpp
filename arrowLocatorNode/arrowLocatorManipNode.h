#ifndef ARROWLOCATORMANIPNODE_H
#define ARROWLOCATORMANIPNODE_H

#include <maya\MPxManipContainer.h>
#include <maya\MTypeid.h>


class ArrowLocatorManip : public MPxManipContainer
{
public:
    ArrowLocatorManip();
    virtual ~ArrowLocatorManip();

    static void* creator();
    static MStatus initialize();
    virtual MStatus createChildren();
    virtual MStatus connectToDependNode(const MObject& dependNode);

    // callback to update manip position
    MManipData centerPointCallback(unsigned int manipIndex);

    virtual void draw(M3dView& view, const MDagPath& path,
        M3dView::DisplayStyle style, M3dView::DisplayStatus status);

    // viewport 2 overrides
    virtual void preDrawUI(const M3dView& view);
    virtual void drawUI(MHWRender::MUIDrawManager& drawManager,
        const MHWRender::MFrameContext& frameContext) const;

public:
    static MTypeId id;
    static MString name;
    static MDagPath fDiscManip;

    // hold locator node
    static MDagPath fNodePath;
};

#endif  // !ARROWLOCATORMANIPNODE_H
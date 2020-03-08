#include "retrieveWeightCmd.h"

#include <iostream>
#include <maya/MGlobal.h>
#include <maya/MSelectionList.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MPlug.h>

MString retrieveWeightCmd::name("retrieveWeight");


MStatus retrieveWeightCmd::doIt(const MArgList& arglist)
{
    MStatus stat = MS::kSuccess;

    MObject blendShapeNode;
    MSelectionList selList;

    stat = MGlobal::getActiveSelectionList(selList);
    if (selList.length() != 0)
    {
        selList.getDependNode(0, blendShapeNode);
        MFnDependencyNode fn_dep(blendShapeNode, &stat);

        if (fn_dep.type() == MFn::kBlendShape)
        {
            MString attr_name("weight");
            MPlug weightArrayPlug = fn_dep.findPlug(attr_name, &stat);

            if (weightArrayPlug.isArray())
            {
                MString plug_name = weightArrayPlug.name();
                cout << "plug " << plug_name << " is an array plug.\n";

                int numberOfElem = weightArrayPlug.numElements();
                cout << "This plug has " << numberOfElem << " elems.\n";

                for (unsigned int j = 0; j < numberOfElem; ++j)
                {
                    MPlug elem_plug = weightArrayPlug.elementByPhysicalIndex(j, &stat);
                    cout << "Physical index: " << j << ";   " << "Logical index: " << elem_plug.logicalIndex(&stat) << std::endl;

                    double valueElem;
                    stat = elem_plug.getValue(valueElem);
                    cout << "The value in this elem plug is: " << valueElem << std::endl;
                }
            }
        }
    }
    return stat;
}
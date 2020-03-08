#include "dynNode.h"
#include <maya/MGlobal.h>

MTypeId dynNode::id(0x00001);
MString dynNode::name = "dynNode";

MObject dynNode::input;
MObject dynNode::output;


void* dynNode::creator()
{
    return new dynNode();
}

void dynNode::postConstructor()
{
    MStatus status;

    MFnNumericAttribute fn_nAttr;
    dynAttr = fn_nAttr.create("dynAttr", "da", MFnNumericData::kDouble, 0.0, &status);
    fn_nAttr.setWritable(true);

    MObject this_node = thisMObject();
    MFnDependencyNode mfn_depend_node(this_node);
    mfn_depend_node.addAttribute(dynAttr);  // it is not a static attribute
}

MStatus dynNode::setDependentsDirty(const MPlug& dirtyplug, MPlugArray &plugarray)
{

    if (dirtyplug.partialName() == "da")
    {
        MObject this_node = thisMObject();
        MPlug outputPlug(this_node, dynNode::output);
        plugarray.append(outputPlug);
    }

    return MS::kSuccess;
}

MStatus dynNode::initialize()
{
    MStatus status;
    MFnNumericAttribute fn_nAttr;

    dynNode::input = fn_nAttr.create("input", "in", MFnNumericData::kFloat, 0.0, &status);
    fn_nAttr.setStorable(true);
    fn_nAttr.setKeyable(true);

    dynNode::output = fn_nAttr.create("output", "out", MFnNumericData::kFloat, 0.0, &status);
    fn_nAttr.setStorable(false);

    addAttribute(dynNode::input);
    addAttribute(dynNode::output);

    attributeAffects(dynNode::input, dynNode::output);

    return MS::kSuccess;
}

MStatus dynNode::compute(const MPlug& plug, MDataBlock& datablock)
{
    MStatus status;

    if (plug == dynNode::output)
    {
        MDataHandle inputData = datablock.inputValue(dynNode::input, &status);
        float input_value = inputData.asFloat();

        MDataHandle dynData = datablock.inputValue(dynAttr, &status);
        float dyn_value = dynData.asFloat();

        MDataHandle outputValue = datablock.outputValue(dynNode::output, &status);
        outputValue.set(input_value + dyn_value);

        return MS::kSuccess;
    }

    return MS::kUnknownParameter;

}
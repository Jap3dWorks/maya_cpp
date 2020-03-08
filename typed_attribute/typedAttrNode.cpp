#include "typedAttrNode.h"

MTypeId TypedNode::id(0x00002);
MString TypedNode::name = "typedAttributeNode";

MObject TypedNode::input;
MObject TypedNode::output;
MObject TypedNode::decString;

MStatus TypedNode::initialize()
{
    MStatus status;

    MFnNumericAttribute fn_nAttr;
    TypedNode::input = fn_nAttr.create("input", "in", MFnNumericData::kDouble, 0.0, &status);
    fn_nAttr.setStorable(true);
    fn_nAttr.setKeyable(true);

    TypedNode::output = fn_nAttr.create("output", "out", MFnNumericData::kDouble, 0.0, &status);
    fn_nAttr.setStorable(false);
    fn_nAttr.setKeyable(false);

    MFnTypedAttribute fn_typeAttr;
    MFnStringData string_data;
    MString default_string("Default string");
    MObject defaultStringObj = string_data.create(default_string);

    TypedNode::decString = fn_typeAttr.create("dec_string", "ds", MFnData::kString, defaultStringObj);
    
    addAttribute(TypedNode::input);
    addAttribute(TypedNode::output);
    addAttribute(TypedNode::decString);

    attributeAffects(TypedNode::input, TypedNode::output);

    return MS::kSuccess;
}


MStatus TypedNode::compute(const MPlug& plug, MDataBlock& datablock)
{
    MStatus status;

    if (plug == TypedNode::output)
    {
        MDataHandle inputHandle = datablock.inputValue(TypedNode::input, &status);
        float inputdata = inputHandle.asFloat();

        MDataHandle outputHandle = datablock.outputValue(TypedNode::output, &status);
        outputHandle.set(inputdata * 2);

        datablock.setClean(plug);

        return MS::kSuccess;
    }
    return MS::kUnknownParameter;
}


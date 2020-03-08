#include "transCircleNode.h"

MTypeId TransCircle::id = 0x80013;
MString TransCircle::name = "transCircle";

MObject TransCircle::input;
MObject TransCircle::inputTranslateX;
MObject TransCircle::inputTranslateY;
MObject TransCircle::inputTranslateZ;
MObject TransCircle::inputTranslate;  // compound attribute

MObject TransCircle::output;
MObject TransCircle::outputTranslateX;
MObject TransCircle::outputTranslateY;
MObject TransCircle::outputTranslateZ;
MObject TransCircle::outputTranslate;  // compound attribute

MObject TransCircle::frames;
MObject TransCircle::scale;  // radius of circle


// called only once when plugin is registered in Maya
MStatus TransCircle::initialize()
{
    // create numeric attribute
    MFnNumericAttribute fn_nAttr;
    input = fn_nAttr.create("input", "in", MFnNumericData::kDouble, 0.0);
    fn_nAttr.setStorable(true);

    // individual attributes
    inputTranslateX = fn_nAttr.create("inputTranslateX", "itX", MFnNumericData::kDouble, 0.0);
    fn_nAttr.setStorable(true);

    inputTranslateY = fn_nAttr.create("inputTranslateY", "itY", MFnNumericData::kDouble, 0.0);
    fn_nAttr.setStorable(true);

    inputTranslateZ = fn_nAttr.create("inputTranslateZ", "itZ", MFnNumericData::kDouble, 0.0);
    fn_nAttr.setStorable(true);

    // crete compound attribute
    MFnCompoundAttribute fn_cAttr;
    inputTranslate = fn_cAttr.create("inputTranslate", "it");
    fn_cAttr.addChild(inputTranslateX);
    fn_cAttr.addChild(inputTranslateY);
    fn_cAttr.addChild(inputTranslateZ);
    fn_cAttr.setStorable(true);

    // output attributes
    // individual attributes
    outputTranslateX = fn_nAttr.create("outputTranslateX", "otX", MFnNumericData::kDouble, 0.0);
    fn_nAttr.setStorable(false);
    fn_nAttr.setWritable(false);

    outputTranslateY = fn_nAttr.create("outputTranslateY", "otY", MFnNumericData::kDouble, 0.0);
    fn_nAttr.setStorable(false);
    fn_nAttr.setWritable(false);

    outputTranslateZ = fn_nAttr.create("outputTranslateZ", "otZ", MFnNumericData::kDouble, 0.0);
    fn_nAttr.setStorable(false);
    fn_nAttr.setWritable(false);

    // crete compound attribute
    outputTranslate = fn_cAttr.create("outputTranslate", "ot");
    fn_cAttr.addChild(outputTranslateX);
    fn_cAttr.addChild(outputTranslateY);
    fn_cAttr.addChild(outputTranslateZ);
    fn_cAttr.setStorable(false);
    fn_cAttr.setWritable(false);

    // other attributes
    scale = fn_nAttr.create("scale", "sc", MFnNumericData::kDouble, 10.0);
    fn_nAttr.setStorable(true);
    
    frames = fn_nAttr.create("frames", "fr", MFnNumericData::kDouble, 48.0);
    fn_nAttr.setStorable(true);

    addAttribute(input);
    addAttribute(inputTranslate);
    addAttribute(outputTranslate);
    addAttribute(scale);
    addAttribute(frames);

    // input output dependency
    attributeAffects(inputTranslateX, outputTranslateX);
    attributeAffects(inputTranslateY, outputTranslateY);
    attributeAffects(inputTranslateZ, outputTranslateZ);
    attributeAffects(inputTranslate, outputTranslateX);
    attributeAffects(inputTranslate, outputTranslateY);
    attributeAffects(inputTranslate, outputTranslateZ);
    attributeAffects(inputTranslate, outputTranslate);

    // other relations
    attributeAffects(input, outputTranslateX);
    attributeAffects(input, outputTranslateY);

    attributeAffects(scale, outputTranslateX);
    attributeAffects(scale, outputTranslateY);
    
    attributeAffects(frames, outputTranslateX);
    attributeAffects(frames, outputTranslateY);
    
    return MS::kSuccess;
}


MStatus TransCircle::compute(const MPlug& plug, MDataBlock& datablock)
{
    MStatus state;

    bool k = (plug == outputTranslateX) |
        (plug == outputTranslateY) |
        (plug == outputTranslateZ) |
        (plug == outputTranslate);

    if (!k) { return MS::kUnknownParameter; }

    MDataHandle inputData = datablock.inputValue(input, &state);
    MDataHandle scaleData = datablock.inputValue(scale, &state);
    MDataHandle framesData = datablock.inputValue(frames, &state);
    MDataHandle transData = datablock.inputValue(inputTranslate, &state);

    double3& iTranslate = transData.asDouble3();
    double currentFrame = inputData.asDouble();
    double scaleFactor = scaleData.asDouble();
    double framesPerCircle = framesData.asDouble();
    double angle = 6.2831853 * (currentFrame / framesPerCircle);
    double3 oTranslate;

    oTranslate[0] = iTranslate[0] + (sin(angle) * scaleFactor);
    oTranslate[1] = iTranslate[1] + 1.0;
    oTranslate[2] = iTranslate[2] + (cos(angle) * scaleFactor);

    MDataHandle otHandle = datablock.outputValue(outputTranslate);
    otHandle.set(oTranslate[0], oTranslate[1], oTranslate[2]);
    datablock.setClean(plug);

    return MS::kSuccess;
}
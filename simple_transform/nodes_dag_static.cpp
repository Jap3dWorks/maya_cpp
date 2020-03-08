#include "nodes.h"

//    -=MAYTIX=-

MTypeId StaticMatrix::id = MATRIX_STATICHR_ID;

// this matris always has identity value
StaticMatrix::StaticMatrix() {}
void* StaticMatrix::creator()
{
    return new StaticMatrix();
}

MMatrix StaticMatrix::asMatrix() const
{
    return MMatrix::identity;
}

MMatrix StaticMatrix::asMatrix(double percent) const
{
    return MMatrix::identity;
}


//    NODE
MTypeId StaticHrc::id = NODE_STATICHR_ID;
MString StaticHrc::name = NODE_STATICHR_NAME;

StaticHrc::StaticHrc() : ParentClass() {}
StaticHrc::StaticHrc(MPxTransformationMatrix* p_mtx) : ParentClass(p_mtx) {}

StaticHrc::~StaticHrc() {}

void* StaticHrc::creator()
{
    return new StaticHrc();
}

MStatus StaticHrc::initialize()
{
    return MStatus::kSuccess;
}

void StaticHrc::postConstructor()
{
    ParentClass::postConstructor();
}

MPxTransformationMatrix* StaticHrc::createTransformationMatrix()
{
    return new StaticMatrix();
}

MStatus StaticHrc::compute(const MPlug& mplug, MDataBlock& datablock)
{
    return MStatus::kSuccess;
}


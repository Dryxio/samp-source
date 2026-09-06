// Native RenderWare wrappers neighbouring accepted preview matrix rotation.
#include "main.h"
extern int iGtaVersion;
extern VECTOR r5MatrixRotationAxes[3];
struct RtQuat; // opaque pointer only; no size/allocation claimed
// B57E0/68, cdecl, native return intentionally ignored by R5 wrapper.
void RotatePreviewQuaternion(RtQuat *quat,int axis,float angle,int combine)
{
    DWORD axisVector=(DWORD)&r5MatrixRotationAxes[axis];
    axis=(iGtaVersion!=1)?0x7EB800:0x7EB7C0;
    _asm push combine
    _asm push angle
    _asm push axisVector
    _asm push quat
    _asm mov edx, axis
    _asm call edx
    _asm pop edx
    _asm pop edx
    _asm pop edx
    _asm pop edx
}
// B58A0/46, outputs unit-axis and angle through actual caller pointers.
void QueryPreviewQuaternionRotation(RtQuat *quat,VECTOR *unitAxis,float *angle)
{
    DWORD function=(iGtaVersion!=1)?0x7EBAC0:0x7EBA80;
    _asm push angle
    _asm push unitAxis
    _asm push quat
    _asm mov edx, function
    _asm call edx
    _asm pop edx
    _asm pop edx
    _asm pop edx
}
// B58D0/57; arbitrary supplied vector rather than index into shared axes.
void RotatePreviewMatrixAroundVector(MATRIX4X4 *matrix,VECTOR *axis,float angle)
{
    DWORD axisVector=(DWORD)axis;
    axis=(VECTOR*)((iGtaVersion!=1)?0x7F2010:0x7F1FD0);
    _asm push 1
    _asm push angle
    _asm push axisVector
    _asm push matrix
    _asm mov edx, axis
    _asm call edx
    _asm pop edx
    _asm pop edx
    _asm pop edx
    _asm pop edx
}

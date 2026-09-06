#include "main.h"
extern int iGtaVersion;
extern VECTOR r5MatrixRotationAxes[3];
extern DWORD r5RwMatrixRotateFunction;
// R5 B5790/72. Original wrapper uses index argument slot for selected axis pointer.
void RotatePreviewMatrix(MATRIX4X4 *matrix,int axis,float angle)
{
    axis=(int)&r5MatrixRotationAxes[axis];
    r5RwMatrixRotateFunction=(iGtaVersion!=1)?0x7F2010:0x7F1FD0;
    _asm push 1
    _asm push angle
    _asm push axis
    _asm push matrix
    _asm mov ebx, r5RwMatrixRotateFunction
    _asm call ebx
    _asm pop ebx
    _asm pop ebx
    _asm pop ebx
    _asm pop ebx
}

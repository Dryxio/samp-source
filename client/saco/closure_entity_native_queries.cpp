#include "main.h"
extern int iGtaVersion;
extern VECTOR r5MatrixRotationAxes[3];
extern VECTOR r5MatrixQueryAxis;
extern VECTOR r5MatrixQueryCenter;
extern float r5MatrixQueryAngle;
extern int __stdcall ModelInfoLoaded(UINT model);
// Actual native RwMatrixQueryRotate, cdecl, USA/other version dispatch.
void QueryEntityMatrixAxis(MATRIX4X4 *matrix,int axis,float *angle)
{
 r5MatrixQueryAxis=r5MatrixRotationAxes[axis];
 axis=(iGtaVersion!=1)?0x7F2760:0x7F2720;
 _asm lea eax,r5MatrixQueryCenter
 _asm push eax
 _asm lea eax,r5MatrixQueryAngle
 _asm push eax
 _asm lea eax,r5MatrixQueryAxis
 _asm push eax
 _asm push matrix
 _asm mov edx,axis
 _asm call edx
 _asm pop edx
 _asm pop edx
 _asm pop edx
 _asm pop edx
 *angle=r5MatrixQueryAngle;
}
// Actual native RpClumpGetNumAtomics7498E0, no invented model-type predicate.
int GetEntityClumpAtomicCount(RpClump *clump)
{
 int result=0;
 _asm push clump
 _asm mov edx,0x7498E0
 _asm call edx
 _asm pop edx
 _asm mov result,eax
 return result;
}
// Non-owning existing entity view, never allocated; full wrapper/native offsets.
struct R5EntityNativeQueryView {
 BYTE prefix[0x40]; ENTITY_TYPE *entity;
 void GetMatrixAxisAngles(float *x,float *y,float *z);
 int GetAtomicCount();
};
void R5EntityNativeQueryView::GetMatrixAxisAngles(float *x,float *y,float *z)
{
 if(entity) {
  MATRIX4X4 *matrix=entity->mat;
  if(matrix) {
   float angle=0.0f;
   QueryEntityMatrixAxis(matrix,0,&angle);*x=angle;
   QueryEntityMatrixAxis(matrix,1,&angle);*y=angle;
   QueryEntityMatrixAxis(matrix,2,&angle);*z=angle;
  }
 }
}
int R5EntityNativeQueryView::GetAtomicCount()
{
 if(entity) {
  RpClump *clump=(RpClump*)ModelInfoLoaded(entity->nModelIndex);
  if(clump)return GetEntityClumpAtomicCount(clump);
 }
 return -1;
}

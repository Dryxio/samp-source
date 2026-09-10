#include "main.h"
struct R5GameQuaternion {float w,x,y,z;};
void R5GameNormalizeQuaternion(R5GameQuaternion *q)
{
 D3DXQUATERNION input,output;
 input.w=q->w;input.x=q->x;input.y=q->y;input.z=q->z;
 D3DXQuaternionNormalize(&output,&input);
 q->w=output.w;q->x=output.x;q->y=output.y;q->z=output.z;
}
void R5GameSlerpQuaternion(R5GameQuaternion *q,const R5GameQuaternion *a,const R5GameQuaternion *b,float t)
{
 D3DXQUATERNION inputA,inputB,output;
 inputA.w=a->w;inputA.x=a->x;inputA.y=a->y;inputA.z=a->z;
 inputB.w=b->w;inputB.x=b->x;inputB.y=b->y;inputB.z=b->z;
 D3DXQuaternionSlerp(&output,&inputA,&inputB,t);
 q->w=output.w;q->x=output.x;q->y=output.y;q->z=output.z;
}

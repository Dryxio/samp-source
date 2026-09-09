#include <math.h>
struct R5GameVector {float x,y,z;};
struct R5GameMatrix {float right[4],up[4],at[4],position[4];};
bool R5GameUnitVectorBounds(const R5GameVector *v)
{
 if(v->x<=1.0f && v->x>=-1.0f && v->y<=1.0f && v->y>=-1.0f && v->z<=1.0f && v->z>=-1.0f)return true;
 return false;
}
bool R5GameVelocityBounds(const R5GameVector *v)
{
 if(v->x<100.0f && v->x>-100.0f && v->y<100.0f && v->y>-100.0f && v->z<100.0f && v->z>-100.0f)return true;
 return false;
}
void R5GameNormalizeVector(R5GameVector *out,const R5GameVector *v)
{
 float x=v->x,y=v->y,z=v->z;
 float scale=1.0f/(float)sqrt(z*z+y*y+x*x);
 out->x=scale*v->x;out->y=scale*v->y;out->z=scale*v->z;
}
void R5GameTransformVector(R5GameVector *out,const R5GameMatrix *m,const R5GameVector *v)
{
 out->x=m->right[0]*v->x+m->up[0]*v->y+m->at[0]*v->z+m->position[0];
 out->y=m->right[1]*v->x+m->up[1]*v->y+m->at[1]*v->z+m->position[1];
 out->z=m->right[2]*v->x+m->up[2]*v->y+m->at[2]*v->z+m->position[2];
}

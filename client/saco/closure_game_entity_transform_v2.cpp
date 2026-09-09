#include <math.h>
struct R5GameVector {float x,y,z;};
struct R5GameMatrix {float right[4],up[4],at[4],position[4];};
// Actual native CPlaceable prefix: vtable, position, heading, matrix pointer.
struct R5GamePlaceableView {void *vtable;R5GameVector position;float heading;R5GameMatrix *matrix;};
void R5GameGetEntityTransform(R5GamePlaceableView *entity,R5GameVector *position,float *heading)
{
 if(entity->matrix){
  position->x=entity->matrix->position[0];position->y=entity->matrix->position[1];position->z=entity->matrix->position[2];
  float angle=(float)(atan2(-entity->matrix->up[0],entity->matrix->up[1])*57.295776f);
  if(angle<0.0f)angle+=360.0f;else if(angle>=360.0f)angle-=360.0f;
  *heading=angle;
 }else{position->x=entity->position.x;position->y=entity->position.y;position->z=entity->position.z;*heading=entity->heading;}
}

#include "main.h"
#pragma pack(push,1)
class R5LabelRemotePlayerView {
public:
 BYTE prefix[0x10A];
 BYTE state;
 BYTE gap10B[0x1DD-0x10B];
 CPlayerPed *ped;
 BOOL IsActive();
};
#pragma pack(pop)
BOOL R5LabelRemotePlayerView::IsActive() {
 if(ped && state) return TRUE;
 return FALSE;
}
void TransformLabelPoint(VECTOR *out,MATRIX4X4 *matrix,VECTOR *point) {
 out->X=matrix->right.X*point->X+matrix->up.X*point->Y+matrix->at.X*point->Z+matrix->pos.X;
 out->Y=matrix->right.Y*point->X+matrix->up.Y*point->Y+matrix->at.Y*point->Z+matrix->pos.Y;
 out->Z=matrix->right.Z*point->X+matrix->up.Z*point->Y+matrix->at.Z*point->Z+matrix->pos.Z;
}

#include "main.h"
#pragma pack(push,1)
class R5DrunkLevelPedView {
 BYTE prefix[0x2c9]; int level;
public:
 void SetDrunkLevel(int value);
};
#pragma pack(pop)
void R5DrunkLevelPedView::SetDrunkLevel(int value) { level=value; }

// Explicit symbolic ABI adapter, not compiler-generated coverage.
// The original vendor constructor independently emits the same complete 11-byte
// RakClientInterface this-adjustor (WNNO), proving the 0xDDE base adjustment.
// Constructor probe: cp32-trial30d-thunks1. Its unrelated unimplemented methods
// and constructor are not included or credited in the accepted capsule.
#include <string.h>
#include "../raknet/RakClient.h"
class R5RakClientDisconnectInterfaceView {
public:
 void Disconnect(unsigned int duration, unsigned char channel);
};
__declspec(naked) void R5RakClientDisconnectInterfaceView::Disconnect(unsigned int duration, unsigned char channel)
{
 __asm sub ecx, 0DDEh
 __asm jmp RakClient::Disconnect
}

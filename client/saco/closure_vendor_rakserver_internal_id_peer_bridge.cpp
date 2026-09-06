// Explicit symbolic ABI bridge for the real R5 RakServer/RakPeer adjusting thunk.
// The normal C++ getter is already compiled. MSVC does not emit this thunk in
// a method-only TU without the complete RakServer vtable. No vtable is invented.
#include "../raknet/RakServer.h"

enum { R5_ServerPeerBaseOffset = sizeof(RakServerInterface) };
typedef char R5_ServerPeerBaseOffsetIsFour[(R5_ServerPeerBaseOffset == 4) ? 1 : -1];

// Assembly-only entry to the actual const member getter. Never call as a C ABI.
// ECX=complete RakServer; [ESP+4]=hidden PlayerID result pointer; EAX returns it.
extern "C" void r5_RakServer_InternalID_target_assembly_only();
#pragma comment(linker, "/alternatename:_r5_RakServer_InternalID_target_assembly_only=?GetInternalID@RakServer@@UBE?AUPlayerID@@XZ")

// Original virtual slot39 receives ECX=RakPeer subobject (complete server+4).
// Keep the caller's return address and hidden result argument untouched; the
// real member getter performs ret4 after filling its six-byte PlayerID result.
extern "C" __declspec(naked) void r5_RakServer_InternalID_PeerBridge()
{
    __asm {
        sub ecx, R5_ServerPeerBaseOffset
        jmp r5_RakServer_InternalID_target_assembly_only
    }
}

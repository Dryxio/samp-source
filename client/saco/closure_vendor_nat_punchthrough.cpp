// R5 wire protocol assigns these two adjacent NAT failure IDs in the opposite
// order from this vendor PacketEnumerations.h. Keep behavior named by meaning.
#include "../raknet/PacketEnumerations.h"
enum R5NatFailureMessageID {
    R5_NAT_TARGET_NOT_CONNECTED = 0x52,
    R5_NAT_TARGET_CONNECTION_LOST = 0x53
};
#define ID_NAT_TARGET_NOT_CONNECTED R5_NAT_TARGET_NOT_CONNECTED
#define ID_NAT_TARGET_CONNECTION_LOST R5_NAT_TARGET_CONNECTION_LOST
#include "../raknet/NatPunchthrough.cpp"
#undef ID_NAT_TARGET_NOT_CONNECTED
#undef ID_NAT_TARGET_CONNECTION_LOST

typedef char R5NatPunchthroughSize[(sizeof(NatPunchthrough)==21)?1:-1];
typedef char R5NatConnectionRequestSize[(sizeof(NatPunchthrough::ConnectionRequest)==36)?1:-1];

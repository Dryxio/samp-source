// Actual separately located typed source storage; no padding or synthetic extent.
#include "../raknet/SocketLayer.h"
WSADATA SocketLayer::winsockInfo;
typedef char R5WSAActualPackedSize[(sizeof(WSADATA)==398)?1:-1];

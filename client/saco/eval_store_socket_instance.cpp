// Isolated whole SocketLayer BSS layout inference probe. No data credit.
#include "../raknet/SocketLayer.h"
bool SocketLayer::socketLayerStarted = false;
WSADATA SocketLayer::winsockInfo;
unsigned char r5DatagramOutput[32768];
SocketLayer SocketLayer::I;
typedef char R5WSASize[(sizeof(WSADATA)==398)?1:-1];
typedef char R5SocketSize[(sizeof(SocketLayer)==1)?1:-1];

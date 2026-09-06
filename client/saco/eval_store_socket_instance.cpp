// Actual separately located typed source storage; no padding or synthetic extent.
#include "../raknet/SocketLayer.h"
SocketLayer SocketLayer::I;
typedef char R5ActualSocketSize[(sizeof(SocketLayer)==1)?1:-1];

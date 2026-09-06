#include <windows.h>
#include "../raknet/GetTime.h"
// Real upstream per-module state; separate original addresses remain explicit.
static bool initialized=false;
static LARGE_INTEGER yo;
RakNetTime RakNet::GetTime(void)
{
    if (initialized == false) {
        QueryPerformanceFrequency(&yo);
        initialized=true;
    }
    LARGE_INTEGER PerfVal;
    QueryPerformanceCounter(&PerfVal);
    return (RakNetTime)(PerfVal.QuadPart*1000 / yo.QuadPart);
}

// Optional next bounded extension; same real static state as GetTime.
RakNetTimeNS RakNet::GetTimeNS(void)
{
    if (initialized == false) {
        QueryPerformanceFrequency(&yo);
        initialized=true;
    }
    LARGE_INTEGER PerfVal;
    QueryPerformanceCounter(&PerfVal);
    __int64 quotient,remainder;
    quotient=((PerfVal.QuadPart*1000) / yo.QuadPart);
    remainder=((PerfVal.QuadPart*1000) % yo.QuadPart);
    return quotient*1000+(remainder*1000/yo.QuadPart);
}

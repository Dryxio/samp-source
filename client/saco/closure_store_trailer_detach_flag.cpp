#include <windows.h>
// R5 module-owned zero-fill flag151744; scoped writes in DetachTrailer.
// Reader is native trailer hookA4C70. Hook implementation is not claimed covered.
BOOL r5TrailerDetachInProgress=FALSE;

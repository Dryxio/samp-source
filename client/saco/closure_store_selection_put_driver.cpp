#include <windows.h>
#define MAX_SCRIPT_VARS 16
struct SCRIPT_COMMAND { WORD OpCode; char Params[MAX_SCRIPT_VARS]; };
extern const SCRIPT_COMMAND r5SelectionPutActorInCarDriver={0x036A,"ii"};

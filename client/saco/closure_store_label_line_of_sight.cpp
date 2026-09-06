// Token-compatible minimal SCRIPT_COMMAND definition; full native descriptor18.
// No main.h/RakNet sentinel may contaminate this complete section.
typedef unsigned short WORD;
#define MAX_SCRIPT_VARS 16
struct SCRIPT_COMMAND { WORD OpCode; char Params[MAX_SCRIPT_VARS]; };
extern const SCRIPT_COMMAND r5LabelLineOfSight={0x06BD,"ffffffiiiii"};
typedef char LabelLOSDescriptorSize[(sizeof(SCRIPT_COMMAND)==18)?1:-1];

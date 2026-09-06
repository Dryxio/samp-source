#include "main.h"
extern BOOL r5ObjectLightingToggle;
extern bool bShowDebugLabels;
void cmdCmpStat(PCHAR szCmd) {}
void cmdToggleObjectLight(PCHAR szCmd) { r5ObjectLightingToggle=!r5ObjectLightingToggle; }
void cmdDebugLabels(PCHAR szCmd) { bShowDebugLabels=!bShowDebugLabels; }

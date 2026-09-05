// Complete source storage from net/menupool.cpp; isolated from unrelated header objects.
#include "game/common.h"
#include "game/menu.h"
CHAR g_szMenuItems[MAX_MENU_ITEMS][MAX_COLUMNS][MAX_MENU_LINE+1] =
{
	{"SAMP000", "SAMP100"},
	{"SAMP001", "SAMP101"},
	{"SAMP002", "SAMP102"},
	{"SAMP003", "SAMP103"},
	{"SAMP004", "SAMP104"},
	{"SAMP005", "SAMP105"},
	{"SAMP006", "SAMP106"},
	{"SAMP007", "SAMP107"},
	{"SAMP008", "SAMP108"},
	{"SAMP009", "SAMP109"},
	{"SAMP010", "SAMP110"},
	{"SAMP011", "SAMP111"},
};
typedef char complete_storage_size[(sizeof(g_szMenuItems)==792)?1:-1];




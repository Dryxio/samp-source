#include "main.h"
#include <stddef.h>
#pragma pack(push,1)
struct R5ImeCandidateListState
{
    WCHAR awszCandidate[10][256];
    CUniBuffer HoriCand;
    int nFirstSelected;
    int nHoriSelectedLen;
    DWORD dwCount;
    DWORD dwSelection;
    DWORD dwPageSize;
    int nReadingError;
    bool bShowWindow;
    RECT rcCandidate;
};
#pragma pack(pop)
typedef char R5CandidateStateSize[(sizeof(R5ImeCandidateListState)==0x143C)?1:-1];
typedef char R5CandidateHorizontalOffset[(offsetof(R5ImeCandidateListState,HoriCand)==0x1400)?1:-1];
typedef char R5CandidateSelectionOffset[(offsetof(R5ImeCandidateListState,dwSelection)==0x141F)?1:-1];
typedef char R5CandidateRectangleOffset[(offsetof(R5ImeCandidateListState,rcCandidate)==0x142C)?1:-1];
// Genuine complete wide candidate state, including real CUniBuffer lifetime.
R5ImeCandidateListState r5ImeCandidateState;

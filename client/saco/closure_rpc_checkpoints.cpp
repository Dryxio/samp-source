// 0.2.5 RPC transfer; R5 flag offsets verified, current base providers expanded.
#include "main.h"
extern CGame *pGame;
struct RPCGameCheckpointFlagsView { BYTE gap[0x29]; BOOL race; BYTE gap2d[0x20]; BOOL normal; };
void SetCheckpointTransfer(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	//PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	float fX, fY, fZ, fSize;

	bsData.Read(fX);
	bsData.Read(fY);
	bsData.Read(fZ);
	bsData.Read(fSize);

	VECTOR Pos,Extent;

	Pos.X = fX;
	Pos.Y = fY;
	Pos.Z = fZ;
	Extent.X = fSize;
	Extent.Y = fSize;
	Extent.Z = fSize;

	pGame->SetCheckpointInformation(&Pos, &Extent);
	((RPCGameCheckpointFlagsView*)pGame)->normal=TRUE;
}

void SetRaceCheckpointTransfer(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	//PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	float fX, fY, fZ;
	BYTE byteType; //, byteSize;
	VECTOR Pos,Next;

	bsData.Read(byteType);
	bsData.Read(fX);
	bsData.Read(fY);
	bsData.Read(fZ);
	Pos.X = fX;
	Pos.Y = fY;
	Pos.Z = fZ;

	bsData.Read(fX);
	bsData.Read(fY);
	bsData.Read(fZ);
	Next.X = fX;
	Next.Y = fY;
	Next.Z = fZ;

	bsData.Read(fX);

	pGame->SetRaceCheckpointInformation(byteType, &Pos, &Next, fX);
	((RPCGameCheckpointFlagsView*)pGame)->race=TRUE;
}

void CGame::SetCheckpointInformation(VECTOR *pos, VECTOR *extent)
{
	memcpy(&m_vecCheckpointPos,pos,sizeof(VECTOR));
	memcpy(&m_vecCheckpointExtent,extent,sizeof(VECTOR));
	if(m_dwCheckpointMarker) {
		ScriptCommand(&disable_marker,m_dwCheckpointMarker);
		m_dwCheckpointMarker = NULL;

		DWORD dwMarkerID = 0;
		ScriptCommand(&create_radar_marker_without_sphere, m_vecCheckpointPos.X, m_vecCheckpointPos.Y, m_vecCheckpointPos.Z, 0, &dwMarkerID);
		ScriptCommand(&set_marker_color, dwMarkerID, 1005);
		ScriptCommand(&show_on_radar, dwMarkerID, 3);
		m_dwCheckpointMarker = dwMarkerID;
	}
}

void CGame::SetRaceCheckpointInformation(BYTE byteType, VECTOR *pos, VECTOR *next, float fSize) //VECTOR *extent)
{
	memcpy(&m_vecRaceCheckpointPos,pos,sizeof(VECTOR));
	memcpy(&m_vecRaceCheckpointNext,next,sizeof(VECTOR));
	m_fRaceCheckpointSize = fSize;
	m_byteRaceType = byteType;
	if(m_dwRaceCheckpointMarker)
	{
		ScriptCommand(&disable_marker,m_dwRaceCheckpointMarker);
		m_dwRaceCheckpointMarker = NULL;

		DWORD dwMarkerID = 0;
		ScriptCommand(&create_radar_marker_without_sphere, m_vecRaceCheckpointPos.X, m_vecRaceCheckpointPos.Y, m_vecRaceCheckpointPos.Z, 0, &dwMarkerID);
		ScriptCommand(&set_marker_color, dwMarkerID, 1005);
		ScriptCommand(&show_on_radar, dwMarkerID, 3);
		m_dwRaceCheckpointMarker = dwMarkerID;
	}
	MakeRaceCheckpoint();
}

void CGame::MakeRaceCheckpoint()
{
	if(m_dwRaceCheckpointHandle) {
 ScriptCommand(&destroy_racing_checkpoint,m_dwRaceCheckpointHandle);
 m_dwRaceCheckpointHandle=NULL;
 }
 m_bRaceCheckpointsEnabled=false;

	ScriptCommand(&create_racing_checkpoint, (int)m_byteRaceType,
				m_vecRaceCheckpointPos.X, m_vecRaceCheckpointPos.Y, m_vecRaceCheckpointPos.Z,
				m_vecRaceCheckpointNext.X, m_vecRaceCheckpointNext.Y, m_vecRaceCheckpointNext.Z,
				m_fRaceCheckpointSize, &m_dwRaceCheckpointHandle);

	m_bRaceCheckpointsEnabled = true;
}

// 0.2.5 transfer proposals. Exact matching and complete EH required.

#include "main.h"

extern CGame *pGame;

extern CNetGame *pNetGame;

struct RPCSettingsNetGameView { BYTE unknown[0x3d5]; struc_41 *settings; };

void ScrSetCameraPosTransfer(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	VECTOR vecPos;
	bsData.Read(vecPos.X);
	bsData.Read(vecPos.Y);
	bsData.Read(vecPos.Z);
	pGame->GetCamera()->SetPosition(vecPos.X,vecPos.Y,vecPos.Z,0.0f,0.0f,0.0f);
}

void ScrSetCameraLookAtTransfer(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	VECTOR vecPos;
 BYTE byteCut=0;
	bsData.Read(vecPos.X);
	bsData.Read(vecPos.Y);
	bsData.Read(vecPos.Z);
	bsData.Read(byteCut);
 if(byteCut<1 || byteCut>2) byteCut=2;
 pGame->GetCamera()->LookAtPoint(vecPos.X,vecPos.Y,vecPos.Z,byteCut);	
}

void ScrHaveSomeMoneyTransfer(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	int iAmount;
	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	bsData.Read(iAmount);
	pGame->AddToLocalMoney(iAmount);
}

void ScrSetGravityTransfer(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	float fGravity;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	bsData.Read(fGravity);

	pGame->SetGravity(fGravity);
}

void WeatherTransfer(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	//PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	BYTE byteWeather;
	bsData.Read(byteWeather);
	((RPCSettingsNetGameView*)pNetGame)->settings->field_2E = byteWeather;
 pGame->SetWorldWeather(byteWeather);	
}

void SetTimeExTransfer(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	//PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	BYTE byteHour;
	BYTE byteMinute;
	bsData.Read(byteHour);
	bsData.Read(byteMinute);
	//pNetGame->m_byteHoldTime = 0;
	pGame->SetWorldTime(byteHour, byteMinute);
	((RPCSettingsNetGameView*)pNetGame)->settings->field_2C = byteHour;
	((RPCSettingsNetGameView*)pNetGame)->settings->field_2D = byteMinute;
}

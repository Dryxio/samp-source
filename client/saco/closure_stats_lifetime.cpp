// Existing client constructors; canonical shared statistics storage stays external.
#include "main.h"
extern RakNetStatisticsStruct RakServerStats;
CNetStats::CNetStats(IDirect3DDevice9 *pD3DDevice)
{
	field_8 = GetTickCount();
	field_4 = 0;
	field_0 = 0;
	field_10 = 0;
	field_C = 0;
	m_pD3DDevice = pD3DDevice;
}

CSvrNetStats::CSvrNetStats(IDirect3DDevice9 *pD3DDevice)
{
	m_dwLastUpdateTick = GetTickCount();
	m_dwLastTotalBytesRecv = 0;
	m_dwLastTotalBytesSent = 0;
	m_dwBPSDownload = 0;
	m_dwBPSUpload = 0;
	m_pD3DDevice = pD3DDevice;
	memset(&RakServerStats,0,sizeof(RakNetStatisticsStruct));
}

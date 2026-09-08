// Source0.2.5 vehicle marker processing with R5 layout/colors.
#include "main.h"
#include "game/scripting.h"
struct R5VehicleMarkersView {
 BYTE unknown0[0x44]; DWORD m_dwGTAId; BYTE unknown48[4]; VEHICLE_TYPE *m_pVehicle;
 BYTE unknown50[0x14]; BYTE m_byteObjectiveVehicle; BOOL m_bSpecialMarkerEnabled;
 BYTE unknown69[0x4b]; DWORD m_dwMarkerID;
 void ProcessMarkers();
};
__declspec(selectany) extern const SCRIPT_COMMAND r5_tie_marker_to_car={0x0161,"iiiv"};
__declspec(selectany) extern const SCRIPT_COMMAND r5_disable_marker={0x0164,"i"};
__declspec(selectany) extern const SCRIPT_COMMAND r5_set_marker_color={0x0165,"ii"};
__declspec(selectany) extern const SCRIPT_COMMAND r5_show_on_radar={0x0168,"ii"};
void R5VehicleMarkersView::ProcessMarkers()
{
	if(!m_pVehicle) return;

    if(m_byteObjectiveVehicle) {
		// SHOW ALWAYS
		if(!m_bSpecialMarkerEnabled) {
			if(m_dwMarkerID) {
				ScriptCommand(&r5_disable_marker, m_dwMarkerID);
				m_dwMarkerID = 0;
			}
			ScriptCommand(&r5_tie_marker_to_car, m_dwGTAId, 1, 3, &m_dwMarkerID);
			ScriptCommand(&r5_set_marker_color,m_dwMarkerID,1006);
			ScriptCommand(&r5_show_on_radar,m_dwMarkerID,3);
			m_bSpecialMarkerEnabled = TRUE;
		}
		return;
	}

	// Disable the special marker if it has been deactivated
	if(!m_byteObjectiveVehicle && m_bSpecialMarkerEnabled) {
		if(m_dwMarkerID) {
			ScriptCommand(&r5_disable_marker, m_dwMarkerID);
			m_bSpecialMarkerEnabled = FALSE;
			m_dwMarkerID = 0;
		}
	}

	// Add or remove car scanning markers.
	if(((CEntity*)this)->GetDistanceFromLocalPlayerPed() < 200.0f && !((CVehicle*)this)->IsOccupied()) {
		// SHOW IT
		if(!m_dwMarkerID)  {
			ScriptCommand(&r5_tie_marker_to_car, m_dwGTAId, 1, 2, &m_dwMarkerID);
			ScriptCommand(&r5_set_marker_color,m_dwMarkerID,1004);
		}	
	} 
	else if(((CVehicle*)this)->IsOccupied() || ((CEntity*)this)->GetDistanceFromLocalPlayerPed() >= 200.0f) {
		// REMOVE IT	
		if(m_dwMarkerID) {
			ScriptCommand(&r5_disable_marker, m_dwMarkerID);
			m_dwMarkerID = 0;
		}
	}
}

//-----------------------------------------------------------


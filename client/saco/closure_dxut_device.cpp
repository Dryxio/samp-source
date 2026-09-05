// Complete original DXUT device predicate.
#include "d3d9/common/dxstdafx.h"
#include <new>
UINT DXUTDepthBits(D3DFORMAT);
UINT DXUTStencilBits(D3DFORMAT);
bool DXUTDoesDeviceComboMatchPreserveOptions( CD3DEnumDeviceSettingsCombo* pDeviceSettingsCombo, 
                                                 DXUTDeviceSettings* pDeviceSettingsIn, 
                                                 DXUTMatchOptions* pMatchOptions )
{
    //---------------------
    // Adapter ordinal
    //---------------------
    if( pMatchOptions->eAdapterOrdinal == DXUTMT_PRESERVE_INPUT && 
        (pDeviceSettingsCombo->AdapterOrdinal != pDeviceSettingsIn->AdapterOrdinal) )
        return false;

    //---------------------
    // Device type
    //---------------------
    if( pMatchOptions->eDeviceType == DXUTMT_PRESERVE_INPUT && 
        (pDeviceSettingsCombo->DeviceType != pDeviceSettingsIn->DeviceType) )
        return false;

    //---------------------
    // Windowed
    //---------------------
    if( pMatchOptions->eWindowed == DXUTMT_PRESERVE_INPUT && 
        (pDeviceSettingsCombo->Windowed != pDeviceSettingsIn->pp.Windowed) )
        return false;

    //---------------------
    // Adapter format
    //---------------------
    if( pMatchOptions->eAdapterFormat == DXUTMT_PRESERVE_INPUT && 
        (pDeviceSettingsCombo->AdapterFormat != pDeviceSettingsIn->AdapterFormat) )
        return false;

    //---------------------
    // Vertex processing
    //---------------------
    // If keep VP and input has HWVP, then skip if this combo doesn't have HWTL 
    if( pMatchOptions->eVertexProcessing == DXUTMT_PRESERVE_INPUT && 
        ((pDeviceSettingsIn->BehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING) != 0) && 
        ((pDeviceSettingsCombo->pDeviceInfo->Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) == 0) )
        return false;

    //---------------------
    // Resolution
    //---------------------
    // If keep resolution then check that width and height supported by this combo
    if( pMatchOptions->eResolution == DXUTMT_PRESERVE_INPUT )
    {
        bool bFound = false;
        for( int i=0; i< pDeviceSettingsCombo->pAdapterInfo->displayModeList.GetSize(); i++ )
        {
            D3DDISPLAYMODE displayMode = pDeviceSettingsCombo->pAdapterInfo->displayModeList.GetAt( i );
            if( displayMode.Format != pDeviceSettingsCombo->AdapterFormat )
                continue; // Skip this display mode if it doesn't match the combo's adapter format

            if( displayMode.Width == pDeviceSettingsIn->pp.BackBufferWidth &&
                displayMode.Height == pDeviceSettingsIn->pp.BackBufferHeight )
            {
                bFound = true;
                break;
            }
        }

        // If the width and height are not supported by this combo, return false
        if( !bFound )
            return false;
    }

    //---------------------
    // Back buffer format
    //---------------------
    if( pMatchOptions->eBackBufferFormat == DXUTMT_PRESERVE_INPUT && 
        pDeviceSettingsCombo->BackBufferFormat != pDeviceSettingsIn->pp.BackBufferFormat )
        return false;

    //---------------------
    // Back buffer count
    //---------------------
    // No caps for the back buffer count

    //---------------------
    // Multisample
    //---------------------
    if( pMatchOptions->eMultiSample == DXUTMT_PRESERVE_INPUT )
    {
        bool bFound = false;
        for( int i=0; i<pDeviceSettingsCombo->multiSampleTypeList.GetSize(); i++ )
        {
            D3DMULTISAMPLE_TYPE msType = pDeviceSettingsCombo->multiSampleTypeList.GetAt(i);
            DWORD msQuality  = pDeviceSettingsCombo->multiSampleQualityList.GetAt(i);

            if( msType == pDeviceSettingsIn->pp.MultiSampleType &&
                msQuality >= pDeviceSettingsIn->pp.MultiSampleQuality )
            {
                bFound = true;
                break;
            }
        }

        // If multisample type/quality not supported by this combo, then return false
        if( !bFound )
            return false;
    }
        
    //---------------------
    // Swap effect
    //---------------------
    // No caps for swap effects

    //---------------------
    // Depth stencil 
    //---------------------
    // If keep depth stencil format then check that the depth stencil format is supported by this combo
    if( pMatchOptions->eDepthFormat == DXUTMT_PRESERVE_INPUT &&
        pMatchOptions->eStencilFormat == DXUTMT_PRESERVE_INPUT )
    {
        if( pDeviceSettingsIn->pp.AutoDepthStencilFormat != D3DFMT_UNKNOWN &&
            !pDeviceSettingsCombo->depthStencilFormatList.Contains( pDeviceSettingsIn->pp.AutoDepthStencilFormat ) )
            return false;
    }

    // If keep depth format then check that the depth format is supported by this combo
    if( pMatchOptions->eDepthFormat == DXUTMT_PRESERVE_INPUT &&
        pDeviceSettingsIn->pp.AutoDepthStencilFormat != D3DFMT_UNKNOWN )
    {
        bool bFound = false;
        UINT dwDepthBits = DXUTDepthBits( pDeviceSettingsIn->pp.AutoDepthStencilFormat );
        for( int i=0; i<pDeviceSettingsCombo->depthStencilFormatList.GetSize(); i++ )
        {
            D3DFORMAT depthStencilFmt = pDeviceSettingsCombo->depthStencilFormatList.GetAt(i);
            UINT dwCurDepthBits = DXUTDepthBits( depthStencilFmt );
            if( dwCurDepthBits - dwDepthBits == 0)
                bFound = true;
        }

        if( !bFound )
            return false;
    }

    // If keep depth format then check that the depth format is supported by this combo
    if( pMatchOptions->eStencilFormat == DXUTMT_PRESERVE_INPUT &&
        pDeviceSettingsIn->pp.AutoDepthStencilFormat != D3DFMT_UNKNOWN )
    {
        bool bFound = false;
        UINT dwStencilBits = DXUTStencilBits( pDeviceSettingsIn->pp.AutoDepthStencilFormat );
        for( int i=0; i<pDeviceSettingsCombo->depthStencilFormatList.GetSize(); i++ )
        {
            D3DFORMAT depthStencilFmt = pDeviceSettingsCombo->depthStencilFormatList.GetAt(i);
            UINT dwCurStencilBits = DXUTStencilBits( depthStencilFmt );
            if( dwCurStencilBits - dwStencilBits == 0)
                bFound = true;
        }

        if( !bFound )
            return false;
    }

    //---------------------
    // Present flags
    //---------------------
    // No caps for the present flags

    //---------------------
    // Refresh rate
    //---------------------
    // If keep refresh rate then check that the resolution is supported by this combo
    if( pMatchOptions->eRefreshRate == DXUTMT_PRESERVE_INPUT )
    {
        bool bFound = false;
        for( int i=0; i<pDeviceSettingsCombo->pAdapterInfo->displayModeList.GetSize(); i++ )
        {
            D3DDISPLAYMODE displayMode = pDeviceSettingsCombo->pAdapterInfo->displayModeList.GetAt( i );
            if( displayMode.Format != pDeviceSettingsCombo->AdapterFormat )
                continue;
            if( displayMode.RefreshRate == pDeviceSettingsIn->pp.FullScreen_RefreshRateInHz )
            {
                bFound = true;
                break;
            }
        }

        // If refresh rate not supported by this combo, then return false
        if( !bFound )
            return false;
    }

    //---------------------
    // Present interval
    //---------------------
    // If keep present interval then check that the present interval is supported by this combo
    if( pMatchOptions->ePresentInterval == DXUTMT_PRESERVE_INPUT &&
        !pDeviceSettingsCombo->presentIntervalList.Contains( pDeviceSettingsIn->pp.PresentationInterval ) )
        return false;

    return true;
}

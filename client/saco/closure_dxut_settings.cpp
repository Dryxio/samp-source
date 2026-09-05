// Complete pinned device settings definitions.
#include "d3d9/common/dxstdafx.h"
UINT DXUTColorChannelBits(D3DFORMAT);
UINT DXUTDepthBits(D3DFORMAT);
UINT DXUTStencilBits(D3DFORMAT);
CD3DEnumeration* DXUTPrepareEnumerationObject(bool);
bool DXUTDoesDeviceComboMatchPreserveOptions(CD3DEnumDeviceSettingsCombo*, DXUTDeviceSettings*, DXUTMatchOptions*);
HRESULT DXUTFindValidResolution( CD3DEnumDeviceSettingsCombo* pBestDeviceSettingsCombo, 
                                D3DDISPLAYMODE displayModeIn, D3DDISPLAYMODE* pBestDisplayMode )
{
    D3DDISPLAYMODE bestDisplayMode;
    ZeroMemory( &bestDisplayMode, sizeof(D3DDISPLAYMODE) );
    
    if( pBestDeviceSettingsCombo->Windowed )
    {
        // In windowed mode, all resolutions are valid but restritions still apply 
        // on the size of the window.  See DXUTChangeDevice() for details
        *pBestDisplayMode = displayModeIn;
    }
    else
    {
        int nBestRanking = 100000;
        int nCurRanking;
        CGrowableArray<D3DDISPLAYMODE>* pDisplayModeList = &pBestDeviceSettingsCombo->pAdapterInfo->displayModeList;
        for( int iDisplayMode=0; iDisplayMode<pDisplayModeList->GetSize(); iDisplayMode++ )
        {
            D3DDISPLAYMODE displayMode = pDisplayModeList->GetAt(iDisplayMode);

            // Skip display modes that don't match the combo's adapter format
            if( displayMode.Format != pBestDeviceSettingsCombo->AdapterFormat )
                continue;

            // Find the delta between the current width/height and the optimal width/height
            nCurRanking = abs((int)displayMode.Width - (int)displayModeIn.Width) + 
                          abs((int)displayMode.Height- (int)displayModeIn.Height);

            if( nCurRanking < nBestRanking )
            {
                bestDisplayMode = displayMode;
                nBestRanking = nCurRanking;

                // Stop if perfect match found
                if( nBestRanking == 0 )
                    break;
            }
        }

        if( bestDisplayMode.Width == 0 )
        {
            *pBestDisplayMode = displayModeIn;
            return E_FAIL; // No valid display modes found
        }

        *pBestDisplayMode = bestDisplayMode;
    }

    return S_OK;
}
void DXUTBuildOptimalDeviceSettings( DXUTDeviceSettings* pOptimalDeviceSettings, 
                                     DXUTDeviceSettings* pDeviceSettingsIn, 
                                     DXUTMatchOptions* pMatchOptions )
{
    IDirect3D9* pD3D = DXUTGetD3DObject();
    D3DDISPLAYMODE adapterDesktopDisplayMode;

    ZeroMemory( pOptimalDeviceSettings, sizeof(DXUTDeviceSettings) ); 

    //---------------------
    // Adapter ordinal
    //---------------------    
    if( pMatchOptions->eAdapterOrdinal == DXUTMT_IGNORE_INPUT )
        pOptimalDeviceSettings->AdapterOrdinal = D3DADAPTER_DEFAULT; 
    else
        pOptimalDeviceSettings->AdapterOrdinal = pDeviceSettingsIn->AdapterOrdinal;      

    //---------------------
    // Device type
    //---------------------
    if( pMatchOptions->eDeviceType == DXUTMT_IGNORE_INPUT )
        pOptimalDeviceSettings->DeviceType = D3DDEVTYPE_HAL; 
    else
        pOptimalDeviceSettings->DeviceType = pDeviceSettingsIn->DeviceType;

    //---------------------
    // Windowed
    //---------------------
    if( pMatchOptions->eWindowed == DXUTMT_IGNORE_INPUT )
        pOptimalDeviceSettings->pp.Windowed = TRUE; 
    else
        pOptimalDeviceSettings->pp.Windowed = pDeviceSettingsIn->pp.Windowed;

    //---------------------
    // Adapter format
    //---------------------
    if( pMatchOptions->eAdapterFormat == DXUTMT_IGNORE_INPUT )
    {
        // If windowed, default to the desktop display mode
        // If fullscreen, default to the desktop display mode for quick mode change or 
        // default to D3DFMT_X8R8G8B8 if the desktop display mode is < 32bit
        pD3D->GetAdapterDisplayMode( pOptimalDeviceSettings->AdapterOrdinal, &adapterDesktopDisplayMode );
        if( pOptimalDeviceSettings->pp.Windowed || DXUTColorChannelBits(adapterDesktopDisplayMode.Format) >= 8 )
            pOptimalDeviceSettings->AdapterFormat = adapterDesktopDisplayMode.Format;
        else
            pOptimalDeviceSettings->AdapterFormat = D3DFMT_X8R8G8B8;
    }
    else
    {
        pOptimalDeviceSettings->AdapterFormat = pDeviceSettingsIn->AdapterFormat;
    }

    //---------------------
    // Vertex processing
    //---------------------
    if( pMatchOptions->eVertexProcessing == DXUTMT_IGNORE_INPUT )
        pOptimalDeviceSettings->BehaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING; 
    else
        pOptimalDeviceSettings->BehaviorFlags = pDeviceSettingsIn->BehaviorFlags;

    //---------------------
    // Resolution
    //---------------------
    if( pMatchOptions->eResolution == DXUTMT_IGNORE_INPUT )
    {
        // If windowed, default to 640x480
        // If fullscreen, default to the desktop res for quick mode change
        if( pOptimalDeviceSettings->pp.Windowed )
        {
            pOptimalDeviceSettings->pp.BackBufferWidth = 640;
            pOptimalDeviceSettings->pp.BackBufferHeight = 480;
        }
        else
        {
            pD3D->GetAdapterDisplayMode( pOptimalDeviceSettings->AdapterOrdinal, &adapterDesktopDisplayMode );
            pOptimalDeviceSettings->pp.BackBufferWidth = adapterDesktopDisplayMode.Width;
            pOptimalDeviceSettings->pp.BackBufferHeight = adapterDesktopDisplayMode.Height;
        }
    }
    else
    {
        pOptimalDeviceSettings->pp.BackBufferWidth = pDeviceSettingsIn->pp.BackBufferWidth;
        pOptimalDeviceSettings->pp.BackBufferHeight = pDeviceSettingsIn->pp.BackBufferHeight;
    }

    //---------------------
    // Back buffer format
    //---------------------
    if( pMatchOptions->eBackBufferFormat == DXUTMT_IGNORE_INPUT )
        pOptimalDeviceSettings->pp.BackBufferFormat = pOptimalDeviceSettings->AdapterFormat; // Default to match the adapter format
    else
        pOptimalDeviceSettings->pp.BackBufferFormat = pDeviceSettingsIn->pp.BackBufferFormat;

    //---------------------
    // Back buffer count
    //---------------------
    if( pMatchOptions->eBackBufferCount == DXUTMT_IGNORE_INPUT )
        pOptimalDeviceSettings->pp.BackBufferCount = 2; // Default to triple buffering for perf gain
    else
        pOptimalDeviceSettings->pp.BackBufferCount = pDeviceSettingsIn->pp.BackBufferCount;
   
    //---------------------
    // Multisample
    //---------------------
    if( pMatchOptions->eMultiSample == DXUTMT_IGNORE_INPUT )
    {
        // Default to no multisampling 
        pOptimalDeviceSettings->pp.MultiSampleType = D3DMULTISAMPLE_NONE;
        pOptimalDeviceSettings->pp.MultiSampleQuality = 0; 
    }
    else
    {
        pOptimalDeviceSettings->pp.MultiSampleType = pDeviceSettingsIn->pp.MultiSampleType;
        pOptimalDeviceSettings->pp.MultiSampleQuality = pDeviceSettingsIn->pp.MultiSampleQuality;
    }

    //---------------------
    // Swap effect
    //---------------------
    if( pMatchOptions->eSwapEffect == DXUTMT_IGNORE_INPUT )
        pOptimalDeviceSettings->pp.SwapEffect = D3DSWAPEFFECT_DISCARD; 
    else
        pOptimalDeviceSettings->pp.SwapEffect = pDeviceSettingsIn->pp.SwapEffect;

    //---------------------
    // Depth stencil 
    //---------------------
    if( pMatchOptions->eDepthFormat == DXUTMT_IGNORE_INPUT &&
        pMatchOptions->eStencilFormat == DXUTMT_IGNORE_INPUT )
    {
        UINT nBackBufferBits = DXUTColorChannelBits( pOptimalDeviceSettings->pp.BackBufferFormat );
        if( nBackBufferBits >= 8 )
            pOptimalDeviceSettings->pp.AutoDepthStencilFormat = D3DFMT_D32; 
        else
            pOptimalDeviceSettings->pp.AutoDepthStencilFormat = D3DFMT_D16; 
    }
    else
    {
        pOptimalDeviceSettings->pp.AutoDepthStencilFormat = pDeviceSettingsIn->pp.AutoDepthStencilFormat;
    }

    //---------------------
    // Present flags
    //---------------------
    if( pMatchOptions->ePresentFlags == DXUTMT_IGNORE_INPUT )
        pOptimalDeviceSettings->pp.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
    else
        pOptimalDeviceSettings->pp.Flags = pDeviceSettingsIn->pp.Flags;

    //---------------------
    // Refresh rate
    //---------------------
    if( pMatchOptions->eRefreshRate == DXUTMT_IGNORE_INPUT )
        pOptimalDeviceSettings->pp.FullScreen_RefreshRateInHz = 0;
    else
        pOptimalDeviceSettings->pp.FullScreen_RefreshRateInHz = pDeviceSettingsIn->pp.FullScreen_RefreshRateInHz;

    //---------------------
    // Present interval
    //---------------------
    if( pMatchOptions->ePresentInterval == DXUTMT_IGNORE_INPUT )
    {
        // For windowed, default to D3DPRESENT_INTERVAL_IMMEDIATE
        // which will wait not for the vertical retrace period to prevent tearing, 
        // but may introduce tearing.
        // For full screen, default to D3DPRESENT_INTERVAL_DEFAULT 
        // which will wait for the vertical retrace period to prevent tearing.
        if( pOptimalDeviceSettings->pp.Windowed )
            pOptimalDeviceSettings->pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
        else
            pOptimalDeviceSettings->pp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
    }
    else
    {
        pOptimalDeviceSettings->pp.PresentationInterval = pDeviceSettingsIn->pp.PresentationInterval;
    }
}
void DXUTBuildValidDeviceSettings( DXUTDeviceSettings* pValidDeviceSettings, 
                                   CD3DEnumDeviceSettingsCombo* pBestDeviceSettingsCombo, 
                                   DXUTDeviceSettings* pDeviceSettingsIn, 
                                   DXUTMatchOptions* pMatchOptions )
{
    IDirect3D9* pD3D = DXUTGetD3DObject();
    D3DDISPLAYMODE adapterDesktopDisplayMode;
    pD3D->GetAdapterDisplayMode( pBestDeviceSettingsCombo->AdapterOrdinal, &adapterDesktopDisplayMode );

    // For each setting pick the best, taking into account the match options and 
    // what's supported by the device

    //---------------------
    // Adapter Ordinal
    //---------------------
    // Just using pBestDeviceSettingsCombo->AdapterOrdinal

    //---------------------
    // Device Type
    //---------------------
    // Just using pBestDeviceSettingsCombo->DeviceType

    //---------------------
    // Windowed 
    //---------------------
    // Just using pBestDeviceSettingsCombo->Windowed

    //---------------------
    // Adapter Format
    //---------------------
    // Just using pBestDeviceSettingsCombo->AdapterFormat

    //---------------------
    // Vertex processing
    //---------------------
    DWORD dwBestBehaviorFlags = 0;
    if( pMatchOptions->eVertexProcessing == DXUTMT_PRESERVE_INPUT )   
    {
        dwBestBehaviorFlags = pDeviceSettingsIn->BehaviorFlags;
    }
    else if( pMatchOptions->eVertexProcessing == DXUTMT_IGNORE_INPUT )    
    {
        // The framework defaults to HWVP if available otherwise use SWVP
        if ((pBestDeviceSettingsCombo->pDeviceInfo->Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) != 0)
            dwBestBehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
        else
            dwBestBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    }
    else // if( pMatchOptions->eVertexProcessing == DXUTMT_CLOSEST_TO_INPUT )    
    {
        // Default to input, and fallback to SWVP if HWVP not available 
        dwBestBehaviorFlags = pDeviceSettingsIn->BehaviorFlags;
        if ((pBestDeviceSettingsCombo->pDeviceInfo->Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) == 0 && 
            ( (dwBestBehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING) != 0 || 
              (dwBestBehaviorFlags & D3DCREATE_MIXED_VERTEXPROCESSING) != 0) )
        {
            dwBestBehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
            dwBestBehaviorFlags &= ~D3DCREATE_MIXED_VERTEXPROCESSING;
            dwBestBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
        }

        // One of these must be selected
        if( (dwBestBehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING) == 0 &&
            (dwBestBehaviorFlags & D3DCREATE_MIXED_VERTEXPROCESSING) == 0 &&
            (dwBestBehaviorFlags & D3DCREATE_SOFTWARE_VERTEXPROCESSING) == 0 )
        {
            if ((pBestDeviceSettingsCombo->pDeviceInfo->Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) != 0)
                dwBestBehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
            else
                dwBestBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
        }
    }

    //---------------------
    // Resolution
    //---------------------
    D3DDISPLAYMODE bestDisplayMode;  
    if( pMatchOptions->eResolution == DXUTMT_PRESERVE_INPUT )   
    {
        bestDisplayMode.Width = pDeviceSettingsIn->pp.BackBufferWidth;
        bestDisplayMode.Height = pDeviceSettingsIn->pp.BackBufferHeight;
    }
    else 
    {
        D3DDISPLAYMODE displayModeIn;  
        if( pMatchOptions->eResolution == DXUTMT_CLOSEST_TO_INPUT &&
            pDeviceSettingsIn )   
        {
            displayModeIn.Width = pDeviceSettingsIn->pp.BackBufferWidth;
            displayModeIn.Height = pDeviceSettingsIn->pp.BackBufferHeight;
        }
        else // if( pMatchOptions->eResolution == DXUTMT_IGNORE_INPUT )   
        {
            if( pBestDeviceSettingsCombo->Windowed )
            {
                // The framework defaults to 640x480 for windowed
                displayModeIn.Width = 640;
                displayModeIn.Height = 480;
            }
            else
            {
                // The framework defaults to desktop resolution for fullscreen to try to avoid slow mode change
                displayModeIn.Width = adapterDesktopDisplayMode.Width;
                displayModeIn.Height = adapterDesktopDisplayMode.Height;
            }
        }

        // Call a helper function to find the closest valid display mode to the optimal 
        DXUTFindValidResolution( pBestDeviceSettingsCombo, displayModeIn, &bestDisplayMode );
    }

    //---------------------
    // Back Buffer Format
    //---------------------
    // Just using pBestDeviceSettingsCombo->BackBufferFormat

    //---------------------
    // Back buffer count
    //---------------------
    UINT bestBackBufferCount;
    if( pMatchOptions->eBackBufferCount == DXUTMT_PRESERVE_INPUT )   
    {
        bestBackBufferCount = pDeviceSettingsIn->pp.BackBufferCount;
    }
    else if( pMatchOptions->eBackBufferCount == DXUTMT_IGNORE_INPUT )   
    {
        // The framework defaults to triple buffering 
        bestBackBufferCount = 2;
    }
    else // if( pMatchOptions->eBackBufferCount == DXUTMT_CLOSEST_TO_INPUT )   
    {
        bestBackBufferCount = pDeviceSettingsIn->pp.BackBufferCount;
        if( bestBackBufferCount > 3 )
            bestBackBufferCount = 3;
        if( bestBackBufferCount < 1 )
            bestBackBufferCount = 1;
    }
    
    //---------------------
    // Multisample
    //---------------------
    D3DMULTISAMPLE_TYPE bestMultiSampleType;
    DWORD bestMultiSampleQuality;
    if( pDeviceSettingsIn && pDeviceSettingsIn->pp.SwapEffect != D3DSWAPEFFECT_DISCARD )
    {
        // Swap effect is not set to discard so multisampling has to off
        bestMultiSampleType = D3DMULTISAMPLE_NONE;
        bestMultiSampleQuality = 0;
    }
    else
    {
        if( pMatchOptions->eBackBufferCount == DXUTMT_PRESERVE_INPUT )   
        {
            bestMultiSampleType    = pDeviceSettingsIn->pp.MultiSampleType;
            bestMultiSampleQuality = pDeviceSettingsIn->pp.MultiSampleQuality;
        }
        else if( pMatchOptions->eBackBufferCount == DXUTMT_IGNORE_INPUT )   
        {
            // Default to no multisampling (always supported)
            bestMultiSampleType = D3DMULTISAMPLE_NONE;
            bestMultiSampleQuality = 0;
        }
        else if( pMatchOptions->eBackBufferCount == DXUTMT_CLOSEST_TO_INPUT )   
        {
            // Default to no multisampling (always supported)
            bestMultiSampleType = D3DMULTISAMPLE_NONE;
            bestMultiSampleQuality = 0;

            for( int i=0; i < pBestDeviceSettingsCombo->multiSampleTypeList.GetSize(); i++ )
            {
                D3DMULTISAMPLE_TYPE type = pBestDeviceSettingsCombo->multiSampleTypeList.GetAt(i);
                DWORD qualityLevels = pBestDeviceSettingsCombo->multiSampleQualityList.GetAt(i);
                
                // Check whether supported type is closer to the input than our current best
                if( abs(type - pDeviceSettingsIn->pp.MultiSampleType) < abs(bestMultiSampleType - pDeviceSettingsIn->pp.MultiSampleType) )
                {
                    bestMultiSampleType = type; 
                    bestMultiSampleQuality = __min( qualityLevels-1, pDeviceSettingsIn->pp.MultiSampleQuality );
                }
            }
        }
        else
        {
            // Error case
            bestMultiSampleType = D3DMULTISAMPLE_NONE;
            bestMultiSampleQuality = 0;
        }
    }

    //---------------------
    // Swap effect
    //---------------------
    D3DSWAPEFFECT bestSwapEffect;
    if( pMatchOptions->eSwapEffect == DXUTMT_PRESERVE_INPUT )   
    {
        bestSwapEffect = pDeviceSettingsIn->pp.SwapEffect;
    }
    else if( pMatchOptions->eSwapEffect == DXUTMT_IGNORE_INPUT )   
    {
        bestSwapEffect = D3DSWAPEFFECT_DISCARD;
    }
    else // if( pMatchOptions->eSwapEffect == DXUTMT_CLOSEST_TO_INPUT )   
    {
        bestSwapEffect = pDeviceSettingsIn->pp.SwapEffect;

        // Swap effect has to be one of these 3
        if( bestSwapEffect != D3DSWAPEFFECT_DISCARD &&
            bestSwapEffect != D3DSWAPEFFECT_FLIP &&
            bestSwapEffect != D3DSWAPEFFECT_COPY )
        {
            bestSwapEffect = D3DSWAPEFFECT_DISCARD;
        }
    }

    //---------------------
    // Depth stencil 
    //---------------------
    D3DFORMAT bestDepthStencilFormat;
    BOOL bestEnableAutoDepthStencil;

    CGrowableArray< int > depthStencilRanking;
    depthStencilRanking.SetSize( pBestDeviceSettingsCombo->depthStencilFormatList.GetSize() );

    UINT dwBackBufferBitDepth = DXUTColorChannelBits( pBestDeviceSettingsCombo->BackBufferFormat );       
    UINT dwInputDepthBitDepth = 0;
    if( pDeviceSettingsIn )
        dwInputDepthBitDepth = DXUTDepthBits( pDeviceSettingsIn->pp.AutoDepthStencilFormat );

    for( int i=0; i<pBestDeviceSettingsCombo->depthStencilFormatList.GetSize(); i++ )
    {
        D3DFORMAT curDepthStencilFmt = pBestDeviceSettingsCombo->depthStencilFormatList.GetAt(i);
        DWORD dwCurDepthBitDepth = DXUTDepthBits( curDepthStencilFmt );
        int nRanking;

        if( pMatchOptions->eDepthFormat == DXUTMT_PRESERVE_INPUT )
        {                       
            // Need to match bit depth of input
            if(dwCurDepthBitDepth == dwInputDepthBitDepth)
                nRanking = 0;
            else
                nRanking = 10000;
        }
        else if( pMatchOptions->eDepthFormat == DXUTMT_IGNORE_INPUT )
        {
            // Prefer match of backbuffer bit depth
            nRanking = abs((int)dwCurDepthBitDepth - (int)dwBackBufferBitDepth*4);
        }
        else // if( pMatchOptions->eDepthFormat == DXUTMT_CLOSEST_TO_INPUT )
        {
            // Prefer match of input depth format bit depth
            nRanking = abs((int)dwCurDepthBitDepth - (int)dwInputDepthBitDepth);
        }

        depthStencilRanking.Add( nRanking );
    }

    UINT dwInputStencilBitDepth = 0;
    if( pDeviceSettingsIn )
        dwInputStencilBitDepth = DXUTStencilBits( pDeviceSettingsIn->pp.AutoDepthStencilFormat );

    for( int i=0; i<pBestDeviceSettingsCombo->depthStencilFormatList.GetSize(); i++ )
    {
        D3DFORMAT curDepthStencilFmt = pBestDeviceSettingsCombo->depthStencilFormatList.GetAt(i);
        int nRanking = depthStencilRanking.GetAt(i);
        DWORD dwCurStencilBitDepth = DXUTStencilBits( curDepthStencilFmt );

        if( pMatchOptions->eStencilFormat == DXUTMT_PRESERVE_INPUT )
        {                       
            // Need to match bit depth of input
            if(dwCurStencilBitDepth == dwInputStencilBitDepth)
                nRanking += 0;
            else
                nRanking += 10000;
        }
        else if( pMatchOptions->eStencilFormat == DXUTMT_IGNORE_INPUT )
        {
            // Prefer 0 stencil bit depth
            nRanking += dwCurStencilBitDepth;
        }
        else // if( pMatchOptions->eStencilFormat == DXUTMT_CLOSEST_TO_INPUT )
        {
            // Prefer match of input stencil format bit depth
            nRanking += abs((int)dwCurStencilBitDepth - (int)dwInputStencilBitDepth);
        }

        depthStencilRanking.SetAt( i, nRanking );
    }

    int nBestRanking = 100000;
    int nBestIndex = -1;
    for( int i=0; i<pBestDeviceSettingsCombo->depthStencilFormatList.GetSize(); i++ )
    {
        int nRanking = depthStencilRanking.GetAt(i);
        if( nRanking < nBestRanking )
        {
            nBestRanking = nRanking;
            nBestIndex = i;
        }
    }

    if( nBestIndex >= 0 )
    {
        bestDepthStencilFormat = pBestDeviceSettingsCombo->depthStencilFormatList.GetAt(nBestIndex);
        bestEnableAutoDepthStencil = true;
    }
    else
    {
        bestDepthStencilFormat = D3DFMT_UNKNOWN;
        bestEnableAutoDepthStencil = false;
    }


    //---------------------
    // Present flags
    //---------------------
    DWORD dwBestFlags;
    if( pMatchOptions->ePresentFlags == DXUTMT_PRESERVE_INPUT )   
    {
        dwBestFlags = pDeviceSettingsIn->pp.Flags;
    }
    else if( pMatchOptions->ePresentFlags == DXUTMT_IGNORE_INPUT )   
    {
        dwBestFlags = 0;
        if( bestEnableAutoDepthStencil )
            dwBestFlags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;            
    }
    else // if( pMatchOptions->ePresentFlags == DXUTMT_CLOSEST_TO_INPUT )   
    {
        dwBestFlags = pDeviceSettingsIn->pp.Flags;
        if( bestEnableAutoDepthStencil )
            dwBestFlags |= D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
    }

    //---------------------
    // Refresh rate
    //---------------------
    if( pBestDeviceSettingsCombo->Windowed )
    {
        // Must be 0 for windowed
        bestDisplayMode.RefreshRate = 0;
    }
    else
    {
        if( pMatchOptions->eRefreshRate == DXUTMT_PRESERVE_INPUT )   
        {
            bestDisplayMode.RefreshRate = pDeviceSettingsIn->pp.FullScreen_RefreshRateInHz;
        }
        else 
        {
            UINT refreshRateMatch;
            if( pMatchOptions->eRefreshRate == DXUTMT_CLOSEST_TO_INPUT )   
            {
                refreshRateMatch = pDeviceSettingsIn->pp.FullScreen_RefreshRateInHz;
            }
            else // if( pMatchOptions->eRefreshRate == DXUTMT_IGNORE_INPUT )   
            {
                refreshRateMatch = adapterDesktopDisplayMode.RefreshRate;
            }

            bestDisplayMode.RefreshRate = 0;

            if( refreshRateMatch != 0 )
            {
                int nBestRefreshRanking = 100000;
                CGrowableArray<D3DDISPLAYMODE>* pDisplayModeList = &pBestDeviceSettingsCombo->pAdapterInfo->displayModeList;
                for( int iDisplayMode=0; iDisplayMode<pDisplayModeList->GetSize(); iDisplayMode++ )
                {
                    D3DDISPLAYMODE displayMode = pDisplayModeList->GetAt(iDisplayMode);                
                    if( displayMode.Format != pBestDeviceSettingsCombo->AdapterFormat || 
                        displayMode.Height != bestDisplayMode.Height ||
                        displayMode.Width != bestDisplayMode.Width )
                        continue; // Skip display modes that don't match 

                    // Find the delta between the current refresh rate and the optimal refresh rate 
                    int nCurRanking = abs((int)displayMode.RefreshRate - (int)refreshRateMatch);
                                        
                    if( nCurRanking < nBestRefreshRanking )
                    {
                        bestDisplayMode.RefreshRate = displayMode.RefreshRate;
                        nBestRefreshRanking = nCurRanking;

                        // Stop if perfect match found
                        if( nBestRefreshRanking == 0 )
                            break;
                    }
                }
            }
        }
    }

    //---------------------
    // Present interval
    //---------------------
    UINT bestPresentInterval;
    if( pMatchOptions->ePresentInterval == DXUTMT_PRESERVE_INPUT )   
    {
        bestPresentInterval = pDeviceSettingsIn->pp.PresentationInterval;
    }
    else if( pMatchOptions->ePresentInterval == DXUTMT_IGNORE_INPUT )   
    {
        if( pBestDeviceSettingsCombo->Windowed )
        {
            // For windowed, the framework defaults to D3DPRESENT_INTERVAL_IMMEDIATE
            // which will wait not for the vertical retrace period to prevent tearing, 
            // but may introduce tearing
            bestPresentInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
        }
        else
        {
            // For full screen, the framework defaults to D3DPRESENT_INTERVAL_DEFAULT 
            // which will wait for the vertical retrace period to prevent tearing
            bestPresentInterval = D3DPRESENT_INTERVAL_DEFAULT;
        }
    }
    else // if( pMatchOptions->ePresentInterval == DXUTMT_CLOSEST_TO_INPUT )   
    {
        if( pBestDeviceSettingsCombo->presentIntervalList.Contains( pDeviceSettingsIn->pp.PresentationInterval ) )
        {
            bestPresentInterval = pDeviceSettingsIn->pp.PresentationInterval;
        }
        else
        {
            if( pBestDeviceSettingsCombo->Windowed )
                bestPresentInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
            else
                bestPresentInterval = D3DPRESENT_INTERVAL_DEFAULT;
        }
    }

    // Fill the device settings struct
    ZeroMemory( pValidDeviceSettings, sizeof(DXUTDeviceSettings) );
    pValidDeviceSettings->AdapterOrdinal                 = pBestDeviceSettingsCombo->AdapterOrdinal;
    pValidDeviceSettings->DeviceType                     = pBestDeviceSettingsCombo->DeviceType;
    pValidDeviceSettings->AdapterFormat                  = pBestDeviceSettingsCombo->AdapterFormat;
    pValidDeviceSettings->BehaviorFlags                  = dwBestBehaviorFlags;
    pValidDeviceSettings->pp.BackBufferWidth             = bestDisplayMode.Width;
    pValidDeviceSettings->pp.BackBufferHeight            = bestDisplayMode.Height;
    pValidDeviceSettings->pp.BackBufferFormat            = pBestDeviceSettingsCombo->BackBufferFormat;
    pValidDeviceSettings->pp.BackBufferCount             = bestBackBufferCount;
    pValidDeviceSettings->pp.MultiSampleType             = bestMultiSampleType;  
    pValidDeviceSettings->pp.MultiSampleQuality          = bestMultiSampleQuality;
    pValidDeviceSettings->pp.SwapEffect                  = bestSwapEffect;
    pValidDeviceSettings->pp.hDeviceWindow               = pBestDeviceSettingsCombo->Windowed ? DXUTGetHWNDDeviceWindowed() : DXUTGetHWNDDeviceFullScreen();
    pValidDeviceSettings->pp.Windowed                    = pBestDeviceSettingsCombo->Windowed;
    pValidDeviceSettings->pp.EnableAutoDepthStencil      = bestEnableAutoDepthStencil;  
    pValidDeviceSettings->pp.AutoDepthStencilFormat      = bestDepthStencilFormat;
    pValidDeviceSettings->pp.Flags                       = dwBestFlags;                   
    pValidDeviceSettings->pp.FullScreen_RefreshRateInHz  = bestDisplayMode.RefreshRate;
    pValidDeviceSettings->pp.PresentationInterval        = bestPresentInterval;
}
float DXUTRankDeviceCombo( CD3DEnumDeviceSettingsCombo* pDeviceSettingsCombo, 
                           DXUTDeviceSettings* pOptimalDeviceSettings,
                           D3DDISPLAYMODE* pAdapterDesktopDisplayMode )
{
    float fCurRanking = 0.0f; 

    // Arbitrary weights.  Gives preference to the ordinal, device type, and windowed
    const float fAdapterOrdinalWeight   = 1000.0f;
    const float fDeviceTypeWeight       = 100.0f;
    const float fWindowWeight           = 10.0f;
    const float fAdapterFormatWeight    = 1.0f;
    const float fVertexProcessingWeight = 1.0f;
    const float fResolutionWeight       = 1.0f;
    const float fBackBufferFormatWeight = 1.0f;
    const float fMultiSampleWeight      = 1.0f;
    const float fDepthStencilWeight     = 1.0f;
    const float fRefreshRateWeight      = 1.0f;
    const float fPresentIntervalWeight  = 1.0f;

    //---------------------
    // Adapter ordinal
    //---------------------
    if( pDeviceSettingsCombo->AdapterOrdinal == pOptimalDeviceSettings->AdapterOrdinal )
        fCurRanking += fAdapterOrdinalWeight;

    //---------------------
    // Device type
    //---------------------
    if( pDeviceSettingsCombo->DeviceType == pOptimalDeviceSettings->DeviceType )
        fCurRanking += fDeviceTypeWeight;
    // Slightly prefer HAL 
    if( pDeviceSettingsCombo->DeviceType == D3DDEVTYPE_HAL )
        fCurRanking += 0.1f; 

    //---------------------
    // Windowed
    //---------------------
    if( pDeviceSettingsCombo->Windowed == pOptimalDeviceSettings->pp.Windowed )
        fCurRanking += fWindowWeight;

    //---------------------
    // Adapter format
    //---------------------
    if( pDeviceSettingsCombo->AdapterFormat == pOptimalDeviceSettings->AdapterFormat )
    {
        fCurRanking += fAdapterFormatWeight;
    }
    else
    {
        long nAdapterBits = (long) DXUTColorChannelBits(pDeviceSettingsCombo->AdapterFormat);
        int nBitDepthDelta = abs( nAdapterBits - (long) DXUTColorChannelBits(pOptimalDeviceSettings->AdapterFormat) );
        float fScale = __max(0.9f - (float)nBitDepthDelta*0.2f, 0.0f);
        fCurRanking += fScale * fAdapterFormatWeight;
    }

    if( !pDeviceSettingsCombo->Windowed )
    {
        // Slightly prefer when it matches the desktop format or is D3DFMT_X8R8G8B8
        bool bAdapterOptimalMatch;
        if( DXUTColorChannelBits(pAdapterDesktopDisplayMode->Format) >= 8 )
            bAdapterOptimalMatch = (pDeviceSettingsCombo->AdapterFormat == pAdapterDesktopDisplayMode->Format);
        else
            bAdapterOptimalMatch = (pDeviceSettingsCombo->AdapterFormat == D3DFMT_X8R8G8B8);

        if( bAdapterOptimalMatch )
            fCurRanking += 0.1f;
    }

    //---------------------
    // Vertex processing
    //---------------------
    if( (pOptimalDeviceSettings->BehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING) != 0 || 
        (pOptimalDeviceSettings->BehaviorFlags & D3DCREATE_MIXED_VERTEXPROCESSING) != 0 )
    {
        if( (pDeviceSettingsCombo->pDeviceInfo->Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) != 0 )
            fCurRanking += fVertexProcessingWeight;
    }
    // Slightly prefer HW T&L
    if( (pDeviceSettingsCombo->pDeviceInfo->Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) != 0 )
        fCurRanking += 0.1f;

    //---------------------
    // Resolution
    //---------------------
    bool bResolutionFound = false;
    for( int idm = 0; idm < pDeviceSettingsCombo->pAdapterInfo->displayModeList.GetSize(); idm++ )
    {
        D3DDISPLAYMODE displayMode = pDeviceSettingsCombo->pAdapterInfo->displayModeList.GetAt( idm );
        if( displayMode.Format != pDeviceSettingsCombo->AdapterFormat )
            continue;
        if( displayMode.Width == pOptimalDeviceSettings->pp.BackBufferWidth &&
            displayMode.Height == pOptimalDeviceSettings->pp.BackBufferHeight )
            bResolutionFound = true;
    }
    if( bResolutionFound )
        fCurRanking += fResolutionWeight;

    //---------------------
    // Back buffer format
    //---------------------
    if( pDeviceSettingsCombo->BackBufferFormat == pOptimalDeviceSettings->pp.BackBufferFormat )
    {
        fCurRanking += fBackBufferFormatWeight;
    }
    else
    {
        int nBitDepthDelta = abs( (long) DXUTColorChannelBits(pDeviceSettingsCombo->BackBufferFormat) -
                                  (long) DXUTColorChannelBits(pOptimalDeviceSettings->pp.BackBufferFormat) );
        float fScale = __max(0.9f - (float)nBitDepthDelta*0.2f, 0.0f);
        fCurRanking += fScale * fBackBufferFormatWeight;
    }

    // Check if this back buffer format is the same as 
    // the adapter format since this is preferred.
    bool bAdapterMatchesBB = (pDeviceSettingsCombo->BackBufferFormat == pDeviceSettingsCombo->AdapterFormat);
    if( bAdapterMatchesBB )
        fCurRanking += 0.1f;

    //---------------------
    // Back buffer count
    //---------------------
    // No caps for the back buffer count

    //---------------------
    // Multisample
    //---------------------
    bool bMultiSampleFound = false;
    for( int i=0; i<pDeviceSettingsCombo->multiSampleTypeList.GetSize(); i++ )
    {
        D3DMULTISAMPLE_TYPE msType = pDeviceSettingsCombo->multiSampleTypeList.GetAt(i);
        DWORD msQuality  = pDeviceSettingsCombo->multiSampleQualityList.GetAt(i);

        if( msType == pOptimalDeviceSettings->pp.MultiSampleType &&
            msQuality >= pOptimalDeviceSettings->pp.MultiSampleQuality )
        {
            bMultiSampleFound = true;
            break;
        }
    }
    if( bMultiSampleFound )
        fCurRanking += fMultiSampleWeight;
        
    //---------------------
    // Swap effect
    //---------------------
    // No caps for swap effects

    //---------------------
    // Depth stencil 
    //---------------------
    if( pDeviceSettingsCombo->depthStencilFormatList.Contains( pOptimalDeviceSettings->pp.AutoDepthStencilFormat ) )
        fCurRanking += fDepthStencilWeight;

    //---------------------
    // Present flags
    //---------------------
    // No caps for the present flags

    //---------------------
    // Refresh rate
    //---------------------
    bool bRefreshFound = false;
    for( int idm = 0; idm < pDeviceSettingsCombo->pAdapterInfo->displayModeList.GetSize(); idm++ )
    {
        D3DDISPLAYMODE displayMode = pDeviceSettingsCombo->pAdapterInfo->displayModeList.GetAt( idm );
        if( displayMode.Format != pDeviceSettingsCombo->AdapterFormat )
            continue;
        if( displayMode.RefreshRate == pOptimalDeviceSettings->pp.FullScreen_RefreshRateInHz )
            bRefreshFound = true;
    }
    if( bRefreshFound )
        fCurRanking += fRefreshRateWeight;

    //---------------------
    // Present interval
    //---------------------
    // If keep present interval then check that the present interval is supported by this combo
    if( pDeviceSettingsCombo->presentIntervalList.Contains( pOptimalDeviceSettings->pp.PresentationInterval ) )
        fCurRanking += fPresentIntervalWeight;

    return fCurRanking;
}
HRESULT DXUTFindValidDeviceSettings( DXUTDeviceSettings* pOut, DXUTDeviceSettings* pIn, 
                                     DXUTMatchOptions* pMatchOptions )
{
    if( pOut == NULL )
        return DXUT_ERR_MSGBOX( "DXUTFindValidDeviceSettings", E_INVALIDARG );

    CD3DEnumeration* pd3dEnum = DXUTPrepareEnumerationObject( false );
    IDirect3D9*      pD3D     = DXUTGetD3DObject();

    // Default to DXUTMT_IGNORE_INPUT for everything unless pMatchOptions isn't NULL
    DXUTMatchOptions defaultMatchOptions;
    if( NULL == pMatchOptions )
    {
        ZeroMemory( &defaultMatchOptions, sizeof(DXUTMatchOptions) );
        pMatchOptions = &defaultMatchOptions;
    }

    // Build an optimal device settings structure based upon the match 
    // options.  If the match option is set to ignore, then a optimal default value is used.
    // The default value may not exist on the system, but later this will be taken 
    // into account.
    DXUTDeviceSettings optimalDeviceSettings;
    DXUTBuildOptimalDeviceSettings( &optimalDeviceSettings, pIn, pMatchOptions );

    // Find the best combination of:
    //      Adapter Ordinal
    //      Device Type
    //      Adapter Format
    //      Back Buffer Format
    //      Windowed
    // given what's available on the system and the match options combined with the device settings input.
    // This combination of settings is encapsulated by the CD3DEnumDeviceSettingsCombo class.
    float fBestRanking = -1.0f;
    CD3DEnumDeviceSettingsCombo* pBestDeviceSettingsCombo = NULL;
    D3DDISPLAYMODE adapterDesktopDisplayMode;

    CGrowableArray<CD3DEnumAdapterInfo*>* pAdapterList = pd3dEnum->GetAdapterInfoList();
    for( int iAdapter=0; iAdapter<pAdapterList->GetSize(); iAdapter++ )
    {
        CD3DEnumAdapterInfo* pAdapterInfo = pAdapterList->GetAt(iAdapter);

        // Get the desktop display mode of adapter 
        pD3D->GetAdapterDisplayMode( pAdapterInfo->AdapterOrdinal, &adapterDesktopDisplayMode );

        // Enum all the device types supported by this adapter to find the best device settings
        for( int iDeviceInfo=0; iDeviceInfo<pAdapterInfo->deviceInfoList.GetSize(); iDeviceInfo++ )
        {
            CD3DEnumDeviceInfo* pDeviceInfo = pAdapterInfo->deviceInfoList.GetAt(iDeviceInfo);

            // Enum all the device settings combinations.  A device settings combination is 
            // a unique set of an adapter format, back buffer format, and IsWindowed.
            for( int iDeviceCombo=0; iDeviceCombo<pDeviceInfo->deviceSettingsComboList.GetSize(); iDeviceCombo++ )
            {
                CD3DEnumDeviceSettingsCombo* pDeviceSettingsCombo = pDeviceInfo->deviceSettingsComboList.GetAt(iDeviceCombo);

                // If windowed mode the adapter format has to be the same as the desktop 
                // display mode format so skip any that don't match
                if (pDeviceSettingsCombo->Windowed && (pDeviceSettingsCombo->AdapterFormat != adapterDesktopDisplayMode.Format))
                    continue;

                // Skip any combo that doesn't meet the preserve match options
                if( false == DXUTDoesDeviceComboMatchPreserveOptions( pDeviceSettingsCombo, pIn, pMatchOptions ) )
                    continue;           

                // Get a ranking number that describes how closely this device combo matches the optimal combo
                float fCurRanking = DXUTRankDeviceCombo( pDeviceSettingsCombo, &optimalDeviceSettings, &adapterDesktopDisplayMode );

                // If this combo better matches the input device settings then save it
                if( fCurRanking > fBestRanking )
                {
                    pBestDeviceSettingsCombo = pDeviceSettingsCombo;
                    fBestRanking = fCurRanking;
                }                
            }
        }
    }

    // If no best device combination was found then fail
    if( pBestDeviceSettingsCombo == NULL ) 
        return DXUTERR_NOCOMPATIBLEDEVICES;

    // Using the best device settings combo found, build valid device settings taking heed of 
    // the match options and the input device settings
    DXUTDeviceSettings validDeviceSettings;
    DXUTBuildValidDeviceSettings( &validDeviceSettings, pBestDeviceSettingsCombo, pIn, pMatchOptions );
    *pOut = validDeviceSettings;

    return S_OK;
}

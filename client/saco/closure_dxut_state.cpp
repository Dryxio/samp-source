// Complete pinned framework state/lifetime and public accessors.
#include "d3d9/common/dxstdafx.h"
#include <strsafe.h>
#undef GetSystemMetrics
void DXUTCleanup3DEnvironment(bool = true);
void DXUTDisplayErrorMessage(HRESULT);
void DXUTAllowShortcutKeys(bool);
CD3DEnumeration* DXUTPrepareEnumerationObject(bool = false);
void DXUTUpdateBackBufferDesc();
void DXUTUpdateDeviceStats(D3DDEVTYPE,DWORD,D3DADAPTER_IDENTIFIER9*);
typedef DECLSPEC_IMPORT UINT (WINAPI* LPTIMEBEGINPERIOD)(UINT);
CRITICAL_SECTION g_cs;  
bool g_bThreadSafe = true;


//--------------------------------------------------------------------------------------
// Automatically enters & leaves the CS upon object creation/deletion
//--------------------------------------------------------------------------------------
class DXUTLock
{
public:
    inline DXUTLock()  { if( g_bThreadSafe ) EnterCriticalSection( &g_cs ); }
    inline ~DXUTLock() { if( g_bThreadSafe ) LeaveCriticalSection( &g_cs ); }
};



//--------------------------------------------------------------------------------------
// Helper macros to build member functions that access member variables with thread safety
//--------------------------------------------------------------------------------------
#define SET_ACCESSOR( x, y )       inline void Set##y( x t )  { DXUTLock l; m_state.m_##y = t; };
#define GET_ACCESSOR( x, y )       inline x Get##y() { DXUTLock l; return m_state.m_##y; };
#define GET_SET_ACCESSOR( x, y )   SET_ACCESSOR( x, y ) GET_ACCESSOR( x, y )

#define SETP_ACCESSOR( x, y )      inline void Set##y( x* t )  { DXUTLock l; m_state.m_##y = *t; };
#define GETP_ACCESSOR( x, y )      inline x* Get##y() { DXUTLock l; return &m_state.m_##y; };
#define GETP_SETP_ACCESSOR( x, y ) SETP_ACCESSOR( x, y ) GETP_ACCESSOR( x, y )


//--------------------------------------------------------------------------------------
// Stores timer callback info
//--------------------------------------------------------------------------------------
struct DXUT_TIMER
{
    LPDXUTCALLBACKTIMER pCallbackTimer;
    void* pCallbackUserContext;
    float fTimeoutInSecs;
    float fCountdown;
    bool  bEnabled;
};


//--------------------------------------------------------------------------------------
// Stores DXUT state and data access is done with thread safety (if g_bThreadSafe==true)
//--------------------------------------------------------------------------------------
class DXUTState
{
protected:
    struct STATE
    {
        IDirect3D9*          m_D3D;                     // the main D3D object

        IDirect3DDevice9*    m_D3DDevice;               // the D3D rendering device
        CD3DEnumeration*     m_D3DEnumeration;          // CD3DEnumeration object

        DXUTDeviceSettings*  m_CurrentDeviceSettings;   // current device settings
        D3DSURFACE_DESC      m_BackBufferSurfaceDesc;   // back buffer surface description
        D3DCAPS9             m_Caps;                    // D3D caps for current device

        HWND  m_HWNDFocus;                  // the main app focus window
        HWND  m_HWNDDeviceFullScreen;       // the main app device window in fullscreen mode
        HWND  m_HWNDDeviceWindowed;         // the main app device window in windowed mode
        HMONITOR m_AdapterMonitor;          // the monitor of the adapter 
        HMENU m_Menu;                       // handle to menu

        UINT m_FullScreenBackBufferWidthAtModeChange;  // back buffer size of fullscreen mode right before switching to windowed mode.  Used to restore to same resolution when toggling back to fullscreen
        UINT m_FullScreenBackBufferHeightAtModeChange; // back buffer size of fullscreen mode right before switching to windowed mode.  Used to restore to same resolution when toggling back to fullscreen
        UINT m_WindowBackBufferWidthAtModeChange;  // back buffer size of windowed mode right before switching to fullscreen mode.  Used to restore to same resolution when toggling back to windowed mode
        UINT m_WindowBackBufferHeightAtModeChange; // back buffer size of windowed mode right before switching to fullscreen mode.  Used to restore to same resolution when toggling back to windowed mode
        DWORD m_WindowedStyleAtModeChange;  // window style
        WINDOWPLACEMENT m_WindowedPlacement; // record of windowed HWND position/show state/etc
        bool  m_Minimized;                  // if true, the HWND is minimized
        bool  m_Maximized;                  // if true, the HWND is maximized
        bool  m_MinimizedWhileFullscreen;   // if true, the HWND is minimized due to a focus switch away when fullscreen mode
        bool  m_IgnoreSizeChange;           // if true, DXUT won't reset the device upon HWND size change

        double m_Time;                      // current time in seconds
        float m_ElapsedTime;                // time elapsed since last frame

        HINSTANCE m_HInstance;              // handle to the app instance
        double m_LastStatsUpdateTime;       // last time the stats were updated
        DWORD m_LastStatsUpdateFrames;      // frames count since last time the stats were updated
        float m_FPS;                        // frames per second
        int   m_CurrentFrameNumber;         // the current frame number
        HHOOK m_KeyboardHook;               // handle to keyboard hook
        bool  m_AllowShortcutKeysWhenFullscreen; // if true, when fullscreen enable shortcut keys (Windows keys, StickyKeys shortcut, ToggleKeys shortcut, FilterKeys shortcut) 
        bool  m_AllowShortcutKeysWhenWindowed;   // if true, when windowed enable shortcut keys (Windows keys, StickyKeys shortcut, ToggleKeys shortcut, FilterKeys shortcut) 
        bool  m_AllowShortcutKeys;          // if true, then shortcut keys are currently disabled (Windows key, etc)
        STICKYKEYS m_StartupStickyKeys;     // StickyKey settings upon startup so they can be restored later
        TOGGLEKEYS m_StartupToggleKeys;     // ToggleKey settings upon startup so they can be restored later
        FILTERKEYS m_StartupFilterKeys;     // FilterKey settings upon startup so they can be restored later

        bool  m_HandleDefaultHotkeys;       // if true, then DXUT will handle some default hotkeys
        bool  m_ShowMsgBoxOnError;          // if true, then msgboxes are displayed upon errors
        bool  m_ClipCursorWhenFullScreen;   // if true, then DXUT will keep the cursor from going outside the window when full screen
        bool  m_ShowCursorWhenFullScreen;   // if true, then DXUT will show a cursor when full screen
        bool  m_ConstantFrameTime;          // if true, then elapsed frame time will always be 0.05f seconds which is good for debugging or automated capture
        float m_TimePerFrame;               // the constant time per frame in seconds, only valid if m_ConstantFrameTime==true
        bool  m_WireframeMode;              // if true, then D3DRS_FILLMODE==D3DFILL_WIREFRAME else D3DRS_FILLMODE==D3DFILL_SOLID 
        bool  m_AutoChangeAdapter;          // if true, then the adapter will automatically change if the window is different monitor
        bool  m_WindowCreatedWithDefaultPositions; // if true, then CW_USEDEFAULT was used and the window should be moved to the right adapter
        int   m_ExitCode;                   // the exit code to be returned to the command line

        bool  m_DXUTInited;                 // if true, then DXUTInit() has succeeded
        bool  m_WindowCreated;              // if true, then DXUTCreateWindow() or DXUTSetWindow() has succeeded
        bool  m_DeviceCreated;              // if true, then DXUTCreateDevice*() or DXUTSetDevice() has succeeded

        bool  m_DXUTInitCalled;             // if true, then DXUTInit() was called
        bool  m_WindowCreateCalled;         // if true, then DXUTCreateWindow() or DXUTSetWindow() was called
        bool  m_DeviceCreateCalled;         // if true, then DXUTCreateDevice*() or DXUTSetDevice() was called

        bool  m_DeviceObjectsCreated;       // if true, then DeviceCreated callback has been called (if non-NULL)
        bool  m_DeviceObjectsReset;         // if true, then DeviceReset callback has been called (if non-NULL)
        bool  m_InsideDeviceCallback;       // if true, then the framework is inside an app device callback
        bool  m_InsideMainloop;             // if true, then the framework is inside the main loop
        bool  m_Active;                     // if true, then the app is the active top level window
        bool  m_TimePaused;                 // if true, then time is paused
        bool  m_RenderingPaused;            // if true, then rendering is paused
        int   m_PauseRenderingCount;        // pause rendering ref count
        int   m_PauseTimeCount;             // pause time ref count
        bool  m_DeviceLost;                 // if true, then the device is lost and needs to be reset
        bool  m_NotifyOnMouseMove;          // if true, include WM_MOUSEMOVE in mousecallback
        bool  m_Automation;                 // if true, automation is enabled
        bool  m_InSizeMove;                 // if true, app is inside a WM_ENTERSIZEMOVE

        int   m_OverrideAdapterOrdinal;     // if != -1, then override to use this adapter ordinal
        bool  m_OverrideWindowed;           // if true, then force to start windowed
        bool  m_OverrideFullScreen;         // if true, then force to start full screen
        int   m_OverrideStartX;             // if != -1, then override to this X position of the window
        int   m_OverrideStartY;             // if != -1, then override to this Y position of the window
        int   m_OverrideWidth;              // if != 0, then override to this width
        int   m_OverrideHeight;             // if != 0, then override to this height
        bool  m_OverrideForceHAL;           // if true, then force to HAL device (failing if one doesn't exist)
        bool  m_OverrideForceREF;           // if true, then force to REF device (failing if one doesn't exist)
        bool  m_OverrideForcePureHWVP;      // if true, then force to use pure HWVP (failing if device doesn't support it)
        bool  m_OverrideForceHWVP;          // if true, then force to use HWVP (failing if device doesn't support it)
        bool  m_OverrideForceSWVP;          // if true, then force to use SWVP 
        bool  m_OverrideConstantFrameTime;  // if true, then force to constant frame time
        float m_OverrideConstantTimePerFrame; // the constant time per frame in seconds if m_OverrideConstantFrameTime==true
        int   m_OverrideQuitAfterFrame;     // if != 0, then it will force the app to quit after that frame

        LPDXUTCALLBACKISDEVICEACCEPTABLE    m_IsDeviceAcceptableFunc;   // is device acceptable callback
        LPDXUTCALLBACKMODIFYDEVICESETTINGS  m_ModifyDeviceSettingsFunc; // modify device settings callback
        LPDXUTCALLBACKDEVICECREATED         m_DeviceCreatedFunc;        // device created callback
        LPDXUTCALLBACKDEVICERESET           m_DeviceResetFunc;          // device reset callback
        LPDXUTCALLBACKDEVICELOST            m_DeviceLostFunc;           // device lost callback
        LPDXUTCALLBACKDEVICEDESTROYED       m_DeviceDestroyedFunc;      // device destroyed callback
        LPDXUTCALLBACKFRAMEMOVE             m_FrameMoveFunc;            // frame move callback
        LPDXUTCALLBACKFRAMERENDER           m_FrameRenderFunc;          // frame render callback
        LPDXUTCALLBACKKEYBOARD              m_KeyboardFunc;             // keyboard callback
        LPDXUTCALLBACKMOUSE                 m_MouseFunc;                // mouse callback
        LPDXUTCALLBACKMSGPROC               m_WindowMsgFunc;            // window messages callback

        void*                               m_IsDeviceAcceptableFuncUserContext;   // user context for is device acceptable callback
        void*                               m_ModifyDeviceSettingsFuncUserContext; // user context for modify device settings callback
        void*                               m_DeviceCreatedUserContext;            // user context for device created callback
        void*                               m_DeviceCreatedFuncUserContext;        // user context for device created callback
        void*                               m_DeviceResetFuncUserContext;          // user context for device reset callback
        void*                               m_DeviceLostFuncUserContext;           // user context for device lost callback
        void*                               m_DeviceDestroyedFuncUserContext;      // user context for device destroyed callback
        void*                               m_FrameMoveFuncUserContext;            // user context for frame move callback
        void*                               m_FrameRenderFuncUserContext;          // user context for frame render callback
        void*                               m_KeyboardFuncUserContext;             // user context for keyboard callback
        void*                               m_MouseFuncUserContext;                // user context for mouse callback
        void*                               m_WindowMsgFuncUserContext;            // user context for window messages callback

        bool                         m_Keys[256];                       // array of key state
        bool                         m_MouseButtons[5];                 // array of mouse states

        CGrowableArray<DXUT_TIMER>*  m_TimerList;                       // list of DXUT_TIMER structs
        TCHAR                        m_StaticFrameStats[256];           // static part of frames stats 
        TCHAR                        m_FrameStats[256];                 // frame stats (fps, width, etc)
        TCHAR                        m_DeviceStats[256];                // device stats (description, device type, etc)
        TCHAR                        m_WindowTitle[256];                // window title
    };
    
    STATE m_state;

public:
    DXUTState()  { Create(); }
    ~DXUTState() { Destroy(); }

	// MATCH
    void Create()
    {
        // Make sure these are created before DXUTState so they 
        // destroyed last because DXUTState cleanup needs them
        DXUTGetGlobalResourceCache();

        ZeroMemory( &m_state, sizeof(STATE) ); 
        g_bThreadSafe = true; 
        InitializeCriticalSection( &g_cs ); 
        m_state.m_OverrideStartX = -1; 
        m_state.m_OverrideStartY = -1; 
        m_state.m_OverrideAdapterOrdinal = -1; 
        m_state.m_AutoChangeAdapter = true; 
        m_state.m_ShowMsgBoxOnError = true;
        m_state.m_AllowShortcutKeysWhenWindowed = true;
        m_state.m_Active = true;
    }

    void Destroy()
    {
        DXUTShutdown();
        DeleteCriticalSection( &g_cs ); 
    }

    // Macros to define access functions for thread safe access into m_state 
    GET_SET_ACCESSOR( IDirect3D9*, D3D );

    GET_SET_ACCESSOR( IDirect3DDevice9*, D3DDevice );
    GET_SET_ACCESSOR( CD3DEnumeration*, D3DEnumeration );   
    GET_SET_ACCESSOR( DXUTDeviceSettings*, CurrentDeviceSettings );   
    GETP_SETP_ACCESSOR( D3DSURFACE_DESC, BackBufferSurfaceDesc );
    GETP_SETP_ACCESSOR( D3DCAPS9, Caps );

    GET_SET_ACCESSOR( HWND, HWNDFocus );
    GET_SET_ACCESSOR( HWND, HWNDDeviceFullScreen );
    GET_SET_ACCESSOR( HWND, HWNDDeviceWindowed );
    GET_SET_ACCESSOR( HMONITOR, AdapterMonitor );
    GET_SET_ACCESSOR( HMENU, Menu );   

    GET_SET_ACCESSOR( UINT, FullScreenBackBufferWidthAtModeChange );
    GET_SET_ACCESSOR( UINT, FullScreenBackBufferHeightAtModeChange );
    GET_SET_ACCESSOR( UINT, WindowBackBufferWidthAtModeChange );
    GET_SET_ACCESSOR( UINT, WindowBackBufferHeightAtModeChange );
    GETP_SETP_ACCESSOR( WINDOWPLACEMENT, WindowedPlacement );
    GET_SET_ACCESSOR( DWORD, WindowedStyleAtModeChange );
    GET_SET_ACCESSOR( bool, Minimized );
    GET_SET_ACCESSOR( bool, Maximized );
    GET_SET_ACCESSOR( bool, MinimizedWhileFullscreen );
    GET_SET_ACCESSOR( bool, IgnoreSizeChange );   

    GET_SET_ACCESSOR( double, Time );
    GET_SET_ACCESSOR( float, ElapsedTime );

    GET_SET_ACCESSOR( HINSTANCE, HInstance );
    GET_SET_ACCESSOR( double, LastStatsUpdateTime );   
    GET_SET_ACCESSOR( DWORD, LastStatsUpdateFrames );   
    GET_SET_ACCESSOR( float, FPS );    
    GET_SET_ACCESSOR( int, CurrentFrameNumber );
    GET_SET_ACCESSOR( HHOOK, KeyboardHook );
    GET_SET_ACCESSOR( bool, AllowShortcutKeysWhenFullscreen );
    GET_SET_ACCESSOR( bool, AllowShortcutKeysWhenWindowed );
    GET_SET_ACCESSOR( bool, AllowShortcutKeys );
    GET_SET_ACCESSOR( STICKYKEYS, StartupStickyKeys );
    GET_SET_ACCESSOR( TOGGLEKEYS, StartupToggleKeys );
    GET_SET_ACCESSOR( FILTERKEYS, StartupFilterKeys );

    GET_SET_ACCESSOR( bool, HandleDefaultHotkeys );
    GET_SET_ACCESSOR( bool, ShowMsgBoxOnError );
    GET_SET_ACCESSOR( bool, ClipCursorWhenFullScreen );   
    GET_SET_ACCESSOR( bool, ShowCursorWhenFullScreen );
    GET_SET_ACCESSOR( bool, ConstantFrameTime );
    GET_SET_ACCESSOR( float, TimePerFrame );
    GET_SET_ACCESSOR( bool, WireframeMode );   
    GET_SET_ACCESSOR( bool, AutoChangeAdapter );
    GET_SET_ACCESSOR( bool, WindowCreatedWithDefaultPositions );
    GET_SET_ACCESSOR( int, ExitCode );

    GET_SET_ACCESSOR( bool, DXUTInited );
    GET_SET_ACCESSOR( bool, WindowCreated );
    GET_SET_ACCESSOR( bool, DeviceCreated );
    GET_SET_ACCESSOR( bool, DXUTInitCalled );
    GET_SET_ACCESSOR( bool, WindowCreateCalled );
    GET_SET_ACCESSOR( bool, DeviceCreateCalled );
    GET_SET_ACCESSOR( bool, InsideDeviceCallback );
    GET_SET_ACCESSOR( bool, InsideMainloop );
    GET_SET_ACCESSOR( bool, DeviceObjectsCreated );
    GET_SET_ACCESSOR( bool, DeviceObjectsReset );
    GET_SET_ACCESSOR( bool, Active );
    GET_SET_ACCESSOR( bool, RenderingPaused );
    GET_SET_ACCESSOR( bool, TimePaused );
    GET_SET_ACCESSOR( int, PauseRenderingCount );
    GET_SET_ACCESSOR( int, PauseTimeCount );
    GET_SET_ACCESSOR( bool, DeviceLost );
    GET_SET_ACCESSOR( bool, NotifyOnMouseMove );
    GET_SET_ACCESSOR( bool, Automation );
    GET_SET_ACCESSOR( bool, InSizeMove );

    GET_SET_ACCESSOR( int, OverrideAdapterOrdinal );
    GET_SET_ACCESSOR( bool, OverrideWindowed );
    GET_SET_ACCESSOR( bool, OverrideFullScreen );
    GET_SET_ACCESSOR( int, OverrideStartX );
    GET_SET_ACCESSOR( int, OverrideStartY );
    GET_SET_ACCESSOR( int, OverrideWidth );
    GET_SET_ACCESSOR( int, OverrideHeight );
    GET_SET_ACCESSOR( bool, OverrideForceHAL );
    GET_SET_ACCESSOR( bool, OverrideForceREF );
    GET_SET_ACCESSOR( bool, OverrideForcePureHWVP );
    GET_SET_ACCESSOR( bool, OverrideForceHWVP );
    GET_SET_ACCESSOR( bool, OverrideForceSWVP );
    GET_SET_ACCESSOR( bool, OverrideConstantFrameTime );
    GET_SET_ACCESSOR( float, OverrideConstantTimePerFrame );
    GET_SET_ACCESSOR( int, OverrideQuitAfterFrame );

    GET_SET_ACCESSOR( LPDXUTCALLBACKISDEVICEACCEPTABLE, IsDeviceAcceptableFunc );
    GET_SET_ACCESSOR( LPDXUTCALLBACKMODIFYDEVICESETTINGS, ModifyDeviceSettingsFunc );
    GET_SET_ACCESSOR( LPDXUTCALLBACKDEVICECREATED, DeviceCreatedFunc );
    GET_SET_ACCESSOR( LPDXUTCALLBACKDEVICERESET, DeviceResetFunc );
    GET_SET_ACCESSOR( LPDXUTCALLBACKDEVICELOST, DeviceLostFunc );
    GET_SET_ACCESSOR( LPDXUTCALLBACKDEVICEDESTROYED, DeviceDestroyedFunc );
    GET_SET_ACCESSOR( LPDXUTCALLBACKFRAMEMOVE, FrameMoveFunc );
    GET_SET_ACCESSOR( LPDXUTCALLBACKFRAMERENDER, FrameRenderFunc );
    GET_SET_ACCESSOR( LPDXUTCALLBACKKEYBOARD, KeyboardFunc );
    GET_SET_ACCESSOR( LPDXUTCALLBACKMOUSE, MouseFunc );
    GET_SET_ACCESSOR( LPDXUTCALLBACKMSGPROC, WindowMsgFunc );

    GET_SET_ACCESSOR( void*, IsDeviceAcceptableFuncUserContext );
    GET_SET_ACCESSOR( void*, ModifyDeviceSettingsFuncUserContext );
    GET_SET_ACCESSOR( void*, DeviceCreatedFuncUserContext );
    GET_SET_ACCESSOR( void*, DeviceResetFuncUserContext );
    GET_SET_ACCESSOR( void*, DeviceLostFuncUserContext );
    GET_SET_ACCESSOR( void*, DeviceDestroyedFuncUserContext );
    GET_SET_ACCESSOR( void*, FrameMoveFuncUserContext );
    GET_SET_ACCESSOR( void*, FrameRenderFuncUserContext );
    GET_SET_ACCESSOR( void*, KeyboardFuncUserContext );
    GET_SET_ACCESSOR( void*, MouseFuncUserContext );
    GET_SET_ACCESSOR( void*, WindowMsgFuncUserContext );

    GET_SET_ACCESSOR( CGrowableArray<DXUT_TIMER>*, TimerList );   
    GET_ACCESSOR( bool*, Keys );
    GET_ACCESSOR( bool*, MouseButtons );
    GET_ACCESSOR( TCHAR*, StaticFrameStats );
    GET_ACCESSOR( TCHAR*, FrameStats );
    GET_ACCESSOR( TCHAR*, DeviceStats );    
    GET_ACCESSOR( TCHAR*, WindowTitle );
};


//--------------------------------------------------------------------------------------
// Global state class
//--------------------------------------------------------------------------------------
DXUTState& GetDXUTState()
{
    // Using an accessor function gives control of the construction order
    static DXUTState state;
    return state;
}


//--------------------------------------------------------------------------------------

IDirect3D9* DXUTGetD3DObject()                      { return GetDXUTState().GetD3D(); }        
IDirect3DDevice9* DXUTGetD3DDevice()                { return GetDXUTState().GetD3DDevice(); }  
const D3DSURFACE_DESC* DXUTGetBackBufferSurfaceDesc() { return GetDXUTState().GetBackBufferSurfaceDesc(); }
const D3DCAPS9* DXUTGetDeviceCaps()                 { return GetDXUTState().GetCaps(); }
HINSTANCE DXUTGetHINSTANCE()                        { return GetDXUTState().GetHInstance(); }
HWND DXUTGetHWND()                                  { return DXUTIsWindowed() ? GetDXUTState().GetHWNDDeviceWindowed() : GetDXUTState().GetHWNDDeviceFullScreen(); }
HWND DXUTGetHWNDFocus()                             { return GetDXUTState().GetHWNDFocus(); }
HWND DXUTGetHWNDDeviceFullScreen()                  { return GetDXUTState().GetHWNDDeviceFullScreen(); }
HWND DXUTGetHWNDDeviceWindowed()                    { return GetDXUTState().GetHWNDDeviceWindowed(); }
RECT DXUTGetWindowClientRect()                      { RECT rc; GetClientRect( DXUTGetHWND(), &rc ); return rc; }
RECT DXUTGetWindowClientRectAtModeChange()          { RECT rc = { 0, 0, GetDXUTState().GetWindowBackBufferWidthAtModeChange(), GetDXUTState().GetWindowBackBufferHeightAtModeChange() }; return rc; }
RECT DXUTGetFullsceenClientRectAtModeChange()       { RECT rc = { 0, 0, GetDXUTState().GetFullScreenBackBufferWidthAtModeChange(), GetDXUTState().GetFullScreenBackBufferHeightAtModeChange() }; return rc; }
double DXUTGetTime()                                { return GetDXUTState().GetTime(); }
float DXUTGetElapsedTime()                          { return GetDXUTState().GetElapsedTime(); }
float DXUTGetFPS()                                  { return GetDXUTState().GetFPS(); }
LPCTSTR DXUTGetWindowTitle()                        { return GetDXUTState().GetWindowTitle(); }
LPCTSTR DXUTGetFrameStats()                         { return GetDXUTState().GetFrameStats(); }
LPCTSTR DXUTGetDeviceStats()                        { return GetDXUTState().GetDeviceStats(); }
bool DXUTIsRenderingPaused()                        { return GetDXUTState().GetPauseRenderingCount() > 0; }
bool DXUTIsTimePaused()                             { return GetDXUTState().GetPauseTimeCount() > 0; }
int DXUTGetExitCode()                               { return GetDXUTState().GetExitCode(); }
bool DXUTGetShowMsgBoxOnError()                     { return GetDXUTState().GetShowMsgBoxOnError(); }
bool DXUTGetAutomation()                            { return GetDXUTState().GetAutomation(); }

void DXUTShutdown( int nExitCode )
{
    HWND hWnd = DXUTGetHWND();
    if( hWnd != NULL )
        SendMessage( hWnd, WM_CLOSE, 0, 0 );

    GetDXUTState().SetExitCode(nExitCode);

    DXUTCleanup3DEnvironment( true );

    // Restore shortcut keys (Windows key, accessibility shortcuts) to original state
    // This is important to call here if the shortcuts are disabled, 
    // because accessibility setting changes are permanent.
    // This means that if this is not done then the accessibility settings 
    // might not be the same as when the app was started. 
    // If the app crashes without restoring the settings, this is also true so it
    // would be wise to backup/restore the settings from a file so they can be 
    // restored when the crashed app is run again.
    DXUTAllowShortcutKeys( true );
    
    GetDXUTState().SetD3DEnumeration( NULL );

    IDirect3D9* pD3D = DXUTGetD3DObject();
    SAFE_RELEASE( pD3D );
    GetDXUTState().SetD3D( NULL );
}
void DXUTCleanup3DEnvironment( bool bReleaseSettings )
{
    IDirect3DDevice9* pd3dDevice = DXUTGetD3DDevice();
    if( pd3dDevice != NULL )
    {
        GetDXUTState().SetInsideDeviceCallback( true );

        // Call the app's device lost callback
        if( GetDXUTState().GetDeviceObjectsReset() == true )
        {
            LPDXUTCALLBACKDEVICELOST pCallbackDeviceLost = GetDXUTState().GetDeviceLostFunc();
            if( pCallbackDeviceLost != NULL )
                pCallbackDeviceLost( GetDXUTState().GetDeviceLostFuncUserContext() );
            GetDXUTState().SetDeviceObjectsReset( false );

            // Call the resource cache device lost function
            DXUTGetGlobalResourceCache().OnLostDevice();
        }

        // Call the app's device destroyed callback
        if( GetDXUTState().GetDeviceObjectsCreated() == true )
        {
            LPDXUTCALLBACKDEVICEDESTROYED pCallbackDeviceDestroyed = GetDXUTState().GetDeviceDestroyedFunc();
            if( pCallbackDeviceDestroyed != NULL )
                pCallbackDeviceDestroyed( GetDXUTState().GetDeviceDestroyedFuncUserContext() );
            GetDXUTState().SetDeviceObjectsCreated( false );

            // Call the resource cache device destory function
            DXUTGetGlobalResourceCache().OnDestroyDevice();
        }

        GetDXUTState().SetInsideDeviceCallback( false );

        // Release the D3D device and in debug configs, displays a message box if there 
        // are unrelease objects.
        if( pd3dDevice )
        {
            if( pd3dDevice->Release() > 0 )  
            {
                DXUTDisplayErrorMessage( DXUTERR_NONZEROREFCOUNT );
                DXUT_ERR( "DXUTCleanup3DEnvironment", DXUTERR_NONZEROREFCOUNT );
            }
        }
        GetDXUTState().SetD3DDevice( NULL );

        if( bReleaseSettings )
        {
            DXUTDeviceSettings* pOldDeviceSettings = GetDXUTState().GetCurrentDeviceSettings();
            SAFE_DELETE(pOldDeviceSettings);  
            GetDXUTState().SetCurrentDeviceSettings( NULL );
        }

        D3DSURFACE_DESC* pbackBufferSurfaceDesc = GetDXUTState().GetBackBufferSurfaceDesc();
        ZeroMemory( pbackBufferSurfaceDesc, sizeof(D3DSURFACE_DESC) );

        D3DCAPS9* pd3dCaps = GetDXUTState().GetCaps();
        ZeroMemory( pd3dCaps, sizeof(D3DCAPS9) );

        GetDXUTState().SetDeviceCreated( false );
    }
}
void DXUTAllowShortcutKeys( bool bAllowKeys )
{
    GetDXUTState().SetAllowShortcutKeys( bAllowKeys );
}
void DXUTDisplayErrorMessage( HRESULT hr )
{
    TCHAR strBuffer[512];

    int nExitCode;
    bool bFound = true; 
    switch( hr )
    {
        case DXUTERR_NODIRECT3D:             nExitCode = 2; StringCchCopy( strBuffer, 512, "Could not initialize Direct3D. You may want to check that the latest version of DirectX is correctly installed on your system.  Also make sure that this program was compiled with header files that match the installed DirectX DLLs." ); break;
        case DXUTERR_INCORRECTVERSION:       nExitCode = 10; StringCchCopy( strBuffer, 512, "Incorrect version of Direct3D and/or D3DX." ); break;
        case DXUTERR_MEDIANOTFOUND:          nExitCode = 4; StringCchCopy( strBuffer, 512, "Could not find required media. Ensure that the DirectX SDK is correctly installed." ); break;
        case DXUTERR_NONZEROREFCOUNT:        nExitCode = 5; StringCchCopy( strBuffer, 512, "The D3D device has a non-zero reference count, meaning some objects were not released." ); break;
        case DXUTERR_CREATINGDEVICE:         nExitCode = 6; StringCchCopy( strBuffer, 512, "Failed creating the Direct3D device." ); break;
        case DXUTERR_RESETTINGDEVICE:        nExitCode = 7; StringCchCopy( strBuffer, 512, "Failed resetting the Direct3D device." ); break;
        case DXUTERR_CREATINGDEVICEOBJECTS:  nExitCode = 8; StringCchCopy( strBuffer, 512, "Failed creating Direct3D device objects." ); break;
        case DXUTERR_RESETTINGDEVICEOBJECTS: nExitCode = 9; StringCchCopy( strBuffer, 512, "Failed resetting Direct3D device objects." ); break;
        case DXUTERR_NOCOMPATIBLEDEVICES:    
            nExitCode = 3; 
            if( GetSystemMetrics(SM_REMOTESESSION) != 0 )
                StringCchCopy( strBuffer, 512, "Direct3D does not work over a remote session." ); 
            else
                StringCchCopy( strBuffer, 512, "Could not find any compatible Direct3D devices." ); 
            break;
        default: bFound = false; nExitCode = 1;break;
    }   

    GetDXUTState().SetExitCode(nExitCode);

    bool bShowMsgBoxOnError = GetDXUTState().GetShowMsgBoxOnError();
    if( bFound && bShowMsgBoxOnError )
    {
        if( DXUTGetWindowTitle()[0] == 0 )
            MessageBox( DXUTGetHWND(), strBuffer, "DirectX Application", MB_ICONERROR|MB_OK );
        else
            MessageBox( DXUTGetHWND(), strBuffer, DXUTGetWindowTitle(), MB_ICONERROR|MB_OK );
    }
}
bool DXUTIsWindowed()                               
{ 
    DXUTDeviceSettings* pDeviceSettings = GetDXUTState().GetCurrentDeviceSettings(); 
    if(pDeviceSettings) 
        return (pDeviceSettings->pp.Windowed != 0); 
    else 
        return false; 
}
bool DXUTGetCmdParam( TCHAR*& strCmdLine, TCHAR* strFlag, int nFlagLen )
{
    strCmdLine += nFlagLen;
    if( *strCmdLine == L':' )
    {       
        strCmdLine++; // Skip ':'

        // Place NULL terminator in strFlag after current token
        StringCchCopy( strFlag, 256, strCmdLine );
        TCHAR* strSpace = strFlag;
        while (*strSpace && (*strSpace > L' '))
            strSpace++;
        *strSpace = 0;
    
        // Update strCmdLine
        strCmdLine += strlen(strFlag);
        return true;
    }
    else
    {
        strFlag[0] = 0;
        return false;
    }
}
void DXUTSetConstantFrameTime( bool bEnabled, float fTimePerFrame ) 
{ 
    if( GetDXUTState().GetOverrideConstantFrameTime() ) 
    { 
        bEnabled = GetDXUTState().GetOverrideConstantFrameTime(); 
        fTimePerFrame = GetDXUTState().GetOverrideConstantTimePerFrame(); 
    } 
    GetDXUTState().SetConstantFrameTime(bEnabled); 
    GetDXUTState().SetTimePerFrame(fTimePerFrame); 
}
void DXUTParseCommandLine()
{
    TCHAR* strCmdLine = GetCommandLine();

    // Skip past program name (first token in command line).
    if (*strCmdLine == L'"')  // Check for and handle quoted program name
    {
        strCmdLine++;

        // Skip over until another double-quote or a null 
        while (*strCmdLine && (*strCmdLine != L'"'))
            strCmdLine++;

        // Skip over double-quote
        if (*strCmdLine == L'"')            
            strCmdLine++;    
    }
    else   
    {
        // First token wasn't a quote
        while (*strCmdLine > L' ')
            strCmdLine++;
    }

    for(;;)
    {
        // Skip past any white space preceding the next token
        while (*strCmdLine && (*strCmdLine <= L' '))
            strCmdLine++;
        if( *strCmdLine == 0 )
            break;

        TCHAR strFlag[256];
        int nFlagLen = 0;

        // Skip past the flag marker
        if( *strCmdLine == L'/' ||
            *strCmdLine == L'-' )
            strCmdLine++;

        // Compare the first N letters w/o regard to case
        StringCchCopy( strFlag, 256, "adapter" ); nFlagLen = (int) strlen(strFlag);
        if( _strnicmp( strCmdLine, strFlag, nFlagLen ) == 0 )
        {
            if( DXUTGetCmdParam( strCmdLine, strFlag, nFlagLen ) )
            {
                int nAdapter = atoi(strFlag);
                GetDXUTState().SetOverrideAdapterOrdinal( nAdapter );
            }
            continue;
        }

        StringCchCopy( strFlag, 256, "windowed" ); nFlagLen = (int) strlen(strFlag);
        if( _strnicmp( strCmdLine, strFlag, nFlagLen ) == 0 )
        {
            GetDXUTState().SetOverrideWindowed( true );
            strCmdLine += nFlagLen;
            continue;
        }

        StringCchCopy( strFlag, 256, "fullscreen" ); nFlagLen = (int) strlen(strFlag);
        if( _strnicmp( strCmdLine, strFlag, nFlagLen ) == 0 )
        {
            GetDXUTState().SetOverrideFullScreen( true );
            strCmdLine += nFlagLen;
            continue;
        }

        StringCchCopy( strFlag, 256, "forcehal" ); nFlagLen = (int) strlen(strFlag);
        if( _strnicmp( strCmdLine, strFlag, nFlagLen ) == 0 )
        {
            GetDXUTState().SetOverrideForceHAL( true );
            strCmdLine += nFlagLen;
            continue;
        }

        StringCchCopy( strFlag, 256, "forceref" ); nFlagLen = (int) strlen(strFlag);
        if( _strnicmp( strCmdLine, strFlag, nFlagLen ) == 0 )
        {
            GetDXUTState().SetOverrideForceREF( true );
            strCmdLine += nFlagLen;
            continue;
        }

        StringCchCopy( strFlag, 256, "forcepurehwvp" ); nFlagLen = (int) strlen(strFlag);
        if( _strnicmp( strCmdLine, strFlag, nFlagLen ) == 0 )
        {
            GetDXUTState().SetOverrideForcePureHWVP( true );
            strCmdLine += nFlagLen;
            continue;
        }

        StringCchCopy( strFlag, 256, "forcehwvp" ); nFlagLen = (int) strlen(strFlag);
        if( _strnicmp( strCmdLine, strFlag, nFlagLen ) == 0 )
        {
            GetDXUTState().SetOverrideForceHWVP( true );
            strCmdLine += nFlagLen;
            continue;
        }

        StringCchCopy( strFlag, 256, "forceswvp" ); nFlagLen = (int) strlen(strFlag);
        if( _strnicmp( strCmdLine, strFlag, nFlagLen ) == 0 )
        {
            GetDXUTState().SetOverrideForceSWVP( true );
            strCmdLine += nFlagLen;
            continue;
        }

        StringCchCopy( strFlag, 256, "width" ); nFlagLen = (int) strlen(strFlag);
        if( _strnicmp( strCmdLine, strFlag, nFlagLen ) == 0 )
        {
            if( DXUTGetCmdParam( strCmdLine, strFlag, nFlagLen ) )
            {
                int nWidth = atoi(strFlag);
                GetDXUTState().SetOverrideWidth( nWidth );
            }
            continue;
        }

        StringCchCopy( strFlag, 256, "height" ); nFlagLen = (int) strlen(strFlag);
        if( _strnicmp( strCmdLine, strFlag, nFlagLen ) == 0 )
        {
            if( DXUTGetCmdParam( strCmdLine, strFlag, nFlagLen ) )
            {
                int nHeight = atoi(strFlag);
                GetDXUTState().SetOverrideHeight( nHeight );
            }
            continue;
        }

        StringCchCopy( strFlag, 256, "startx" );
        nFlagLen = (int) strlen(strFlag);
        if( _strnicmp( strCmdLine, strFlag, nFlagLen ) == 0 )
        {
            if( DXUTGetCmdParam( strCmdLine, strFlag, nFlagLen ) )
            {
                int nX = atoi(strFlag);
                GetDXUTState().SetOverrideStartX( nX );
            }
            continue;
        }

        StringCchCopy( strFlag, 256, "starty" );
        nFlagLen = (int) strlen(strFlag);
        if( _strnicmp( strCmdLine, strFlag, nFlagLen ) == 0 )
        {
            if( DXUTGetCmdParam( strCmdLine, strFlag, nFlagLen ) )
            {
                int nY = atoi(strFlag);
                GetDXUTState().SetOverrideStartY( nY );
            }
            continue;
        }

        StringCchCopy( strFlag, 256, "constantframetime" ); nFlagLen = (int) strlen(strFlag);
        if( _strnicmp( strCmdLine, strFlag, nFlagLen ) == 0 )
        {
            float fTimePerFrame;
            if( DXUTGetCmdParam( strCmdLine, strFlag, nFlagLen ) )
                fTimePerFrame = (float)strtod( strFlag, NULL );
            else
                fTimePerFrame = 0.0333f;
            GetDXUTState().SetOverrideConstantFrameTime( true );
            GetDXUTState().SetOverrideConstantTimePerFrame( fTimePerFrame );
            DXUTSetConstantFrameTime( true, fTimePerFrame );
            continue;
        }

        StringCchCopy( strFlag, 256, "quitafterframe" ); nFlagLen = (int) strlen(strFlag);
        if( _strnicmp( strCmdLine, strFlag, nFlagLen ) == 0 )
        {
            if( DXUTGetCmdParam( strCmdLine, strFlag, nFlagLen ) )
            {
                int nFrame = atoi(strFlag);
                GetDXUTState().SetOverrideQuitAfterFrame( nFrame );
            }
            continue;
        }      
        
        StringCchCopy( strFlag, 256, "noerrormsgboxes" ); nFlagLen = (int) strlen(strFlag);
        if( _strnicmp( strCmdLine, strFlag, nFlagLen ) == 0 )
        {
            GetDXUTState().SetShowMsgBoxOnError( false );
            strCmdLine += nFlagLen;
            continue;
        }        

        StringCchCopy( strFlag, 256, "automation" ); nFlagLen = (int) strlen(strFlag);
        if( _strnicmp( strCmdLine, strFlag, nFlagLen ) == 0 )
        {
            GetDXUTState().SetAutomation( true );
            strCmdLine += nFlagLen;
            continue;
        }

        // Unrecognized flag
        StringCchCopy( strFlag, 256, strCmdLine ); 
        TCHAR* strSpace = strFlag;
        while (*strSpace && (*strSpace > L' '))
            strSpace++;
        *strSpace = 0;

        DXUTOutputDebugString( "Unrecognized flag: %s", strFlag );
        strCmdLine += strlen(strFlag);
    }
}
HRESULT DXUTInit( bool bParseCommandLine, bool bHandleDefaultHotkeys, bool bShowMsgBoxOnError )
{
    GetDXUTState().SetDXUTInitCalled( true );

    // Not always needed, but lets the app create GDI dialogs
    InitCommonControls();

    // Save the current sticky/toggle/filter key settings so DXUT can restore them later
    STICKYKEYS sk = {sizeof(STICKYKEYS), 0};
    SystemParametersInfo(SPI_GETSTICKYKEYS, sizeof(STICKYKEYS), &sk, 0);
    GetDXUTState().SetStartupStickyKeys( sk );

    TOGGLEKEYS tk = {sizeof(TOGGLEKEYS), 0};
    SystemParametersInfo(SPI_GETTOGGLEKEYS, sizeof(TOGGLEKEYS), &tk, 0);
    GetDXUTState().SetStartupToggleKeys( tk );

    FILTERKEYS fk = {sizeof(FILTERKEYS), 0};
    SystemParametersInfo(SPI_GETFILTERKEYS, sizeof(FILTERKEYS), &fk, 0);
    GetDXUTState().SetStartupFilterKeys( fk );

    // Increase the accuracy of Sleep() without needing to link to winmm.lib
    TCHAR wszPath[MAX_PATH+1];
    if( GetSystemDirectory( wszPath, MAX_PATH+1 ) )
    {
        StringCchCat( wszPath, MAX_PATH, "\\winmm.dll" );
        HINSTANCE hInstWinMM = LoadLibrary( wszPath );
        if( hInstWinMM ) 
        {
            LPTIMEBEGINPERIOD pTimeBeginPeriod = (LPTIMEBEGINPERIOD)GetProcAddress( hInstWinMM, "timeBeginPeriod" );
            if( NULL != pTimeBeginPeriod )
                pTimeBeginPeriod(1);

            FreeLibrary(hInstWinMM);
        }
    }

    GetDXUTState().SetShowMsgBoxOnError( bShowMsgBoxOnError );
    GetDXUTState().SetHandleDefaultHotkeys( bHandleDefaultHotkeys );

    if( bParseCommandLine )
        DXUTParseCommandLine();

    // Verify D3DX version
    if( !D3DXCheckVersion( D3D_SDK_VERSION, D3DX_SDK_VERSION ) )
    {
        DXUTDisplayErrorMessage( DXUTERR_INCORRECTVERSION );
        return DXUT_ERR( "D3DXCheckVersion", DXUTERR_INCORRECTVERSION );
    }

    // Create a Direct3D object if one has not already been created
    IDirect3D9* pD3D = DXUTGetD3DObject();
    if( pD3D == NULL )
    {
        // This may fail if DirectX 9 isn't installed
        // This may fail if the DirectX headers are out of sync with the installed DirectX DLLs
        pD3D = DXUT_Dynamic_Direct3DCreate9( D3D_SDK_VERSION );
        GetDXUTState().SetD3D( pD3D );
    }

    if( pD3D == NULL )
    {
        // If still NULL, then something went wrong
        DXUTDisplayErrorMessage( DXUTERR_NODIRECT3D );
        return DXUT_ERR( "Direct3DCreate9", DXUTERR_NODIRECT3D );
    }

    // Reset the timer
    DXUTGetGlobalTimer()->Reset();

    GetDXUTState().SetDXUTInited( true );

    return S_OK;
}
CD3DEnumeration* DXUTPrepareEnumerationObject( bool bEnumerate )
{
    // Create a new CD3DEnumeration object and enumerate all devices unless its already been done
    CD3DEnumeration* pd3dEnum = GetDXUTState().GetD3DEnumeration();
    if( pd3dEnum == NULL )
    {
        pd3dEnum = DXUTGetEnumeration(); 
        GetDXUTState().SetD3DEnumeration( pd3dEnum );

        bEnumerate = true;
    }

    if( bEnumerate )
    {
        // Enumerate for each adapter all of the supported display modes, 
        // device types, adapter formats, back buffer formats, window/full screen support, 
        // depth stencil formats, multisampling types/qualities, and presentations intervals.
        //
        // For each combination of device type (HAL/REF), adapter format, back buffer format, and
        // IsWindowed it will call the app's ConfirmDevice callback.  This allows the app
        // to reject or allow that combination based on its caps/etc.  It also allows the 
        // app to change the BehaviorFlags.  The BehaviorFlags defaults non-pure HWVP 
        // if supported otherwise it will default to SWVP, however the app can change this 
        // through the ConfirmDevice callback.
        IDirect3D9* pD3D = DXUTGetD3DObject();
        pd3dEnum->Enumerate( pD3D, GetDXUTState().GetIsDeviceAcceptableFunc(), GetDXUTState().GetIsDeviceAcceptableFuncUserContext() );
    }
    
    return pd3dEnum;
}
void DXUTUpdateBackBufferDesc()
{
    HRESULT hr;
    IDirect3DSurface9* pBackBuffer;
    hr = GetDXUTState().GetD3DDevice()->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
    D3DSURFACE_DESC* pBBufferSurfaceDesc = GetDXUTState().GetBackBufferSurfaceDesc();
    ZeroMemory( pBBufferSurfaceDesc, sizeof(D3DSURFACE_DESC) );
    if( SUCCEEDED(hr) )
    {
        pBackBuffer->GetDesc( pBBufferSurfaceDesc );
        SAFE_RELEASE( pBackBuffer );
    }
}
void DXUTUpdateDeviceStats( D3DDEVTYPE DeviceType, DWORD BehaviorFlags, D3DADAPTER_IDENTIFIER9* pAdapterIdentifier )
{
    // Store device description
    TCHAR* pstrDeviceStats = GetDXUTState().GetDeviceStats();
    if( DeviceType == D3DDEVTYPE_REF )
        StringCchCopy( pstrDeviceStats, 256, "REF" );
    else if( DeviceType == D3DDEVTYPE_HAL )
        StringCchCopy( pstrDeviceStats, 256, "HA" );
    else if( DeviceType == D3DDEVTYPE_SW )
        StringCchCopy( pstrDeviceStats, 256, "SW" );

    if( BehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING &&
        BehaviorFlags & D3DCREATE_PUREDEVICE )
    {
        if( DeviceType == D3DDEVTYPE_HAL )
            StringCchCat( pstrDeviceStats, 256, " (pure hw vp)" );
        else
            StringCchCat( pstrDeviceStats, 256, " (simulated pure hw vp)" );
    }
    else if( BehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING )
    {
        if( DeviceType == D3DDEVTYPE_HAL )
            StringCchCat( pstrDeviceStats, 256, " (hw vp)" );
        else
            StringCchCat( pstrDeviceStats, 256, " (simulated hw vp)" );
    }
    else if( BehaviorFlags & D3DCREATE_MIXED_VERTEXPROCESSING )
    {
        if( DeviceType == D3DDEVTYPE_HAL )
            StringCchCat( pstrDeviceStats, 256, " (mixed vp)" );
        else
            StringCchCat( pstrDeviceStats, 256, " (simulated mixed vp)" );
    }
    else if( BehaviorFlags & D3DCREATE_SOFTWARE_VERTEXPROCESSING )
    {
        StringCchCat( pstrDeviceStats, 256, " (sw vp)" );
    }

    if( DeviceType == D3DDEVTYPE_HAL )
    {
        // Be sure not to overflow m_strDeviceStats when appending the adapter 
        // description, since it can be long.  
        StringCchCat( pstrDeviceStats, 256, ": " );

        // Try to get a unique description from the CD3DEnumDeviceSettingsCombo
        DXUTDeviceSettings* pDeviceSettings = GetDXUTState().GetCurrentDeviceSettings();
        CD3DEnumeration* pd3dEnum = DXUTPrepareEnumerationObject();
        CD3DEnumDeviceSettingsCombo* pDeviceSettingsCombo = pd3dEnum->GetDeviceSettingsCombo( pDeviceSettings->AdapterOrdinal, pDeviceSettings->DeviceType, pDeviceSettings->AdapterFormat, pDeviceSettings->pp.BackBufferFormat, pDeviceSettings->pp.Windowed );
        if( pDeviceSettingsCombo )
        {
            StringCchCat( pstrDeviceStats, 256, pDeviceSettingsCombo->pAdapterInfo->szUniqueDescription );
        }
        else
        {
            const int cchDesc = sizeof(pAdapterIdentifier->Description);
            TCHAR szDescription[cchDesc];
            strcpy(szDescription, pAdapterIdentifier->Description);
            szDescription[cchDesc-1] = 0;
            StringCchCat( pstrDeviceStats, 256, szDescription );
        }
    }
}
void DXUTUpdateStaticFrameStats()
{
    DXUTDeviceSettings* pDeviceSettings = GetDXUTState().GetCurrentDeviceSettings();
    if( NULL == pDeviceSettings )
        return;
    CD3DEnumeration* pd3dEnum = DXUTPrepareEnumerationObject();
    if( NULL == pd3dEnum )
        return;

    CD3DEnumDeviceSettingsCombo* pDeviceSettingsCombo = pd3dEnum->GetDeviceSettingsCombo( pDeviceSettings->AdapterOrdinal, pDeviceSettings->DeviceType, pDeviceSettings->AdapterFormat, pDeviceSettings->pp.BackBufferFormat, pDeviceSettings->pp.Windowed );
    if( NULL == pDeviceSettingsCombo )
        return;

    TCHAR strFmt[100];
    D3DPRESENT_PARAMETERS* pPP = &pDeviceSettings->pp;

    if( pDeviceSettingsCombo->AdapterFormat == pDeviceSettingsCombo->BackBufferFormat )
    {
        StringCchCopy( strFmt, 100, DXUTD3DFormatToString( pDeviceSettingsCombo->AdapterFormat, false ) );
    }
    else
    {
        StringCchPrintf( strFmt, 100, "backbuf %s, adapter %s", 
            DXUTD3DFormatToString( pDeviceSettingsCombo->BackBufferFormat, false ), 
            DXUTD3DFormatToString( pDeviceSettingsCombo->AdapterFormat, false ) );
    }

    TCHAR strDepthFmt[100];
    if( pPP->EnableAutoDepthStencil )
    {
        StringCchPrintf( strDepthFmt, 100, " (%s)", DXUTD3DFormatToString( pPP->AutoDepthStencilFormat, false ) );
    }
    else
    {
        // No depth buffer
        strDepthFmt[0] = 0;
    }

    TCHAR strMultiSample[100];
    switch( pPP->MultiSampleType )
    {
        case D3DMULTISAMPLE_NONMASKABLE: StringCchCopy( strMultiSample, 100, " (Nonmaskable Multisample)" ); break;
        case D3DMULTISAMPLE_NONE:        StringCchCopy( strMultiSample, 100, "" ); break;
        default:                         StringCchPrintf( strMultiSample, 100, " (%dx Multisample)", pPP->MultiSampleType ); break;
    }

    TCHAR* pstrStaticFrameStats = GetDXUTState().GetStaticFrameStats();
    StringCchPrintf( pstrStaticFrameStats, 256, "%%.02f fps (%dx%d), %s%s%s", 
                pPP->BackBufferWidth, pPP->BackBufferHeight,
                strFmt, strDepthFmt, strMultiSample );
}
HRESULT DXUTCreate3DEnvironment( IDirect3DDevice9* pd3dDeviceFromApp )
{
    HRESULT hr = S_OK;

    IDirect3DDevice9* pd3dDevice = NULL;
    DXUTDeviceSettings* pNewDeviceSettings = GetDXUTState().GetCurrentDeviceSettings();

    // Only create a Direct3D device if one hasn't been supplied by the app
    if( pd3dDeviceFromApp == NULL )
    {
        // Try to create the device with the chosen settings
        IDirect3D9* pD3D = DXUTGetD3DObject();
        hr = pD3D->CreateDevice( pNewDeviceSettings->AdapterOrdinal, pNewDeviceSettings->DeviceType, 
                                DXUTGetHWNDFocus(), pNewDeviceSettings->BehaviorFlags,
                                &pNewDeviceSettings->pp, &pd3dDevice );
        if( FAILED(hr) )
        {
            DXUT_ERR( "CreateDevice", hr );
            return DXUTERR_CREATINGDEVICE;
        }
    }
    else
    {
        pd3dDeviceFromApp->AddRef();
        pd3dDevice = pd3dDeviceFromApp;
    }

    GetDXUTState().SetD3DDevice( pd3dDevice );

    // If switching to REF, set the exit code to 11.  If switching to HAL and exit code was 11, then set it back to 0.
    if( pNewDeviceSettings->DeviceType == D3DDEVTYPE_REF && GetDXUTState().GetExitCode() == 0 )
        GetDXUTState().SetExitCode(11);
    else if( pNewDeviceSettings->DeviceType == D3DDEVTYPE_HAL && GetDXUTState().GetExitCode() == 11 )
        GetDXUTState().SetExitCode(0);

    // Update back buffer desc before calling app's device callbacks
    DXUTUpdateBackBufferDesc();

    // Update the device stats text
    CD3DEnumeration* pd3dEnum = DXUTPrepareEnumerationObject();
    CD3DEnumAdapterInfo* pAdapterInfo = pd3dEnum->GetAdapterInfo( pNewDeviceSettings->AdapterOrdinal );
    DXUTUpdateDeviceStats( pNewDeviceSettings->DeviceType, 
                        pNewDeviceSettings->BehaviorFlags, 
                        &pAdapterInfo->AdapterIdentifier );

    // Call the resource cache created function
    hr = DXUTGetGlobalResourceCache().OnCreateDevice( pd3dDevice );
    if( FAILED(hr) )
        return DXUT_ERR( "OnCreateDevice", ( hr == DXUTERR_MEDIANOTFOUND ) ? DXUTERR_MEDIANOTFOUND : DXUTERR_CREATINGDEVICEOBJECTS );

    // Call the app's device created callback if non-NULL
    const D3DSURFACE_DESC* pbackBufferSurfaceDesc = DXUTGetBackBufferSurfaceDesc();
    GetDXUTState().SetInsideDeviceCallback( true );
    LPDXUTCALLBACKDEVICECREATED pCallbackDeviceCreated = GetDXUTState().GetDeviceCreatedFunc();
    hr = S_OK;
    if( pCallbackDeviceCreated != NULL )
        hr = pCallbackDeviceCreated( DXUTGetD3DDevice(), pbackBufferSurfaceDesc, GetDXUTState().GetDeviceCreatedFuncUserContext() );
    GetDXUTState().SetInsideDeviceCallback( false );
    if( DXUTGetD3DDevice() == NULL ) // Handle DXUTShutdown from inside callback
        return E_FAIL;
    if( FAILED(hr) )  
    {
        DXUT_ERR( "DeviceCreated callback", hr );        
        return ( hr == DXUTERR_MEDIANOTFOUND ) ? DXUTERR_MEDIANOTFOUND : DXUTERR_CREATINGDEVICEOBJECTS;
    }
    GetDXUTState().SetDeviceObjectsCreated( true );

    // Call the resource cache device reset function
    hr = DXUTGetGlobalResourceCache().OnResetDevice( pd3dDevice );
    if( FAILED(hr) )
        return DXUT_ERR( "OnResetDevice", DXUTERR_RESETTINGDEVICEOBJECTS );

    // Call the app's device reset callback if non-NULL
    GetDXUTState().SetInsideDeviceCallback( true );
    LPDXUTCALLBACKDEVICERESET pCallbackDeviceReset = GetDXUTState().GetDeviceResetFunc();
    hr = S_OK;
    if( pCallbackDeviceReset != NULL )
        hr = pCallbackDeviceReset( DXUTGetD3DDevice(), pbackBufferSurfaceDesc, GetDXUTState().GetDeviceResetFuncUserContext() );
    GetDXUTState().SetInsideDeviceCallback( false );
    if( DXUTGetD3DDevice() == NULL ) // Handle DXUTShutdown from inside callback
        return E_FAIL;
    if( FAILED(hr) )
    {
        DXUT_ERR( "DeviceReset callback", hr );
        return ( hr == DXUTERR_MEDIANOTFOUND ) ? DXUTERR_MEDIANOTFOUND : DXUTERR_RESETTINGDEVICEOBJECTS;
    }
    GetDXUTState().SetDeviceObjectsReset( true );

    return S_OK;
}
HRESULT DXUTReset3DEnvironment()
{
    HRESULT hr;

    IDirect3DDevice9* pd3dDevice = DXUTGetD3DDevice();
    assert( pd3dDevice != NULL );

    // Call the app's device lost callback
    if( GetDXUTState().GetDeviceObjectsReset() == true )
    {
        GetDXUTState().SetInsideDeviceCallback( true );
        LPDXUTCALLBACKDEVICELOST pCallbackDeviceLost = GetDXUTState().GetDeviceLostFunc();
        if( pCallbackDeviceLost != NULL )
            pCallbackDeviceLost( GetDXUTState().GetDeviceLostFuncUserContext() );
        GetDXUTState().SetDeviceObjectsReset( false );
        GetDXUTState().SetInsideDeviceCallback( false );

        // Call the resource cache device lost function
        DXUTGetGlobalResourceCache().OnLostDevice();
    }

    // Reset the device
    DXUTDeviceSettings* pDeviceSettings = GetDXUTState().GetCurrentDeviceSettings();
    hr = pd3dDevice->Reset( &pDeviceSettings->pp );
    if( FAILED(hr) )  
    {
        if( hr == D3DERR_DEVICELOST )
            return D3DERR_DEVICELOST; // Reset could legitimately fail if the device is lost
        else
            return DXUT_ERR( "Reset", DXUTERR_RESETTINGDEVICE );
    }

    // Update back buffer desc before calling app's device callbacks
    DXUTUpdateBackBufferDesc();

    hr = DXUTGetGlobalResourceCache().OnResetDevice( pd3dDevice );
    if( FAILED(hr) )
        return DXUT_ERR( "OnResetDevice", DXUTERR_RESETTINGDEVICEOBJECTS );

    // Call the app's OnDeviceReset callback
    GetDXUTState().SetInsideDeviceCallback( true );
    const D3DSURFACE_DESC* pbackBufferSurfaceDesc = DXUTGetBackBufferSurfaceDesc();
    LPDXUTCALLBACKDEVICERESET pCallbackDeviceReset = GetDXUTState().GetDeviceResetFunc();
    hr = S_OK;
    if( pCallbackDeviceReset != NULL )
        hr = pCallbackDeviceReset( pd3dDevice, pbackBufferSurfaceDesc, GetDXUTState().GetDeviceResetFuncUserContext() );
    GetDXUTState().SetInsideDeviceCallback( false );
    if( FAILED(hr) )
    {
        // If callback failed, cleanup
        DXUT_ERR( "DeviceResetCallback", hr );
        if( hr != DXUTERR_MEDIANOTFOUND )
            hr = DXUTERR_RESETTINGDEVICEOBJECTS;

        GetDXUTState().SetInsideDeviceCallback( true );
        LPDXUTCALLBACKDEVICELOST pCallbackDeviceLost = GetDXUTState().GetDeviceLostFunc();
        if( pCallbackDeviceLost != NULL )
            pCallbackDeviceLost( GetDXUTState().GetDeviceLostFuncUserContext() );
        GetDXUTState().SetInsideDeviceCallback( false );

        DXUTGetGlobalResourceCache().OnLostDevice();       
        return hr;
    }

    // Success
    GetDXUTState().SetDeviceObjectsReset( true );

    return S_OK;
}
void DXUTPause( bool bPauseTime, bool bPauseRendering )
{
    int nPauseTimeCount = GetDXUTState().GetPauseTimeCount();
    nPauseTimeCount += ( bPauseTime ? +1 : -1 );
    if( nPauseTimeCount < 0 )
        nPauseTimeCount = 0;
    GetDXUTState().SetPauseTimeCount( nPauseTimeCount );

    int nPauseRenderingCount = GetDXUTState().GetPauseRenderingCount();
    nPauseRenderingCount += ( bPauseRendering ? +1 : -1 );
    if( nPauseRenderingCount < 0 )
        nPauseRenderingCount = 0;
    GetDXUTState().SetPauseRenderingCount( nPauseRenderingCount );

    if( nPauseTimeCount > 0 )
    {
        // Stop the scene from animating
        DXUTGetGlobalTimer()->Stop();
    }
    else
    {
        // Restart the timer
        DXUTGetGlobalTimer()->Start();
    }

    GetDXUTState().SetRenderingPaused( nPauseRenderingCount > 0 );
    GetDXUTState().SetTimePaused( nPauseTimeCount > 0 );
}
DXUTDeviceSettings DXUTGetDeviceSettings()   
{ 
    DXUTDeviceSettings* pDS = GetDXUTState().GetCurrentDeviceSettings();
    if( pDS )
    {
        return *pDS;
    }
    else
    {
        DXUTDeviceSettings ds;
        ZeroMemory( &ds, sizeof(DXUTDeviceSettings) );
        return ds;
    }
}

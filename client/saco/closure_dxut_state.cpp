// Complete pinned framework state/lifetime and public accessors.
#include "d3d9/common/dxstdafx.h"
#include <strsafe.h>
#undef GetSystemMetrics
#define DXUT_MIN_WINDOW_SIZE_X 200
#define DXUT_MIN_WINDOW_SIZE_Y 200
typedef IDirect3D9* (WINAPI* LPDIRECT3DCREATE9)(UINT SDKVersion);
typedef DECLSPEC_IMPORT UINT (WINAPI* LPTIMEBEGINPERIOD)( UINT uPeriod );
int     DXUTMapButtonToArrayIndex( BYTE vButton );
void    DXUTParseCommandLine();
CD3DEnumeration* DXUTPrepareEnumerationObject( bool bEnumerate = false );
void    DXUTBuildOptimalDeviceSettings( DXUTDeviceSettings* pOptimalDeviceSettings, DXUTDeviceSettings* pDeviceSettingsIn, DXUTMatchOptions* pMatchOptions );
bool    DXUTDoesDeviceComboMatchPreserveOptions( CD3DEnumDeviceSettingsCombo* pDeviceSettingsCombo, DXUTDeviceSettings* pDeviceSettingsIn, DXUTMatchOptions* pMatchOptions );
float   DXUTRankDeviceCombo( CD3DEnumDeviceSettingsCombo* pDeviceSettingsCombo, DXUTDeviceSettings* pDeviceSettingsIn, D3DDISPLAYMODE* pAdapterDesktopDisplayMode );
void    DXUTBuildValidDeviceSettings( DXUTDeviceSettings* pDeviceSettings, CD3DEnumDeviceSettingsCombo* pBestDeviceSettingsCombo, DXUTDeviceSettings* pDeviceSettingsIn, DXUTMatchOptions* pMatchOptions );
HRESULT DXUTFindValidResolution( CD3DEnumDeviceSettingsCombo* pBestDeviceSettingsCombo, D3DDISPLAYMODE displayModeIn, D3DDISPLAYMODE* pBestDisplayMode );
HRESULT DXUTFindAdapterFormat( UINT AdapterOrdinal, D3DDEVTYPE DeviceType, D3DFORMAT BackBufferFormat, BOOL Windowed, D3DFORMAT* pAdapterFormat );
HRESULT DXUTChangeDevice( DXUTDeviceSettings* pNewDeviceSettings, IDirect3DDevice9* pd3dDeviceFromApp, bool bForceRecreate, bool bClipWindowToSingleAdapter );
void    DXUTUpdateDeviceSettingsWithOverrides( DXUTDeviceSettings* pNewDeviceSettings );
HRESULT DXUTCreate3DEnvironment( IDirect3DDevice9* pd3dDeviceFromApp );
HRESULT DXUTReset3DEnvironment();
void    DXUTRender3DEnvironment();
void    DXUTCleanup3DEnvironment( bool bReleaseSettings = true );
void    DXUTUpdateFrameStats();
void    DXUTUpdateDeviceStats( D3DDEVTYPE DeviceType, DWORD BehaviorFlags, D3DADAPTER_IDENTIFIER9* pAdapterIdentifier );
void    DXUTUpdateStaticFrameStats();
void    DXUTHandleTimers();
bool    DXUTGetCmdParam( TCHAR*& strCmdLine, TCHAR* strFlag, int nFlagLen );
void    DXUTDisplayErrorMessage( HRESULT hr );
LRESULT CALLBACK DXUTStaticWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
void    DXUTCheckForWindowSizeChange();
void    DXUTCheckForWindowChangingMonitors();
UINT    DXUTColorChannelBits( D3DFORMAT fmt );
UINT    DXUTStencilBits( D3DFORMAT fmt );
UINT    DXUTDepthBits( D3DFORMAT fmt );
HRESULT DXUTGetAdapterOrdinalFromMonitor( HMONITOR hMonitor, UINT* pAdapterOrdinal );
void    DXUTAllowShortcutKeys( bool bAllowKeys );
void    DXUTUpdateBackBufferDesc();
void    DXUTInitHWCursor();
HRESULT DXUTSetDeviceCursor( IDirect3DDevice9* pd3dDevice, HCURSOR hCursor, bool bAddWatermark );
void    DXUTGetDesktopResolution( UINT AdapterOrdinal, DWORD* pdwWidth, DWORD* pdwHeight );


//--------------------------------------------------------------------------------------

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
void DXUTUpdateDeviceSettingsWithOverrides( DXUTDeviceSettings* pDeviceSettings )
{
    if( GetDXUTState().GetOverrideAdapterOrdinal() != -1 )
        pDeviceSettings->AdapterOrdinal = GetDXUTState().GetOverrideAdapterOrdinal();

    if( GetDXUTState().GetOverrideFullScreen() )
        pDeviceSettings->pp.Windowed = false;
    if( GetDXUTState().GetOverrideWindowed() )
        pDeviceSettings->pp.Windowed = true;

    if( GetDXUTState().GetOverrideForceREF() )
        pDeviceSettings->DeviceType = D3DDEVTYPE_REF;
    else if( GetDXUTState().GetOverrideForceHAL() )
        pDeviceSettings->DeviceType = D3DDEVTYPE_HAL;

    if( GetDXUTState().GetOverrideWidth() != 0 )
        pDeviceSettings->pp.BackBufferWidth = GetDXUTState().GetOverrideWidth();
    if( GetDXUTState().GetOverrideHeight() != 0 )
        pDeviceSettings->pp.BackBufferHeight = GetDXUTState().GetOverrideHeight();

    if( GetDXUTState().GetOverrideForcePureHWVP() )
    {
        pDeviceSettings->BehaviorFlags &= ~D3DCREATE_SOFTWARE_VERTEXPROCESSING;
        pDeviceSettings->BehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
        pDeviceSettings->BehaviorFlags |= D3DCREATE_PUREDEVICE;
    }
    else if( GetDXUTState().GetOverrideForceHWVP() )
    {
        pDeviceSettings->BehaviorFlags &= ~D3DCREATE_SOFTWARE_VERTEXPROCESSING;
        pDeviceSettings->BehaviorFlags &= ~D3DCREATE_PUREDEVICE;
        pDeviceSettings->BehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
    }
    else if( GetDXUTState().GetOverrideForceSWVP() )
    {
        pDeviceSettings->BehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
        pDeviceSettings->BehaviorFlags &= ~D3DCREATE_PUREDEVICE;
        pDeviceSettings->BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    }
}
void DXUTInitHWCursor()
{
    // Set up the full screen cursor 
    if( !DXUTIsWindowed() )
    {
        HCURSOR hCursor = (HCURSOR)(ULONG_PTR)GetClassLongPtr( DXUTGetHWNDDeviceFullScreen(), GCLP_HCURSOR );
        DXUTSetDeviceCursor( GetDXUTState().GetD3DDevice(), hCursor, false );
        DXUTGetD3DDevice()->ShowCursor( true );

        // Confine cursor to full screen window
        RECT rcWindow;
        GetWindowRect( DXUTGetHWNDDeviceFullScreen(), &rcWindow );
        ClipCursor( &rcWindow );
    }
    else
    {
        ClipCursor( NULL );
    }
}
void DXUTGetDesktopResolution( UINT AdapterOrdinal, DWORD* pdwWidth, DWORD* pdwHeight )
{
    CD3DEnumeration* pd3dEnum = DXUTPrepareEnumerationObject();
    CD3DEnumAdapterInfo* pAdapterInfo = pd3dEnum->GetAdapterInfo( AdapterOrdinal );                       
    DEVMODE devMode;
    ZeroMemory( &devMode, sizeof(DEVMODE) );
    devMode.dmSize = sizeof(DEVMODE);
    TCHAR strDeviceName[256];
    strcpy(strDeviceName,pAdapterInfo->AdapterIdentifier.DeviceName);
    strDeviceName[255] = 0;
    EnumDisplaySettings( strDeviceName, ENUM_REGISTRY_SETTINGS, &devMode );
    *pdwWidth = devMode.dmPelsWidth;
    *pdwHeight = devMode.dmPelsHeight;
}
HRESULT DXUTChangeDevice( DXUTDeviceSettings* pNewDeviceSettings, IDirect3DDevice9* pd3dDeviceFromApp, bool bForceRecreate, bool bClipWindowToSingleAdapter )
{
    HRESULT hr;
    DXUTDeviceSettings* pOldDeviceSettings = GetDXUTState().GetCurrentDeviceSettings();

    if( DXUTGetD3DObject() == NULL )
        return S_FALSE;

    // Make a copy of the pNewDeviceSettings on the heap
    DXUTDeviceSettings* pNewDeviceSettingsOnHeap = new DXUTDeviceSettings;
    if( pNewDeviceSettingsOnHeap == NULL )
        return E_OUTOFMEMORY;
    memcpy( pNewDeviceSettingsOnHeap, pNewDeviceSettings, sizeof(DXUTDeviceSettings) );
    pNewDeviceSettings = pNewDeviceSettingsOnHeap;

    // If the ModifyDeviceSettings callback is non-NULL, then call it to let the app 
    // change the settings or reject the device change by returning false.
    LPDXUTCALLBACKMODIFYDEVICESETTINGS pCallbackModifyDeviceSettings = GetDXUTState().GetModifyDeviceSettingsFunc();
    if( pCallbackModifyDeviceSettings )
    {
        D3DCAPS9 caps;
        IDirect3D9* pD3D = DXUTGetD3DObject();
        pD3D->GetDeviceCaps( pNewDeviceSettings->AdapterOrdinal, pNewDeviceSettings->DeviceType, &caps );

        bool bContinue = pCallbackModifyDeviceSettings( pNewDeviceSettings, &caps, GetDXUTState().GetModifyDeviceSettingsFuncUserContext() );
        if( !bContinue )
        {
            // The app rejected the device change by returning false, so just use the current device if there is one.
            if( pOldDeviceSettings == NULL )
                DXUTDisplayErrorMessage( DXUTERR_NOCOMPATIBLEDEVICES );
            SAFE_DELETE( pNewDeviceSettings );
            return E_ABORT;
        }
        if( GetDXUTState().GetD3D() == NULL )
        {
            SAFE_DELETE( pNewDeviceSettings );
            return S_FALSE;
        }
    }

    GetDXUTState().SetCurrentDeviceSettings( pNewDeviceSettings );

    DXUTPause( true, true );

    // When a WM_SIZE message is received, it calls DXUTCheckForWindowSizeChange().
    // A WM_SIZE message might be sent when adjusting the window, so tell 
    // DXUTCheckForWindowSizeChange() to ignore size changes temporarily
    GetDXUTState().SetIgnoreSizeChange( true );

    // Update thread safety on/off depending on Direct3D device's thread safety
    g_bThreadSafe = ((pNewDeviceSettings->BehaviorFlags & D3DCREATE_MULTITHREADED) != 0 );

    // Only apply the cmd line overrides if this is the first device created
    // and DXUTSetDevice() isn't used
    if( NULL == pd3dDeviceFromApp && NULL == pOldDeviceSettings )
    {
        // Updates the device settings struct based on the cmd line args.  
        // Warning: if the device doesn't support these new settings then CreateDevice() will fail.
        DXUTUpdateDeviceSettingsWithOverrides( pNewDeviceSettings );
    }

    // Take note if the backbuffer width & height are 0 now as they will change after pd3dDevice->Reset()
    bool bKeepCurrentWindowSize = false;
    if( pNewDeviceSettings->pp.BackBufferWidth == 0 && pNewDeviceSettings->pp.BackBufferHeight == 0 )
        bKeepCurrentWindowSize = true;

    //////////////////////////
    // Before reset
    /////////////////////////
    if( pNewDeviceSettings->pp.Windowed )
    {
        // Going to windowed mode

        if( pOldDeviceSettings && !pOldDeviceSettings->pp.Windowed )
        {
            // Going from fullscreen -> windowed
            GetDXUTState().SetFullScreenBackBufferWidthAtModeChange( pOldDeviceSettings->pp.BackBufferWidth );
            GetDXUTState().SetFullScreenBackBufferHeightAtModeChange( pOldDeviceSettings->pp.BackBufferHeight );

            // Restore windowed mode style
            SetWindowLongW( DXUTGetHWNDDeviceWindowed(), GWL_STYLE, GetDXUTState().GetWindowedStyleAtModeChange() );
        }

        // If different device windows are used for windowed mode and fullscreen mode,
        // hide the fullscreen window so that it doesn't obscure the screen.
        HWND hWindowedForCompare1 = DXUTGetHWNDDeviceWindowed();
        if( DXUTGetHWNDDeviceFullScreen() != hWindowedForCompare1 )
            ShowWindow( DXUTGetHWNDDeviceFullScreen(), SW_HIDE );

        // If using the same window for windowed and fullscreen mode, reattach menu if one exists
        HWND hWindowedForCompare2 = DXUTGetHWNDDeviceWindowed();
        if( DXUTGetHWNDDeviceFullScreen() == hWindowedForCompare2 )
        {
            if( GetDXUTState().GetMenu() != NULL )
                SetMenu( DXUTGetHWNDDeviceWindowed(), GetDXUTState().GetMenu() );
        }
    }
    else 
    {
        // Going to fullscreen mode

        if( pOldDeviceSettings == NULL || (pOldDeviceSettings && pOldDeviceSettings->pp.Windowed) )
        {
            // Transistioning to full screen mode from a standard window so 
            // save current window position/size/style now in case the user toggles to windowed mode later 
            WINDOWPLACEMENT* pwp = GetDXUTState().GetWindowedPlacement();
            ZeroMemory( pwp, sizeof(WINDOWPLACEMENT) );
            pwp->length = sizeof(WINDOWPLACEMENT);
            GetWindowPlacement( DXUTGetHWNDDeviceWindowed(), pwp );
            DWORD dwStyle = GetWindowLong( DXUTGetHWNDDeviceWindowed(), GWL_STYLE );
            dwStyle &= ~WS_MAXIMIZE & ~WS_MINIMIZE; // remove minimize/maximize style
            GetDXUTState().SetWindowedStyleAtModeChange( dwStyle );
            if( pOldDeviceSettings )
            {
                GetDXUTState().SetWindowBackBufferWidthAtModeChange( pOldDeviceSettings->pp.BackBufferWidth );
                GetDXUTState().SetWindowBackBufferHeightAtModeChange( pOldDeviceSettings->pp.BackBufferHeight );
            }
        }

        // Hide the window to avoid animation of blank windows
        ShowWindow( DXUTGetHWNDDeviceFullScreen(), SW_HIDE );

        // Set FS window style
        SetWindowLongW( DXUTGetHWNDDeviceFullScreen(), GWL_STYLE, WS_POPUP|WS_SYSMENU );

        // If using the same window for windowed and fullscreen mode, save and remove menu 
        HWND hWindowedForCompare3 = DXUTGetHWNDDeviceWindowed();
        if( DXUTGetHWNDDeviceFullScreen() == hWindowedForCompare3 )
        {
            HMENU hMenu = GetMenu( DXUTGetHWNDDeviceFullScreen() );
            GetDXUTState().SetMenu( hMenu );
            SetMenu( DXUTGetHWNDDeviceFullScreen(), NULL );
        }

        // Restore the window to normal.  Note that if the window was maximized then minimized, the
        // WPF_RESTORETOMAXIMIZED flag will be set which will cause SW_RESTORE to restore the 
        // window from minimized to maxmized which isn't what we want right now.
        WINDOWPLACEMENT wpFullscreen;
        ZeroMemory( &wpFullscreen, sizeof(WINDOWPLACEMENT) );
        wpFullscreen.length = sizeof(WINDOWPLACEMENT);
        GetWindowPlacement( DXUTGetHWNDDeviceFullScreen(), &wpFullscreen );
        wpFullscreen.flags &= ~WPF_RESTORETOMAXIMIZED;
        wpFullscreen.showCmd = SW_RESTORE;
        SetWindowPlacement( DXUTGetHWNDDeviceFullScreen(), &wpFullscreen );
    }

    // If AdapterOrdinal and DeviceType are the same, we can just do a Reset().
    // If they've changed, we need to do a complete device tear down/rebuild.
    // Also only allow a reset if pd3dDevice is the same as the current device 
    if( !bForceRecreate && 
        (pd3dDeviceFromApp == NULL || pd3dDeviceFromApp == DXUTGetD3DDevice()) && 
        pOldDeviceSettings &&
        pOldDeviceSettings->AdapterOrdinal == pNewDeviceSettings->AdapterOrdinal &&
        pOldDeviceSettings->DeviceType == pNewDeviceSettings->DeviceType &&
        pOldDeviceSettings->BehaviorFlags == pNewDeviceSettings->BehaviorFlags )
    {
        // Reset the Direct3D device and call the app's device callbacks
        hr = DXUTReset3DEnvironment();
        if( FAILED(hr) )
        {
            if( D3DERR_DEVICELOST == hr )
            {
                // The device is lost, just mark it as so and continue on with 
                // capturing the state and resizing the window/etc.
                GetDXUTState().SetDeviceLost( true );
            }
            else if( DXUTERR_RESETTINGDEVICEOBJECTS == hr || 
                     DXUTERR_MEDIANOTFOUND == hr )
            {
                // Something bad happened in the app callbacks
                SAFE_DELETE( pOldDeviceSettings );
                DXUTDisplayErrorMessage( hr );
                DXUTShutdown();
                return hr;
            }
            else // DXUTERR_RESETTINGDEVICE
            {
                // Reset failed and the device wasn't lost and it wasn't the apps fault, 
                // so recreate the device to try to recover
                GetDXUTState().SetCurrentDeviceSettings( pOldDeviceSettings );
                if( FAILED( DXUTChangeDevice( pNewDeviceSettings, pd3dDeviceFromApp, true, bClipWindowToSingleAdapter ) ) )
                {
                    // If that fails, then shutdown
                    SAFE_DELETE( pOldDeviceSettings );
                    DXUTShutdown();
                    return DXUTERR_CREATINGDEVICE;
                }
                else
                {
                    SAFE_DELETE( pOldDeviceSettings );
                    return S_OK;
                }
            }
        }
    }
    else
    {
        // Cleanup if not first device created
        if( pOldDeviceSettings )
            DXUTCleanup3DEnvironment( false );

        // Create the D3D device and call the app's device callbacks
        hr = DXUTCreate3DEnvironment( pd3dDeviceFromApp );
        if( FAILED(hr) )
        {
            SAFE_DELETE( pOldDeviceSettings );
            DXUTCleanup3DEnvironment();
            DXUTDisplayErrorMessage( hr );
            DXUTPause( false, false );
            GetDXUTState().SetIgnoreSizeChange( false );
            return hr;
        }
    }

    // Enable/disable StickKeys shortcut, ToggleKeys shortcut, FilterKeys shortcut, and Windows key 
    // to prevent accidental task switching
    DXUTAllowShortcutKeys( ( pNewDeviceSettings->pp.Windowed  ) ? GetDXUTState().GetAllowShortcutKeysWhenWindowed() : GetDXUTState().GetAllowShortcutKeysWhenFullscreen() );

    IDirect3D9* pD3D = DXUTGetD3DObject();
    HMONITOR hAdapterMonitor = pD3D->GetAdapterMonitor( pNewDeviceSettings->AdapterOrdinal );
    GetDXUTState().SetAdapterMonitor( hAdapterMonitor );

    // Update the device stats text
    DXUTUpdateStaticFrameStats();

    // Update GetDXUTState()'s copy of D3D caps 
    D3DCAPS9* pd3dCaps = GetDXUTState().GetCaps();
    DXUTGetD3DDevice()->GetDeviceCaps( pd3dCaps );

    if( pOldDeviceSettings && !pOldDeviceSettings->pp.Windowed && pNewDeviceSettings->pp.Windowed )
    {
        // Going from fullscreen -> windowed

        // Restore the show state, and positions/size of the window to what it was
        // It is important to adjust the window size 
        // after resetting the device rather than beforehand to ensure 
        // that the monitor resolution is correct and does not limit the size of the new window.
        WINDOWPLACEMENT* pwp = GetDXUTState().GetWindowedPlacement();
        SetWindowPlacement( DXUTGetHWNDDeviceWindowed(), pwp );
    }

    // Check to see if the window needs to be resized.  
    // Handle cases where the window is minimized and maxmimized as well.
    bool bNeedToResize = false;
    if( pNewDeviceSettings->pp.Windowed && // only resize if in windowed mode
        !bKeepCurrentWindowSize )          // only resize if pp.BackbufferWidth/Height were not 0
    {
        UINT nClientWidth;
        UINT nClientHeight;    
        if( IsIconic(DXUTGetHWNDDeviceWindowed()) )
        {
            // Window is currently minimized. To tell if it needs to resize, 
            // get the client rect of window when its restored the 
            // hard way using GetWindowPlacement()
            WINDOWPLACEMENT wp;
            ZeroMemory( &wp, sizeof(WINDOWPLACEMENT) );
            wp.length = sizeof(WINDOWPLACEMENT);
            GetWindowPlacement( DXUTGetHWNDDeviceWindowed(), &wp );

            if( (wp.flags & WPF_RESTORETOMAXIMIZED) != 0 && wp.showCmd == SW_SHOWMINIMIZED )
            {
                // WPF_RESTORETOMAXIMIZED means that when the window is restored it will
                // be maximized.  So maximize the window temporarily to get the client rect 
                // when the window is maximized.  GetSystemMetrics( SM_CXMAXIMIZED ) will give this 
                // information if the window is on the primary but this will work on multimon.
                ShowWindow( DXUTGetHWNDDeviceWindowed(), SW_RESTORE );
                RECT rcClient;
                GetClientRect( DXUTGetHWNDDeviceWindowed(), &rcClient );
                nClientWidth  = (UINT)(rcClient.right - rcClient.left);
                nClientHeight = (UINT)(rcClient.bottom - rcClient.top);
                ShowWindow( DXUTGetHWNDDeviceWindowed(), SW_MINIMIZE );
            }
            else
            {
                // Use wp.rcNormalPosition to get the client rect, but wp.rcNormalPosition 
                // includes the window frame so subtract it
                RECT rcFrame = {0};
                AdjustWindowRect( &rcFrame, GetDXUTState().GetWindowedStyleAtModeChange(), GetDXUTState().GetMenu() != NULL );
                LONG nFrameWidth = rcFrame.right - rcFrame.left;
                LONG nFrameHeight = rcFrame.bottom - rcFrame.top;
                nClientWidth  = (UINT)(wp.rcNormalPosition.right - wp.rcNormalPosition.left - nFrameWidth);
                nClientHeight = (UINT)(wp.rcNormalPosition.bottom - wp.rcNormalPosition.top - nFrameHeight);
            }
        }
        else
        {
            // Window is restored or maximized so just get its client rect
            RECT rcClient;
            GetClientRect( DXUTGetHWNDDeviceWindowed(), &rcClient );
            nClientWidth  = (UINT)(rcClient.right - rcClient.left);
            nClientHeight = (UINT)(rcClient.bottom - rcClient.top);
        }

        // Now that we know the client rect, compare it against the back buffer size
        // to see if the client rect is already the right size
        if( nClientWidth  != pNewDeviceSettings->pp.BackBufferWidth ||
            nClientHeight != pNewDeviceSettings->pp.BackBufferHeight )
        {
            bNeedToResize = true;
        }       

        if( bClipWindowToSingleAdapter && !IsIconic(DXUTGetHWNDDeviceWindowed()) )
        {
            // Get the rect of the monitor attached to the adapter
            MONITORINFO miAdapter;
            miAdapter.cbSize = sizeof(MONITORINFO);
            HMONITOR hAdapterMonitor = DXUTGetD3DObject()->GetAdapterMonitor( pNewDeviceSettings->AdapterOrdinal );
            GetMonitorInfo( hAdapterMonitor, &miAdapter );
            HMONITOR hWindowMonitor = MonitorFromWindow( DXUTGetHWND(), MONITOR_DEFAULTTOPRIMARY );

            // Get the rect of the window
            RECT rcWindow;
            GetWindowRect( DXUTGetHWNDDeviceWindowed(), &rcWindow );

            // Check if the window rect is fully inside the adapter's vitural screen rect
            if( (rcWindow.left   < miAdapter.rcWork.left  ||
                 rcWindow.right  > miAdapter.rcWork.right ||
                 rcWindow.top    < miAdapter.rcWork.top   ||
                 rcWindow.bottom > miAdapter.rcWork.bottom) )
            {
                if( hWindowMonitor == hAdapterMonitor && IsZoomed(DXUTGetHWNDDeviceWindowed()) )
                {
                    // If the window is maximized and on the same monitor as the adapter, then 
                    // no need to clip to single adapter as the window is already clipped 
                    // even though the rcWindow rect is outside of the miAdapter.rcWork
                }
                else
                {
                    bNeedToResize = true;
                }
            }
        }
    }

    // Only resize window if needed 
    if( bNeedToResize ) 
    {
        // Need to resize, so if window is maximized or minimized then restore the window
        if( IsIconic(DXUTGetHWNDDeviceWindowed()) ) 
            ShowWindow( DXUTGetHWNDDeviceWindowed(), SW_RESTORE );
        if( IsZoomed(DXUTGetHWNDDeviceWindowed()) ) // doing the IsIconic() check first also handles the WPF_RESTORETOMAXIMIZED case
            ShowWindow( DXUTGetHWNDDeviceWindowed(), SW_RESTORE );

        if( bClipWindowToSingleAdapter )
        {
            // Get the rect of the monitor attached to the adapter
            MONITORINFO miAdapter;
            miAdapter.cbSize = sizeof(MONITORINFO);
            GetMonitorInfo( DXUTGetD3DObject()->GetAdapterMonitor( pNewDeviceSettings->AdapterOrdinal ), &miAdapter );

            // Get the rect of the monitor attached to the window
            MONITORINFO miWindow;
            miWindow.cbSize = sizeof(MONITORINFO);
            GetMonitorInfo( MonitorFromWindow( DXUTGetHWND(), MONITOR_DEFAULTTOPRIMARY ), &miWindow );

            // Do something reasonable if the BackBuffer size is greater than the monitor size
            int nAdapterMonitorWidth = miAdapter.rcWork.right - miAdapter.rcWork.left;
            int nAdapterMonitorHeight = miAdapter.rcWork.bottom - miAdapter.rcWork.top;

            int nClientWidth = pNewDeviceSettings->pp.BackBufferWidth;
            int nClientHeight = pNewDeviceSettings->pp.BackBufferHeight;

            // Get the rect of the window
            RECT rcWindow;
            GetWindowRect( DXUTGetHWNDDeviceWindowed(), &rcWindow );

            // Make a window rect with a client rect that is the same size as the backbuffer
            RECT rcResizedWindow;
            rcResizedWindow.left = 0;
            rcResizedWindow.right = nClientWidth;
            rcResizedWindow.top = 0;
            rcResizedWindow.bottom = nClientHeight;
            AdjustWindowRect( &rcResizedWindow, GetWindowLong( DXUTGetHWNDDeviceWindowed(), GWL_STYLE ), GetDXUTState().GetMenu() != NULL );

            int nWindowWidth = rcResizedWindow.right - rcResizedWindow.left;
            int nWindowHeight = rcResizedWindow.bottom - rcResizedWindow.top;

            if( nWindowWidth > nAdapterMonitorWidth )
                nWindowWidth = (nAdapterMonitorWidth - 0);
            if( nWindowHeight > nAdapterMonitorHeight )
                nWindowHeight = (nAdapterMonitorHeight - 0);

            if( rcResizedWindow.left < miAdapter.rcWork.left ||
                rcResizedWindow.top < miAdapter.rcWork.top ||
                rcResizedWindow.right > miAdapter.rcWork.right ||
                rcResizedWindow.bottom > miAdapter.rcWork.bottom )
            {
                int nWindowOffsetX = (nAdapterMonitorWidth - nWindowWidth) / 2;
                int nWindowOffsetY = (nAdapterMonitorHeight - nWindowHeight) / 2;

                rcResizedWindow.left = miAdapter.rcWork.left + nWindowOffsetX;
                rcResizedWindow.top = miAdapter.rcWork.top + nWindowOffsetY;
                rcResizedWindow.right = miAdapter.rcWork.left + nWindowOffsetX + nWindowWidth;
                rcResizedWindow.bottom = miAdapter.rcWork.top + nWindowOffsetY + nWindowHeight;
            }

            // Resize the window.  It is important to adjust the window size 
            // after resetting the device rather than beforehand to ensure 
            // that the monitor resolution is correct and does not limit the size of the new window.
            SetWindowPos( DXUTGetHWNDDeviceWindowed(), 0, rcResizedWindow.left, rcResizedWindow.top, nWindowWidth, nWindowHeight, SWP_NOZORDER );
        }        
        else
        {      
            // Make a window rect with a client rect that is the same size as the backbuffer
            RECT rcWindow = {0};
            rcWindow.right = (long)(pNewDeviceSettings->pp.BackBufferWidth);
            rcWindow.bottom = (long)(pNewDeviceSettings->pp.BackBufferHeight);
            AdjustWindowRect( &rcWindow, GetWindowLong( DXUTGetHWNDDeviceWindowed(), GWL_STYLE ), GetDXUTState().GetMenu() != NULL );

            // Resize the window.  It is important to adjust the window size 
            // after resetting the device rather than beforehand to ensure 
            // that the monitor resolution is correct and does not limit the size of the new window.
            int cx = (int)(rcWindow.right - rcWindow.left);
            int cy = (int)(rcWindow.bottom - rcWindow.top);
            SetWindowPos( DXUTGetHWNDDeviceWindowed(), 0, 0, 0, cx, cy, SWP_NOZORDER|SWP_NOMOVE );
        }

        // Its possible that the new window size is not what we asked for.  
        // No window can be sized larger than the desktop, so see see if the Windows OS resized the 
        // window to something smaller to fit on the desktop.  Also if WM_GETMINMAXINFO
        // will put a limit on the smallest/largest window size.
        RECT rcClient;
        GetClientRect( DXUTGetHWNDDeviceWindowed(), &rcClient );
        UINT nClientWidth  = (UINT)(rcClient.right - rcClient.left);
        UINT nClientHeight = (UINT)(rcClient.bottom - rcClient.top);
        if( nClientWidth  != pNewDeviceSettings->pp.BackBufferWidth ||
            nClientHeight != pNewDeviceSettings->pp.BackBufferHeight )
        {
            // If its different, then resize the backbuffer again.  This time create a backbuffer that matches the 
            // client rect of the current window w/o resizing the window.
            DXUTDeviceSettings deviceSettings = DXUTGetDeviceSettings();
            deviceSettings.pp.BackBufferWidth  = 0; 
            deviceSettings.pp.BackBufferHeight = 0;
            hr = DXUTChangeDevice( &deviceSettings, NULL, false, bClipWindowToSingleAdapter );
            if( FAILED( hr ) )
            {
                SAFE_DELETE( pOldDeviceSettings );
                DXUTCleanup3DEnvironment();
                DXUTPause( false, false );
                GetDXUTState().SetIgnoreSizeChange( false );
                return hr;
            }
        }
    }

    // Make the window visible
    if( !IsWindowVisible( DXUTGetHWND() ) )
        ShowWindow( DXUTGetHWND(), SW_SHOW );

    // Show/hide the HW cursor after the window is its proper place if the application requested it
    if( GetDXUTState().GetShowCursorWhenFullScreen() )
        DXUTInitHWCursor();

    SAFE_DELETE( pOldDeviceSettings );
    GetDXUTState().SetIgnoreSizeChange( false );
    DXUTPause( false, false );
    GetDXUTState().SetDeviceCreated( true );

    return S_OK;
}
HRESULT DXUTCreateWindow( const TCHAR* strWindowTitle, HINSTANCE hInstance, 
                          HICON hIcon, HMENU hMenu, int x, int y )
{
    HRESULT hr;

    // Not allowed to call this from inside the device callbacks
    if( GetDXUTState().GetInsideDeviceCallback() )
        return DXUT_ERR_MSGBOX( "DXUTCreateWindow", E_FAIL );

    GetDXUTState().SetWindowCreateCalled( true );

    if( !GetDXUTState().GetDXUTInited() ) 
    {
        // If DXUTInit() was already called and failed, then fail.
        // DXUTInit() must first succeed for this function to succeed
        if( GetDXUTState().GetDXUTInitCalled() )
            return E_FAIL; 

        // If DXUTInit() hasn't been called, then automatically call it
        // with default params
        hr = DXUTInit();
        if( FAILED(hr) )
            return hr;
    }

    if( DXUTGetHWNDFocus() == NULL )
    {
        if( hInstance == NULL ) 
            hInstance = (HINSTANCE)GetModuleHandle(NULL);
        GetDXUTState().SetHInstance( hInstance );

        TCHAR szExePath[MAX_PATH];
        GetModuleFileName( NULL, szExePath, MAX_PATH );
        if( hIcon == NULL ) // If the icon is NULL, then use the first one found in the exe
            hIcon = ExtractIcon( hInstance, szExePath, 0 ); 

        // Register the windows class
        WNDCLASS wndClass;
        wndClass.style = CS_DBLCLKS;
        wndClass.lpfnWndProc = DXUTStaticWndProc;
        wndClass.cbClsExtra = 0;
        wndClass.cbWndExtra = 0;
        wndClass.hInstance = hInstance;
        wndClass.hIcon = hIcon;
        wndClass.hCursor = LoadCursor( NULL, IDC_ARROW );
        wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
        wndClass.lpszMenuName = NULL;
        wndClass.lpszClassName = "Direct3DWindowClass";

        if( !RegisterClass( &wndClass ) )
        {
            DWORD dwError = GetLastError();
            if( dwError != ERROR_CLASS_ALREADY_EXISTS )
                return DXUT_ERR_MSGBOX( "RegisterClass", HRESULT_FROM_WIN32(dwError) );
        }

        // Set the window's initial style.  It is invisible initially since it might
        // be resized later
        DWORD dwWindowStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | 
                              WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
        
        RECT rc;

        // Override the window's initial & size position if there were cmd line args
        if( GetDXUTState().GetOverrideStartX() != -1 )
            x = GetDXUTState().GetOverrideStartX();
        if( GetDXUTState().GetOverrideStartY() != -1 )
            y = GetDXUTState().GetOverrideStartY();

        GetDXUTState().SetWindowCreatedWithDefaultPositions( false );
        if( x == CW_USEDEFAULT && y == CW_USEDEFAULT )
            GetDXUTState().SetWindowCreatedWithDefaultPositions( true );

        // Find the window's initial size, but it might be changed later
        int nDefaultWidth = 640;
        int nDefaultHeight = 480;
        if( GetDXUTState().GetOverrideWidth() != 0 )
            nDefaultWidth = GetDXUTState().GetOverrideWidth();
        if( GetDXUTState().GetOverrideHeight() != 0 )
            nDefaultHeight = GetDXUTState().GetOverrideHeight();
        SetRect( &rc, 0, 0, nDefaultWidth, nDefaultHeight );        
        AdjustWindowRect( &rc, dwWindowStyle, ( hMenu != NULL ) ? true : false );

        TCHAR* strCachedWindowTitle = GetDXUTState().GetWindowTitle();
        StringCchCopy( strCachedWindowTitle, 256, strWindowTitle );

        // Create the render window
        HWND hWnd = CreateWindow( "Direct3DWindowClass", strWindowTitle, dwWindowStyle,
                               x, y, (rc.right-rc.left), (rc.bottom-rc.top), 0,
                               hMenu, hInstance, 0 );
        if( hWnd == NULL )
        {
            DWORD dwError = GetLastError();
            return DXUT_ERR_MSGBOX( "CreateWindow", HRESULT_FROM_WIN32(dwError) );
        }

        GetDXUTState().SetWindowCreated( true );
        GetDXUTState().SetHWNDFocus( hWnd );
        GetDXUTState().SetHWNDDeviceFullScreen( hWnd );
        GetDXUTState().SetHWNDDeviceWindowed( hWnd );
    }

    return S_OK;
}
HRESULT DXUTSetWindow( HWND hWndFocus, HWND hWndDeviceFullScreen, HWND hWndDeviceWindowed, bool bHandleMessages )
{
    HRESULT hr;
 
    // Not allowed to call this from inside the device callbacks
    if( GetDXUTState().GetInsideDeviceCallback() )
        return DXUT_ERR_MSGBOX( "DXUTCreateWindow", E_FAIL );

    GetDXUTState().SetWindowCreateCalled( true );

    // To avoid confusion, we do not allow any HWND to be NULL here.  The
    // caller must pass in valid HWND for all three parameters.  The same
    // HWND may be used for more than one parameter.
    if( hWndFocus == NULL || hWndDeviceFullScreen == NULL || hWndDeviceWindowed == NULL )
        return DXUT_ERR_MSGBOX( "DXUTSetWindow", E_INVALIDARG );

    // If subclassing the window, set the pointer to the local window procedure
    if( bHandleMessages )
    {
        // Switch window procedures
#ifdef _WIN64
        LONG_PTR nResult = SetWindowLongPtr( hWndFocus, GWLP_WNDPROC, (LONG_PTR)DXUTStaticWndProc );
#else
        LONG_PTR nResult = SetWindowLongPtr( hWndFocus, GWLP_WNDPROC, (LONG)(LONG_PTR)DXUTStaticWndProc );
#endif 
 
        DWORD dwError = GetLastError();
        if( nResult == 0 )
            return DXUT_ERR_MSGBOX( "SetWindowLongPtr", HRESULT_FROM_WIN32(dwError) );
    }
 
    if( !GetDXUTState().GetDXUTInited() ) 
    {
        // If DXUTInit() was already called and failed, then fail.
        // DXUTInit() must first succeed for this function to succeed
        if( GetDXUTState().GetDXUTInitCalled() )
            return E_FAIL; 
 
        // If DXUTInit() hasn't been called, then automatically call it
        // with default params
        hr = DXUTInit();
        if( FAILED(hr) )
            return hr;
    }
 
    TCHAR* strCachedWindowTitle = GetDXUTState().GetWindowTitle();
    GetWindowText( hWndFocus, strCachedWindowTitle, 255 );
    strCachedWindowTitle[255] = 0;
   
    HINSTANCE hInstance = (HINSTANCE) (LONG_PTR) GetWindowLongPtr( hWndFocus, GWLP_HINSTANCE ); 
    GetDXUTState().SetHInstance( hInstance );
    GetDXUTState().SetWindowCreatedWithDefaultPositions( false );
    GetDXUTState().SetWindowCreated( true );
    GetDXUTState().SetHWNDFocus( hWndFocus );
    GetDXUTState().SetHWNDDeviceFullScreen( hWndDeviceFullScreen );
    GetDXUTState().SetHWNDDeviceWindowed( hWndDeviceWindowed );

    return S_OK;
}
HRESULT DXUTCreateDevice( UINT AdapterOrdinal, bool bWindowed, 
                          int nSuggestedWidth, int nSuggestedHeight,
                          LPDXUTCALLBACKISDEVICEACCEPTABLE pCallbackIsDeviceAcceptable,
                          LPDXUTCALLBACKMODIFYDEVICESETTINGS pCallbackModifyDeviceSettings,
                          void* pUserContext )
{
    HRESULT hr;

    // Not allowed to call this from inside the device callbacks
    if( GetDXUTState().GetInsideDeviceCallback() )
        return DXUT_ERR_MSGBOX( "DXUTCreateWindow", E_FAIL );

    // Record the function arguments in the global state 
    GetDXUTState().SetIsDeviceAcceptableFunc( pCallbackIsDeviceAcceptable );
    GetDXUTState().SetModifyDeviceSettingsFunc( pCallbackModifyDeviceSettings );
    GetDXUTState().SetIsDeviceAcceptableFuncUserContext( pUserContext );
    GetDXUTState().SetModifyDeviceSettingsFuncUserContext( pUserContext );

    GetDXUTState().SetDeviceCreateCalled( true );

    // If DXUTCreateWindow() or DXUTSetWindow() has not already been called, 
    // then call DXUTCreateWindow() with the default parameters.         
    if( !GetDXUTState().GetWindowCreated() ) 
    {
        // If DXUTCreateWindow() or DXUTSetWindow() was already called and failed, then fail.
        // DXUTCreateWindow() or DXUTSetWindow() must first succeed for this function to succeed
        if( GetDXUTState().GetWindowCreateCalled() )
            return E_FAIL; 

        // If DXUTCreateWindow() or DXUTSetWindow() hasn't been called, then 
        // automatically call DXUTCreateWindow() with default params
        hr = DXUTCreateWindow();
        if( FAILED(hr) )
            return hr;
    }

    // Force an enumeration with the new IsDeviceAcceptable callback
    DXUTPrepareEnumerationObject( true );

    DXUTMatchOptions matchOptions;
    matchOptions.eAdapterOrdinal     = DXUTMT_PRESERVE_INPUT;
    matchOptions.eDeviceType         = DXUTMT_IGNORE_INPUT;
    matchOptions.eWindowed           = DXUTMT_PRESERVE_INPUT;
    matchOptions.eAdapterFormat      = DXUTMT_IGNORE_INPUT;
    matchOptions.eVertexProcessing   = DXUTMT_IGNORE_INPUT;
    matchOptions.eResolution         = DXUTMT_CLOSEST_TO_INPUT;
    matchOptions.eBackBufferFormat   = DXUTMT_IGNORE_INPUT;
    matchOptions.eBackBufferCount    = DXUTMT_IGNORE_INPUT;
    matchOptions.eMultiSample        = DXUTMT_IGNORE_INPUT;
    matchOptions.eSwapEffect         = DXUTMT_IGNORE_INPUT;
    matchOptions.eDepthFormat        = DXUTMT_IGNORE_INPUT;
    matchOptions.eStencilFormat      = DXUTMT_IGNORE_INPUT;
    matchOptions.ePresentFlags       = DXUTMT_IGNORE_INPUT;
    matchOptions.eRefreshRate        = DXUTMT_IGNORE_INPUT;
    matchOptions.ePresentInterval    = DXUTMT_IGNORE_INPUT;

    DXUTDeviceSettings deviceSettings;
    ZeroMemory( &deviceSettings, sizeof(DXUTDeviceSettings) );
    deviceSettings.AdapterOrdinal      = AdapterOrdinal;
    deviceSettings.pp.Windowed         = bWindowed;
    deviceSettings.pp.BackBufferWidth  = nSuggestedWidth;
    deviceSettings.pp.BackBufferHeight = nSuggestedHeight;

    // Override with settings from the command line
    if( GetDXUTState().GetOverrideWidth() != 0 )
        deviceSettings.pp.BackBufferWidth = GetDXUTState().GetOverrideWidth();
    if( GetDXUTState().GetOverrideHeight() != 0 )
        deviceSettings.pp.BackBufferHeight = GetDXUTState().GetOverrideHeight();

    if( GetDXUTState().GetOverrideAdapterOrdinal() != -1 )
        deviceSettings.AdapterOrdinal = GetDXUTState().GetOverrideAdapterOrdinal();

    if( GetDXUTState().GetOverrideFullScreen() )
    {
        deviceSettings.pp.Windowed = FALSE;
        if( GetDXUTState().GetOverrideWidth() == 0 && GetDXUTState().GetOverrideHeight() == 0 )
            matchOptions.eResolution = DXUTMT_IGNORE_INPUT;
    }
    if( GetDXUTState().GetOverrideWindowed() )
        deviceSettings.pp.Windowed = TRUE;

    if( GetDXUTState().GetOverrideForceHAL() )
    {
        deviceSettings.DeviceType = D3DDEVTYPE_HAL;
        matchOptions.eDeviceType = DXUTMT_PRESERVE_INPUT;
    }
    if( GetDXUTState().GetOverrideForceREF() )
    {
        deviceSettings.DeviceType = D3DDEVTYPE_REF;
        matchOptions.eDeviceType = DXUTMT_PRESERVE_INPUT;
    }

    if( GetDXUTState().GetOverrideForcePureHWVP() )
    {
        deviceSettings.BehaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE;
        matchOptions.eVertexProcessing = DXUTMT_PRESERVE_INPUT;
    }
    else if( GetDXUTState().GetOverrideForceHWVP() )
    {
        deviceSettings.BehaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING;
        matchOptions.eVertexProcessing = DXUTMT_PRESERVE_INPUT;
    }
    else if( GetDXUTState().GetOverrideForceSWVP() )
    {
        deviceSettings.BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
        matchOptions.eVertexProcessing = DXUTMT_PRESERVE_INPUT;
    }

    hr = DXUTFindValidDeviceSettings( &deviceSettings, &deviceSettings, &matchOptions );
    if( FAILED(hr) ) // the call will fail if no valid devices were found
    {
        DXUTDisplayErrorMessage( hr );
        return DXUT_ERR( "DXUTFindValidDeviceSettings", hr );
    }

    // Change to a Direct3D device created from the new device settings.  
    // If there is an existing device, then either reset or recreated the scene
    hr = DXUTChangeDevice( &deviceSettings, NULL, false, true );
    if( FAILED(hr) )
        return hr;

    return S_OK;
}
HRESULT DXUTCreateDeviceFromSettings( DXUTDeviceSettings* pDeviceSettings, bool bPreserveInput, bool bClipWindowToSingleAdapter )
{
    HRESULT hr;

    GetDXUTState().SetDeviceCreateCalled( true );

    // If DXUTCreateWindow() or DXUTSetWindow() has not already been called, 
    // then call DXUTCreateWindow() with the default parameters.         
    if( !GetDXUTState().GetWindowCreated() ) 
    {
        // If DXUTCreateWindow() or DXUTSetWindow() was already called and failed, then fail.
        // DXUTCreateWindow() or DXUTSetWindow() must first succeed for this function to succeed
        if( GetDXUTState().GetWindowCreateCalled() )
            return E_FAIL; 

        // If DXUTCreateWindow() or DXUTSetWindow() hasn't been called, then 
        // automatically call DXUTCreateWindow() with default params
        hr = DXUTCreateWindow();
        if( FAILED(hr) )
            return hr;
    }

    if( !bPreserveInput )
    {
        // If not preserving the input, then find the closest valid to it
        DXUTMatchOptions matchOptions;
        matchOptions.eAdapterOrdinal     = DXUTMT_CLOSEST_TO_INPUT;
        matchOptions.eDeviceType         = DXUTMT_CLOSEST_TO_INPUT;
        matchOptions.eWindowed           = DXUTMT_CLOSEST_TO_INPUT;
        matchOptions.eAdapterFormat      = DXUTMT_CLOSEST_TO_INPUT;
        matchOptions.eVertexProcessing   = DXUTMT_CLOSEST_TO_INPUT;
        matchOptions.eResolution         = DXUTMT_CLOSEST_TO_INPUT;
        matchOptions.eBackBufferFormat   = DXUTMT_CLOSEST_TO_INPUT;
        matchOptions.eBackBufferCount    = DXUTMT_CLOSEST_TO_INPUT;
        matchOptions.eMultiSample        = DXUTMT_CLOSEST_TO_INPUT;
        matchOptions.eSwapEffect         = DXUTMT_CLOSEST_TO_INPUT;
        matchOptions.eDepthFormat        = DXUTMT_CLOSEST_TO_INPUT;
        matchOptions.eStencilFormat      = DXUTMT_CLOSEST_TO_INPUT;
        matchOptions.ePresentFlags       = DXUTMT_CLOSEST_TO_INPUT;
        matchOptions.eRefreshRate        = DXUTMT_CLOSEST_TO_INPUT;
        matchOptions.ePresentInterval    = DXUTMT_CLOSEST_TO_INPUT;

        hr = DXUTFindValidDeviceSettings( pDeviceSettings, pDeviceSettings, &matchOptions );
        if( FAILED(hr) ) // the call will fail if no valid devices were found
        {
            DXUTDisplayErrorMessage( hr );
            return DXUT_ERR( "DXUTFindValidDeviceSettings", hr );
        }
    }

    // Change to a Direct3D device created from the new device settings.  
    // If there is an existing device, then either reset or recreate the scene
    hr = DXUTChangeDevice( pDeviceSettings, NULL, false, bClipWindowToSingleAdapter );
    if( FAILED(hr) )
        return hr;

    return S_OK;
}
HRESULT DXUTSetDevice( IDirect3DDevice9* pd3dDevice )
{
    HRESULT hr;

    if( pd3dDevice == NULL )
        return DXUT_ERR_MSGBOX( "DXUTSetDevice", E_INVALIDARG );

    // Not allowed to call this from inside the device callbacks
    if( GetDXUTState().GetInsideDeviceCallback() )
        return DXUT_ERR_MSGBOX( "DXUTCreateWindow", E_FAIL );

    GetDXUTState().SetDeviceCreateCalled( true );

    // If DXUTCreateWindow() or DXUTSetWindow() has not already been called, 
    // then call DXUTCreateWindow() with the default parameters.         
    if( !GetDXUTState().GetWindowCreated() ) 
    {
        // If DXUTCreateWindow() or DXUTSetWindow() was already called and failed, then fail.
        // DXUTCreateWindow() or DXUTSetWindow() must first succeed for this function to succeed
        if( GetDXUTState().GetWindowCreateCalled() )
            return E_FAIL; 

        // If DXUTCreateWindow() or DXUTSetWindow() hasn't been called, then 
        // automatically call DXUTCreateWindow() with default params
        hr = DXUTCreateWindow();
        if( FAILED(hr) )
            return hr;
    }

    DXUTDeviceSettings* pDeviceSettings = new DXUTDeviceSettings;
    if( pDeviceSettings == NULL )
        return E_OUTOFMEMORY;
    ZeroMemory( pDeviceSettings, sizeof(DXUTDeviceSettings) );

    // Get the present params from the swap chain
    IDirect3DSurface9* pBackBuffer = NULL;
    hr = pd3dDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
    if( SUCCEEDED(hr) )
    {
        IDirect3DSwapChain9* pSwapChain = NULL;
        hr = pBackBuffer->GetContainer( IID_IDirect3DSwapChain9, (void**) &pSwapChain );
        if( SUCCEEDED(hr) )
        {
            pSwapChain->GetPresentParameters( &pDeviceSettings->pp );
            SAFE_RELEASE( pSwapChain );
        }

        SAFE_RELEASE( pBackBuffer );
    }

    D3DDEVICE_CREATION_PARAMETERS d3dCreationParams;
    pd3dDevice->GetCreationParameters( &d3dCreationParams );

    // Fill out the rest of the device settings struct
    pDeviceSettings->AdapterOrdinal = d3dCreationParams.AdapterOrdinal;
    pDeviceSettings->DeviceType     = d3dCreationParams.DeviceType;
    DXUTFindAdapterFormat( pDeviceSettings->AdapterOrdinal, pDeviceSettings->DeviceType, 
                           pDeviceSettings->pp.BackBufferFormat, pDeviceSettings->pp.Windowed, 
                           &pDeviceSettings->AdapterFormat );
    pDeviceSettings->BehaviorFlags  = d3dCreationParams.BehaviorFlags;

    // Change to the Direct3D device passed in
    hr = DXUTChangeDevice( pDeviceSettings, pd3dDevice, false, false );
    if( FAILED(hr) ) 
        return hr;

    return S_OK;
}
HRESULT DXUTFindAdapterFormat( UINT AdapterOrdinal, D3DDEVTYPE DeviceType, D3DFORMAT BackBufferFormat, 
                               BOOL Windowed, D3DFORMAT* pAdapterFormat )
{
    CD3DEnumeration* pd3dEnum = DXUTPrepareEnumerationObject();
    CD3DEnumDeviceInfo* pDeviceInfo = pd3dEnum->GetDeviceInfo( AdapterOrdinal, DeviceType );
    if( pDeviceInfo )
    {
        for( int iDeviceCombo=0; iDeviceCombo<pDeviceInfo->deviceSettingsComboList.GetSize(); iDeviceCombo++ )
        {
            CD3DEnumDeviceSettingsCombo* pDeviceSettingsCombo = pDeviceInfo->deviceSettingsComboList.GetAt(iDeviceCombo);
            if( pDeviceSettingsCombo->BackBufferFormat == BackBufferFormat &&
                pDeviceSettingsCombo->Windowed == Windowed )
            {
                // Return the adapter format from the first match
                *pAdapterFormat = pDeviceSettingsCombo->AdapterFormat;
                return S_OK;
            }
        }
    }

    *pAdapterFormat = BackBufferFormat;
    return E_FAIL;
}
LRESULT CALLBACK DXUTStaticWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    // Consolidate the keyboard messages and pass them to the app's keyboard callback
    if( uMsg == WM_KEYDOWN ||
        uMsg == WM_SYSKEYDOWN || 
        uMsg == WM_KEYUP ||
        uMsg == WM_SYSKEYUP )
    {
        bool bKeyDown = (uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN);
        DWORD dwMask = (1 << 29);
        bool bAltDown = ( (lParam & dwMask) != 0 );

        bool* bKeys = GetDXUTState().GetKeys();
        bKeys[ (BYTE) (wParam & 0xFF) ] = bKeyDown;

        LPDXUTCALLBACKKEYBOARD pCallbackKeyboard = GetDXUTState().GetKeyboardFunc();
        if( pCallbackKeyboard )
            pCallbackKeyboard( (UINT)wParam, bKeyDown, bAltDown, GetDXUTState().GetKeyboardFuncUserContext() );           
    }

    // Consolidate the mouse button messages and pass them to the app's mouse callback
    if( uMsg == WM_LBUTTONDOWN ||
        uMsg == WM_LBUTTONUP ||
        uMsg == WM_LBUTTONDBLCLK ||
        uMsg == WM_MBUTTONDOWN ||
        uMsg == WM_MBUTTONUP ||
        uMsg == WM_MBUTTONDBLCLK ||
        uMsg == WM_RBUTTONDOWN ||
        uMsg == WM_RBUTTONUP ||
        uMsg == WM_RBUTTONDBLCLK ||
        uMsg == WM_MOUSEWHEEL || 
        (GetDXUTState().GetNotifyOnMouseMove() && uMsg == WM_MOUSEMOVE) )
    {
        int xPos = (short)LOWORD(lParam);
        int yPos = (short)HIWORD(lParam);

        if( uMsg == WM_MOUSEWHEEL )
        {
            // WM_MOUSEWHEEL passes screen mouse coords
            // so convert them to client coords
            POINT pt;
            pt.x = xPos; pt.y = yPos;
            ScreenToClient( hWnd, &pt );
            xPos = pt.x; yPos = pt.y;
        }

        int nMouseWheelDelta = 0;
        if( uMsg == WM_MOUSEWHEEL ) 
            nMouseWheelDelta = (short) HIWORD(wParam);

        int nMouseButtonState = LOWORD(wParam);
        bool bLeftButton  = ((nMouseButtonState & MK_LBUTTON) != 0);
        bool bRightButton = ((nMouseButtonState & MK_RBUTTON) != 0);
        bool bMiddleButton = ((nMouseButtonState & MK_MBUTTON) != 0);
  
		bool* bMouseButtons = GetDXUTState().GetMouseButtons();
        bMouseButtons[0] = bLeftButton;
        bMouseButtons[1] = bMiddleButton;
        bMouseButtons[2] = bRightButton;
        bMouseButtons[3] = false;
        bMouseButtons[4] = false;

        LPDXUTCALLBACKMOUSE pCallbackMouse = GetDXUTState().GetMouseFunc();
        if( pCallbackMouse )
            pCallbackMouse( bLeftButton, bRightButton, bMiddleButton, false, false, nMouseWheelDelta, xPos, yPos, GetDXUTState().GetMouseFuncUserContext() );
    }

    // Pass all messages to the app's MsgProc callback, and don't 
    // process further messages if the apps says not to.
    //NewWndProc(hWnd,uMsg,wParam,lParam);

    switch( uMsg )
    {
        case WM_PAINT:
        {
            IDirect3DDevice9* pd3dDevice = DXUTGetD3DDevice();

            // Handle paint messages when the app is paused
            if( pd3dDevice && DXUTIsRenderingPaused() && 
                GetDXUTState().GetDeviceObjectsCreated() && GetDXUTState().GetDeviceObjectsReset() )
            {
                HRESULT hr;
                double fTime = DXUTGetTime();
                float fElapsedTime = DXUTGetElapsedTime();

                LPDXUTCALLBACKFRAMERENDER pCallbackFrameRender = GetDXUTState().GetFrameRenderFunc();
                if( pCallbackFrameRender != NULL )
                    pCallbackFrameRender( pd3dDevice, fTime, fElapsedTime, GetDXUTState().GetFrameRenderFuncUserContext() );

                hr = pd3dDevice->Present( NULL, NULL, NULL, NULL );
                if( D3DERR_DEVICELOST == hr )
                {
                    GetDXUTState().SetDeviceLost( true );
                }
                else if( D3DERR_DRIVERINTERNALERROR == hr )
                {
                    // When D3DERR_DRIVERINTERNALERROR is returned from Present(),
                    // the application can do one of the following:
                    // 
                    // - End, with the pop-up window saying that the application cannot continue 
                    //   because of problems in the display adapter and that the user should 
                    //   contact the adapter manufacturer.
                    //
                    // - Attempt to restart by calling IDirect3DDevice9::Reset, which is essentially the same 
                    //   path as recovering from a lost device. If IDirect3DDevice9::Reset fails with 
                    //   D3DERR_DRIVERINTERNALERROR, the application should end immediately with the message 
                    //   that the user should contact the adapter manufacturer.
                    // 
                    // The framework attempts the path of resetting the device
                    // 
                    GetDXUTState().SetDeviceLost( true );
                }
            }
            break;
        }

        case WM_SIZE:
            if( SIZE_MINIMIZED == wParam )
            {
                if( GetDXUTState().GetClipCursorWhenFullScreen() && !DXUTIsWindowed() )
                    ClipCursor( NULL );
                DXUTPause( true, true ); // Pause while we're minimized

                GetDXUTState().SetMinimized( true );
                GetDXUTState().SetMaximized( false );


            }
            else
            {
                RECT rcCurrentClient;
                GetClientRect( DXUTGetHWND(), &rcCurrentClient );
                if( rcCurrentClient.top == 0 && rcCurrentClient.bottom == 0 )
                {
                    // Rapidly clicking the task bar to minimize and restore a window
                    // can cause a WM_SIZE message with SIZE_RESTORED when 
                    // the window has actually become minimized due to rapid change
                    // so just ignore this message
                }
                else if( SIZE_MAXIMIZED == wParam )
                {
                    if( GetDXUTState().GetMinimized() )
                        DXUTPause( false, false ); // Unpause since we're no longer minimized
                    GetDXUTState().SetMinimized( false );
                    GetDXUTState().SetMaximized( true );
                    DXUTCheckForWindowSizeChange();
                    DXUTCheckForWindowChangingMonitors();
                }
                else if( SIZE_RESTORED == wParam )
                {      
                    if( GetDXUTState().GetMaximized() )
                    {
                        GetDXUTState().SetMaximized( false );
                        DXUTCheckForWindowSizeChange();
                        DXUTCheckForWindowChangingMonitors();
                    }
                    else if( GetDXUTState().GetMinimized() )
                    {
                        DXUTPause( false, false ); // Unpause since we're no longer minimized
                        GetDXUTState().SetMinimized( false );
                        DXUTCheckForWindowSizeChange();
                        DXUTCheckForWindowChangingMonitors();
                    }
                    else if( GetDXUTState().GetInSizeMove() )
                    {
                        // If we're neither maximized nor minimized, the window size 
                        // is changing by the user dragging the window edges.  In this 
                        // case, we don't reset the device yet -- we wait until the 
                        // user stops dragging, and a WM_EXITSIZEMOVE message comes.
                    }
                    else
                    {
                        // This WM_SIZE come from resizing the window via an API like SetWindowPos() so 
                        // resize and reset the device now.
                        DXUTCheckForWindowSizeChange();
                        DXUTCheckForWindowChangingMonitors();
                    }
                }
            }
            break;

        case WM_GETMINMAXINFO:
            ((MINMAXINFO*)lParam)->ptMinTrackSize.x = DXUT_MIN_WINDOW_SIZE_X;
            ((MINMAXINFO*)lParam)->ptMinTrackSize.y = DXUT_MIN_WINDOW_SIZE_Y;
            break;

        case WM_ENTERSIZEMOVE:
            // Halt frame movement while the app is sizing or moving
            DXUTPause( true, true );
            GetDXUTState().SetInSizeMove( true );
            break;

        case WM_EXITSIZEMOVE:
            DXUTPause( false, false );
            DXUTCheckForWindowSizeChange();
            DXUTCheckForWindowChangingMonitors();
            GetDXUTState().SetInSizeMove( false );
            break;

         case WM_MOUSEMOVE:
            if( !DXUTIsRenderingPaused() && !DXUTIsWindowed() )
            {
                IDirect3DDevice9* pd3dDevice = DXUTGetD3DDevice();
                if( pd3dDevice )
                {
                    POINT ptCursor;
                    GetCursorPos( &ptCursor );
                    pd3dDevice->SetCursorPosition( ptCursor.x, ptCursor.y, 0 );
                }
            }
            break;

       case WM_ACTIVATEAPP:
            if( wParam == TRUE && !GetDXUTState().GetActive() ) // Handle only if previously not active 
            {
                GetDXUTState().SetActive( true );

                // Show the cursor again if returning to fullscreen 
                if( !DXUTIsWindowed() && GetDXUTState().GetShowCursorWhenFullScreen() )
                    DXUTInitHWCursor();

                // The GetMinimizedWhileFullscreen() varible is used instead of !DXUTIsWindowed()
                // to handle the rare case toggling to windowed mode while the fullscreen application 
                // is minimized and thus making the pause count wrong
                if( GetDXUTState().GetMinimizedWhileFullscreen() ) 
                {
                    DXUTPause( false, false ); // Unpause since we're no longer minimized
                    GetDXUTState().SetMinimizedWhileFullscreen( false );
                }

                // Upon returning to this app, potentially disable shortcut keys 
                // (Windows key, accessibility shortcuts) 
                DXUTAllowShortcutKeys( ( DXUTIsWindowed() ) ? GetDXUTState().GetAllowShortcutKeysWhenWindowed() : 
                                                              GetDXUTState().GetAllowShortcutKeysWhenFullscreen() );

            }
            else if( wParam == FALSE && GetDXUTState().GetActive() ) // Handle only if previously active 
            {               
                GetDXUTState().SetActive( false );

                if( !DXUTIsWindowed() )
                {
                    // Going from full screen to a minimized state 
                    ClipCursor( NULL );      // don't limit the cursor anymore
                    DXUTPause( true, true ); // Pause while we're minimized (take care not to pause twice by handling this message twice)
                    GetDXUTState().SetMinimizedWhileFullscreen( true ); 
                }

                // Restore shortcut keys (Windows key, accessibility shortcuts) to original state
                //
                // This is important to call here if the shortcuts are disabled, 
                // because if this is not done then the Windows key will continue to 
                // be disabled while this app is running which is very bad.
                // If the app crashes, the Windows key will return to normal.
                DXUTAllowShortcutKeys( true );
            }
            break;

       case WM_ENTERMENULOOP:
            // Pause the app when menus are displayed
            DXUTPause( true, true );
            break;

        case WM_EXITMENULOOP:
            DXUTPause( false, false );
            break;

        case WM_MENUCHAR:
            // A menu is active and the user presses a key that does not correspond to any mnemonic or accelerator key
            // So just ignore and don't beep
            return MAKELRESULT(0,MNC_CLOSE);
            break;

        case WM_NCHITTEST:
            // Prevent the user from selecting the menu in full screen mode
            if( !DXUTIsWindowed() )
                return HTCLIENT;
            break;

        case WM_POWERBROADCAST:
            switch( wParam )
            {
                #ifndef PBT_APMQUERYSUSPEND
                    #define PBT_APMQUERYSUSPEND 0x0000
                #endif
                case PBT_APMQUERYSUSPEND:
                    // At this point, the app should save any data for open
                    // network connections, files, etc., and prepare to go into
                    // a suspended mode.  The app can use the MsgProc callback
                    // to handle this if desired.
                    return true;

                #ifndef PBT_APMRESUMESUSPEND
                    #define PBT_APMRESUMESUSPEND 0x0007
                #endif
                case PBT_APMRESUMESUSPEND:
                    // At this point, the app should recover any data, network
                    // connections, files, etc., and resume running from when
                    // the app was suspended. The app can use the MsgProc callback
                    // to handle this if desired.
                   
                   // QPC may lose consistency when suspending, so reset the timer
                   // upon resume.
                   DXUTGetGlobalTimer()->Reset();                   
                   GetDXUTState().SetLastStatsUpdateTime( 0 );
                   return true;
            }
            break;

        case WM_SYSCOMMAND:
            // Prevent moving/sizing and power loss in full screen mode
            switch( wParam )
            {
                case SC_MOVE:
                case SC_SIZE:
                case SC_MAXIMIZE:
                case SC_KEYMENU:
                case SC_MONITORPOWER:
                    if( !DXUTIsWindowed() )
                        return 0;
                    break;
            }
            break;

        case WM_SYSKEYDOWN:
        {
            if( GetDXUTState().GetHandleDefaultHotkeys() )
            {
                switch( wParam )
                {
                    case VK_RETURN:
                    {
                        // Toggle full screen upon alt-enter 
                        DWORD dwMask = (1 << 29);
                        if( (lParam & dwMask) != 0 ) // Alt is down also
                        {
                            // Toggle the full screen/window mode
                            DXUTPause( true, true );
                            DXUTToggleFullScreen();
                            DXUTPause( false, false );                        
                            return 0;
                        }
                    }
                }
            }
            break;
        }

        case WM_KEYDOWN:
        {
            if( GetDXUTState().GetHandleDefaultHotkeys() )
            {
                switch( wParam )
                {
                    case VK_F3:
                    {
                        DXUTPause( true, true );
                        DXUTToggleREF();
                        DXUTPause( false, false );                        
                        break;
                    }

                    case VK_F8:
                    {
                        bool bWireFrame = GetDXUTState().GetWireframeMode();
                        bWireFrame = !bWireFrame; 
                        GetDXUTState().SetWireframeMode( bWireFrame );

                        IDirect3DDevice9* pd3dDevice = DXUTGetD3DDevice();
                        if( pd3dDevice )
                            pd3dDevice->SetRenderState( D3DRS_FILLMODE, (bWireFrame) ? D3DFILL_WIREFRAME : D3DFILL_SOLID ); 
                        break;
                    }

                    case VK_ESCAPE:
                    {
                        // Received key to exit app
                        SendMessage( hWnd, WM_CLOSE, 0, 0 );
                    }

                    case VK_PAUSE: 
                    {
                        bool bTimePaused = DXUTIsTimePaused();
                        bTimePaused = !bTimePaused;
                        if( bTimePaused ) 
                            DXUTPause( true, false ); 
                        else
                            DXUTPause( false, false ); 
                        break; 
                    }
                }
            }
            break;
        }

        case WM_CLOSE:
        {
            HMENU hMenu;
            hMenu = GetMenu(hWnd);
            if( hMenu != NULL )
                DestroyMenu( hMenu );
            DestroyWindow( hWnd );
            UnregisterClass( "Direct3DWindowClass", NULL );
            GetDXUTState().SetHWNDFocus( NULL );
            GetDXUTState().SetHWNDDeviceFullScreen( NULL );
            GetDXUTState().SetHWNDDeviceWindowed( NULL );
            return 0;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            break;
    }

    // Don't allow the F10 key to act as a shortcut to the menu bar
    // by not passing these messages to the DefWindowProc only when
    // there's no menu present
    if( GetDXUTState().GetMenu() == NULL && (uMsg == WM_SYSKEYDOWN || uMsg == WM_SYSKEYUP) && wParam == VK_F10 )
        return 0;
    else
        return DefWindowProc( hWnd, uMsg, wParam, lParam );
}
void DXUTCheckForWindowSizeChange()
{
    // Skip the check for various reasons
    if( GetDXUTState().GetIgnoreSizeChange() || 
        !GetDXUTState().GetDeviceCreated() || 
        !GetDXUTState().GetCurrentDeviceSettings()->pp.Windowed )
        return;

    RECT rcCurrentClient;
    GetClientRect( DXUTGetHWND(), &rcCurrentClient );
    
    if( (UINT)rcCurrentClient.right != GetDXUTState().GetCurrentDeviceSettings()->pp.BackBufferWidth ||
        (UINT)rcCurrentClient.bottom != GetDXUTState().GetCurrentDeviceSettings()->pp.BackBufferHeight )
    {
        // A new window size will require a new backbuffer size
        // size, so the device must be reset and the D3D structures updated accordingly.

        // Tell DXUTChangeDevice and D3D to size according to the HWND's client rect
        DXUTDeviceSettings deviceSettings = DXUTGetDeviceSettings();
        deviceSettings.pp.BackBufferWidth  = 0; 
        deviceSettings.pp.BackBufferHeight = 0;
        DXUTChangeDevice( &deviceSettings, NULL, false, false );
    }
}
void DXUTCheckForWindowChangingMonitors()
{
    // Skip this check for various reasons
    if( !GetDXUTState().GetAutoChangeAdapter() || 
         GetDXUTState().GetIgnoreSizeChange() ||
        !GetDXUTState().GetDeviceCreated() ||
        !GetDXUTState().GetCurrentDeviceSettings()->pp.Windowed )
    {
        return;
    }

    HRESULT hr;
    HMONITOR hWindowMonitor = MonitorFromWindow( DXUTGetHWND(), MONITOR_DEFAULTTOPRIMARY );
    HMONITOR hAdapterMonitor = GetDXUTState().GetAdapterMonitor();
    if( hWindowMonitor != hAdapterMonitor )
    {
        UINT newOrdinal;
        if( SUCCEEDED( DXUTGetAdapterOrdinalFromMonitor( hWindowMonitor, &newOrdinal ) ) )
        {
            // Find the closest valid device settings with the new ordinal
            DXUTDeviceSettings deviceSettings = DXUTGetDeviceSettings();
            deviceSettings.AdapterOrdinal = newOrdinal;
            
            DXUTMatchOptions matchOptions;
            matchOptions.eAdapterOrdinal     = DXUTMT_PRESERVE_INPUT;
            matchOptions.eDeviceType         = DXUTMT_CLOSEST_TO_INPUT;
            matchOptions.eWindowed           = DXUTMT_CLOSEST_TO_INPUT;
            matchOptions.eAdapterFormat      = DXUTMT_CLOSEST_TO_INPUT;
            matchOptions.eVertexProcessing   = DXUTMT_CLOSEST_TO_INPUT;
            matchOptions.eResolution         = DXUTMT_CLOSEST_TO_INPUT;
            matchOptions.eBackBufferFormat   = DXUTMT_CLOSEST_TO_INPUT;
            matchOptions.eBackBufferCount    = DXUTMT_CLOSEST_TO_INPUT;
            matchOptions.eMultiSample        = DXUTMT_CLOSEST_TO_INPUT;
            matchOptions.eSwapEffect         = DXUTMT_CLOSEST_TO_INPUT;
            matchOptions.eDepthFormat        = DXUTMT_CLOSEST_TO_INPUT;
            matchOptions.eStencilFormat      = DXUTMT_CLOSEST_TO_INPUT;
            matchOptions.ePresentFlags       = DXUTMT_CLOSEST_TO_INPUT;
            matchOptions.eRefreshRate        = DXUTMT_CLOSEST_TO_INPUT;
            matchOptions.ePresentInterval    = DXUTMT_CLOSEST_TO_INPUT;

            hr = DXUTFindValidDeviceSettings( &deviceSettings, &deviceSettings, &matchOptions );
            if( SUCCEEDED(hr) ) 
            {
                // Create a Direct3D device using the new device settings.  
                // If there is an existing device, then it will either reset or recreate the scene.
                hr = DXUTChangeDevice( &deviceSettings, NULL, false, false );

                // If hr == E_ABORT, this means the app rejected the device settings in the ModifySettingsCallback
                if( hr == E_ABORT )
                {
                    // so nothing changed and keep from attempting to switch adapters next time
                    GetDXUTState().SetAutoChangeAdapter( false );
                }
                else if( FAILED(hr) )
                {
                    DXUTShutdown();
                    DXUTPause( false, false );
                    return;
                }
            }
        }
    }    
}
HRESULT DXUTGetAdapterOrdinalFromMonitor( HMONITOR hMonitor, UINT* pAdapterOrdinal )
{
    *pAdapterOrdinal = 0;

    CD3DEnumeration* pd3dEnum = DXUTPrepareEnumerationObject();
    IDirect3D9*      pD3D     = DXUTGetD3DObject();

    CGrowableArray<CD3DEnumAdapterInfo*>* pAdapterList = pd3dEnum->GetAdapterInfoList();
    for( int iAdapter=0; iAdapter<pAdapterList->GetSize(); iAdapter++ )
    {
        CD3DEnumAdapterInfo* pAdapterInfo = pAdapterList->GetAt(iAdapter);
        HMONITOR hAdapterMonitor = pD3D->GetAdapterMonitor( pAdapterInfo->AdapterOrdinal );
        if( hAdapterMonitor == hMonitor )
        {
            *pAdapterOrdinal = pAdapterInfo->AdapterOrdinal;
            return S_OK;
        }
    }

    return E_FAIL;
}
HRESULT DXUTToggleFullScreen()
{
    HRESULT hr;

    // Get the current device settings and flip the windowed state then
    // find the closest valid device settings with this change
    DXUTDeviceSettings deviceSettings = DXUTGetDeviceSettings();
    deviceSettings.pp.Windowed = !deviceSettings.pp.Windowed;

    DXUTMatchOptions matchOptions;
    matchOptions.eAdapterOrdinal     = DXUTMT_PRESERVE_INPUT;
    matchOptions.eDeviceType         = DXUTMT_CLOSEST_TO_INPUT;
    matchOptions.eWindowed           = DXUTMT_PRESERVE_INPUT;
    matchOptions.eAdapterFormat      = DXUTMT_IGNORE_INPUT;
    matchOptions.eVertexProcessing   = DXUTMT_CLOSEST_TO_INPUT;
    matchOptions.eBackBufferFormat   = DXUTMT_IGNORE_INPUT;
    matchOptions.eBackBufferCount    = DXUTMT_CLOSEST_TO_INPUT;
    matchOptions.eMultiSample        = DXUTMT_CLOSEST_TO_INPUT;
    matchOptions.eSwapEffect         = DXUTMT_CLOSEST_TO_INPUT;
    matchOptions.eDepthFormat        = DXUTMT_CLOSEST_TO_INPUT;
    matchOptions.eStencilFormat      = DXUTMT_CLOSEST_TO_INPUT;
    matchOptions.ePresentFlags       = DXUTMT_CLOSEST_TO_INPUT;
    matchOptions.eRefreshRate        = DXUTMT_IGNORE_INPUT;
    matchOptions.ePresentInterval    = DXUTMT_IGNORE_INPUT;

    // Go back to previous state

    UINT nWidth  = ( deviceSettings.pp.Windowed ) ? GetDXUTState().GetWindowBackBufferWidthAtModeChange() : GetDXUTState().GetFullScreenBackBufferWidthAtModeChange();
    UINT nHeight = ( deviceSettings.pp.Windowed ) ? GetDXUTState().GetWindowBackBufferHeightAtModeChange() : GetDXUTState().GetFullScreenBackBufferHeightAtModeChange();
    if( nWidth > 0 && nHeight > 0 )
    {
        matchOptions.eResolution = DXUTMT_CLOSEST_TO_INPUT;
        deviceSettings.pp.BackBufferWidth = nWidth;
        deviceSettings.pp.BackBufferHeight = nHeight;
    }
    else
    {
        // No previous data, so just switch to defaults
        matchOptions.eResolution = DXUTMT_IGNORE_INPUT;
    }
    
    hr = DXUTFindValidDeviceSettings( &deviceSettings, &deviceSettings, &matchOptions );
    if( SUCCEEDED(hr) ) 
    {
        // Create a Direct3D device using the new device settings.  
        // If there is an existing device, then it will either reset or recreate the scene.
        hr = DXUTChangeDevice( &deviceSettings, NULL, false, false );

        // If hr == E_ABORT, this means the app rejected the device settings in the ModifySettingsCallback so nothing changed
        if( FAILED(hr) && (hr != E_ABORT) )
        {
            // Failed creating device, try to switch back.
            deviceSettings.pp.Windowed = !deviceSettings.pp.Windowed;
            UINT nWidth  = ( deviceSettings.pp.Windowed ) ? GetDXUTState().GetWindowBackBufferWidthAtModeChange() : GetDXUTState().GetFullScreenBackBufferWidthAtModeChange();
            UINT nHeight = ( deviceSettings.pp.Windowed ) ? GetDXUTState().GetWindowBackBufferHeightAtModeChange() : GetDXUTState().GetFullScreenBackBufferHeightAtModeChange();
            if( nWidth > 0 && nHeight > 0 )
            {
                matchOptions.eResolution = DXUTMT_CLOSEST_TO_INPUT;
                deviceSettings.pp.BackBufferWidth = nWidth;
                deviceSettings.pp.BackBufferHeight = nHeight;
            }
            else
            {
                matchOptions.eResolution = DXUTMT_IGNORE_INPUT;
            }
            
            DXUTFindValidDeviceSettings( &deviceSettings, &deviceSettings, &matchOptions );

            HRESULT hr2 = DXUTChangeDevice( &deviceSettings, NULL, false, false );
            if( FAILED(hr2) )
            {
                // If this failed, then shutdown
                DXUTShutdown();
            }
        }
    }

    return hr;
}
HRESULT DXUTToggleREF()
{
    HRESULT hr;

    DXUTDeviceSettings deviceSettings = DXUTGetDeviceSettings();
    if( deviceSettings.DeviceType == D3DDEVTYPE_HAL )
        deviceSettings.DeviceType = D3DDEVTYPE_REF;
    else if( deviceSettings.DeviceType == D3DDEVTYPE_REF )
        deviceSettings.DeviceType = D3DDEVTYPE_HAL;

    DXUTMatchOptions matchOptions;
    matchOptions.eAdapterOrdinal     = DXUTMT_PRESERVE_INPUT;
    matchOptions.eDeviceType         = DXUTMT_PRESERVE_INPUT;
    matchOptions.eWindowed           = DXUTMT_CLOSEST_TO_INPUT;
    matchOptions.eAdapterFormat      = DXUTMT_CLOSEST_TO_INPUT;
    matchOptions.eVertexProcessing   = DXUTMT_CLOSEST_TO_INPUT;
    matchOptions.eResolution         = DXUTMT_CLOSEST_TO_INPUT;
    matchOptions.eBackBufferFormat   = DXUTMT_CLOSEST_TO_INPUT;
    matchOptions.eBackBufferCount    = DXUTMT_CLOSEST_TO_INPUT;
    matchOptions.eMultiSample        = DXUTMT_CLOSEST_TO_INPUT;
    matchOptions.eSwapEffect         = DXUTMT_CLOSEST_TO_INPUT;
    matchOptions.eDepthFormat        = DXUTMT_CLOSEST_TO_INPUT;
    matchOptions.eStencilFormat      = DXUTMT_CLOSEST_TO_INPUT;
    matchOptions.ePresentFlags       = DXUTMT_CLOSEST_TO_INPUT;
    matchOptions.eRefreshRate        = DXUTMT_CLOSEST_TO_INPUT;
    matchOptions.ePresentInterval    = DXUTMT_CLOSEST_TO_INPUT;
    
    hr = DXUTFindValidDeviceSettings( &deviceSettings, &deviceSettings, &matchOptions );
    if( SUCCEEDED(hr) ) 
    {
        // Create a Direct3D device using the new device settings.  
        // If there is an existing device, then it will either reset or recreate the scene.
        hr = DXUTChangeDevice( &deviceSettings, NULL, false, false );

        // If hr == E_ABORT, this means the app rejected the device settings in the ModifySettingsCallback so nothing changed
        if( FAILED( hr ) && (hr != E_ABORT) )
        {
            // Failed creating device, try to switch back.
            if( deviceSettings.DeviceType == D3DDEVTYPE_HAL )
                deviceSettings.DeviceType = D3DDEVTYPE_REF;
            else if( deviceSettings.DeviceType == D3DDEVTYPE_REF )
                deviceSettings.DeviceType = D3DDEVTYPE_HAL;

            DXUTFindValidDeviceSettings( &deviceSettings, &deviceSettings, &matchOptions );

            HRESULT hr2 = DXUTChangeDevice( &deviceSettings, NULL, false, false );
            if( FAILED(hr2) )
            {
                // If this failed, then shutdown
                DXUTShutdown();
            }
        }
    }

    return hr;
}

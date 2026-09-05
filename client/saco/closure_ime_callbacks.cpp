// Complete original DXUT static objects; no unrelated client globals.
#include "d3d9/common/dxstdafx.h"
INPUTCONTEXT* (WINAPI * CDXUTIMEEditBox::_ImmLockIMC)( HIMC ) = CDXUTIMEEditBox::Dummy_ImmLockIMC;
BOOL (WINAPI * CDXUTIMEEditBox::_ImmUnlockIMC)( HIMC ) = CDXUTIMEEditBox::Dummy_ImmUnlockIMC;
LPVOID (WINAPI * CDXUTIMEEditBox::_ImmLockIMCC)( HIMCC ) = CDXUTIMEEditBox::Dummy_ImmLockIMCC;
BOOL (WINAPI * CDXUTIMEEditBox::_ImmUnlockIMCC)( HIMCC ) = CDXUTIMEEditBox::Dummy_ImmUnlockIMCC;
BOOL (WINAPI * CDXUTIMEEditBox::_ImmDisableTextFrameService)( DWORD ) = CDXUTIMEEditBox::Dummy_ImmDisableTextFrameService;
LONG (WINAPI * CDXUTIMEEditBox::_ImmGetCompositionStringW)( HIMC, DWORD, LPVOID, DWORD ) = CDXUTIMEEditBox::Dummy_ImmGetCompositionStringW;
DWORD (WINAPI * CDXUTIMEEditBox::_ImmGetCandidateListW)( HIMC, DWORD, LPCANDIDATELIST, DWORD ) = CDXUTIMEEditBox::Dummy_ImmGetCandidateListW;
HIMC (WINAPI * CDXUTIMEEditBox::_ImmGetContext)( HWND ) = CDXUTIMEEditBox::Dummy_ImmGetContext;
BOOL (WINAPI * CDXUTIMEEditBox::_ImmReleaseContext)( HWND, HIMC ) = CDXUTIMEEditBox::Dummy_ImmReleaseContext;
HIMC (WINAPI * CDXUTIMEEditBox::_ImmAssociateContext)( HWND, HIMC ) = CDXUTIMEEditBox::Dummy_ImmAssociateContext;
BOOL (WINAPI * CDXUTIMEEditBox::_ImmGetOpenStatus)( HIMC ) = CDXUTIMEEditBox::Dummy_ImmGetOpenStatus;
BOOL (WINAPI * CDXUTIMEEditBox::_ImmSetOpenStatus)( HIMC, BOOL ) = CDXUTIMEEditBox::Dummy_ImmSetOpenStatus;
BOOL (WINAPI * CDXUTIMEEditBox::_ImmGetConversionStatus)( HIMC, LPDWORD, LPDWORD ) = CDXUTIMEEditBox::Dummy_ImmGetConversionStatus;
HWND (WINAPI * CDXUTIMEEditBox::_ImmGetDefaultIMEWnd)( HWND ) = CDXUTIMEEditBox::Dummy_ImmGetDefaultIMEWnd;
UINT (WINAPI * CDXUTIMEEditBox::_ImmGetIMEFileNameA)( HKL, LPSTR, UINT ) = CDXUTIMEEditBox::Dummy_ImmGetIMEFileNameA;
UINT (WINAPI * CDXUTIMEEditBox::_ImmGetVirtualKey)( HWND ) = CDXUTIMEEditBox::Dummy_ImmGetVirtualKey;
BOOL (WINAPI * CDXUTIMEEditBox::_ImmNotifyIME)( HIMC, DWORD, DWORD, DWORD ) = CDXUTIMEEditBox::Dummy_ImmNotifyIME;
BOOL (WINAPI * CDXUTIMEEditBox::_ImmSetConversionStatus)( HIMC, DWORD, DWORD ) = CDXUTIMEEditBox::Dummy_ImmSetConversionStatus;
BOOL (WINAPI * CDXUTIMEEditBox::_ImmSimulateHotKey)( HWND, DWORD ) = CDXUTIMEEditBox::Dummy_ImmSimulateHotKey;
BOOL (WINAPI * CDXUTIMEEditBox::_ImmIsIME)( HKL ) = CDXUTIMEEditBox::Dummy_ImmIsIME;
UINT (WINAPI * CDXUTIMEEditBox::_GetReadingString)( HIMC, UINT, PCHAR, PINT, BOOL*, PUINT ) = CDXUTIMEEditBox::Dummy_GetReadingString; // Traditional Chinese IME
BOOL (WINAPI * CDXUTIMEEditBox::_ShowReadingWindow)( HIMC, BOOL ) = CDXUTIMEEditBox::Dummy_ShowReadingWindow; // Traditional Chinese IME
BOOL (APIENTRY * CDXUTIMEEditBox::_VerQueryValueA)( const LPVOID, LPSTR, LPVOID *, PUINT ) = CDXUTIMEEditBox::Dummy_VerQueryValueA;
BOOL (APIENTRY * CDXUTIMEEditBox::_GetFileVersionInfoA)( LPSTR, DWORD, DWORD, LPVOID )= CDXUTIMEEditBox::Dummy_GetFileVersionInfoA;
DWORD (APIENTRY * CDXUTIMEEditBox::_GetFileVersionInfoSizeA)( LPSTR, LPDWORD ) = CDXUTIMEEditBox::Dummy_GetFileVersionInfoSizeA;

WCHAR     CDXUTIMEEditBox::s_aszIndicator[5][3] = // String to draw to indicate current input locale
            {
                L"En",
                L"\x7B80",
                L"\x7E41",
                L"\xAC00",
                L"\x3042",
            };
PWCHAR    CDXUTIMEEditBox::s_wszCurrIndicator = CDXUTIMEEditBox::s_aszIndicator[0];  // Points to an indicator string that corresponds to current input locale

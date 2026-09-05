// Complete original SDK REF warning and callback.
#include "d3d9/common/dxstdafx.h"
INT_PTR CALLBACK DisplaySwitchToREFWarningProc(HWND,UINT,WPARAM,LPARAM);
void DXUTDisplaySwitchingToREFWarning()
{
    if( DXUTGetShowMsgBoxOnError() )
    {
        // Open the appropriate registry key
        DWORD dwSkipWarning = 0;
        HKEY hKey;
        LONG lResult = RegOpenKeyEx( HKEY_CURRENT_USER, "Software\\Microsoft\\DirectX 9.0 SDK", 0, KEY_READ, &hKey );
        if( ERROR_SUCCESS == lResult ) 
        {
            DWORD dwType;
            DWORD dwSize = sizeof(DWORD);
            lResult = RegQueryValueEx( hKey, "Skip Warning On REF", NULL, &dwType, (BYTE*)&dwSkipWarning, &dwSize );
            RegCloseKey( hKey );
        }

        if( dwSkipWarning == 0 )
        {
            // Compact code to create a custom dialog box without using a template in a resource file.
            // If this dialog were in a .rc file, this would be a lot simpler but every sample calling this function would
            // need a copy of the dialog in its own .rc file. Also MessageBox API could be used here instead, but 
            // the MessageBox API is simpler to call but it can't provide a "Don't show again" checkbox
            typedef struct { DLGITEMTEMPLATE a; WORD b; WORD c; WORD d; WORD e; WORD f; } DXUT_DLG_ITEM; 
            typedef struct { DLGTEMPLATE a; WORD b; WORD c; TCHAR d[2]; WORD e; TCHAR f[14]; DXUT_DLG_ITEM i1; DXUT_DLG_ITEM i2; DXUT_DLG_ITEM i3; DXUT_DLG_ITEM i4; DXUT_DLG_ITEM i5; } DXUT_DLG_DATA; 

            DXUT_DLG_DATA dtp = 
            {                                                                                                                                                  
                {WS_CAPTION|WS_POPUP|WS_VISIBLE|WS_SYSMENU|DS_ABSALIGN|DS_3DLOOK|DS_SETFONT|DS_MODALFRAME|DS_CENTER,0,5,0,0,269,82},0,0," ",8,"MS Sans Serif", 
                {{WS_CHILD|WS_VISIBLE|SS_ICON|SS_CENTERIMAGE,0,7,7,24,24,0x100},0xFFFF,0x0082,0,0,0}, // icon
                {{WS_CHILD|WS_VISIBLE,0,40,7,230,25,0x101},0xFFFF,0x0082,0,0,0}, // static text
                {{WS_CHILD|WS_VISIBLE|BS_DEFPUSHBUTTON,0,80,39,50,14,IDYES},0xFFFF,0x0080,0,0,0}, // Yes button
                {{WS_CHILD|WS_VISIBLE,0,133,39,50,14,IDNO},0xFFFF,0x0080,0,0,0}, // No button
                {{WS_CHILD|WS_VISIBLE|BS_CHECKBOX,0,7,59,70,16,IDIGNORE},0xFFFF,0x0080,0,0,0}, // checkbox
            }; 

            int nResult = (int) DialogBoxIndirect( DXUTGetHINSTANCE(), (DLGTEMPLATE*)&dtp, DXUTGetHWND(), DisplaySwitchToREFWarningProc ); 

            if( (nResult & 0x80) == 0x80 ) // "Don't show again" checkbox was checked
            {
                lResult = RegOpenKeyEx( HKEY_CURRENT_USER, "Software\\Microsoft\\DirectX 9.0 SDK", 0, KEY_WRITE, &hKey );
                if( ERROR_SUCCESS == lResult ) 
                {
                    dwSkipWarning = 1;
                    RegSetValueEx( hKey, "Skip Warning On REF", 0, REG_DWORD, (BYTE*)&dwSkipWarning, sizeof(DWORD) );
                    RegCloseKey( hKey );
                }
            }

            // User choose not to continue
            if( (nResult & 0x0F) == IDNO )
                DXUTShutdown(1);
        }
    }
}
INT_PTR CALLBACK DisplaySwitchToREFWarningProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{ 
    switch (message) 
    { 
        case WM_INITDIALOG:
            // Easier to set text here than in the DLGITEMTEMPLATE
            SetWindowText( hDlg, DXUTGetWindowTitle() );
            SendMessage( GetDlgItem(hDlg, 0x100), STM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIcon(0, IDI_QUESTION));
            SetDlgItemText( hDlg, 0x101, "Switching to the Direct3D reference rasterizer, a software device\nthat implements the entire Direct3D feature set, but runs very slowly.\nDo you wish to continue?" ); 
            SetDlgItemText( hDlg, IDYES, "&Yes" );
            SetDlgItemText( hDlg, IDNO, "&No" );
            SetDlgItemText( hDlg, IDIGNORE, "&Don't show again" );
            break;

        case WM_COMMAND: 
            switch (LOWORD(wParam)) 
            { 
                case IDIGNORE: CheckDlgButton( hDlg, IDIGNORE, (IsDlgButtonChecked( hDlg, IDIGNORE ) == BST_CHECKED) ? BST_UNCHECKED : BST_CHECKED ); EnableWindow( GetDlgItem( hDlg, IDNO ), (IsDlgButtonChecked( hDlg, IDIGNORE ) != BST_CHECKED) ); break;
                case IDNO: EndDialog(hDlg, (IsDlgButtonChecked( hDlg, IDIGNORE ) == BST_CHECKED) ? IDNO|0x80 : IDNO|0x00 ); return TRUE; 
                case IDCANCEL:
                case IDYES: EndDialog(hDlg, (IsDlgButtonChecked( hDlg, IDIGNORE ) == BST_CHECKED) ? IDYES|0x80 : IDYES|0x00 ); return TRUE; 
            } 
            break;
    } 
    return FALSE; 
}

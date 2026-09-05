// Complete original DXUT dialog definitions.
#include "main.h"
DXUTFontNode* CDXUTDialog::GetFont( UINT index )
{
    if( NULL == m_pManager )
        return NULL;
    return m_pManager->GetFontNode( m_Fonts.GetAt( index ) );
}
void CDXUTDialog::RequestFocus( CDXUTControl* pControl )
{
    if( s_pControlFocus == pControl )
        return;

    if( !pControl->CanHaveFocus() )
        return;

    if( s_pControlFocus )
        s_pControlFocus->OnFocusOut();

    pControl->OnFocusIn();
    s_pControlFocus = pControl;
}

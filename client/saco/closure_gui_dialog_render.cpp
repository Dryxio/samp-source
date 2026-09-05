// Complete original dialog refresh and focus providers.
#include "d3d9/common/dxstdafx.h"
#include <new>
void CDXUTDialog::Refresh()
{
    if( s_pControlFocus )
        s_pControlFocus->OnFocusOut();

    if( m_pControlMouseOver )
        m_pControlMouseOver->OnMouseLeave();

    s_pControlFocus = NULL;
    s_pControlPressed = NULL;
    m_pControlMouseOver = NULL;

    for( int i=0; i < m_Controls.GetSize(); i++ )
    {
        CDXUTControl* pControl = m_Controls.GetAt(i);
        pControl->Refresh();
    }

    if( m_bKeyboardInput )
        FocusDefaultControl();
}
void CDXUTDialog::FocusDefaultControl()
{
    // Check for default control in this dialog
    for( int i=0; i < m_Controls.GetSize(); i++ )
    {
        CDXUTControl* pControl = m_Controls.GetAt( i );
        if( pControl->m_bIsDefault )
        {
            // Remove focus from the current control
            ClearFocus();

            // Give focus to the default control
            s_pControlFocus = pControl;
            s_pControlFocus->OnFocusIn();
            return;
        }
    }
}
void CDXUTControl::Refresh()
{
    m_bMouseOver = false;
    m_bHasFocus = false;

    for( int i=0; i < m_Elements.GetSize(); i++ )
    {
        CDXUTElement* pElement = m_Elements.GetAt( i );
        pElement->Refresh();
    }
}

// Partial views: reserved bytes are unknown, not reconstructed item fields.
// No allocation, sizeof-based access or coverage claim for these reserved ranges.
#include "d3d9/common/dxstdafx.h"
#include <new>
#include <stddef.h>
#pragma pack(push, 1)
struct R5ListBoxScrollView {
    unsigned char uncharacterized_prefix[0x5d];
    CDXUTScrollBar scrollbar;
};
struct R5ListBoxItemSelectionView {
    unsigned char uncharacterized_prefix[0x298];
    bool selected;
};
#pragma pack(pop)
typedef char VerifyScrollOffset[offsetof(R5ListBoxScrollView, scrollbar)==0x5d ? 1 : -1];
typedef char VerifySelectedOffset[offsetof(R5ListBoxItemSelectionView, selected)==0x298 ? 1 : -1];
bool CDXUTListBox::HandleKeyboard( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    if( !m_bEnabled || !m_bVisible )
        return false;

    // Let the scroll bar have a chance to handle it first
    if( reinterpret_cast<R5ListBoxScrollView*>(this)->scrollbar.HandleKeyboard( uMsg, wParam, lParam ) )
        return true;

    switch( uMsg )
    {
        case WM_KEYDOWN:
            switch( wParam )
            {
                case VK_UP:
                case VK_DOWN:
                case VK_NEXT:
                case VK_PRIOR:
                case VK_HOME:
                case VK_END:

                    // If no item exists, do nothing.
                    if( m_Items.GetSize() == 0 )
                        return true;

                    int nOldSelected = m_nSelected;

                    // Adjust m_nSelected
                    switch( wParam )
                    {
                        case VK_UP: --m_nSelected; break;
                        case VK_DOWN: ++m_nSelected; break;
                        case VK_NEXT: m_nSelected += reinterpret_cast<R5ListBoxScrollView*>(this)->scrollbar.GetPageSize() - 1; break;
                        case VK_PRIOR: m_nSelected -= reinterpret_cast<R5ListBoxScrollView*>(this)->scrollbar.GetPageSize() - 1; break;
                        case VK_HOME: m_nSelected = 0; break;
                        case VK_END: m_nSelected = m_Items.GetSize() - 1; break;
                    }

                    // Perform capping
                    if( m_nSelected < 0 )
                        m_nSelected = 0;
                    if( m_nSelected >= (int)m_Items.GetSize() )
                        m_nSelected = m_Items.GetSize() - 1;

                    if( nOldSelected != m_nSelected )
                    {
                        if( m_dwStyle & MULTISELECTION )
                        {
                            // Multiple selection

                            // Clear all selection
                            for( int i = 0; i < (int)m_Items.GetSize(); ++i )
                            {
                                DXUTListBoxItem *pItem = m_Items[i];
                                reinterpret_cast<R5ListBoxItemSelectionView*>(pItem)->selected = false;
                            }

                            if( GetKeyState( VK_SHIFT ) < 0 )
                            {
                                // Select all items from m_nSelStart to
                                // m_nSelected
                                int nEnd = __max( m_nSelStart, m_nSelected );

                                for( int n = __min( m_nSelStart, m_nSelected ); n <= nEnd; ++n )
                                    reinterpret_cast<R5ListBoxItemSelectionView*>(m_Items[n])->selected = true;
                            }
                            else
                            {
                                reinterpret_cast<R5ListBoxItemSelectionView*>(m_Items[m_nSelected])->selected = true;

                                // Update selection start
                                m_nSelStart = m_nSelected;
                            }
                        } else
                            m_nSelStart = m_nSelected;

                        // Adjust scroll bar

                        reinterpret_cast<R5ListBoxScrollView*>(this)->scrollbar.ShowItem( m_nSelected );

                        // Send notification

                        m_pDialog->SendEvent( EVENT_LISTBOX_SELECTION, true, this );
                    }
                    return true;
            }
            break;
    }

    return false;
}
bool CDXUTScrollBar::HandleKeyboard( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    return false;
}

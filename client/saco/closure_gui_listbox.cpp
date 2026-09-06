// Partial views: reserved bytes are unknown, not reconstructed item fields.
// No allocation, sizeof-based access or coverage claim for these reserved ranges.
#include "d3d9/common/dxstdafx.h"
#include <new>
#include <stddef.h>
#ifndef WHEEL_DELTA
#define WHEEL_DELTA 120
#endif
#pragma pack(push, 1)
struct R5ListBoxScrollView {
    unsigned char uncharacterized_prefix[0x5d];
    CDXUTScrollBar scrollbar;
    RECT text;
    RECT selection;
};
struct R5ListBoxItemSelectionView {
    unsigned char uncharacterized_prefix[0x298];
    bool selected;
};
#pragma pack(pop)
typedef char VerifyScrollOffset[offsetof(R5ListBoxScrollView, scrollbar)==0x5d ? 1 : -1];
typedef char VerifyTextOffset[offsetof(R5ListBoxScrollView, text)==0x10f ? 1 : -1];
typedef char VerifySelectionOffset[offsetof(R5ListBoxScrollView, selection)==0x11f ? 1 : -1];
typedef char VerifySelectedOffset[offsetof(R5ListBoxItemSelectionView, selected)==0x298 ? 1 : -1];
inline int RectWidth( RECT &rc ) { return ( (rc).right - (rc).left ); }
inline int RectHeight( RECT &rc ) { return ( (rc).bottom - (rc).top ); }
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
bool CDXUTListBox::HandleMouse( UINT uMsg, POINT pt, WPARAM wParam, LPARAM lParam )
{
    if( !m_bEnabled || !m_bVisible )
        return false;

    // First acquire focus
    if( WM_LBUTTONDOWN == uMsg )
        if( !m_bHasFocus )
            m_pDialog->RequestFocus( this );

    // Let the scroll bar handle it first.
    if( reinterpret_cast<R5ListBoxScrollView*>(this)->scrollbar.HandleMouse( uMsg, pt, wParam, lParam ) )
        return true;

    switch( uMsg )
    {
        case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
            // Check for clicks in the text area
            if( m_Items.GetSize() > 0 && PtInRect( &reinterpret_cast<R5ListBoxScrollView*>(this)->selection, pt ) )
            {
                // Compute the index of the clicked item

                int nClicked;
                if( m_nTextHeight )
                    nClicked = reinterpret_cast<R5ListBoxScrollView*>(this)->scrollbar.GetTrackPos() + ( pt.y - reinterpret_cast<R5ListBoxScrollView*>(this)->text.top ) / m_nTextHeight;
                else
                    nClicked = -1;

                // Only proceed if the click falls on top of an item.

                if( nClicked >= reinterpret_cast<R5ListBoxScrollView*>(this)->scrollbar.GetTrackPos() &&
                    nClicked < (int)m_Items.GetSize() &&
                    nClicked < reinterpret_cast<R5ListBoxScrollView*>(this)->scrollbar.GetTrackPos() + reinterpret_cast<R5ListBoxScrollView*>(this)->scrollbar.GetPageSize() )
                {
                    SetCapture( DXUTGetHWND() );
                    

                    // If this is a double click, fire off an event and exit
                    // since the first click would have taken care of the selection
                    // updating.
                    if( uMsg == WM_LBUTTONDBLCLK )
                    {
                        m_pDialog->SendEvent( EVENT_LISTBOX_ITEM_DBLCLK, true, this );
                        return true;
                    }

                    m_nSelected = nClicked;
                    if( !( wParam & MK_SHIFT ) )
                        m_nSelStart = m_nSelected;

                    // If this is a multi-selection listbox, update per-item
                    // selection data.

                    if( m_dwStyle & MULTISELECTION )
                    {
                        // Determine behavior based on the state of Shift and Ctrl

                        DXUTListBoxItem *pSelItem = m_Items.GetAt( m_nSelected );
                        if( ( wParam & (MK_SHIFT|MK_CONTROL) ) == MK_CONTROL )
                        {
                            // Control click. Reverse the selection of this item.

                            reinterpret_cast<R5ListBoxItemSelectionView*>(pSelItem)->selected = !reinterpret_cast<R5ListBoxItemSelectionView*>(pSelItem)->selected;
                        } else
                        if( ( wParam & (MK_SHIFT|MK_CONTROL) ) == MK_SHIFT )
                        {
                            // Shift click. Set the selection for all items
                            // from last selected item to the current item.
                            // Clear everything else.

                            int nBegin = __min( m_nSelStart, m_nSelected );
                            int nEnd = __max( m_nSelStart, m_nSelected );

                            for( int i = 0; i < nBegin; ++i )
                            {
                                DXUTListBoxItem *pItem = m_Items.GetAt( i );
                                reinterpret_cast<R5ListBoxItemSelectionView*>(pItem)->selected = false;
                            }

                            for( int i = nEnd + 1; i < (int)m_Items.GetSize(); ++i )
                            {
                                DXUTListBoxItem *pItem = m_Items.GetAt( i );
                                reinterpret_cast<R5ListBoxItemSelectionView*>(pItem)->selected = false;
                            }

                            for( int i = nBegin; i <= nEnd; ++i )
                            {
                                DXUTListBoxItem *pItem = m_Items.GetAt( i );
                                reinterpret_cast<R5ListBoxItemSelectionView*>(pItem)->selected = true;
                            }
                        } else
                        if( ( wParam & (MK_SHIFT|MK_CONTROL) ) == ( MK_SHIFT|MK_CONTROL ) )
                        {
                            // Control-Shift-click.

                            // The behavior is:
                            //   Set all items from m_nSelStart to m_nSelected to
                            //     the same state as m_nSelStart, not including m_nSelected.
                            //   Set m_nSelected to selected.

                            int nBegin = __min( m_nSelStart, m_nSelected );
                            int nEnd = __max( m_nSelStart, m_nSelected );

                            // The two ends do not need to be set here.

                            bool bLastSelected = reinterpret_cast<R5ListBoxItemSelectionView*>(m_Items.GetAt( m_nSelStart ))->selected;
                            for( int i = nBegin + 1; i < nEnd; ++i )
                            {
                                DXUTListBoxItem *pItem = m_Items.GetAt( i );
                                reinterpret_cast<R5ListBoxItemSelectionView*>(pItem)->selected = bLastSelected;
                            }

                            reinterpret_cast<R5ListBoxItemSelectionView*>(pSelItem)->selected = true;

                            // Restore m_nSelected to the previous value
                            // This matches the Windows behavior

                            m_nSelected = m_nSelStart;
                        } else
                        {
                            // Simple click.  Clear all items and select the clicked
                            // item.


                            for( int i = 0; i < (int)m_Items.GetSize(); ++i )
                            {
                                DXUTListBoxItem *pItem = m_Items.GetAt( i );
                                reinterpret_cast<R5ListBoxItemSelectionView*>(pItem)->selected = false;
                            }

                            reinterpret_cast<R5ListBoxItemSelectionView*>(pSelItem)->selected = true;
                        }
                    }  // End of multi-selection case

                    m_pDialog->SendEvent( EVENT_LISTBOX_SELECTION, true, this );
                }

                return true;
            }
            break;

        case WM_LBUTTONUP:
        {
            ReleaseCapture();
            m_bDrag = false;

            if( m_nSelected < m_Items.GetSize() && m_nSelected != -1 )
            {
                // Set all items between m_nSelStart and m_nSelected to
                // the same state as m_nSelStart
                int nEnd = __max( m_nSelStart, m_nSelected );

                for( int n = __min( m_nSelStart, m_nSelected ) + 1; n < nEnd; ++n )
                    reinterpret_cast<R5ListBoxItemSelectionView*>(m_Items[n])->selected = reinterpret_cast<R5ListBoxItemSelectionView*>(m_Items[m_nSelStart])->selected;
                reinterpret_cast<R5ListBoxItemSelectionView*>(m_Items[m_nSelected])->selected = reinterpret_cast<R5ListBoxItemSelectionView*>(m_Items[m_nSelStart])->selected;

                // If m_nSelStart and m_nSelected are not the same,
                // the user has dragged the mouse to make a selection.
                // Notify the application of this.
                if( m_nSelStart != m_nSelected )
                    m_pDialog->SendEvent( EVENT_LISTBOX_SELECTION, true, this );
            }
            return false;
        }

        case WM_MOUSEMOVE:
            if( m_bDrag )
            {
                // Compute the index of the item below cursor

                int nItem;
                if( m_nTextHeight )
                    nItem = reinterpret_cast<R5ListBoxScrollView*>(this)->scrollbar.GetTrackPos() + ( pt.y - reinterpret_cast<R5ListBoxScrollView*>(this)->text.top ) / m_nTextHeight;
                else
                    nItem = -1;

                // Only proceed if the cursor is on top of an item.

                if( nItem >= (int)reinterpret_cast<R5ListBoxScrollView*>(this)->scrollbar.GetTrackPos() &&
                    nItem < (int)m_Items.GetSize() &&
                    nItem < reinterpret_cast<R5ListBoxScrollView*>(this)->scrollbar.GetTrackPos() + reinterpret_cast<R5ListBoxScrollView*>(this)->scrollbar.GetPageSize() )
                {
                    m_nSelected = nItem;
                    m_pDialog->SendEvent( EVENT_LISTBOX_SELECTION, true, this );
                } else
                if( nItem < (int)reinterpret_cast<R5ListBoxScrollView*>(this)->scrollbar.GetTrackPos() )
                {
                    // User drags the mouse above window top
                    reinterpret_cast<R5ListBoxScrollView*>(this)->scrollbar.Scroll( -1 );
                    m_nSelected = reinterpret_cast<R5ListBoxScrollView*>(this)->scrollbar.GetTrackPos();
                    m_pDialog->SendEvent( EVENT_LISTBOX_SELECTION, true, this );
                } else
                if( nItem >= reinterpret_cast<R5ListBoxScrollView*>(this)->scrollbar.GetTrackPos() + reinterpret_cast<R5ListBoxScrollView*>(this)->scrollbar.GetPageSize() )
                {
                    // User drags the mouse below window bottom
                    reinterpret_cast<R5ListBoxScrollView*>(this)->scrollbar.Scroll( 1 );
                    m_nSelected = __min( (int)m_Items.GetSize(), reinterpret_cast<R5ListBoxScrollView*>(this)->scrollbar.GetTrackPos() + reinterpret_cast<R5ListBoxScrollView*>(this)->scrollbar.GetPageSize() ) - 1;
                    m_pDialog->SendEvent( EVENT_LISTBOX_SELECTION, true, this );
                }
            }
            break;

        case WM_MOUSEWHEEL:
        {
            UINT uLines;
            SystemParametersInfo( SPI_GETWHEELSCROLLLINES, 0, &uLines, 0 );
            int nScrollAmount = int((short)HIWORD(wParam)) / WHEEL_DELTA * uLines;
            reinterpret_cast<R5ListBoxScrollView*>(this)->scrollbar.Scroll( -nScrollAmount );
            return true;
        }
    }

    return false;
}
void CDXUTListBox::SelectItem( int nNewIndex )
{
    // If no item exists, do nothing.
    if( m_Items.GetSize() == 0 )
        return;

    int nOldSelected = m_nSelected;

    // Adjust m_nSelected
    m_nSelected = nNewIndex;

    // Perform capping
    if( m_nSelected < 0 )
        m_nSelected = 0;
    if( m_nSelected >= (int)m_Items.GetSize() )
        m_nSelected = m_Items.GetSize() - 1;

    if( nOldSelected != m_nSelected )
    {
        if( m_dwStyle & MULTISELECTION )
        {
            reinterpret_cast<R5ListBoxItemSelectionView*>(m_Items[m_nSelected])->selected = true;
        }

        // Update selection start
        m_nSelStart = m_nSelected;

        // Adjust scroll bar
        reinterpret_cast<R5ListBoxScrollView*>(this)->scrollbar.ShowItem( m_nSelected );
    }

    m_pDialog->SendEvent( EVENT_LISTBOX_SELECTION, true, this );
}
int CDXUTListBox::GetSelectedIndex( int nPreviousSelected )
{
    if( nPreviousSelected < -1 )
        return -1;

    if( m_dwStyle & MULTISELECTION )
    {
        // Multiple selection enabled. Search for the next item with the selected flag.
        for( int i = nPreviousSelected + 1; i < (int)m_Items.GetSize(); ++i )
        {
            DXUTListBoxItem *pItem = m_Items.GetAt( i );

            if( reinterpret_cast<R5ListBoxItemSelectionView*>(pItem)->selected )
                return i;
        }

        return -1;
    }
    else
    {
        // Single selection
        return m_nSelected;
    }
}
DXUTListBoxItem *CDXUTListBox::GetItem( int nIndex )
{
    if( nIndex < 0 || nIndex >= (int)m_Items.GetSize() )
        return NULL;

    return m_Items[nIndex];
}
void CDXUTListBox::RemoveItem( int nIndex )
{
    if( nIndex < 0 || nIndex >= (int)m_Items.GetSize() )
        return;

    DXUTListBoxItem *pItem = m_Items.GetAt( nIndex );

    delete pItem;
    m_Items.Remove( nIndex );
    reinterpret_cast<R5ListBoxScrollView*>(this)->scrollbar.SetTrackRange( 0, m_Items.GetSize() );
    if( m_nSelected >= (int)m_Items.GetSize() )
        m_nSelected = m_Items.GetSize() - 1;

    m_pDialog->SendEvent( EVENT_LISTBOX_SELECTION, true, this );
}
// RemoveAllItems moves, unchanged, to widget_lifetime with the full ListBox destructor.
bool CDXUTScrollBar::HandleKeyboard( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    return false;
}

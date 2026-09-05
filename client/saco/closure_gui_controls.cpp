// Complete original DXUT control render/update definitions.
#include "d3d9/common/dxstdafx.h"
#include <new>
#define SCROLLBAR_MINTHUMBSIZE 8
#define SCROLLBAR_ARROWCLICK_DELAY 0.33
#define SCROLLBAR_ARROWCLICK_REPEAT 0.05
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
void CDXUTComboBox::Render( IDirect3DDevice9* pd3dDevice, float fElapsedTime )
{
    DXUT_CONTROL_STATE iState = DXUT_STATE_NORMAL;
    
    if( !m_bOpened )
        iState = DXUT_STATE_HIDDEN;

    // Dropdown box
    CDXUTElement* pElement = m_Elements.GetAt( 2 );

    // If we have not initialized the scroll bar page size,
    // do that now.
    static bool bSBInit;
    if( !bSBInit )
    {
        // Update the page size of the scroll bar
        if( m_pDialog->GetManager()->GetFontNode( pElement->iFont )->nHeight )
            m_ScrollBar.SetPageSize( RectHeight( m_rcDropdownText ) / m_pDialog->GetManager()->GetFontNode( pElement->iFont )->nHeight );
        else
            m_ScrollBar.SetPageSize( RectHeight( m_rcDropdownText ) );
        bSBInit = true;
    }

    // Scroll bar
    if( m_bOpened )
        m_ScrollBar.Render( pd3dDevice, fElapsedTime );

    // Blend current color
    pElement->TextureColor.Blend( iState, fElapsedTime );
    pElement->FontColor.Blend( iState, fElapsedTime );

    m_pDialog->DrawSprite( pElement, &m_rcDropdown );

    // Selection outline
    CDXUTElement* pSelectionElement = m_Elements.GetAt( 3 );
    pSelectionElement->TextureColor.Current = pElement->TextureColor.Current;
    pSelectionElement->FontColor.Current = pSelectionElement->FontColor.States[ DXUT_STATE_NORMAL ];

    DXUTFontNode* pFont = m_pDialog->GetFont( pElement->iFont );
    if( pFont )
    {
        int curY = m_rcDropdownText.top;
        int nRemainingHeight = RectHeight( m_rcDropdownText );
        //TCHAR strDropdown[4096] = {0};

        for( int i = m_ScrollBar.GetTrackPos(); i < m_Items.GetSize(); i++ )
        {
            DXUTComboBoxItem* pItem = m_Items.GetAt( i );

            // Make sure there's room left in the dropdown
            nRemainingHeight -= pFont->nHeight;
            if( nRemainingHeight < 0 )
            {
                pItem->bVisible = false;
                continue;
            }

            SetRect( &pItem->rcActive, m_rcDropdownText.left, curY, m_rcDropdownText.right, curY + pFont->nHeight );
            curY += pFont->nHeight;
            
            //debug
            //int blue = 50 * i;
            //m_pDialog->DrawRect( &pItem->rcActive, 0xFFFF0000 | blue );

            pItem->bVisible = true;

            if( m_bOpened )
            {
                if( (int)i == m_iFocused )
                {
                    RECT rc;
                    SetRect( &rc, m_rcDropdown.left, pItem->rcActive.top-2, m_rcDropdown.right, pItem->rcActive.bottom+2 );
                    m_pDialog->DrawSprite( pSelectionElement, &rc );
                    m_pDialog->DrawText( pItem->strText, pSelectionElement, &pItem->rcActive );
                }
                else
                {
                    m_pDialog->DrawText( pItem->strText, pElement, &pItem->rcActive );
                }
            }
        }
    }

    int nOffsetX = 0;
    int nOffsetY = 0;

    iState = DXUT_STATE_NORMAL;
    
    if( m_bVisible == false )
        iState = DXUT_STATE_HIDDEN;
    else if( m_bEnabled == false )
        iState = DXUT_STATE_DISABLED;
    else if( m_bPressed )
    {
        iState = DXUT_STATE_PRESSED;

        nOffsetX = 1;
        nOffsetY = 2;
    }
    else if( m_bMouseOver )
    {
        iState = DXUT_STATE_MOUSEOVER;

        nOffsetX = -1;
        nOffsetY = -2;
    }
    else if( m_bHasFocus )
        iState = DXUT_STATE_FOCUS;

    float fBlendRate = ( iState == DXUT_STATE_PRESSED ) ? 0.0f : 0.8f;
    
    // Button
    pElement = m_Elements.GetAt( 1 );
    
    // Blend current color
    pElement->TextureColor.Blend( iState, fElapsedTime, fBlendRate );
    
    RECT rcWindow = m_rcButton;
    OffsetRect( &rcWindow, nOffsetX, nOffsetY );
    m_pDialog->DrawSprite( pElement, &rcWindow );

    if( m_bOpened )
        iState = DXUT_STATE_PRESSED;

    // Main text box
    //TODO: remove magic numbers
    pElement = m_Elements.GetAt( 0 );
    
    // Blend current color
    pElement->TextureColor.Blend( iState, fElapsedTime, fBlendRate );
    pElement->FontColor.Blend( iState, fElapsedTime, fBlendRate );

    m_pDialog->DrawSprite( pElement, &m_rcText);
    
    if( m_iSelected >= 0 && m_iSelected < (int) m_Items.GetSize() )
    {
        DXUTComboBoxItem* pItem = m_Items.GetAt( m_iSelected );
        if( pItem != NULL )
        {
            m_pDialog->DrawText( pItem->strText, pElement, &m_rcText );
        
        }
    }
}
void CDXUTButton::Render( IDirect3DDevice9* pd3dDevice, float fElapsedTime )
{
    int nOffsetX = 0;
    int nOffsetY = 0;

    DXUT_CONTROL_STATE iState = DXUT_STATE_NORMAL;

    if( m_bVisible == false )
    {
        iState = DXUT_STATE_HIDDEN;
    }
    else if( m_bEnabled == false )
    {
        iState = DXUT_STATE_DISABLED;
    }
    else if( m_bPressed )
    {
        iState = DXUT_STATE_PRESSED;

        nOffsetX = 1;
        nOffsetY = 2;
    }
    else if( m_bMouseOver )
    {
        iState = DXUT_STATE_MOUSEOVER;

        nOffsetX = -1;
        nOffsetY = -2;
    }
    else if( m_bHasFocus )
    {
        iState = DXUT_STATE_FOCUS;
    }
    
    // Background fill layer
    //TODO: remove magic numbers
    CDXUTElement* pElement = m_Elements.GetAt( 0 );
    
    float fBlendRate = ( iState == DXUT_STATE_PRESSED ) ? 0.0f : 0.8f;

    RECT rcWindow = m_rcBoundingBox;
    OffsetRect( &rcWindow, nOffsetX, nOffsetY );

 
    // Blend current color
    pElement->TextureColor.Blend( iState, fElapsedTime, fBlendRate );
    pElement->FontColor.Blend( iState, fElapsedTime, fBlendRate );

    m_pDialog->DrawSprite( pElement, &rcWindow );
    m_pDialog->DrawText( m_strText, pElement, &rcWindow );

    // Main button
    pElement = m_Elements.GetAt( 1 );


    // Blend current color
    pElement->TextureColor.Blend( iState, fElapsedTime, fBlendRate );
    pElement->FontColor.Blend( iState, fElapsedTime, fBlendRate );

    m_pDialog->DrawSprite( pElement, &rcWindow );
    m_pDialog->DrawText( m_strText, pElement, &rcWindow );
}
void CDXUTCheckBox::Render( IDirect3DDevice9* pd3dDevice, float fElapsedTime )
{
    DXUT_CONTROL_STATE iState = DXUT_STATE_NORMAL;

    if( m_bVisible == false )
        iState = DXUT_STATE_HIDDEN;
    else if( m_bEnabled == false )
        iState = DXUT_STATE_DISABLED;
    else if( m_bPressed )
        iState = DXUT_STATE_PRESSED;
    else if( m_bMouseOver )
        iState = DXUT_STATE_MOUSEOVER;
    else if( m_bHasFocus )
        iState = DXUT_STATE_FOCUS;

    CDXUTElement* pElement = m_Elements.GetAt( 0 );

    float fBlendRate = ( iState == DXUT_STATE_PRESSED ) ? 0.0f : 0.8f;

    pElement->TextureColor.Blend( iState, fElapsedTime, fBlendRate );
    pElement->FontColor.Blend( iState, fElapsedTime, fBlendRate );

    m_pDialog->DrawSprite( pElement, &m_rcButton );
    m_pDialog->DrawText( m_strText, pElement, &m_rcText, true );

    if( !m_bChecked )
        iState = DXUT_STATE_HIDDEN;

    pElement = m_Elements.GetAt( 1 );

    pElement->TextureColor.Blend( iState, fElapsedTime, fBlendRate );
    m_pDialog->DrawSprite( pElement, &m_rcButton );
}
void CDXUTSlider::Render( IDirect3DDevice9* pd3dDevice, float fElapsedTime )
{
    int nOffsetX = 0;
    int nOffsetY = 0;

    DXUT_CONTROL_STATE iState = DXUT_STATE_NORMAL;

    if( m_bVisible == false )
    {
        iState = DXUT_STATE_HIDDEN;
    }
    else if( m_bEnabled == false )
    {
        iState = DXUT_STATE_DISABLED;
    }
    else if( m_bPressed )
    {
        iState = DXUT_STATE_PRESSED;

        nOffsetX = 1;
        nOffsetY = 2;
    }
    else if( m_bMouseOver )
    {
        iState = DXUT_STATE_MOUSEOVER;
        
        nOffsetX = -1;
        nOffsetY = -2;
    }
    else if( m_bHasFocus )
    {
        iState = DXUT_STATE_FOCUS;
    }

    float fBlendRate = ( iState == DXUT_STATE_PRESSED ) ? 0.0f : 0.8f;

    CDXUTElement* pElement = m_Elements.GetAt( 0 );
    
    // Blend current color
    pElement->TextureColor.Blend( iState, fElapsedTime, fBlendRate ); 
    m_pDialog->DrawSprite( pElement, &m_rcBoundingBox );

    //TODO: remove magic numbers
    pElement = m_Elements.GetAt( 1 );

	// Blend current color
	if(field_76)
		pElement->TextureColor.Current = field_77;
	else
		pElement->TextureColor.Blend( iState, fElapsedTime, fBlendRate );

    m_pDialog->DrawSprite( pElement, &m_rcButton );
}
void CDXUTStatic::Render( IDirect3DDevice9* pd3dDevice, float fElapsedTime )
{    
    if( m_bVisible == false )
        return;

    DXUT_CONTROL_STATE iState = DXUT_STATE_NORMAL;

    if( m_bEnabled == false )
        iState = DXUT_STATE_DISABLED;
        
    CDXUTElement* pElement = m_Elements.GetAt( 0 );

    pElement->FontColor.Blend( iState, fElapsedTime );
    
    m_pDialog->DrawText( m_strText, pElement, &m_rcBoundingBox, true );
}
void CDXUTControl::UpdateRects()
{
    SetRect( &m_rcBoundingBox, m_x, m_y, m_x + m_width, m_y + m_height );
}
void CDXUTComboBox::UpdateRects()
{
    
    CDXUTButton::UpdateRects();

    m_rcButton = m_rcBoundingBox;
    m_rcButton.left = m_rcButton.right - RectHeight( m_rcButton );

    m_rcText = m_rcBoundingBox;
    m_rcText.right = m_rcButton.left;

    m_rcDropdown = m_rcText;
    OffsetRect( &m_rcDropdown, 0, (int) (0.90f * RectHeight( m_rcText )) );
    m_rcDropdown.bottom += m_nDropHeight;
    m_rcDropdown.right -= m_nSBWidth;

    m_rcDropdownText = m_rcDropdown;
    m_rcDropdownText.left += (int) (0.1f * RectWidth( m_rcDropdown ));
    m_rcDropdownText.right -= (int) (0.1f * RectWidth( m_rcDropdown ));
    m_rcDropdownText.top += (int) (0.1f * RectHeight( m_rcDropdown ));
    m_rcDropdownText.bottom -= (int) (0.1f * RectHeight( m_rcDropdown ));

    // Update the scrollbar's rects
    m_ScrollBar.SetLocation( m_rcDropdown.right, m_rcDropdown.top+2 );
    m_ScrollBar.SetSize( m_nSBWidth, RectHeight( m_rcDropdown )-2 );
    DXUTFontNode* pFontNode = m_pDialog->GetManager()->GetFontNode( m_Elements.GetAt( 2 )->iFont );
    if( pFontNode && pFontNode->nHeight )
    {
        m_ScrollBar.SetPageSize( RectHeight( m_rcDropdownText ) / pFontNode->nHeight );

        // The selected item may have been scrolled off the page.
        // Ensure that it is in page again.
        m_ScrollBar.ShowItem( m_iSelected );
    }
}
void CDXUTEditBox::UpdateRects()
{
    CDXUTControl::UpdateRects();

    // Update the text rectangle
    m_rcText = m_rcBoundingBox;
    // First inflate by m_nBorder to compute render rects
    InflateRect( &m_rcText, -m_nBorder, -m_nBorder );

    // Update the render rectangles
    m_rcRender[0] = m_rcText;
    SetRect( &m_rcRender[1], m_rcBoundingBox.left, m_rcBoundingBox.top, m_rcText.left, m_rcText.top );
    SetRect( &m_rcRender[2], m_rcText.left, m_rcBoundingBox.top, m_rcText.right, m_rcText.top );
    SetRect( &m_rcRender[3], m_rcText.right, m_rcBoundingBox.top, m_rcBoundingBox.right, m_rcText.top );
    SetRect( &m_rcRender[4], m_rcBoundingBox.left, m_rcText.top, m_rcText.left, m_rcText.bottom );
    SetRect( &m_rcRender[5], m_rcText.right, m_rcText.top, m_rcBoundingBox.right, m_rcText.bottom );
    SetRect( &m_rcRender[6], m_rcBoundingBox.left, m_rcText.bottom, m_rcText.left, m_rcBoundingBox.bottom );
    SetRect( &m_rcRender[7], m_rcText.left, m_rcText.bottom, m_rcText.right, m_rcBoundingBox.bottom );
    SetRect( &m_rcRender[8], m_rcText.right, m_rcText.bottom, m_rcBoundingBox.right, m_rcBoundingBox.bottom );

    // Inflate further by m_nSpacing
    InflateRect( &m_rcText, -m_nSpacing, -m_nSpacing );
}
void CDXUTSlider::UpdateRects()
{
    CDXUTControl::UpdateRects();

    m_rcButton = m_rcBoundingBox;
    m_rcButton.right = m_rcButton.left + RectHeight( m_rcButton );
    OffsetRect( &m_rcButton, -RectWidth( m_rcButton )/2, 0 );

    m_nButtonX = (int) ( (m_nValue - m_nMin) * (float)RectWidth( m_rcBoundingBox ) / (m_nMax - m_nMin) );
    OffsetRect( &m_rcButton, m_nButtonX, 0 );
}
void CDXUTCheckBox::UpdateRects()
{
    CDXUTButton::UpdateRects();

    m_rcButton = m_rcBoundingBox;
    m_rcButton.right = m_rcButton.left + RectHeight( m_rcButton );

    m_rcText = m_rcBoundingBox;
    m_rcText.left += (int) ( 1.25f * RectWidth( m_rcButton ) );
}
void CDXUTScrollBar::UpdateRects()
{
    CDXUTControl::UpdateRects();

    // Make the buttons square

    SetRect( &m_rcUpButton, m_rcBoundingBox.left, m_rcBoundingBox.top,
                            m_rcBoundingBox.right, m_rcBoundingBox.top + RectWidth( m_rcBoundingBox ) );
    SetRect( &m_rcDownButton, m_rcBoundingBox.left, m_rcBoundingBox.bottom - RectWidth( m_rcBoundingBox ),
                              m_rcBoundingBox.right, m_rcBoundingBox.bottom );
    SetRect( &m_rcTrack, m_rcUpButton.left, m_rcUpButton.bottom,
                         m_rcDownButton.right, m_rcDownButton.top );
    m_rcThumb.left = m_rcUpButton.left;
    m_rcThumb.right = m_rcUpButton.right;

    UpdateThumbRect();
}
void CDXUTScrollBar::ShowItem( int nIndex )
{
    // Cap the index

    if( nIndex < 0 )
        nIndex = 0;

    if( nIndex >= m_nEnd )
        nIndex = m_nEnd - 1;

    // Adjust position

    if( m_nPosition > nIndex )
        m_nPosition = nIndex;
    else
    if( m_nPosition + m_nPageSize <= nIndex )
        m_nPosition = nIndex - m_nPageSize + 1;

    UpdateThumbRect();
}
void CDXUTScrollBar::Cap()  // Clips position at boundaries. Ensures it stays within legal range.
{
    if( m_nPosition < m_nStart ||
        m_nEnd - m_nStart <= m_nPageSize )
    {
        m_nPosition = m_nStart;
    }
    else
    if( m_nPosition + m_nPageSize > m_nEnd )
        m_nPosition = m_nEnd - m_nPageSize;
}
void CDXUTScrollBar::UpdateThumbRect()
{
    if( m_nEnd - m_nStart > m_nPageSize )
    {
        int nThumbHeight = __max( RectHeight( m_rcTrack ) * m_nPageSize / ( m_nEnd - m_nStart ), SCROLLBAR_MINTHUMBSIZE );
        int nMaxPosition = m_nEnd - m_nStart - m_nPageSize;
        m_rcThumb.top = m_rcTrack.top + ( m_nPosition - m_nStart ) * ( RectHeight( m_rcTrack ) - nThumbHeight )
                        / nMaxPosition;
        m_rcThumb.bottom = m_rcThumb.top + nThumbHeight;
        m_bShowThumb = true;

    } 
    else
    {
        // No content to scroll
        m_rcThumb.bottom = m_rcThumb.top;
        m_bShowThumb = false;
    }
}
void CDXUTScrollBar::Render( IDirect3DDevice9* pd3dDevice, float fElapsedTime )
{
    // Check if the arrow button has been held for a while.
    // If so, update the thumb position to simulate repeated
    // scroll.
    if( m_Arrow != CLEAR )
    {
        double dCurrTime = DXUTGetTime();
        if( PtInRect( &m_rcUpButton, m_LastMouse ) )
        {
            switch( m_Arrow )
            {
                case CLICKED_UP:
                    if( SCROLLBAR_ARROWCLICK_DELAY < dCurrTime - m_dArrowTS )
                    {
                        Scroll( -1 );
                        m_Arrow = HELD_UP;
                        m_dArrowTS = dCurrTime;
                    }
                    break;
                case HELD_UP:
                    if( SCROLLBAR_ARROWCLICK_REPEAT < dCurrTime - m_dArrowTS )
                    {
                        Scroll( -1 );
                        m_dArrowTS = dCurrTime;
                    }
                    break;
            }
        } else
        if( PtInRect( &m_rcDownButton, m_LastMouse ) )
        {
            switch( m_Arrow )
            {
                case CLICKED_DOWN:
                    if( SCROLLBAR_ARROWCLICK_DELAY < dCurrTime - m_dArrowTS )
                    {
                        Scroll( 1 );
                        m_Arrow = HELD_DOWN;
                        m_dArrowTS = dCurrTime;
                    }
                    break;
                case HELD_DOWN:
                    if( SCROLLBAR_ARROWCLICK_REPEAT < dCurrTime - m_dArrowTS )
                    {
                        Scroll( 1 );
                        m_dArrowTS = dCurrTime;
                    }
                    break;
            }
        }
    }

    DXUT_CONTROL_STATE iState = DXUT_STATE_NORMAL;

    if( m_bVisible == false )
        iState = DXUT_STATE_HIDDEN;
    else if( m_bEnabled == false || m_bShowThumb == false )
        iState = DXUT_STATE_DISABLED;
    else if( m_bMouseOver )
        iState = DXUT_STATE_MOUSEOVER;
    else if( m_bHasFocus )
        iState = DXUT_STATE_FOCUS;


    float fBlendRate = ( iState == DXUT_STATE_PRESSED ) ? 0.0f : 0.8f;

    // Background track layer
    CDXUTElement* pElement = m_Elements.GetAt( 0 );
    
    // Blend current color
    pElement->TextureColor.Blend( iState, fElapsedTime, fBlendRate );
    m_pDialog->DrawSprite( pElement, &m_rcTrack );

    // Up Arrow
    pElement = m_Elements.GetAt( 1 );
    
    // Blend current color
    pElement->TextureColor.Blend( iState, fElapsedTime, fBlendRate );
    m_pDialog->DrawSprite( pElement, &m_rcUpButton );

    // Down Arrow
    pElement = m_Elements.GetAt( 2 );
    
    // Blend current color
    pElement->TextureColor.Blend( iState, fElapsedTime, fBlendRate );
    m_pDialog->DrawSprite( pElement, &m_rcDownButton );

    // Thumb button
    pElement = m_Elements.GetAt( 3 );
    
    // Blend current color
    pElement->TextureColor.Blend( iState, fElapsedTime, fBlendRate );
    m_pDialog->DrawSprite( pElement, &m_rcThumb );
 
}
void CDXUTScrollBar::Scroll( int nDelta )
{
    // Perform scroll
    m_nPosition += nDelta;

    // Cap position
    Cap();

    // Update thumb position
    UpdateThumbRect();
}
void CDXUTScrollBar::SetTrackRange( int nStart, int nEnd )
{
    m_nStart = nStart; m_nEnd = nEnd;
    Cap();
    UpdateThumbRect();
}
void CDXUTListBox::UpdateRects()
{
    CDXUTControl::UpdateRects();

    reinterpret_cast<R5ListBoxScrollView*>(this)->selection = m_rcBoundingBox;
    reinterpret_cast<R5ListBoxScrollView*>(this)->selection.right -= m_nSBWidth;
    InflateRect( &reinterpret_cast<R5ListBoxScrollView*>(this)->selection, -m_nBorder, -m_nBorder );
    reinterpret_cast<R5ListBoxScrollView*>(this)->text = reinterpret_cast<R5ListBoxScrollView*>(this)->selection;
    InflateRect( &reinterpret_cast<R5ListBoxScrollView*>(this)->text, -m_nMargin, 0 );

    // Update the scrollbar's rects
    reinterpret_cast<R5ListBoxScrollView*>(this)->scrollbar.SetLocation( m_rcBoundingBox.right - m_nSBWidth, m_rcBoundingBox.top );
    reinterpret_cast<R5ListBoxScrollView*>(this)->scrollbar.SetSize( m_nSBWidth, m_height );
    DXUTFontNode* pFontNode = m_pDialog->GetFont( m_Elements.GetAt( 0 )->iFont );
    if( pFontNode && pFontNode->nHeight )
    {
        reinterpret_cast<R5ListBoxScrollView*>(this)->scrollbar.SetPageSize( RectHeight( reinterpret_cast<R5ListBoxScrollView*>(this)->text ) / pFontNode->nHeight );

        // The selected item may have been scrolled off the page.
        // Ensure that it is in page again.
        reinterpret_cast<R5ListBoxScrollView*>(this)->scrollbar.ShowItem( m_nSelected );
    }
}

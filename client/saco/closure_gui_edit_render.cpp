#include "d3d9/common/dxstdafx.h"
#include <new>
WCHAR *MaskPasswordString(const WCHAR *text);
void CDXUTEditBox::Render( IDirect3DDevice9* pd3dDevice, float fElapsedTime )
{
    if( m_bVisible == false )
        return;

    HRESULT hr;
    int nSelStartX = 0, nCaretX = 0;  // Left and right X cordinates of the selection region

    CDXUTElement* pElement = GetElement( 0 );
    if( pElement )
    {
        m_Buffer.SetFontNode( m_pDialog->GetFont( pElement->iFont ) );
        PlaceCaret( m_nCaret );  // Call PlaceCaret now that we have the font info (node),
                                 // so that scrolling can be handled.
    }

    // Render the control graphics
    for( int e = 0; e < 9; ++e )
    {
        pElement = m_Elements.GetAt( e );
        pElement->TextureColor.Blend( DXUT_STATE_NORMAL, fElapsedTime );

        m_pDialog->DrawSprite( pElement, &m_rcRender[e] );
    }

    //
    // Compute the X coordinates of the first visible character.
    //
    int nXFirst;
    m_Buffer.CPtoX( m_nFirstVisible, FALSE, &nXFirst );

    //
    // Compute the X coordinates of the selection rectangle
    //
    hr = m_Buffer.CPtoX( m_nCaret, FALSE, &nCaretX );
    if( m_nCaret != m_nSelStart )
        hr = m_Buffer.CPtoX( m_nSelStart, FALSE, &nSelStartX );
    else
        nSelStartX = nCaretX;

    //
    // Render the selection rectangle
    //
    RECT rcSelection;  // Make this available for rendering selected text
    if( m_nCaret != m_nSelStart )
    {
        int nSelLeftX = nCaretX, nSelRightX = nSelStartX;
        // Swap if left is bigger than right
        if( nSelLeftX > nSelRightX )
            { int nTemp = nSelLeftX; nSelLeftX = nSelRightX; nSelRightX = nTemp; }

        SetRect( &rcSelection, nSelLeftX, m_rcText.top, nSelRightX, m_rcText.bottom );
        OffsetRect( &rcSelection, m_rcText.left - nXFirst, 0 );
        IntersectRect( &rcSelection, &m_rcText, &rcSelection );
        m_pDialog->DrawRect( &rcSelection, m_SelBkColor );
    }

    //
    // Render the text
    //
    // Element 0 for text
    if(!field_126) {
    m_Elements.GetAt( 0 )->FontColor.Current = m_TextColor;
    m_pDialog->DrawText( m_Buffer.GetBuffer() + m_nFirstVisible, m_Elements.GetAt( 0 ), &m_rcText );

    // Render the selected text
    if( m_nCaret != m_nSelStart )
    {
        int nFirstToRender = __max( m_nFirstVisible, __min( m_nSelStart, m_nCaret ) );
        int nNumChatToRender = __max( m_nSelStart, m_nCaret ) - nFirstToRender;
        m_Elements.GetAt( 0 )->FontColor.Current = m_SelTextColor;
        m_pDialog->DrawText( m_Buffer.GetBuffer() + nFirstToRender,
                             m_Elements.GetAt( 0 ), &rcSelection, false, nNumChatToRender );
    }

    } else {
    m_Elements.GetAt( 0 )->FontColor.Current = m_TextColor;
    m_pDialog->DrawText( MaskPasswordString(m_Buffer.GetBuffer() + m_nFirstVisible), m_Elements.GetAt( 0 ), &m_rcText );

    // Render the selected text
    if( m_nCaret != m_nSelStart )
    {
        int nFirstToRender = __max( m_nFirstVisible, __min( m_nSelStart, m_nCaret ) );
        int nNumChatToRender = __max( m_nSelStart, m_nCaret ) - nFirstToRender;
        m_Elements.GetAt( 0 )->FontColor.Current = m_SelTextColor;
        m_pDialog->DrawText( MaskPasswordString(m_Buffer.GetBuffer() + nFirstToRender),
                             m_Elements.GetAt( 0 ), &rcSelection, false, nNumChatToRender );
    }

    }

    //
    /* Blink the caret
    //
    if( DXUTGetGlobalTimer()->GetAbsoluteTime() - m_dfLastBlink >= m_dfBlink )
    {
        m_bCaretOn = !m_bCaretOn;
        m_dfLastBlink = DXUTGetGlobalTimer()->GetAbsoluteTime();
    }*/

    //
    // Render the caret if this control has the focus
    //
    //if( m_bHasFocus && m_bCaretOn && !s_bHideCaret )
    //{
	if(m_bVisible && m_bEnabled) {
        // Start the rectangle with insert mode caret
        RECT rcCaret = { m_rcText.left - nXFirst + nCaretX - 1, m_rcText.top,
                         m_rcText.left - nXFirst + nCaretX + 1, m_rcText.bottom };

        // If we are in overwrite mode, adjust the caret rectangle
        // to fill the entire character.
        if( !m_bInsertMode )
        {
            // Obtain the right edge X coord of the current character
            int nRightEdgeX;
            m_Buffer.CPtoX( m_nCaret, TRUE, &nRightEdgeX );
            rcCaret.right = m_rcText.left - nXFirst + nRightEdgeX;
        }

        m_pDialog->DrawRect( &rcCaret, m_CaretColor );
    }
}

#include "main.h"
#include <stddef.h>
#define LANG_CHT MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL)
#pragma pack(push,1)
struct R5ImeCandidateListState
{
    WCHAR awszCandidate[10][256];
    CUniBuffer HoriCand;
    int nFirstSelected;
    int nHoriSelectedLen;
    DWORD dwCount;
    DWORD dwSelection;
    DWORD dwPageSize;
    int nReadingError;
    bool bShowWindow;
    RECT rcCandidate;
};
#pragma pack(pop)
typedef char R5CandidateStateSize[(sizeof(R5ImeCandidateListState)==0x143C)?1:-1];
typedef char R5CandidateHorizontalOffset[(offsetof(R5ImeCandidateListState,HoriCand)==0x1400)?1:-1];
typedef char R5CandidateSelectionOffset[(offsetof(R5ImeCandidateListState,dwSelection)==0x141F)?1:-1];
typedef char R5CandidateRectangleOffset[(offsetof(R5ImeCandidateListState,rcCandidate)==0x142C)?1:-1];
extern R5ImeCandidateListState r5ImeCandidateState;
extern WCHAR r5ImeReadingString[32];
void CDXUTIMEEditBox::RenderCandidateReadingWindow( IDirect3DDevice9* pd3dDevice, float fElapsedTime, bool bReading )
{
    RECT rc;
    UINT nNumEntries = bReading ? 4 : MAX_CANDLIST;
    D3DCOLOR TextColor, TextBkColor, SelTextColor, SelBkColor;
    int nX, nXFirst, nXComp;
    m_Buffer.CPtoX( m_nCaret, FALSE, &nX );
    m_Buffer.CPtoX( m_nFirstVisible, FALSE, &nXFirst );

    if( bReading )
    {
        TextColor = m_ReadingColor;
        TextBkColor = m_ReadingWinColor;
        SelTextColor = m_ReadingSelColor;
        SelBkColor = m_ReadingSelBkColor;
    } else
    {
        TextColor = m_CandidateColor;
        TextBkColor = m_CandidateWinColor;
        SelTextColor = m_CandidateSelColor;
        SelBkColor = m_CandidateSelBkColor;
    }

    // For Japanese IME, align the window with the first target converted character.
    // For all other IMEs, align with the caret.  This is because the caret
    // does not move for Japanese IME.
    if ( GetLanguage() == LANG_CHT && !GetImeId() )
        nXComp = 0;
    else
    if( GetPrimaryLanguage() == LANG_JAPANESE )
        s_CompString.CPtoX( s_nFirstTargetConv, FALSE, &nXComp );
    else
        s_CompString.CPtoX( s_nCompCaret, FALSE, &nXComp );

    // Compute the size of the candidate window
    int nWidthRequired = 0;
    int nHeightRequired = 0;
    int nSingleLineHeight = 0;

    if( ( s_bVerticalCand && !bReading ) ||
        ( !s_bHorizontalReading && bReading ) )
    {
        // Vertical window
        for( UINT i = 0; i < nNumEntries; ++i )
        {
            if( r5ImeCandidateState.awszCandidate[i][0] == L'\0' )
                break;
            SetRect( &rc, 0, 0, 0, 0 );
            m_pDialog->CalcTextRect( r5ImeCandidateState.awszCandidate[i], m_Elements.GetAt( 1 ), &rc );
            nWidthRequired = __max( nWidthRequired, rc.right - rc.left );
            nSingleLineHeight = __max( nSingleLineHeight, rc.bottom - rc.top );
        }
        nHeightRequired = nSingleLineHeight * nNumEntries;
    } else
    {
        // Horizontal window
        SetRect( &rc, 0, 0, 0, 0 );
        if( bReading )
            m_pDialog->CalcTextRect( r5ImeReadingString, m_Elements.GetAt( 1 ), &rc );
        else
            m_pDialog->CalcTextRect( r5ImeCandidateState.HoriCand.GetBuffer(), m_Elements.GetAt( 1 ), &rc );
        nWidthRequired = rc.right - rc.left;
        nSingleLineHeight = nHeightRequired = rc.bottom - rc.top;
    }

    // Now that we have the dimension, calculate the location for the candidate window.
    // We attempt to fit the window in this order:
    // bottom, top, right, left.

    // Bottom
    SetRect( &rc, s_ptCompString.x + nXComp, s_ptCompString.y + m_rcText.bottom - m_rcText.top,
                  s_ptCompString.x + nXComp + nWidthRequired, s_ptCompString.y + m_rcText.bottom - m_rcText.top + nHeightRequired );
    // if the right edge is cut off, move it left.
    if( rc.right > m_pDialog->GetWidth() )
    {
        rc.left -= rc.right - m_pDialog->GetWidth();
        rc.right = m_pDialog->GetWidth();
    }

    // If we are rendering the candidate window, save the position
    // so that mouse clicks are checked properly.
    if( !bReading )
        r5ImeCandidateState.rcCandidate = rc;

    // Render the elements
    m_pDialog->DrawRect( &rc, TextBkColor );
    if( ( s_bVerticalCand && !bReading ) ||
        ( !s_bHorizontalReading && bReading ) )
    {
        // Vertical candidate window
        for( UINT i = 0; i < nNumEntries; ++i )
        {
            // Here we are rendering one line at a time
            rc.bottom = rc.top + nSingleLineHeight;
            // Use a different color for the selected string
            if( r5ImeCandidateState.dwSelection == i )
            {
                m_pDialog->DrawRect( &rc, SelBkColor );
                m_Elements.GetAt( 1 )->FontColor.Current = SelTextColor;
            } else
                m_Elements.GetAt( 1 )->FontColor.Current = TextColor;

            m_pDialog->DrawText( r5ImeCandidateState.awszCandidate[i], m_Elements.GetAt( 1 ), &rc );

            rc.top += nSingleLineHeight;
        }
    } else
    {
        // Horizontal candidate window
        m_Elements.GetAt( 1 )->FontColor.Current = TextColor;
        if( bReading )
            m_pDialog->DrawText( r5ImeReadingString, m_Elements.GetAt( 1 ), &rc );
        else
            m_pDialog->DrawText( r5ImeCandidateState.HoriCand.GetBuffer(), m_Elements.GetAt( 1 ), &rc );

        // Render the selected entry differently
        if( !bReading )
        {
            int nXLeft, nXRight;
            r5ImeCandidateState.HoriCand.CPtoX( r5ImeCandidateState.nFirstSelected, FALSE, &nXLeft );
            r5ImeCandidateState.HoriCand.CPtoX( r5ImeCandidateState.nFirstSelected + r5ImeCandidateState.nHoriSelectedLen, FALSE, &nXRight );

            rc.right = rc.left + nXRight;
            rc.left += nXLeft;
            m_pDialog->DrawRect( &rc, SelBkColor );
            m_Elements.GetAt( 1 )->FontColor.Current = SelTextColor;
            m_pDialog->DrawText( r5ImeCandidateState.HoriCand.GetBuffer() + r5ImeCandidateState.nFirstSelected,
                                m_Elements.GetAt( 1 ), &rc, false, r5ImeCandidateState.nHoriSelectedLen );
        }
    }
}
// Complete original dialog default/resource definitions.
#include "d3d9/common/dxstdafx.h"
#include <new>
#undef min
#undef max
int GetUIFontSize();
int GetFontWeight();
char *GetFontFace();
void DXUTBlendColor::Init( D3DCOLOR defaultColor, D3DCOLOR disabledColor, D3DCOLOR hiddenColor )
{
    for( int i=0; i < MAX_CONTROL_STATES; i++ )
    {
        States[ i ] = defaultColor;
    }

    States[ DXUT_STATE_DISABLED ] = disabledColor;
    States[ DXUT_STATE_HIDDEN ] = hiddenColor;
    Current = hiddenColor;
}
void DXUTBlendColor::Blend( UINT iState, float fElapsedTime, float fRate )
{
    D3DXCOLOR destColor = States[ iState ];
    D3DXColorLerp( &Current, &Current, &destColor, 1.0f - powf( fRate, 30 * fElapsedTime ) );
}
void CDXUTElement::SetTexture( UINT iTexture, RECT* prcTexture, D3DCOLOR defaultTextureColor )
{
    this->iTexture = iTexture;
    
    if( prcTexture )
        rcTexture = *prcTexture;
    else
        SetRectEmpty( &rcTexture );
    
    TextureColor.Init( defaultTextureColor );
}
void CDXUTElement::SetFont( UINT iFont, D3DCOLOR defaultFontColor, DWORD dwTextFormat )
{
    this->iFont = iFont;
    this->dwTextFormat = dwTextFormat;

    FontColor.Init( defaultFontColor );
}
void CDXUTElement::Refresh()
{
    TextureColor.Current = TextureColor.States[ DXUT_STATE_HIDDEN ];
    FontColor.Current = FontColor.States[ DXUT_STATE_HIDDEN ];
}
HRESULT CDXUTDialog::SetDefaultElement( UINT nControlType, UINT iElement, CDXUTElement* pElement )
{
    // If this Element type already exist in the list, simply update the stored Element
    for( int i=0; i < m_DefaultElements.GetSize(); i++ )
    {
        DXUTElementHolder* pElementHolder = m_DefaultElements.GetAt( i );
        
        if( pElementHolder->nControlType == nControlType &&
            pElementHolder->iElement == iElement )
        {
            pElementHolder->Element = *pElement;
            return S_OK;
        }
    }

    // Otherwise, add a new entry
    DXUTElementHolder* pNewHolder;
    pNewHolder = new DXUTElementHolder;
    if( pNewHolder == NULL )
        return E_OUTOFMEMORY;

    pNewHolder->nControlType = nControlType;
    pNewHolder->iElement = iElement;
    pNewHolder->Element = *pElement;

    m_DefaultElements.Add( pNewHolder );
    return S_OK;
}
HRESULT CDXUTDialog::SetTexture( UINT index, LPCTSTR strFilename )
{
    // If this assert triggers, you need to call CDXUTDialog::Init() first.  This change
    // was made so that the DXUT's GUI could become seperate and optional from DXUT's core.  The 
    // creation and interfacing with CDXUTDialogResourceManager is now the responsibility 
    // of the application if it wishes to use DXUT's GUI.
    assert( m_pManager != NULL && "To fix call CDXUTDialog::Init() first.  See comments for details." ); 

    // Make sure the list is at least as large as the index being set
    for( UINT i=m_Textures.GetSize(); i <= index; i++ )
    {
        m_Textures.Add( -1 );
    }

    int iTexture = m_pManager->AddTexture( strFilename );

    m_Textures.SetAt( index, iTexture );
    return S_OK;
}
HRESULT CDXUTDialog::SetFont( UINT index, LPCTSTR strFaceName, LONG height, LONG weight )
{
    // If this assert triggers, you need to call CDXUTDialog::Init() first.  This change
    // was made so that the DXUT's GUI could become seperate and optional from DXUT's core.  The 
    // creation and interfacing with CDXUTDialogResourceManager is now the responsibility 
    // of the application if it wishes to use DXUT's GUI.
    assert( m_pManager != NULL && "To fix call CDXUTDialog::Init() first.  See comments for details." ); 

    // Make sure the list is at least as large as the index being set
    UINT i;
    for( i=m_Fonts.GetSize(); i <= index; i++ )
    {
        m_Fonts.Add( -1 );
    }

    int iFont = m_pManager->AddFont( strFaceName, height, weight );
    m_Fonts.SetAt( index, iFont );

    return S_OK;
}
void CDXUTDialog::UpdateFont()
{
	int nFontSize = GetUIFontSize();
	int nFontWeight = GetFontWeight();

	SetFont(0, GetFontFace(), nFontSize, nFontWeight);
	SetFont(1, GetFontFace(), nFontSize - 2, nFontWeight);
}
void CDXUTDialog::InitDefaultElements()
{
    //SetTexture( 0, "DXUTControls.dds" );
	SetTexture( 0, "sampgui.png" );

    //SetFont( 0, "Arial", 16, FW_BOLD );
    UpdateFont();

    CDXUTElement Element;
    RECT rcTexture;

    //-------------------------------------
    // Element for the caption
    //-------------------------------------
    m_CapElement.SetFont( 1 );
    SetRect( &rcTexture, 17, 269, 241, 287 );
    m_CapElement.SetTexture( 0, &rcTexture );
    m_CapElement.TextureColor.States[ DXUT_STATE_NORMAL ] = D3DCOLOR_ARGB(200, 10, 10, 10);
    m_CapElement.FontColor.States[ DXUT_STATE_NORMAL ] = D3DCOLOR_ARGB(255, 210, 210, 210);
    m_CapElement.SetFont( 1, D3DCOLOR_ARGB(255, 210, 210, 210), DT_LEFT | DT_VCENTER );
    // Pre-blend as we don't need to transition the state
    m_CapElement.TextureColor.Blend( DXUT_STATE_NORMAL, 10.0f );
    m_CapElement.FontColor.Blend( DXUT_STATE_NORMAL, 10.0f );

    //-------------------------------------
    // CDXUTStatic
    //-------------------------------------
    Element.SetFont( 1 );
	Element.SetFont( 1, D3DCOLOR_ARGB( 255, 190, 190, 190 ), DT_VCENTER | DT_EXPANDTABS );
    Element.FontColor.States[ DXUT_STATE_DISABLED ] = D3DCOLOR_ARGB( 200, 200, 200, 200 );

    // Assign the Element
    SetDefaultElement( DXUT_CONTROL_STATIC, 0, &Element );
    

    //-------------------------------------
    // CDXUTButton - Button
    //-------------------------------------
    SetRect( &rcTexture, 0, 0, 136, 54 );
    Element.SetTexture( 0, &rcTexture );
    Element.SetFont( 0 );
    Element.TextureColor.States[ DXUT_STATE_NORMAL ] = D3DCOLOR_ARGB(200, 255, 255, 255);
    Element.TextureColor.States[ DXUT_STATE_PRESSED ] = D3DCOLOR_ARGB(255, 149, 176, 208);
	Element.TextureColor.States[ DXUT_STATE_MOUSEOVER ] = D3DCOLOR_ARGB(255, 185, 34, 40);
    Element.FontColor.States[ DXUT_STATE_MOUSEOVER ] = D3DCOLOR_ARGB(255, 255, 255, 255);
    Element.FontColor.States[ DXUT_STATE_NORMAL ] = D3DCOLOR_ARGB(255, 200, 200, 200);

    // Assign the Element
    SetDefaultElement( DXUT_CONTROL_BUTTON, 0, &Element );

	//-------------------------------------
    // CDXUTButton - Fill layer
    //-------------------------------------
	SetRect( &rcTexture, 0, 0, 136, 54 );
    Element.SetTexture( 0, &rcTexture, D3DCOLOR_ARGB(0, 255, 255, 255) );
	Element.TextureColor.States[ DXUT_STATE_MOUSEOVER ] = D3DCOLOR_ARGB(255, 185, 34, 40);
    Element.TextureColor.States[ DXUT_STATE_PRESSED ] = D3DCOLOR_ARGB(60, 80, 80, 80);
	Element.TextureColor.States[ DXUT_STATE_FOCUS ] = D3DCOLOR_ARGB(200, 255, 255, 255);

    // Assign the Element
    SetDefaultElement( DXUT_CONTROL_BUTTON, 1, &Element );


    //-------------------------------------
    // CDXUTCheckBox - Box
    //-------------------------------------
    SetRect( &rcTexture, 0, 54, 27, 81 );
    Element.SetTexture( 0, &rcTexture );
    Element.SetFont( 0, D3DCOLOR_ARGB(255, 255, 255, 255), DT_LEFT | DT_VCENTER );
    Element.FontColor.States[ DXUT_STATE_DISABLED ] = D3DCOLOR_ARGB( 200, 200, 200, 200 );
    Element.TextureColor.States[ DXUT_STATE_NORMAL ] = D3DCOLOR_ARGB(150, 255, 255, 255);
    Element.TextureColor.States[ DXUT_STATE_FOCUS ] = D3DCOLOR_ARGB(200, 255, 255, 255);
    Element.TextureColor.States[ DXUT_STATE_PRESSED ] = D3DCOLOR_ARGB(255, 255, 255, 255);
    
    // Assign the Element
    SetDefaultElement( DXUT_CONTROL_CHECKBOX, 0, &Element );


    //-------------------------------------
    // CDXUTCheckBox - Check
    //-------------------------------------
    SetRect( &rcTexture, 27, 54, 54, 81 );
    Element.SetTexture( 0, &rcTexture );
    
    // Assign the Element
    SetDefaultElement( DXUT_CONTROL_CHECKBOX, 1, &Element );


    //-------------------------------------
    // CDXUTRadioButton - Box
    //-------------------------------------
    SetRect( &rcTexture, 54, 54, 81, 81 );
    Element.SetTexture( 0, &rcTexture );
    Element.SetFont( 0, D3DCOLOR_ARGB(255, 255, 255, 255), DT_LEFT | DT_VCENTER );
    Element.FontColor.States[ DXUT_STATE_DISABLED ] = D3DCOLOR_ARGB( 200, 200, 200, 200 );
    Element.TextureColor.States[ DXUT_STATE_NORMAL ] = D3DCOLOR_ARGB(150, 255, 255, 255);
    Element.TextureColor.States[ DXUT_STATE_FOCUS ] = D3DCOLOR_ARGB(200, 255, 255, 255);
    Element.TextureColor.States[ DXUT_STATE_PRESSED ] = D3DCOLOR_ARGB(255, 255, 255, 255);
    
    // Assign the Element
    SetDefaultElement( DXUT_CONTROL_RADIOBUTTON, 0, &Element );


    //-------------------------------------
    // CDXUTRadioButton - Check
    //-------------------------------------
    SetRect( &rcTexture, 81, 54, 108, 81 );
    Element.SetTexture( 0, &rcTexture );
    
    // Assign the Element
    SetDefaultElement( DXUT_CONTROL_RADIOBUTTON, 1, &Element );


    //-------------------------------------
    // CDXUTComboBox - Main
    //-------------------------------------
    SetRect( &rcTexture, 7, 81, 247, 123 );
    Element.SetTexture( 0, &rcTexture );
    Element.SetFont( 0 );
    Element.TextureColor.States[ DXUT_STATE_NORMAL ] = D3DCOLOR_ARGB(200, 255, 255, 255);
	Element.TextureColor.States[ DXUT_STATE_MOUSEOVER ] = D3DCOLOR_ARGB(255, 185, 34, 40);
    Element.TextureColor.States[ DXUT_STATE_FOCUS ] = D3DCOLOR_ARGB(255, 185, 34, 40);
    Element.TextureColor.States[ DXUT_STATE_DISABLED ] = D3DCOLOR_ARGB(70, 200, 200, 200);
    Element.FontColor.States[ DXUT_STATE_MOUSEOVER ] = D3DCOLOR_ARGB(255, 255, 255, 255);
    Element.FontColor.States[ DXUT_STATE_PRESSED ] = D3DCOLOR_ARGB(255, 0, 0, 0);
    Element.FontColor.States[ DXUT_STATE_DISABLED ] = D3DCOLOR_ARGB(200, 200, 200, 200);
    
    
    // Assign the Element
    SetDefaultElement( DXUT_CONTROL_COMBOBOX, 0, &Element );


    //-------------------------------------
    // CDXUTComboBox - Button
    //-------------------------------------
    SetRect( &rcTexture, 272, 0, 325, 49 );
    Element.SetTexture( 0, &rcTexture );
    Element.TextureColor.States[ DXUT_STATE_NORMAL ] = D3DCOLOR_ARGB(200, 255, 255, 255);
    Element.TextureColor.States[ DXUT_STATE_PRESSED ] = D3DCOLOR_ARGB(255, 150, 150, 150);
    Element.TextureColor.States[ DXUT_STATE_FOCUS ] = D3DCOLOR_ARGB(255, 150, 150, 150);
    Element.TextureColor.States[ DXUT_STATE_DISABLED ] = D3DCOLOR_ARGB(70, 255, 255, 255);
    
    // Assign the Element
    SetDefaultElement( DXUT_CONTROL_COMBOBOX, 1, &Element );


    //-------------------------------------
    // CDXUTComboBox - Dropdown
    //-------------------------------------
    SetRect( &rcTexture, 7, 123, 241, 265 );
    Element.SetTexture( 0, &rcTexture );
    Element.SetFont( 0, D3DCOLOR_ARGB(255, 255, 255, 255), DT_LEFT | DT_TOP );
	Element.TextureColor.Init( D3DCOLOR_ARGB(255, 185, 34, 40) );

    // Assign the Element
    SetDefaultElement( DXUT_CONTROL_COMBOBOX, 2, &Element );


    //-------------------------------------
    // CDXUTComboBox - Selection
    //-------------------------------------
    SetRect( &rcTexture, 7, 266, 241, 289 );
    Element.SetTexture( 0, &rcTexture );
    Element.SetFont( 0, D3DCOLOR_ARGB(255, 255, 255, 255), DT_LEFT | DT_TOP );
	Element.TextureColor.Init( D3DCOLOR_ARGB(255, 200, 200, 200) );

    // Assign the Element
    SetDefaultElement( DXUT_CONTROL_COMBOBOX, 3, &Element );


    //-------------------------------------
    // CDXUTSlider - Track
    //-------------------------------------
    SetRect( &rcTexture, 1, 290, 280, 331 );
    Element.SetTexture( 0, &rcTexture );
    Element.TextureColor.States[ DXUT_STATE_NORMAL ] = D3DCOLOR_ARGB(150, 255, 255, 255);
    Element.TextureColor.States[ DXUT_STATE_FOCUS ] = D3DCOLOR_ARGB(200, 255, 255, 255);
    Element.TextureColor.States[ DXUT_STATE_DISABLED ] = D3DCOLOR_ARGB(70, 255, 255, 255);
    
    // Assign the Element
    SetDefaultElement( DXUT_CONTROL_SLIDER, 0, &Element );


	//-------------------------------------
    // CDXUTSlider - Button
    //-------------------------------------
	SetRect( &rcTexture, 54, 54, 81, 81 );
    Element.SetTexture( 0, &rcTexture );
	Element.TextureColor.Init( D3DCOLOR_ARGB(255, 185, 34, 40), D3DCOLOR_ARGB(255, 149, 176, 208) );
	Element.TextureColor.States[ DXUT_STATE_MOUSEOVER ] = D3DCOLOR_ARGB(255, 240, 240, 240);
	Element.TextureColor.States[ DXUT_STATE_PRESSED ] = D3DCOLOR_ARGB(255, 185, 34, 40);
    Element.TextureColor.States[ DXUT_STATE_FOCUS ] = D3DCOLOR_ARGB(255, 220, 220, 220);

    // Assign the Element
    SetDefaultElement( DXUT_CONTROL_SLIDER, 1, &Element );


    //-------------------------------------
    // CDXUTScrollBar - Track
    //-------------------------------------
    SetRect( &rcTexture, 243, 144, 265, 155 );
    Element.SetTexture( 0, &rcTexture );
    Element.TextureColor.Init(D3DCOLOR_ARGB(255, 255, 255, 255),D3DCOLOR_ARGB(255, 255, 255, 255));

    // Assign the Element
    SetDefaultElement( DXUT_CONTROL_SCROLLBAR, 0, &Element );

    //-------------------------------------
    // CDXUTScrollBar - Up Arrow
    //-------------------------------------
    SetRect( &rcTexture, 243, 124, 265, 144 );
    Element.SetTexture( 0, &rcTexture );
	Element.TextureColor.Init(D3DCOLOR_ARGB(255, 255, 255, 255),D3DCOLOR_ARGB(255, 255, 255, 255));
        
    
    
    // Assign the Element
    SetDefaultElement( DXUT_CONTROL_SCROLLBAR, 1, &Element );

    //-------------------------------------
    // CDXUTScrollBar - Down Arrow
    //-------------------------------------
    SetRect( &rcTexture, 243, 155, 265, 176 );
    Element.SetTexture( 0, &rcTexture );
    Element.TextureColor.Init(D3DCOLOR_ARGB(255, 255, 255, 255),D3DCOLOR_ARGB(255, 255, 255, 255));
        
    
    // Assign the Element
    SetDefaultElement( DXUT_CONTROL_SCROLLBAR, 2, &Element );

    //-------------------------------------
    // CDXUTScrollBar - Button
    //-------------------------------------
    SetRect( &rcTexture, 266, 123, 286, 167 );
    Element.SetTexture( 0, &rcTexture );
    Element.TextureColor.Init(D3DCOLOR_ARGB(255, 185, 34, 40),D3DCOLOR_ARGB(255, 149, 176, 208));

    // Assign the Element
    SetDefaultElement( DXUT_CONTROL_SCROLLBAR, 3, &Element );


    //-------------------------------------
    // CDXUTEditBox
    //-------------------------------------
    // Element assignment:
    //   0 - text area
    //   1 - top left border
    //   2 - top border
    //   3 - top right border
    //   4 - left border
    //   5 - right border
    //   6 - lower left border
    //   7 - lower border
    //   8 - lower right border

    Element.SetFont( 0, D3DCOLOR_ARGB( 255, 0, 0, 0 ), DT_LEFT | DT_TOP | DT_VCENTER );

    // Assign the style
    SetRect( &rcTexture, 14, 90, 241, 113 );
    Element.SetTexture( 0, &rcTexture );
    SetDefaultElement( DXUT_CONTROL_EDITBOX, 0, &Element );
    SetRect( &rcTexture, 8, 82, 14, 90 );
    Element.SetTexture( 0, &rcTexture );
    SetDefaultElement( DXUT_CONTROL_EDITBOX, 1, &Element );
    SetRect( &rcTexture, 14, 82, 241, 90 );
    Element.SetTexture( 0, &rcTexture );
    SetDefaultElement( DXUT_CONTROL_EDITBOX, 2, &Element );
    SetRect( &rcTexture, 241, 82, 246, 90 );
    Element.SetTexture( 0, &rcTexture );
    SetDefaultElement( DXUT_CONTROL_EDITBOX, 3, &Element );
    SetRect( &rcTexture, 8, 90, 14, 113 );
    Element.SetTexture( 0, &rcTexture );
    SetDefaultElement( DXUT_CONTROL_EDITBOX, 4, &Element );
    SetRect( &rcTexture, 241, 90, 246, 113 );
    Element.SetTexture( 0, &rcTexture );
    SetDefaultElement( DXUT_CONTROL_EDITBOX, 5, &Element );
    SetRect( &rcTexture, 8, 113, 14, 121 );
    Element.SetTexture( 0, &rcTexture );
    SetDefaultElement( DXUT_CONTROL_EDITBOX, 6, &Element );
    SetRect( &rcTexture, 14, 113, 241, 121 );
    Element.SetTexture( 0, &rcTexture );
    SetDefaultElement( DXUT_CONTROL_EDITBOX, 7, &Element );
    SetRect( &rcTexture, 241, 113, 246, 121 );
    Element.SetTexture( 0, &rcTexture );
    SetDefaultElement( DXUT_CONTROL_EDITBOX, 8, &Element );


    //-------------------------------------
    // CDXUTIMEEditBox
    //-------------------------------------

    Element.SetFont( 0, D3DCOLOR_ARGB( 255, 0, 0, 0 ), DT_LEFT | DT_TOP | DT_VCENTER );

    // Assign the style
    SetRect( &rcTexture, 14, 90, 241, 113 );
    Element.SetTexture( 0, &rcTexture );
    SetDefaultElement( DXUT_CONTROL_IMEEDITBOX, 0, &Element );
    SetRect( &rcTexture, 8, 82, 14, 90 );
    Element.SetTexture( 0, &rcTexture );
    SetDefaultElement( DXUT_CONTROL_IMEEDITBOX, 1, &Element );
    SetRect( &rcTexture, 14, 82, 241, 90 );
    Element.SetTexture( 0, &rcTexture );
    SetDefaultElement( DXUT_CONTROL_IMEEDITBOX, 2, &Element );
    SetRect( &rcTexture, 241, 82, 246, 90 );
    Element.SetTexture( 0, &rcTexture );
    SetDefaultElement( DXUT_CONTROL_IMEEDITBOX, 3, &Element );
    SetRect( &rcTexture, 8, 90, 14, 113 );
    Element.SetTexture( 0, &rcTexture );
    SetDefaultElement( DXUT_CONTROL_IMEEDITBOX, 4, &Element );
    SetRect( &rcTexture, 241, 90, 246, 113 );
    Element.SetTexture( 0, &rcTexture );
    SetDefaultElement( DXUT_CONTROL_IMEEDITBOX, 5, &Element );
    SetRect( &rcTexture, 8, 113, 14, 121 );
    Element.SetTexture( 0, &rcTexture );
    SetDefaultElement( DXUT_CONTROL_IMEEDITBOX, 6, &Element );
    SetRect( &rcTexture, 14, 113, 241, 121 );
    Element.SetTexture( 0, &rcTexture );
    SetDefaultElement( DXUT_CONTROL_IMEEDITBOX, 7, &Element );
    SetRect( &rcTexture, 241, 113, 246, 121 );
    Element.SetTexture( 0, &rcTexture );
    SetDefaultElement( DXUT_CONTROL_IMEEDITBOX, 8, &Element );
    // Element 9 for IME text, and indicator button
    SetRect( &rcTexture, 0, 0, 136, 54 );
    Element.SetTexture( 0, &rcTexture );
    Element.SetFont( 0, D3DCOLOR_ARGB( 255, 255, 255, 255 ), DT_CENTER | DT_VCENTER );
    SetDefaultElement( DXUT_CONTROL_IMEEDITBOX, 9, &Element );

    //-------------------------------------
    // CDXUTListBox - Main
    //-------------------------------------

    SetRect( &rcTexture, 13, 124, 241, 265 );
    Element.SetTexture( 0, &rcTexture );
    Element.SetFont( 1, D3DCOLOR_ARGB(255, 255, 255, 255), DT_LEFT | DT_TOP | DT_EXPANDTABS );

    // Assign the Element
    SetDefaultElement( DXUT_CONTROL_LISTBOX, 0, &Element );

    //-------------------------------------
    // CDXUTListBox - Selection
    //-------------------------------------

    SetRect( &rcTexture, 17, 269, 241, 287 );
    Element.SetTexture( 0, &rcTexture );
    Element.SetFont( 1, D3DCOLOR_ARGB(255, 255, 255, 255), DT_LEFT | DT_TOP | DT_EXPANDTABS );
	Element.TextureColor.Init( D3DCOLOR_ARGB(255, 185, 34, 40) );

    // Assign the Element
    SetDefaultElement( DXUT_CONTROL_LISTBOX, 1, &Element );
}
int CDXUTDialogResourceManager::AddFont( LPCTSTR strFaceName, LONG height, LONG weight )
{
    // See if this font already exists
    for( int i=0; i < m_FontCache.GetSize(); i++ )
    {
        DXUTFontNode* pFontNode = m_FontCache.GetAt(i);
        if( 0 == _strnicmp( pFontNode->strFace, strFaceName, MAX_PATH-1 ) &&
            pFontNode->nHeight == height &&
            pFontNode->nWeight == weight )
        {
            return i;
        }
    }

    // Add a new font and try to create it
    DXUTFontNode* pNewFontNode = new DXUTFontNode();
    if( pNewFontNode == NULL )
        return -1;

    ZeroMemory( pNewFontNode, sizeof(DXUTFontNode) );
    StringCchCopy( pNewFontNode->strFace, MAX_PATH, strFaceName );
    pNewFontNode->nHeight = height;
    pNewFontNode->nWeight = weight;
    m_FontCache.Add( pNewFontNode );
    
    int iFont = m_FontCache.GetSize()-1;

    // If a device is available, try to create immediately
    if( m_pd3dDevice )
        CreateFont( iFont );

    return iFont;
}
int CDXUTDialogResourceManager::AddTexture( LPCTSTR strFilename )
{
    // See if this texture already exists
    for( int i=0; i < m_TextureCache.GetSize(); i++ )
    {
        DXUTTextureNode* pTextureNode = m_TextureCache.GetAt(i);
        if( 0 == _strnicmp(pTextureNode->strFilename, strFilename, MAX_PATH-1 ) )
        {
            return i;
        }
    }

    // Add a new texture and try to create it
    DXUTTextureNode* pNewTextureNode = new DXUTTextureNode();
    if( pNewTextureNode == NULL )
        return -1;

    ZeroMemory( pNewTextureNode, sizeof(DXUTTextureNode) );
    StringCchCopy( pNewTextureNode->strFilename, MAX_PATH, strFilename );
    m_TextureCache.Add( pNewTextureNode );
    
    int iTexture = m_TextureCache.GetSize()-1;

    // If a device is available, try to create immediately
    if( m_pd3dDevice )
        CreateTexture( iTexture );

    return iTexture;
}
HRESULT CDXUTDialogResourceManager::CreateTexture( UINT iTexture )
{
    HRESULT hr = S_OK;

    DXUTTextureNode* pTextureNode = m_TextureCache.GetAt( iTexture );
   
	//_ASSERT(pTextureNode->strFilename[0] == 0);

    // Make sure there's a texture to create
    if( pTextureNode->strFilename[0] == 0 )
	{
		//OutputDebugString("CDXUTDialogResourceManager::CreateTexture(BAD_TEXFILE)");
        return S_OK;
	}
    
    // Find the texture on the hard drive
    TCHAR strPath[MAX_PATH];
    hr = DXUTFindDXSDKMediaFileCch( strPath, MAX_PATH, pTextureNode->strFilename );
    if( FAILED(hr) )
    {
        return DXTRACE_ERR( "DXUTFindDXSDKMediaFileCch", hr );
    }

    // Create texture
    D3DXIMAGE_INFO info;
    hr =  D3DXCreateTextureFromFileEx( m_pd3dDevice, strPath, D3DX_DEFAULT, D3DX_DEFAULT, 
                                       D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, 
                                       D3DX_DEFAULT, D3DX_DEFAULT, 0, 
                                       &info, NULL, &pTextureNode->pTexture );
    if( FAILED(hr) )
    {
        return DXTRACE_ERR( "D3DXCreateTextureFromFileEx", hr );
    }

    // Store dimensions
    pTextureNode->dwWidth = info.Width;
    pTextureNode->dwHeight = info.Height;

    return S_OK;
}

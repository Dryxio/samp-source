#include "d3d9/common/dxstdafx.h"
#include <new>
CDXUTControl::CDXUTControl( CDXUTDialog *pDialog )
{
    m_Type = DXUT_CONTROL_BUTTON;
    m_pDialog = pDialog;
    m_ID = 0;
    m_Index = 0;
    m_pUserData = NULL;

    m_bEnabled = true;
    m_bVisible = true;
    m_bMouseOver = false;
    m_bHasFocus = false;
    m_bIsDefault = false;

    m_pDialog = NULL;

    m_x = 0;
    m_y = 0;
    m_width = 0;
    m_height = 0;

   ZeroMemory( &m_rcBoundingBox, sizeof( m_rcBoundingBox ) );
}
CDXUTControl::~CDXUTControl()
{
    for( int i = 0; i < m_Elements.GetSize(); ++i )
    {
        delete m_Elements[i];
    }
    m_Elements.RemoveAll();
}
CDXUTScrollBar::CDXUTScrollBar( CDXUTDialog *pDialog )
{
    m_Type = DXUT_CONTROL_SCROLLBAR;
    m_pDialog = pDialog;

    m_bShowThumb = true;

    SetRect( &m_rcUpButton, 0, 0, 0, 0 );
    SetRect( &m_rcDownButton, 0, 0, 0, 0 );
    SetRect( &m_rcTrack, 0, 0, 0, 0 );
    SetRect( &m_rcThumb, 0, 0, 0, 0 );
    m_nPosition = 0;
    m_nPageSize = 1;
    m_nStart = 0;
    m_nEnd = 1;
    m_Arrow = CLEAR;
    m_dArrowTS = 0.0;
}
CDXUTScrollBar::~CDXUTScrollBar()
{
}
void CDXUTControl::SetTextColor( D3DCOLOR Color )
{
    CDXUTElement* pElement = m_Elements.GetAt( 0 );

    if( pElement )
        pElement->FontColor.States[DXUT_STATE_NORMAL] = Color;
}
CDXUTStatic::CDXUTStatic( CDXUTDialog *pDialog )
{
    m_Type = DXUT_CONTROL_STATIC;
    m_pDialog = pDialog;

    ZeroMemory( &m_strText, sizeof(m_strText) );  

    for( int i=0; i < m_Elements.GetSize(); i++ )
    {
        CDXUTElement* pElement = m_Elements.GetAt( i );
        SAFE_DELETE( pElement );
    }

    m_Elements.RemoveAll();
}
CDXUTButton::CDXUTButton( CDXUTDialog *pDialog )
{
    m_Type = DXUT_CONTROL_BUTTON;
    m_pDialog = pDialog;

    m_bPressed = false;
    m_nHotkey = 0;
}
CDXUTCheckBox::CDXUTCheckBox( CDXUTDialog *pDialog )
{
    m_Type = DXUT_CONTROL_CHECKBOX;
    m_pDialog = pDialog;

    m_bChecked = false;
}
CDXUTRadioButton::CDXUTRadioButton( CDXUTDialog *pDialog )
{
    m_Type = DXUT_CONTROL_RADIOBUTTON;
    m_pDialog = pDialog;
}
CDXUTSlider::CDXUTSlider( CDXUTDialog *pDialog )
{
    m_Type = DXUT_CONTROL_SLIDER;
    m_pDialog = pDialog;

    m_nMin = 0;
    m_nMax = 100;
    m_nValue = 50;

    m_bPressed = false;

	field_76 = false;
}
BOOL CDXUTSlider::ContainsPoint( POINT pt ) 
{ 
    return ( PtInRect( &m_rcBoundingBox, pt ) || 
             PtInRect( &m_rcButton, pt ) ); 
}
BOOL CDXUTCheckBox::ContainsPoint( POINT pt ) 
{ 
    return ( PtInRect( &m_rcBoundingBox, pt ) || 
             PtInRect( &m_rcButton, pt ) ); 
}
HRESULT CDXUTDialog::AddSlider( int ID, int x, int y, int width, int height, int min, int max, int value, bool bIsDefault, CDXUTSlider** ppCreated )
{
    HRESULT hr = S_OK;

    CDXUTSlider* pSlider = new CDXUTSlider( this );

    if( ppCreated != NULL )
        *ppCreated = pSlider;

    if( pSlider == NULL )
        return E_OUTOFMEMORY;

    hr = AddControl( pSlider );
    if( FAILED(hr) )
        return hr;

    // Set the ID and list index
    pSlider->SetID( ID ); 
    pSlider->SetLocation( x, y );
    pSlider->SetSize( width, height );
    pSlider->m_bIsDefault = bIsDefault;
    pSlider->SetRange( min, max );
    pSlider->SetValue( value );

    return S_OK;
}
HRESULT CDXUTDialog::AddStatic( int ID, LPCTSTR strText, int x, int y, int width, int height, bool bIsDefault, CDXUTStatic** ppCreated )
{
    HRESULT hr = S_OK;

    CDXUTStatic* pStatic = new CDXUTStatic( this );

    if( ppCreated != NULL )
        *ppCreated = pStatic;

    if( pStatic == NULL )
        return E_OUTOFMEMORY;

    hr = AddControl( pStatic );
    if( FAILED(hr) )
        return hr;

    // Set the ID and list index
    pStatic->SetID( ID ); 
    pStatic->SetText( strText );
    pStatic->SetLocation( x, y );
    pStatic->SetSize( width, height );
    pStatic->m_bIsDefault = bIsDefault;

    return S_OK;
}
HRESULT CDXUTDialog::AddButton( int ID, LPCTSTR strText, int x, int y, int width, int height, UINT nHotkey, bool bIsDefault, CDXUTButton** ppCreated )
{
    HRESULT hr = S_OK;

    CDXUTButton* pButton = new CDXUTButton( this );

    if( ppCreated != NULL )
        *ppCreated = pButton;

    if( pButton == NULL )
        return E_OUTOFMEMORY;

    hr = AddControl( pButton );
    if( FAILED(hr) )
        return hr;

    // Set the ID and list index
    pButton->SetID( ID ); 
    pButton->SetText( strText );
    pButton->SetLocation( x, y );
    pButton->SetSize( width, height );
    pButton->SetHotkey( nHotkey );
    pButton->m_bIsDefault = bIsDefault;

    return S_OK;
}
HRESULT CDXUTDialog::AddCheckBox( int ID, LPCTSTR strText, int x, int y, int width, int height, bool bChecked, UINT nHotkey, bool bIsDefault, CDXUTCheckBox** ppCreated )
{
    HRESULT hr = S_OK;

    CDXUTCheckBox* pCheckBox = new CDXUTCheckBox( this );

    if( ppCreated != NULL )
        *ppCreated = pCheckBox;

    if( pCheckBox == NULL )
        return E_OUTOFMEMORY;

    hr = AddControl( pCheckBox );
    if( FAILED(hr) )
        return hr;

    // Set the ID and list index
    pCheckBox->SetID( ID ); 
    pCheckBox->SetText( strText );
    pCheckBox->SetLocation( x, y );
    pCheckBox->SetSize( width, height );
    pCheckBox->SetHotkey( nHotkey );
    pCheckBox->m_bIsDefault = bIsDefault;
    pCheckBox->SetChecked( bChecked );
    
    return S_OK;
}
HRESULT CDXUTDialog::AddRadioButton( int ID, UINT nButtonGroup, LPCTSTR strText, int x, int y, int width, int height, bool bChecked, UINT nHotkey, bool bIsDefault, CDXUTRadioButton** ppCreated )
{
    HRESULT hr = S_OK;

    CDXUTRadioButton* pRadioButton = new CDXUTRadioButton( this );

    if( ppCreated != NULL )
        *ppCreated = pRadioButton;

    if( pRadioButton == NULL )
        return E_OUTOFMEMORY;

    hr = AddControl( pRadioButton );
    if( FAILED(hr) )
        return hr;

    // Set the ID and list index
    pRadioButton->SetID( ID ); 
    pRadioButton->SetText( strText );
    pRadioButton->SetButtonGroup( nButtonGroup );
    pRadioButton->SetLocation( x, y );
    pRadioButton->SetSize( width, height );
    pRadioButton->SetHotkey( nHotkey );
    pRadioButton->SetChecked( bChecked );
    pRadioButton->m_bIsDefault = bIsDefault;
    pRadioButton->SetChecked( bChecked );

    return S_OK;
}

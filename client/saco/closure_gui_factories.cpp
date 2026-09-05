#include "d3d9/common/dxstdafx.h"
#include <new>
HRESULT CDXUTDialog::InitControl( CDXUTControl* pControl )
{
    HRESULT hr;

    if( pControl == NULL )
        return E_INVALIDARG;

    pControl->m_Index = m_Controls.GetSize();
    
    // Look for a default Element entries
    for( int i=0; i < m_DefaultElements.GetSize(); i++ )
    {
        DXUTElementHolder* pElementHolder = m_DefaultElements.GetAt( i );
        if( pElementHolder->nControlType == pControl->GetType() )
            pControl->SetElement( pElementHolder->iElement, &pElementHolder->Element );
    }

    V_RETURN( pControl->OnInit() );

    return S_OK;
}
HRESULT CDXUTDialog::AddControl( CDXUTControl* pControl )
{
    HRESULT hr = S_OK;

    hr = InitControl( pControl );
    if( FAILED(hr) )
        return DXTRACE_ERR( "CDXUTDialog::InitControl", hr );

    // Add to the list
    hr = m_Controls.Add( pControl );
    if( FAILED(hr) )
    {
        return DXTRACE_ERR( "CGrowableArray::Add", hr );
    }

    return S_OK;
}
void CDXUTSlider::SetRange( int nMin, int nMax ) 
{
    m_nMin = nMin;
    m_nMax = nMax;

    SetValueInternal( m_nValue, false );
}
HRESULT CDXUTStatic::SetText( LPCTSTR strText )
{
    if( strText == NULL )
    {
        m_strText[0] = 0;
        return S_OK;
    }
    
    StringCchCopy( m_strText, MAX_PATH, strText); 
    return S_OK;
}

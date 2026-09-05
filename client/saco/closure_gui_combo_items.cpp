#include "d3d9/common/dxstdafx.h"
#include <new>
HRESULT CDXUTComboBox::AddItem( const TCHAR* strText, void* pData )
{
    // Validate parameters
    if( strText== NULL )
    {
        return E_INVALIDARG;
    }
    
    // Create a new item and set the data
    DXUTComboBoxItem* pItem = new DXUTComboBoxItem;
    if( pItem == NULL )
    {
        return DXTRACE_ERR_MSGBOX( "new", E_OUTOFMEMORY );
    }
    
    ZeroMemory( pItem, sizeof(DXUTComboBoxItem) );
    StringCchCopy( pItem->strText, 256, strText );
    pItem->pData = pData;

    m_Items.Add( pItem );

    // Update the scroll bar with new range
    m_ScrollBar.SetTrackRange( 0, m_Items.GetSize() );

    // If this is the only item in the list, it's selected
    if( GetNumItems() == 1 )
    {
        m_iSelected = 0;
        m_iFocused = 0;
        m_pDialog->SendEvent( EVENT_COMBOBOX_SELECTION_CHANGED, false, this );
    }

    return S_OK;
}


//--------------------------------------------------------------------------------------
void CDXUTComboBox::RemoveItem( UINT index )
{
    DXUTComboBoxItem* pItem = m_Items.GetAt( index );
    SAFE_DELETE( pItem );
    m_Items.Remove( index );
    m_ScrollBar.SetTrackRange( 0, m_Items.GetSize() );
    if( m_iSelected >= m_Items.GetSize() )
        m_iSelected = m_Items.GetSize() - 1;
}


//--------------------------------------------------------------------------------------
// MATCH
void CDXUTComboBox::RemoveAllItems()
{
    for( int i=0; i < m_Items.GetSize(); i++ )
    {
        DXUTComboBoxItem* pItem = m_Items.GetAt( i );
        SAFE_DELETE( pItem );
    }

    m_Items.RemoveAll();
    m_ScrollBar.SetTrackRange( 0, 1 );
    m_iFocused = m_iSelected = -1;
}



//--------------------------------------------------------------------------------------
bool CDXUTComboBox::ContainsItem( const TCHAR* strText, UINT iStart )
{
    return ( -1 != FindItem( strText, iStart ) );
}


//--------------------------------------------------------------------------------------
int CDXUTComboBox::FindItem( const TCHAR* strText, UINT iStart )
{
    if( strText == NULL )
        return -1;

    for( int i = iStart; i < m_Items.GetSize(); i++ )
    {
        DXUTComboBoxItem* pItem = m_Items.GetAt(i);

        if( 0 == strcmp( pItem->strText, strText ) )
        {
            return i;
        }
    }

    return -1;
}


//--------------------------------------------------------------------------------------
void* CDXUTComboBox::GetSelectedData()
{
    if( m_iSelected < 0 )
        return NULL;

    DXUTComboBoxItem* pItem = m_Items.GetAt( m_iSelected );
    return pItem->pData;
}


//--------------------------------------------------------------------------------------
DXUTComboBoxItem* CDXUTComboBox::GetSelectedItem()
{
    if( m_iSelected < 0 )
        return NULL;

    return m_Items.GetAt( m_iSelected );
}


//--------------------------------------------------------------------------------------
void* CDXUTComboBox::GetItemData( const TCHAR* strText )
{
    int index = FindItem( strText );
    if( index == -1 )
    {
        return NULL;
    }

    DXUTComboBoxItem* pItem = m_Items.GetAt(index);
    if( pItem == NULL )
    {
        DXTRACE_ERR( "CGrowableArray::GetAt", E_FAIL );
        return NULL;
    }

    return pItem->pData;
}


//--------------------------------------------------------------------------------------
void* CDXUTComboBox::GetItemData( int nIndex )
{
    if( nIndex < 0 || nIndex >= m_Items.GetSize() )
        return NULL;

    return m_Items.GetAt( nIndex )->pData;
}


//--------------------------------------------------------------------------------------
HRESULT CDXUTComboBox::SetSelectedByIndex( UINT index )
{
    if( index >= GetNumItems() )
        return E_INVALIDARG;

    m_iFocused = m_iSelected = index;
    m_pDialog->SendEvent( EVENT_COMBOBOX_SELECTION_CHANGED, false, this );

    return S_OK;
}



//--------------------------------------------------------------------------------------
HRESULT CDXUTComboBox::SetSelectedByText( const TCHAR* strText )
{
    if( strText == NULL )
        return E_INVALIDARG;

    int index = FindItem( strText );
    if( index == -1 )
        return E_FAIL;

    m_iFocused = m_iSelected = index;
    m_pDialog->SendEvent( EVENT_COMBOBOX_SELECTION_CHANGED, false, this );

    return S_OK;
}



//--------------------------------------------------------------------------------------
HRESULT CDXUTComboBox::SetSelectedByData( void* pData )
{
    for( int i=0; i < m_Items.GetSize(); i++ )
    {
        DXUTComboBoxItem* pItem = m_Items.GetAt(i);

        if( pItem->pData == pData )
        {
            m_iFocused = m_iSelected = i;
            m_pDialog->SendEvent( EVENT_COMBOBOX_SELECTION_CHANGED, false, this );
            return S_OK;
        }
    }

    return E_FAIL;
}

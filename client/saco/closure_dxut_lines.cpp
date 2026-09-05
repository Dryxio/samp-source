// Complete original SDK line-manager implementation.
#include "d3d9/common/dxstdafx.h"
CDXUTLineManager::CDXUTLineManager()
{
    m_pd3dDevice = NULL;
    m_pD3DXLine = NULL;
}


//--------------------------------------------------------------------------------------
CDXUTLineManager::~CDXUTLineManager()
{
    OnDeletedDevice();
}


//--------------------------------------------------------------------------------------
HRESULT CDXUTLineManager::OnCreatedDevice( IDirect3DDevice9* pd3dDevice )
{
    m_pd3dDevice = pd3dDevice;

    HRESULT hr;
    hr = D3DXCreateLine( m_pd3dDevice, &m_pD3DXLine );
    if( FAILED(hr) )
        return hr;

    return S_OK;
}


//--------------------------------------------------------------------------------------
HRESULT CDXUTLineManager::OnResetDevice()
{
    if( m_pD3DXLine )
        m_pD3DXLine->OnResetDevice();

    return S_OK;
}


//--------------------------------------------------------------------------------------
HRESULT CDXUTLineManager::OnRender()
{
    HRESULT hr;
    if( NULL == m_pD3DXLine )
        return E_INVALIDARG;

    bool bDrawingHasBegun = false;
    float fLastWidth = 0.0f;
    bool bLastAntiAlias = false;
    
    for( int i=0; i<m_LinesList.GetSize(); i++ )
    {
        LINE_NODE* pLineNode = m_LinesList.GetAt(i);
        if( pLineNode )
        {
            if( !bDrawingHasBegun || 
                fLastWidth != pLineNode->fWidth || 
                bLastAntiAlias != pLineNode->bAntiAlias )
            {
                if( bDrawingHasBegun )
                {
                    hr = m_pD3DXLine->End();
                    if( FAILED(hr) )
                        return hr;
                }

                m_pD3DXLine->SetWidth( pLineNode->fWidth );
                m_pD3DXLine->SetAntialias( pLineNode->bAntiAlias );

                fLastWidth = pLineNode->fWidth;
                bLastAntiAlias = pLineNode->bAntiAlias;

                hr = m_pD3DXLine->Begin();
                if( FAILED(hr) )
                    return hr;
                bDrawingHasBegun = true;
            }

            hr = m_pD3DXLine->Draw( pLineNode->pVertexList, pLineNode->dwVertexListCount, pLineNode->Color );
            if( FAILED(hr) )
                return hr;
        }
    }

    if( bDrawingHasBegun )
    {
        hr = m_pD3DXLine->End();
        if( FAILED(hr) )
            return hr;
    }

    return S_OK;
}


//--------------------------------------------------------------------------------------
HRESULT CDXUTLineManager::OnLostDevice()
{
    if( m_pD3DXLine )
        m_pD3DXLine->OnLostDevice();

    return S_OK;
}


//--------------------------------------------------------------------------------------
HRESULT CDXUTLineManager::OnDeletedDevice()
{
    RemoveAllLines();
    SAFE_RELEASE( m_pD3DXLine );

    return S_OK;
}


//--------------------------------------------------------------------------------------
HRESULT CDXUTLineManager::AddLine( int* pnLineID, D3DXVECTOR2* pVertexList, DWORD dwVertexListCount, D3DCOLOR Color, float fWidth, float fScaleRatio, bool bAntiAlias )
{
    if( pVertexList == NULL || dwVertexListCount == 0 )
        return E_INVALIDARG;

    LINE_NODE* pLineNode = new LINE_NODE;
    if( pLineNode == NULL )
        return E_OUTOFMEMORY;
    ZeroMemory( pLineNode, sizeof(LINE_NODE) );

    pLineNode->nLineID = m_LinesList.GetSize();
    pLineNode->Color = Color;
    pLineNode->fWidth = fWidth;
    pLineNode->bAntiAlias = bAntiAlias;
    pLineNode->dwVertexListCount = dwVertexListCount;

    if( pnLineID )
        *pnLineID = pLineNode->nLineID;

    pLineNode->pVertexList = new D3DXVECTOR2[dwVertexListCount];
    if( pLineNode->pVertexList == NULL )
    {
        delete pLineNode;
        return E_OUTOFMEMORY;
    }
    for( DWORD i=0; i<dwVertexListCount; i++ )
    {
        pLineNode->pVertexList[i] = pVertexList[i] * fScaleRatio;
    }

    m_LinesList.Add( pLineNode );

    return S_OK;
}


//--------------------------------------------------------------------------------------
HRESULT CDXUTLineManager::AddRect( int* pnLineID, RECT rc, D3DCOLOR Color, float fWidth, float fScaleRatio, bool bAntiAlias )
{
    if( fWidth > 2.0f )
    {
        D3DXVECTOR2 vertexList[8];

        vertexList[0].x = (float)rc.left;
        vertexList[0].y = (float)rc.top - (fWidth/2.0f);

        vertexList[1].x = (float)rc.left;
        vertexList[1].y = (float)rc.bottom + (fWidth/2.0f);

        vertexList[2].x = (float)rc.left;
        vertexList[2].y = (float)rc.bottom - 0.5f;

        vertexList[3].x = (float)rc.right;
        vertexList[3].y = (float)rc.bottom - 0.5f;

        vertexList[4].x = (float)rc.right;
        vertexList[4].y = (float)rc.bottom + (fWidth/2.0f);

        vertexList[5].x = (float)rc.right;
        vertexList[5].y = (float)rc.top - (fWidth/2.0f);

        vertexList[6].x = (float)rc.right;
        vertexList[6].y = (float)rc.top;

        vertexList[7].x = (float)rc.left;
        vertexList[7].y = (float)rc.top;
        
        return AddLine( pnLineID, vertexList, 8, Color, fWidth, fScaleRatio, bAntiAlias );
    }
    else
    {
        D3DXVECTOR2 vertexList[5];
        vertexList[0].x = (float)rc.left;
        vertexList[0].y = (float)rc.top;

        vertexList[1].x = (float)rc.left;
        vertexList[1].y = (float)rc.bottom;

        vertexList[2].x = (float)rc.right;
        vertexList[2].y = (float)rc.bottom;

        vertexList[3].x = (float)rc.right;
        vertexList[3].y = (float)rc.top;

        vertexList[4].x = (float)rc.left;
        vertexList[4].y = (float)rc.top;
        
        return AddLine( pnLineID, vertexList, 5, Color, fWidth, fScaleRatio, bAntiAlias );
    }
}



//--------------------------------------------------------------------------------------
HRESULT CDXUTLineManager::RemoveLine( int nLineID )
{
    for( int i=0; i<m_LinesList.GetSize(); i++ )
    {
        LINE_NODE* pLineNode = m_LinesList.GetAt(i);
        if( pLineNode && pLineNode->nLineID == nLineID )
        {
            SAFE_DELETE_ARRAY( pLineNode->pVertexList );
            delete pLineNode;
            m_LinesList.SetAt(i, NULL);
        }
    }

    return S_OK;
}


//--------------------------------------------------------------------------------------
HRESULT CDXUTLineManager::RemoveAllLines()
{
    for( int i=0; i<m_LinesList.GetSize(); i++ )
    {
        LINE_NODE* pLineNode = m_LinesList.GetAt(i);
        if( pLineNode )
        {
            SAFE_DELETE_ARRAY( pLineNode->pVertexList );
            delete pLineNode;
        }
    }
    m_LinesList.RemoveAll();

    return S_OK;
}


//--------------------------------------------------------------------------------------

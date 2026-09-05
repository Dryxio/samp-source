// Complete original SDK direction-widget implementation.
#include "d3d9/common/dxstdafx.h"
IDirect3DDevice9* CDXUTDirectionWidget::s_pd3dDevice = NULL;
ID3DXEffect*      CDXUTDirectionWidget::s_pEffect = NULL;       
ID3DXMesh*        CDXUTDirectionWidget::s_pMesh = NULL;    


//--------------------------------------------------------------------------------------
CDXUTDirectionWidget::CDXUTDirectionWidget()
{
    m_fRadius = 1.0f;
    m_vDefaultDir = D3DXVECTOR3(0,1,0);
    m_vCurrentDir = m_vDefaultDir;
    m_nRotateMask = MOUSE_RIGHT_BUTTON;

    D3DXMatrixIdentity( &m_mView );
    D3DXMatrixIdentity( &m_mRot );
    D3DXMatrixIdentity( &m_mRotSnapshot );
}


//--------------------------------------------------------------------------------------
HRESULT CDXUTDirectionWidget::StaticOnCreateDevice( IDirect3DDevice9* pd3dDevice )
{
    TCHAR str[MAX_PATH];
    HRESULT hr;

    s_pd3dDevice = pd3dDevice;
   
    // Read the D3DX effect file
    DXUTFindDXSDKMediaFileCch( str, MAX_PATH, "UI\\DXUTShared.fx" );

    // If this fails, there should be debug output as to 
    // why the .fx file failed to compile
    D3DXCreateEffectFromFileA( s_pd3dDevice, str, NULL, NULL, D3DXFX_NOT_CLONEABLE, NULL, &s_pEffect, NULL );

    // Load the mesh with D3DX and get back a ID3DXMesh*.  For this
    // sample we'll ignore the X file's embedded materials since we know 
    // exactly the model we're loading.  See the mesh samples such as
    // "OptimizedMesh" for a more generic mesh loading example.
    DXUTFindDXSDKMediaFileCch( str, MAX_PATH, "UI\\arrow.x" );

    D3DXLoadMeshFromX( str, D3DXMESH_MANAGED, s_pd3dDevice, NULL, 
                                 NULL, NULL, NULL, &s_pMesh);

    // Optimize the mesh for this graphics card's vertex cache 
    // so when rendering the mesh's triangle list the vertices will 
    // cache hit more often so it won't have to re-execute the vertex shader 
    // on those vertices so it will improve perf.     
    DWORD* rgdwAdjacency = new DWORD[s_pMesh->GetNumFaces() * 3];
    if( rgdwAdjacency == NULL )
        return E_OUTOFMEMORY;
    s_pMesh->GenerateAdjacency(1e-6f,rgdwAdjacency);
    s_pMesh->OptimizeInplace(D3DXMESHOPT_VERTEXCACHE, rgdwAdjacency, NULL, NULL, NULL);
    delete []rgdwAdjacency;

    return S_OK;
}


//--------------------------------------------------------------------------------------
HRESULT CDXUTDirectionWidget::OnResetDevice( const D3DSURFACE_DESC* pBackBufferSurfaceDesc )
{
    m_ArcBall.SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );
    return S_OK;
}


//--------------------------------------------------------------------------------------
void CDXUTDirectionWidget::StaticOnLostDevice()
{
    if( s_pEffect )
        s_pEffect->OnLostDevice();
}


//--------------------------------------------------------------------------------------
void CDXUTDirectionWidget::StaticOnDestroyDevice()
{
    SAFE_RELEASE(s_pEffect);
    SAFE_RELEASE(s_pMesh);
}    


//--------------------------------------------------------------------------------------
LRESULT CDXUTDirectionWidget::HandleMessages( HWND hWnd, UINT uMsg, 
                                              WPARAM wParam, LPARAM lParam )
{
    switch( uMsg )
    {
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        {
            if( ((m_nRotateMask & MOUSE_LEFT_BUTTON) != 0 && uMsg == WM_LBUTTONDOWN) ||
                ((m_nRotateMask & MOUSE_MIDDLE_BUTTON) != 0 && uMsg == WM_MBUTTONDOWN) ||
                ((m_nRotateMask & MOUSE_RIGHT_BUTTON) != 0 && uMsg == WM_RBUTTONDOWN) )
            {
                int iMouseX = (int)(short)LOWORD(lParam);
                int iMouseY = (int)(short)HIWORD(lParam);
                m_ArcBall.OnBegin( iMouseX, iMouseY );
                SetCapture(hWnd);
            }
            return TRUE;
        }

        case WM_MOUSEMOVE:
        {
            if( m_ArcBall.IsBeingDragged() )
            {
                int iMouseX = (int)(short)LOWORD(lParam);
                int iMouseY = (int)(short)HIWORD(lParam);
                m_ArcBall.OnMove( iMouseX, iMouseY );
                UpdateLightDir();
            }
            return TRUE;
        }

        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
        {
            if( ((m_nRotateMask & MOUSE_LEFT_BUTTON) != 0 && uMsg == WM_LBUTTONUP) ||
                ((m_nRotateMask & MOUSE_MIDDLE_BUTTON) != 0 && uMsg == WM_MBUTTONUP) ||
                ((m_nRotateMask & MOUSE_RIGHT_BUTTON) != 0 && uMsg == WM_RBUTTONUP) )
            {
                m_ArcBall.OnEnd();
                ReleaseCapture();
            }

            UpdateLightDir();
            return TRUE;
        }
    }

    return 0;
}


//--------------------------------------------------------------------------------------
HRESULT CDXUTDirectionWidget::OnRender( D3DXCOLOR color, const D3DXMATRIX* pmView, 
                                        const D3DXMATRIX* pmProj, const D3DXVECTOR3* pEyePt )
{
    m_mView = *pmView;

    // Render the light spheres so the user can visually see the light dir
    UINT iPass, cPasses;
    D3DXMATRIX mRotate;
    D3DXMATRIX mScale;
    D3DXMATRIX mTrans;
    D3DXMATRIXA16 mWorldViewProj;
    HRESULT hr;

    s_pEffect->SetTechnique( "RenderWith1LightNoTexture" );
    s_pEffect->SetVector( "g_MaterialDiffuseColor", (D3DXVECTOR4*)&color);

    D3DXVECTOR3 vEyePt;
    D3DXVec3Normalize( &vEyePt, pEyePt );
    V( s_pEffect->SetValue( "g_LightDir", &vEyePt, sizeof(D3DXVECTOR3) ) );

    // Rotate arrow model to point towards origin
    D3DXMATRIX mRotateA, mRotateB;
    D3DXVECTOR3 vAt = D3DXVECTOR3(0,0,0);
    D3DXVECTOR3 vUp = D3DXVECTOR3(0,1,0);
    D3DXMatrixRotationX( &mRotateB, D3DX_PI );
    D3DXMatrixLookAtLH( &mRotateA, &m_vCurrentDir, &vAt, &vUp );
    D3DXMatrixInverse( &mRotateA, NULL, &mRotateA );
    mRotate = mRotateB * mRotateA;

    D3DXVECTOR3 vL = m_vCurrentDir * m_fRadius * 1.0f;
    D3DXMatrixTranslation( &mTrans, vL.x, vL.y, vL.z );
    D3DXMatrixScaling( &mScale, m_fRadius*0.2f, m_fRadius*0.2f, m_fRadius*0.2f );

    D3DXMATRIX mWorld = mRotate * mScale * mTrans;
    mWorldViewProj = mWorld * (m_mView) * (*pmProj);

    V( s_pEffect->SetMatrix( "g_mWorldViewProjection", &mWorldViewProj ) );
    V( s_pEffect->SetMatrix( "g_mWorld", &mWorld ) );

    for( int iSubset=0; iSubset<2; iSubset++ )
    {
        V( s_pEffect->Begin(&cPasses, 0) );
        for (iPass = 0; iPass < cPasses; iPass++)
        {
            V( s_pEffect->BeginPass(iPass) );
            V( s_pMesh->DrawSubset(iSubset) );
            V( s_pEffect->EndPass() );
        }
        V( s_pEffect->End() );
    }

    return S_OK;
}


//--------------------------------------------------------------------------------------
HRESULT CDXUTDirectionWidget::UpdateLightDir()
{
    D3DXMATRIX mInvView;
    D3DXMatrixInverse(&mInvView, NULL, &m_mView);
    mInvView._41 = mInvView._42 = mInvView._43 = 0;

    D3DXMATRIX mLastRotInv;
    D3DXMatrixInverse(&mLastRotInv, NULL, &m_mRotSnapshot);

    D3DXMATRIX mRot = *m_ArcBall.GetRotationMatrix();
    m_mRotSnapshot = mRot;

    // Accumulate the delta of the arcball's rotation in view space.
    // Note that per-frame delta rotations could be problematic over long periods of time.
    m_mRot *= m_mView * mLastRotInv * mRot * mInvView;

    // Since we're accumulating delta rotations, we need to orthonormalize 
    // the matrix to prevent eventual matrix skew
    D3DXVECTOR3* pXBasis = (D3DXVECTOR3*) &m_mRot._11;
    D3DXVECTOR3* pYBasis = (D3DXVECTOR3*) &m_mRot._21;
    D3DXVECTOR3* pZBasis = (D3DXVECTOR3*) &m_mRot._31;
    D3DXVec3Normalize( pXBasis, pXBasis );
    D3DXVec3Cross( pYBasis, pZBasis, pXBasis );
    D3DXVec3Normalize( pYBasis, pYBasis );
    D3DXVec3Cross( pZBasis, pXBasis, pYBasis );

    // Transform the default direction vector by the light's rotation matrix
    D3DXVec3TransformNormal( &m_vCurrentDir, &m_vDefaultDir, &m_mRot );

    return S_OK;
}

//--------------------------------------------------------------------------------------

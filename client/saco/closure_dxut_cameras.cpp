// Complete original camera/arcball family.
#include "d3d9/common/dxstdafx.h"
CD3DArcBall::CD3DArcBall()
{
    Reset();
    m_vDownPt = D3DXVECTOR3(0,0,0);
    m_vCurrentPt = D3DXVECTOR3(0,0,0);
    m_Offset.x = m_Offset.y = 0;

    RECT rc;
    GetClientRect( GetForegroundWindow(), &rc );
    SetWindow( rc.right, rc.bottom );
}





//--------------------------------------------------------------------------------------
void CD3DArcBall::Reset()
{
    D3DXQuaternionIdentity( &m_qDown );
    D3DXQuaternionIdentity( &m_qNow );
    D3DXMatrixIdentity( &m_mRotation );
    D3DXMatrixIdentity( &m_mTranslation );
    D3DXMatrixIdentity( &m_mTranslationDelta );
    m_bDrag = FALSE;
    m_fRadiusTranslation = 1.0f;
    m_fRadius = 1.0f;
}




//--------------------------------------------------------------------------------------
D3DXVECTOR3 CD3DArcBall::ScreenToVector( float fScreenPtX, float fScreenPtY )
{
    // Scale to screen
    FLOAT x   = -(fScreenPtX - m_Offset.x - m_nWidth/2)  / (m_fRadius*m_nWidth/2);
    FLOAT y   =  (fScreenPtY - m_Offset.y - m_nHeight/2) / (m_fRadius*m_nHeight/2);

    FLOAT z   = 0.0f;
    FLOAT mag = x*x + y*y;

    if( mag > 1.0f )
    {
        FLOAT scale = 1.0f/sqrtf(mag);
        x *= scale;
        y *= scale;
    }
    else
        z = sqrtf( 1.0f - mag );

    // Return vector
    return D3DXVECTOR3( x, y, z );
}




//--------------------------------------------------------------------------------------
D3DXQUATERNION CD3DArcBall::QuatFromBallPoints(const D3DXVECTOR3 &vFrom, const D3DXVECTOR3 &vTo)
{
    D3DXVECTOR3 vPart;
    float fDot = D3DXVec3Dot(&vFrom, &vTo);
    D3DXVec3Cross(&vPart, &vFrom, &vTo);

    return D3DXQUATERNION(vPart.x, vPart.y, vPart.z, fDot);
}




//--------------------------------------------------------------------------------------
void CD3DArcBall::OnBegin( int nX, int nY )
{
    // Only enter the drag state if the click falls
    // inside the click rectangle.
    if( nX >= m_Offset.x &&
        nX < m_Offset.x + m_nWidth &&
        nY >= m_Offset.y &&
        nY < m_Offset.y + m_nHeight )
    {
        m_bDrag = true;
        m_qDown = m_qNow;
        m_vDownPt = ScreenToVector( (float)nX, (float)nY );
    }
}




//--------------------------------------------------------------------------------------
void CD3DArcBall::OnMove( int nX, int nY )
{
    if (m_bDrag) 
    { 
        m_vCurrentPt = ScreenToVector( (float)nX, (float)nY );
        m_qNow = m_qDown * QuatFromBallPoints( m_vDownPt, m_vCurrentPt );
    }
}




//--------------------------------------------------------------------------------------
void CD3DArcBall::OnEnd()
{
    m_bDrag = false;
}




//--------------------------------------------------------------------------------------
// Desc:
//--------------------------------------------------------------------------------------
LRESULT CD3DArcBall::HandleMessages( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    // Current mouse position
    int iMouseX = (short)LOWORD(lParam);
    int iMouseY = (short)HIWORD(lParam);

    switch( uMsg )
    {
        case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
            SetCapture( hWnd );
            OnBegin( iMouseX, iMouseY );
            return TRUE;

        case WM_LBUTTONUP:
            ReleaseCapture();
            OnEnd();
            return TRUE;

        case WM_RBUTTONDOWN:
        case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONDBLCLK:
            SetCapture( hWnd );
            // Store off the position of the cursor when the button is pressed
            m_ptLastMouse.x = iMouseX;
            m_ptLastMouse.y = iMouseY;
            return TRUE;

        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
            ReleaseCapture();
            return TRUE;

        case WM_MOUSEMOVE:
            if( MK_LBUTTON&wParam )
            {
                OnMove( iMouseX, iMouseY );
            }
            else if( (MK_RBUTTON&wParam) || (MK_MBUTTON&wParam) )
            {
                // Normalize based on size of window and bounding sphere radius
                FLOAT fDeltaX = ( m_ptLastMouse.x-iMouseX ) * m_fRadiusTranslation / m_nWidth;
                FLOAT fDeltaY = ( m_ptLastMouse.y-iMouseY ) * m_fRadiusTranslation / m_nHeight;

                if( wParam & MK_RBUTTON )
                {
                    D3DXMatrixTranslation( &m_mTranslationDelta, -2*fDeltaX, 2*fDeltaY, 0.0f );
                    D3DXMatrixMultiply( &m_mTranslation, &m_mTranslation, &m_mTranslationDelta );
                }
                else  // wParam & MK_MBUTTON
                {
                    D3DXMatrixTranslation( &m_mTranslationDelta, 0.0f, 0.0f, 5*fDeltaY );
                    D3DXMatrixMultiply( &m_mTranslation, &m_mTranslation, &m_mTranslationDelta );
                }

                // Store mouse coordinate
                m_ptLastMouse.x = iMouseX;
                m_ptLastMouse.y = iMouseY;
            }
            return TRUE;
    }

    return FALSE;
}




//--------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------
CBaseCamera::CBaseCamera()
{
    ZeroMemory( m_aKeys, sizeof(BYTE)*CAM_MAX_KEYS );

    // Set attributes for the view matrix
    D3DXVECTOR3 vEyePt    = D3DXVECTOR3(0.0f,0.0f,0.0f);
    D3DXVECTOR3 vLookatPt = D3DXVECTOR3(0.0f,0.0f,1.0f);

    // Setup the view matrix
    SetViewParams( &vEyePt, &vLookatPt );

    // Setup the projection matrix
    SetProjParams( D3DX_PI/4, 1.0f, 1.0f, 1000.0f );

    GetCursorPos( &m_ptLastMousePosition );
    m_bMouseLButtonDown = false;
    m_bMouseMButtonDown = false;
    m_bMouseRButtonDown = false;
    m_nCurrentButtonMask = 0;
    m_nMouseWheelDelta = 0;

    m_fCameraYawAngle = 0.0f;
    m_fCameraPitchAngle = 0.0f;

    SetRect( &m_rcDrag, LONG_MIN, LONG_MIN, LONG_MAX, LONG_MAX );
    m_vVelocity     = D3DXVECTOR3(0,0,0);
    m_bMovementDrag = false;
    m_vVelocityDrag = D3DXVECTOR3(0,0,0);
    m_fDragTimer    = 0.0f;
    m_fTotalDragTimeToZero = 0.25;
    m_vRotVelocity = D3DXVECTOR2(0,0);

    m_fRotationScaler = 0.01f;           
    m_fMoveScaler = 5.0f;           

    m_bInvertPitch = false;
    m_bEnableYAxisMovement = true;
    m_bEnablePositionMovement = true;

    m_vMouseDelta   = D3DXVECTOR2(0,0);
    m_fFramesToSmoothMouseData = 2.0f;

    m_bClipToBoundary = false;
    m_vMinBoundary = D3DXVECTOR3(-1,-1,-1);
    m_vMaxBoundary = D3DXVECTOR3(1,1,1);

    m_bResetCursorAfterMove = false;
}


//--------------------------------------------------------------------------------------
// Client can call this to change the position and direction of camera
//--------------------------------------------------------------------------------------
VOID CBaseCamera::SetViewParams( D3DXVECTOR3* pvEyePt, D3DXVECTOR3* pvLookatPt )
{
    if( NULL == pvEyePt || NULL == pvLookatPt )
        return;

    m_vDefaultEye = m_vEye = *pvEyePt;
    m_vDefaultLookAt = m_vLookAt = *pvLookatPt;

    // Calc the view matrix
    D3DXVECTOR3 vUp(0,1,0);
    D3DXMatrixLookAtLH( &m_mView, pvEyePt, pvLookatPt, &vUp );

    D3DXMATRIX mInvView;
    D3DXMatrixInverse( &mInvView, NULL, &m_mView );

    // The axis basis vectors and camera position are stored inside the 
    // position matrix in the 4 rows of the camera's world matrix.
    // To figure out the yaw/pitch of the camera, we just need the Z basis vector
    D3DXVECTOR3* pZBasis = (D3DXVECTOR3*) &mInvView._31;

    m_fCameraYawAngle   = atan2f( pZBasis->x, pZBasis->z );
    float fLen = sqrtf(pZBasis->z*pZBasis->z + pZBasis->x*pZBasis->x);
    m_fCameraPitchAngle = -atan2f( pZBasis->y, fLen );
}




//--------------------------------------------------------------------------------------
// Calculates the projection matrix based on input params
//--------------------------------------------------------------------------------------
VOID CBaseCamera::SetProjParams( FLOAT fFOV, FLOAT fAspect, FLOAT fNearPlane,
                                   FLOAT fFarPlane )
{
    // Set attributes for the projection matrix
    m_fFOV        = fFOV;
    m_fAspect     = fAspect;
    m_fNearPlane  = fNearPlane;
    m_fFarPlane   = fFarPlane;

    D3DXMatrixPerspectiveFovLH( &m_mProj, fFOV, fAspect, fNearPlane, fFarPlane );
}




//--------------------------------------------------------------------------------------
// Call this from your message proc so this class can handle window messages
//--------------------------------------------------------------------------------------
LRESULT CBaseCamera::HandleMessages( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    UNREFERENCED_PARAMETER( hWnd );
    UNREFERENCED_PARAMETER( lParam );

    switch( uMsg )
    {
        case WM_KEYDOWN:
        {
            // Map this key to a D3DUtil_CameraKeys enum and update the
            // state of m_aKeys[] by adding the KEY_WAS_DOWN_MASK|KEY_IS_DOWN_MASK mask
            // only if the key is not down
            D3DUtil_CameraKeys mappedKey = MapKey( (UINT)wParam );
            if( mappedKey != CAM_UNKNOWN )
            {
                if( FALSE == IsKeyDown(m_aKeys[mappedKey]) )
                    m_aKeys[ mappedKey ] = KEY_WAS_DOWN_MASK | KEY_IS_DOWN_MASK;
            }
            break;
        }

        case WM_KEYUP:
        {
            // Map this key to a D3DUtil_CameraKeys enum and update the
            // state of m_aKeys[] by removing the KEY_IS_DOWN_MASK mask.
            D3DUtil_CameraKeys mappedKey = MapKey( (UINT)wParam );
            if( mappedKey != CAM_UNKNOWN && (DWORD)mappedKey < 8 )
                m_aKeys[ mappedKey ] &= ~KEY_IS_DOWN_MASK;
            break;
        }

        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDBLCLK:
        case WM_LBUTTONDBLCLK:
        {
            // Compute the drag rectangle in screen coord.
            POINT ptCursor = { (short)LOWORD(lParam), (short)HIWORD(lParam) };

            // Update member var state
            if( ( uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONDBLCLK ) && PtInRect( &m_rcDrag, ptCursor ) )
                { m_bMouseLButtonDown = true; m_nCurrentButtonMask |= MOUSE_LEFT_BUTTON; }
            if( ( uMsg == WM_MBUTTONDOWN || uMsg == WM_MBUTTONDBLCLK ) && PtInRect( &m_rcDrag, ptCursor ) )
                { m_bMouseMButtonDown = true; m_nCurrentButtonMask |= MOUSE_MIDDLE_BUTTON; }
            if( ( uMsg == WM_RBUTTONDOWN || uMsg == WM_RBUTTONDBLCLK ) && PtInRect( &m_rcDrag, ptCursor ) )
                { m_bMouseRButtonDown = true; m_nCurrentButtonMask |= MOUSE_RIGHT_BUTTON; }

            // Capture the mouse, so if the mouse button is 
            // released outside the window, we'll get the WM_LBUTTONUP message
            SetCapture(hWnd);
            GetCursorPos( &m_ptLastMousePosition );
            return TRUE;
        }

        case WM_RBUTTONUP: 
        case WM_MBUTTONUP: 
        case WM_LBUTTONUP:   
        {
            // Update member var state
            if( uMsg == WM_LBUTTONUP ) { m_bMouseLButtonDown = false; m_nCurrentButtonMask &= ~MOUSE_LEFT_BUTTON; }
            if( uMsg == WM_MBUTTONUP ) { m_bMouseMButtonDown = false; m_nCurrentButtonMask &= ~MOUSE_MIDDLE_BUTTON; }
            if( uMsg == WM_RBUTTONUP ) { m_bMouseRButtonDown = false; m_nCurrentButtonMask &= ~MOUSE_RIGHT_BUTTON; }

            // Release the capture if no mouse buttons down
            if( !m_bMouseLButtonDown  && 
                !m_bMouseRButtonDown &&
                !m_bMouseMButtonDown )
            {
                ReleaseCapture();
            }
            break;
        }

        case WM_MOUSEWHEEL: 
            // Update member var state
            m_nMouseWheelDelta = (short)HIWORD(wParam) / 120;
            break;
    }

    return FALSE;
}




//--------------------------------------------------------------------------------------
// Figure out the mouse delta based on mouse movement
//--------------------------------------------------------------------------------------
void CBaseCamera::UpdateMouseDelta( float fElapsedTime )
{
    UNREFERENCED_PARAMETER( fElapsedTime );

    POINT ptCurMouseDelta;
    POINT ptCurMousePos;
    
    // Get current position of mouse
    GetCursorPos( &ptCurMousePos );

    // Calc how far it's moved since last frame
    ptCurMouseDelta.x = ptCurMousePos.x - m_ptLastMousePosition.x;
    ptCurMouseDelta.y = ptCurMousePos.y - m_ptLastMousePosition.y;

    // Record current position for next time
    m_ptLastMousePosition = ptCurMousePos;

    if( m_bResetCursorAfterMove )
    {
        // Set position of camera to center of desktop, 
        // so it always has room to move.  This is very useful
        // if the cursor is hidden.  If this isn't done and cursor is hidden, 
        // then invisible cursor will hit the edge of the screen 
        // and the user can't tell what happened
        POINT ptCenter;
        RECT rcDesktop;
        GetWindowRect( GetDesktopWindow(), &rcDesktop );
        ptCenter.x = (rcDesktop.right - rcDesktop.left) / 2;
        ptCenter.y = (rcDesktop.bottom - rcDesktop.top) / 2;   
        SetCursorPos( ptCenter.x, ptCenter.y );
        m_ptLastMousePosition = ptCenter;
    }

    // Smooth the relative mouse data over a few frames so it isn't 
    // jerky when moving slowly at low frame rates.
    float fPercentOfNew =  1.0f / m_fFramesToSmoothMouseData;
    float fPercentOfOld =  1.0f - fPercentOfNew;
    m_vMouseDelta.x = m_vMouseDelta.x*fPercentOfOld + ptCurMouseDelta.x*fPercentOfNew;
    m_vMouseDelta.y = m_vMouseDelta.y*fPercentOfOld + ptCurMouseDelta.y*fPercentOfNew;

    m_vRotVelocity = m_vMouseDelta * m_fRotationScaler;
}




//--------------------------------------------------------------------------------------
// Figure out the velocity based on keyboard input & drag if any
//--------------------------------------------------------------------------------------
void CBaseCamera::UpdateVelocity( float fElapsedTime )
{
    D3DXMATRIX mRotDelta;
    D3DXVECTOR3 vAccel = D3DXVECTOR3(0,0,0);

    if( m_bEnablePositionMovement )
    {
        // Update acceleration vector based on keyboard state
        if( IsKeyDown(m_aKeys[CAM_MOVE_FORWARD]) )
            vAccel.z += 1.0f;
        if( IsKeyDown(m_aKeys[CAM_MOVE_BACKWARD]) )
            vAccel.z -= 1.0f;
        if( m_bEnableYAxisMovement )
        {
            if( IsKeyDown(m_aKeys[CAM_MOVE_UP]) )
                vAccel.y += 1.0f;
            if( IsKeyDown(m_aKeys[CAM_MOVE_DOWN]) )
                vAccel.y -= 1.0f;
        }
        if( IsKeyDown(m_aKeys[CAM_STRAFE_RIGHT]) )
            vAccel.x += 1.0f;
        if( IsKeyDown(m_aKeys[CAM_STRAFE_LEFT]) )
            vAccel.x -= 1.0f;
    }

    // Normalize vector so if moving 2 dirs (left & forward), 
    // the camera doesn't move faster than if moving in 1 dir
    D3DXVec3Normalize( &vAccel, &vAccel );

    // Scale the acceleration vector
    vAccel *= m_fMoveScaler;

    if( m_bMovementDrag )
    {
        // Is there any acceleration this frame?
        if( D3DXVec3LengthSq( &vAccel ) > 0 )
        {
            // If so, then this means the user has pressed a movement key\
            // so change the velocity immediately to acceleration 
            // upon keyboard input.  This isn't normal physics
            // but it will give a quick response to keyboard input
            m_vVelocity = vAccel;
            m_fDragTimer = m_fTotalDragTimeToZero;
            m_vVelocityDrag = vAccel / m_fDragTimer;
        }
        else 
        {
            // If no key being pressed, then slowly decrease velocity to 0
            if( m_fDragTimer > 0 )
            {
                // Drag until timer is <= 0
                m_vVelocity -= m_vVelocityDrag * fElapsedTime;
                m_fDragTimer -= fElapsedTime;
            }
            else
            {
                // Zero velocity
                m_vVelocity = D3DXVECTOR3(0,0,0);
            }
        }
    }
    else
    {
        // No drag, so immediately change the velocity
        m_vVelocity = vAccel;
    }
}




//--------------------------------------------------------------------------------------
// Clamps pV to lie inside m_vMinBoundary & m_vMaxBoundary
//--------------------------------------------------------------------------------------
void CBaseCamera::ConstrainToBoundary( D3DXVECTOR3* pV )
{
    // Constrain vector to a bounding box 
    pV->x = __max(pV->x, m_vMinBoundary.x);
    pV->y = __max(pV->y, m_vMinBoundary.y);
    pV->z = __max(pV->z, m_vMinBoundary.z);

    pV->x = __min(pV->x, m_vMaxBoundary.x);
    pV->y = __min(pV->y, m_vMaxBoundary.y);
    pV->z = __min(pV->z, m_vMaxBoundary.z);
}




//--------------------------------------------------------------------------------------
// Maps a windows virtual key to an enum
//--------------------------------------------------------------------------------------
D3DUtil_CameraKeys CBaseCamera::MapKey( UINT nKey )
{
    // This could be upgraded to a method that's user-definable but for 
    // simplicity, we'll use a hardcoded mapping.
    switch( nKey )
    {
        case VK_CONTROL:  return CAM_CONTROLDOWN;
        case VK_LEFT:  return CAM_STRAFE_LEFT;
        case VK_RIGHT: return CAM_STRAFE_RIGHT;
        case VK_UP:    return CAM_MOVE_FORWARD;
        case VK_DOWN:  return CAM_MOVE_BACKWARD;
        case VK_PRIOR: return CAM_MOVE_UP;        // pgup
        case VK_NEXT:  return CAM_MOVE_DOWN;      // pgdn

        case 'A':      return CAM_STRAFE_LEFT;
        case 'D':      return CAM_STRAFE_RIGHT;
        case 'W':      return CAM_MOVE_FORWARD;
        case 'S':      return CAM_MOVE_BACKWARD;
        case 'Q':      return CAM_MOVE_DOWN;
        case 'E':      return CAM_MOVE_UP;

        case VK_NUMPAD4: return CAM_STRAFE_LEFT;
        case VK_NUMPAD6: return CAM_STRAFE_RIGHT;
        case VK_NUMPAD8: return CAM_MOVE_FORWARD;
        case VK_NUMPAD2: return CAM_MOVE_BACKWARD;
        case VK_NUMPAD9: return CAM_MOVE_UP;        
        case VK_NUMPAD3: return CAM_MOVE_DOWN;      

        case VK_HOME:   return CAM_RESET;
    }

    return CAM_UNKNOWN;
}




//--------------------------------------------------------------------------------------
// Reset the camera's position back to the default
//--------------------------------------------------------------------------------------
VOID CBaseCamera::Reset()
{
    SetViewParams( &m_vDefaultEye, &m_vDefaultLookAt );
}




//--------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------
CFirstPersonCamera::CFirstPersonCamera() :
    m_nActiveButtonMask( 0x07 )
{
}




//--------------------------------------------------------------------------------------
// Update the view matrix based on user input & elapsed time
//--------------------------------------------------------------------------------------
VOID CFirstPersonCamera::FrameMove( FLOAT fElapsedTime )
{
    if( DXUTGetGlobalTimer()->IsStopped() )
        fElapsedTime = 1.0f / DXUTGetFPS();

    if( IsKeyDown(m_aKeys[CAM_RESET]) )
        Reset();

    // Get the mouse movement (if any) if the mouse button are down
    if( m_nActiveButtonMask & m_nCurrentButtonMask )
        UpdateMouseDelta( fElapsedTime );

    // Get amount of velocity based on the keyboard input and drag (if any)
    UpdateVelocity( fElapsedTime );

    // Simple euler method to calculate position delta
    D3DXVECTOR3 vPosDelta = m_vVelocity * fElapsedTime;

    // If rotating the camera 
    if( m_nActiveButtonMask & m_nCurrentButtonMask )
    {
        // Update the pitch & yaw angle based on mouse movement
        float fYawDelta   = m_vRotVelocity.x;
        float fPitchDelta = m_vRotVelocity.y;

        // Invert pitch if requested
        if( m_bInvertPitch )
            fPitchDelta = -fPitchDelta;

        m_fCameraPitchAngle += fPitchDelta;
        m_fCameraYawAngle   += fYawDelta;

        // Limit pitch to straight up or straight down
        m_fCameraPitchAngle = __max( -D3DX_PI/2.0f,  m_fCameraPitchAngle );
        m_fCameraPitchAngle = __min( +D3DX_PI/2.0f,  m_fCameraPitchAngle );
    }

    // Make a rotation matrix based on the camera's yaw & pitch
    D3DXMATRIX mCameraRot;
    D3DXMatrixRotationYawPitchRoll( &mCameraRot, m_fCameraYawAngle, m_fCameraPitchAngle, 0 );

    // Transform vectors based on camera's rotation matrix
    D3DXVECTOR3 vWorldUp, vWorldAhead;
    D3DXVECTOR3 vLocalUp    = D3DXVECTOR3(0,1,0);
    D3DXVECTOR3 vLocalAhead = D3DXVECTOR3(0,0,1);
    D3DXVec3TransformCoord( &vWorldUp, &vLocalUp, &mCameraRot );
    D3DXVec3TransformCoord( &vWorldAhead, &vLocalAhead, &mCameraRot );

    // Transform the position delta by the camera's rotation 
    D3DXVECTOR3 vPosDeltaWorld;
    if( !m_bEnableYAxisMovement )
    {
        // If restricting Y movement, do not include pitch
        // when transforming position delta vector.
        D3DXMatrixRotationYawPitchRoll( &mCameraRot, m_fCameraYawAngle, 0.0f, 0.0f );
    }
    D3DXVec3TransformCoord( &vPosDeltaWorld, &vPosDelta, &mCameraRot );

    // Move the eye position 
    m_vEye += vPosDeltaWorld;
    if( m_bClipToBoundary )
        ConstrainToBoundary( &m_vEye );

    // Update the lookAt position based on the eye position 
    m_vLookAt = m_vEye + vWorldAhead;

    // Update the view matrix
    D3DXMatrixLookAtLH( &m_mView, &m_vEye, &m_vLookAt, &vWorldUp );

    D3DXMatrixInverse( &m_mCameraWorld, NULL, &m_mView );
}


//--------------------------------------------------------------------------------------
// Enable or disable each of the mouse buttons for rotation drag.
//--------------------------------------------------------------------------------------
void CFirstPersonCamera::SetRotateButtons( bool bLeft, bool bMiddle, bool bRight )
{
    m_nActiveButtonMask = ( bLeft ? MOUSE_LEFT_BUTTON : 0 ) |
                          ( bMiddle ? MOUSE_MIDDLE_BUTTON : 0 ) |
                          ( bRight ? MOUSE_RIGHT_BUTTON : 0 );
}


//--------------------------------------------------------------------------------------
// Constructor 
//--------------------------------------------------------------------------------------
CModelViewerCamera::CModelViewerCamera()
{
    D3DXMatrixIdentity( &m_mWorld );
    D3DXMatrixIdentity( &m_mModelRot );
    D3DXMatrixIdentity( &m_mModelLastRot );    
    D3DXMatrixIdentity( &m_mCameraRotLast );    
    m_vModelCenter = D3DXVECTOR3(0,0,0);
    m_fRadius    = 5.0f;
    m_fDefaultRadius = 5.0f;
    m_fMinRadius = 1.0f;
    m_fMaxRadius = FLT_MAX;
    m_bLimitPitch = false;
    m_bEnablePositionMovement = false;
    m_bAttachCameraToModel = false;

    m_nRotateModelButtonMask  = MOUSE_LEFT_BUTTON;
    m_nZoomButtonMask         = MOUSE_WHEEL;
    m_nRotateCameraButtonMask = MOUSE_RIGHT_BUTTON;
    m_bDragSinceLastUpdate    = true;
}




//--------------------------------------------------------------------------------------
// Update the view matrix & the model's world matrix based 
//       on user input & elapsed time
//--------------------------------------------------------------------------------------
VOID CModelViewerCamera::FrameMove( FLOAT fElapsedTime )
{
    if( IsKeyDown(m_aKeys[CAM_RESET]) )
        Reset();

    // If no dragged has happend since last time FrameMove is called,
    // no need to handle again.
    if( !m_bDragSinceLastUpdate )
        return;
    m_bDragSinceLastUpdate = false;

    // If no mouse button is held down, 
    // Get the mouse movement (if any) if the mouse button are down
    if( m_nCurrentButtonMask != 0 ) 
        UpdateMouseDelta( fElapsedTime );

    // Get amount of velocity based on the keyboard input and drag (if any)
    UpdateVelocity( fElapsedTime );

    // Simple euler method to calculate position delta
    D3DXVECTOR3 vPosDelta = m_vVelocity * fElapsedTime;

    // Change the radius from the camera to the model based on wheel scrolling
    if( m_nMouseWheelDelta && m_nZoomButtonMask == MOUSE_WHEEL )
        m_fRadius -= m_nMouseWheelDelta * m_fRadius * 0.1f;
    m_fRadius = __min( m_fMaxRadius, m_fRadius );
    m_fRadius = __max( m_fMinRadius, m_fRadius );
    m_nMouseWheelDelta = 0;

    // Get the inverse of the arcball's rotation matrix
    D3DXMATRIX mCameraRot;
    D3DXMatrixInverse( &mCameraRot, NULL, m_ViewArcBall.GetRotationMatrix() );

    // Transform vectors based on camera's rotation matrix
    D3DXVECTOR3 vWorldUp, vWorldAhead;
    D3DXVECTOR3 vLocalUp    = D3DXVECTOR3(0,1,0);
    D3DXVECTOR3 vLocalAhead = D3DXVECTOR3(0,0,1);
    D3DXVec3TransformCoord( &vWorldUp, &vLocalUp, &mCameraRot );
    D3DXVec3TransformCoord( &vWorldAhead, &vLocalAhead, &mCameraRot );

    // Transform the position delta by the camera's rotation 
    D3DXVECTOR3 vPosDeltaWorld;
    D3DXVec3TransformCoord( &vPosDeltaWorld, &vPosDelta, &mCameraRot );

    // Move the lookAt position 
    m_vLookAt += vPosDeltaWorld;
    if( m_bClipToBoundary )
        ConstrainToBoundary( &m_vLookAt );

    // Update the eye point based on a radius away from the lookAt position
    m_vEye = m_vLookAt - vWorldAhead * m_fRadius;

    // Update the view matrix
    D3DXMatrixLookAtLH( &m_mView, &m_vEye, &m_vLookAt, &vWorldUp );

    D3DXMATRIX mInvView;
    D3DXMatrixInverse( &mInvView, NULL, &m_mView );
    mInvView._41 = mInvView._42 = mInvView._43 = 0;

    D3DXMATRIX mModelLastRotInv;
    D3DXMatrixInverse(&mModelLastRotInv, NULL, &m_mModelLastRot);

    // Accumulate the delta of the arcball's rotation in view space.
    // Note that per-frame delta rotations could be problematic over long periods of time.
    D3DXMATRIX mModelRot;
    mModelRot = *m_WorldArcBall.GetRotationMatrix();
    m_mModelRot *= m_mView * mModelLastRotInv * mModelRot * mInvView;

    if( m_ViewArcBall.IsBeingDragged() && m_bAttachCameraToModel && !IsKeyDown(m_aKeys[CAM_CONTROLDOWN]) )
    {
        // Attach camera to model by inverse of the model rotation
        D3DXMATRIX mCameraLastRotInv;
        D3DXMatrixInverse(&mCameraLastRotInv, NULL, &m_mCameraRotLast);
        D3DXMATRIX mCameraRotDelta = mCameraLastRotInv * mCameraRot; // local to world matrix
        m_mModelRot *= mCameraRotDelta;
    }
    m_mCameraRotLast = mCameraRot; 

    m_mModelLastRot = mModelRot;

    // Since we're accumulating delta rotations, we need to orthonormalize 
    // the matrix to prevent eventual matrix skew
    D3DXVECTOR3* pXBasis = (D3DXVECTOR3*) &m_mModelRot._11;
    D3DXVECTOR3* pYBasis = (D3DXVECTOR3*) &m_mModelRot._21;
    D3DXVECTOR3* pZBasis = (D3DXVECTOR3*) &m_mModelRot._31;
    D3DXVec3Normalize( pXBasis, pXBasis );
    D3DXVec3Cross( pYBasis, pZBasis, pXBasis );
    D3DXVec3Normalize( pYBasis, pYBasis );
    D3DXVec3Cross( pZBasis, pXBasis, pYBasis );

    // Translate the rotation matrix to the same position as the lookAt position
    m_mModelRot._41 = m_vLookAt.x;
    m_mModelRot._42 = m_vLookAt.y;
    m_mModelRot._43 = m_vLookAt.z;

    // Translate world matrix so its at the center of the model
    D3DXMATRIX mTrans;
    D3DXMatrixTranslation( &mTrans, -m_vModelCenter.x, -m_vModelCenter.y, -m_vModelCenter.z );
    m_mWorld = mTrans * m_mModelRot;
}


void CModelViewerCamera::SetDragRect( RECT &rc )
{
    CBaseCamera::SetDragRect( rc );

    m_WorldArcBall.SetOffset( rc.left, rc.top );
    m_ViewArcBall.SetOffset( rc.left, rc.top );
    SetWindow( rc.right - rc.left, rc.bottom - rc.top );
}


//--------------------------------------------------------------------------------------
// Reset the camera's position back to the default
//--------------------------------------------------------------------------------------
VOID CModelViewerCamera::Reset()
{
    CBaseCamera::Reset();

    D3DXMatrixIdentity( &m_mWorld );
    D3DXMatrixIdentity( &m_mModelRot );
    D3DXMatrixIdentity( &m_mModelLastRot );    
    D3DXMatrixIdentity( &m_mCameraRotLast );    

    m_fRadius = m_fDefaultRadius;
    m_WorldArcBall.Reset();
    m_ViewArcBall.Reset();
}


//--------------------------------------------------------------------------------------
// Override for setting the view parameters
//--------------------------------------------------------------------------------------
void CModelViewerCamera::SetViewParams( D3DXVECTOR3* pvEyePt, D3DXVECTOR3* pvLookatPt )
{
    CBaseCamera::SetViewParams( pvEyePt, pvLookatPt );

    // Propogate changes to the member arcball
    D3DXQUATERNION quat;
    D3DXMATRIXA16 mRotation;
    D3DXVECTOR3 vUp(0,1,0);
    D3DXMatrixLookAtLH( &mRotation, pvEyePt, pvLookatPt, &vUp );
    D3DXQuaternionRotationMatrix( &quat, &mRotation );
    m_ViewArcBall.SetQuatNow( quat );

    // Set the radius according to the distance
    D3DXVECTOR3 vEyeToPoint;
    D3DXVec3Subtract( &vEyeToPoint, pvLookatPt, pvEyePt );
    SetRadius( D3DXVec3Length( &vEyeToPoint ) );

    // View information changed. FrameMove should be called.
    m_bDragSinceLastUpdate = true;
}



//--------------------------------------------------------------------------------------
// Call this from your message proc so this class can handle window messages
//--------------------------------------------------------------------------------------
LRESULT CModelViewerCamera::HandleMessages( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    CBaseCamera::HandleMessages( hWnd, uMsg, wParam, lParam );

    if( ( (uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONDBLCLK ) && m_nRotateModelButtonMask & MOUSE_LEFT_BUTTON) ||
        ( (uMsg == WM_MBUTTONDOWN || uMsg == WM_MBUTTONDBLCLK ) && m_nRotateModelButtonMask & MOUSE_MIDDLE_BUTTON) ||
        ( (uMsg == WM_RBUTTONDOWN || uMsg == WM_RBUTTONDBLCLK ) && m_nRotateModelButtonMask & MOUSE_RIGHT_BUTTON) )
    {
        int iMouseX = (short)LOWORD(lParam);
        int iMouseY = (short)HIWORD(lParam);
        m_WorldArcBall.OnBegin( iMouseX, iMouseY );
    }

    if( ( (uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONDBLCLK ) && m_nRotateCameraButtonMask & MOUSE_LEFT_BUTTON) ||
        ( (uMsg == WM_MBUTTONDOWN || uMsg == WM_MBUTTONDBLCLK ) && m_nRotateCameraButtonMask & MOUSE_MIDDLE_BUTTON) ||
        ( (uMsg == WM_RBUTTONDOWN || uMsg == WM_RBUTTONDBLCLK ) && m_nRotateCameraButtonMask & MOUSE_RIGHT_BUTTON) )
    {
        int iMouseX = (short)LOWORD(lParam);
        int iMouseY = (short)HIWORD(lParam);
        m_ViewArcBall.OnBegin( iMouseX, iMouseY );
    }

    if( uMsg == WM_MOUSEMOVE )
    {
        int iMouseX = (short)LOWORD(lParam);
        int iMouseY = (short)HIWORD(lParam);
        m_WorldArcBall.OnMove( iMouseX, iMouseY );
        m_ViewArcBall.OnMove( iMouseX, iMouseY );
    }

    if( (uMsg == WM_LBUTTONUP && m_nRotateModelButtonMask & MOUSE_LEFT_BUTTON) ||
        (uMsg == WM_MBUTTONUP && m_nRotateModelButtonMask & MOUSE_MIDDLE_BUTTON) ||
        (uMsg == WM_RBUTTONUP && m_nRotateModelButtonMask & MOUSE_RIGHT_BUTTON) )
    {
        m_WorldArcBall.OnEnd();
    }

    if( (uMsg == WM_LBUTTONUP && m_nRotateCameraButtonMask & MOUSE_LEFT_BUTTON) ||
        (uMsg == WM_MBUTTONUP && m_nRotateCameraButtonMask & MOUSE_MIDDLE_BUTTON) ||
        (uMsg == WM_RBUTTONUP && m_nRotateCameraButtonMask & MOUSE_RIGHT_BUTTON) )
    {
        m_ViewArcBall.OnEnd();
    }

    if( uMsg == WM_LBUTTONDOWN ||
        uMsg == WM_LBUTTONDBLCLK ||
        uMsg == WM_MBUTTONDOWN ||
        uMsg == WM_MBUTTONDBLCLK ||
        uMsg == WM_RBUTTONDOWN ||
        uMsg == WM_RBUTTONDBLCLK ||
        uMsg == WM_LBUTTONUP ||
        uMsg == WM_MBUTTONUP ||
        uMsg == WM_RBUTTONUP ||
        uMsg == WM_MOUSEWHEEL ||
        uMsg == WM_MOUSEMOVE )
    {
        m_bDragSinceLastUpdate = true;
    }

    return FALSE;
}




//--------------------------------------------------------------------------------------
// Desc: Returns a view matrix for rendering to a face of a cubemap.
//--------------------------------------------------------------------------------------
D3DXMATRIX DXUTGetCubeMapViewMatrix( DWORD dwFace )
{
    D3DXVECTOR3 vEyePt   = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
    D3DXVECTOR3 vLookDir;
    D3DXVECTOR3 vUpDir;

    switch( dwFace )
    {
        case D3DCUBEMAP_FACE_POSITIVE_X:
            vLookDir = D3DXVECTOR3( 1.0f, 0.0f, 0.0f );
            vUpDir   = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
            break;
        case D3DCUBEMAP_FACE_NEGATIVE_X:
            vLookDir = D3DXVECTOR3(-1.0f, 0.0f, 0.0f );
            vUpDir   = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
            break;
        case D3DCUBEMAP_FACE_POSITIVE_Y:
            vLookDir = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
            vUpDir   = D3DXVECTOR3( 0.0f, 0.0f,-1.0f );
            break;
        case D3DCUBEMAP_FACE_NEGATIVE_Y:
            vLookDir = D3DXVECTOR3( 0.0f,-1.0f, 0.0f );
            vUpDir   = D3DXVECTOR3( 0.0f, 0.0f, 1.0f );
            break;
        case D3DCUBEMAP_FACE_POSITIVE_Z:
            vLookDir = D3DXVECTOR3( 0.0f, 0.0f, 1.0f );
            vUpDir   = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
            break;
        case D3DCUBEMAP_FACE_NEGATIVE_Z:
            vLookDir = D3DXVECTOR3( 0.0f, 0.0f,-1.0f );
            vUpDir   = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
            break;
    }

    // Set the view transform for this cubemap surface
    D3DXMATRIXA16 mView;
    D3DXMatrixLookAtLH( &mView, &vEyePt, &vLookDir, &vUpDir );
    return mView;
}


//--------------------------------------------------------------------------------------

// Complete existing Microsoft DXUT methods; no behavior or layout change.
#include "main.h"
bool CDXUTDialog::GetControlEnabled( int ID )
{
    CDXUTControl* pControl = GetControl( ID );
    if( pControl == NULL )
        return false;

    return pControl->GetEnabled();
}

void CDXUTDialog::SetControlEnabled( int ID, bool bEnabled )
{
    CDXUTControl* pControl = GetControl( ID );
    if( pControl == NULL )
        return;

    pControl->SetEnabled( bEnabled );
}

void CDXUTDialog::OnMouseUp( POINT pt )
{
    s_pControlPressed = NULL;
    m_pControlMouseOver = NULL;
}

void CDXUTDialog::ClearComboBox( int ID )
{
    CDXUTComboBox* pComboBox = GetComboBox( ID );
    if( pComboBox == NULL )
        return;

    pComboBox->RemoveAllItems();
}

void CDXUTDialog::Init( CDXUTDialogResourceManager* pManager )
{
    m_pManager = pManager;
    InitDefaultElements(); // requires a CDXUTDialogResourceManager to store textures/fonts
}

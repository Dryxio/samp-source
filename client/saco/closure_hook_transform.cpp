// Genuine existing hook method; producer establishes view/projection matrix identities.
#include "main.h"
extern D3DXMATRIX matView,matProj,matWorld;
extern IDirect3DDevice9 *pD3DDevice;
HRESULT __stdcall IDirect3DDevice9Hook::SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* mat)
{
	switch (State)
	{
	case D3DTS_PROJECTION:
		matProj = *mat;
		break;
	case D3DTS_VIEW:
		matView = *mat;
		break;
	case D3DTS_WORLD:
		matWorld = *mat;
		break;
	}

	return pD3DDevice->SetTransform(State, mat);
}


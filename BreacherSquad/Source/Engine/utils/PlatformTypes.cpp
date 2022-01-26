#include "dxstdafx.h"
#include "PlatformTypes.h"

OPRESULT UT3DBeginScene(PDEVICE pDevice)
{
	HRESULT hr = pDevice->BeginScene();
	if (FAILED(hr))
	{
		return OPRESULT(K_OP_FAILED, K_SEVERITY_CRITICAL, L"UT3DBeginScene failed! hresult:%x", hr);
	}
	return K_OP_OK;
}

OPRESULT UT3DEndScene(PDEVICE pDevice)
{
	HRESULT hr = pDevice->EndScene();
	if (FAILED(hr))
	{
		return OPRESULT(K_OP_FAILED, K_SEVERITY_CRITICAL, L"UT3DEndScene failed! hresult:%x", hr);
	}
	return K_OP_OK;
}

OPRESULT UT3DClear(PDEVICE pDevice, DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil)
{
	HRESULT hr = pDevice->Clear(Count, pRects, Flags, Color, Z, Stencil);
	if (FAILED(hr))
	{
		return OPRESULT(K_OP_FAILED, K_SEVERITY_CRITICAL, L"UT3DClear failed! hresult:%x", hr);
	}
	return K_OP_OK;
}


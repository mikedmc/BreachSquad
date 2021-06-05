#pragma once
// Abstraction for platform specific types

// Rendering device
#define				PDEVICE				LPDIRECT3DDEVICE9 
#define				PVERTEXBUFFER		LPDIRECT3DVERTEXBUFFER9
#define				PINDEXBUFFER		LPDIRECT3DINDEXBUFFER9
#define				SURFACE_DESC		D3DSURFACE_DESC
#define				PTEXTURE			LPDIRECT3DTEXTURE9
#define				PSURFACE			LPDIRECT3DSURFACE9
#define				PRENDERTOSURFACE	LPD3DXRENDERTOSURFACE
#define				FORMAT3D			D3DFORMAT
#define				PVERTEXSHADER		LPDIRECT3DVERTEXSHADER9
#define				PPIXELSHADER		LPDIRECT3DPIXELSHADER9
#define				PVERTEXDECL			LPDIRECT3DVERTEXDECLARATION9 
#define				TEXTURE_INFO		D3DXIMAGE_INFO
#define				TEXTURE_LOCKRECT	D3DLOCKED_RECT

#define				UT3DCreateTexture	D3DXCreateTexture

// Announces beginning of rendering for specified device
OPRESULT			UT3DBeginScene(PDEVICE pDevice);

// Announces ending of rendering for specified device
OPRESULT			UT3DEndScene(PDEVICE pDevice);

// Use it to clear the scene. Uses the DX9 format.
OPRESULT			UT3DClear(PDEVICE pDevice, DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil);

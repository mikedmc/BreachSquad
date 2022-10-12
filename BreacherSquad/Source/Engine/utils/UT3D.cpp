#include "dxstdafx.h"
#include "UT3D.h"

const DWORD VERT_TL2T::FVF = D3DFVF_XYZ | D3DFVF_TEX2;
const DWORD VERT_TL1T::FVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;
const DWORD VERT_TL1TS::FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;
const DWORD VERT_TL1TC::FVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE;

void UT3D::DrawRectUP_TL1T( PDEVICE pDevice, RectLTRB pos, RectLTRB uv, DWORD color )
{
	VERT_TL1T verts[4];
	verts[0].pos = Vec4( pos.left, pos.top, 0.0f, 1.0f );
	verts[1].pos = Vec4( pos.right, pos.top, 0.0f, 1.0f );
	verts[2].pos = Vec4( pos.left, pos.bottom, 0.0f, 1.0f );
	verts[3].pos = Vec4( pos.right, pos.bottom, 0.0f, 1.0f );
	verts[0].tu = uv.left; verts[1].tu = uv.right; verts[2].tu = uv.left; verts[3].tu = uv.right;
	verts[0].tv = uv.top; verts[1].tv = uv.top; verts[2].tv = uv.bottom; verts[3].tv = uv.bottom;
	verts[0].color = verts[1].color = verts[2].color = verts[3].color = color;

	pDevice->SetFVF( VERT_TL1T::FVF );
	pDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, &verts, sizeof( VERT_TL1T ) );
}

void UT3D::DrawFullscreenVignette( LPDIRECT3DDEVICE9 pDevice, float alpha )
{
	if ( alpha <= 0.0f )
		return;
	/*
		// vignette
		RECT rect;
		SetRect(&rect, g_renderRect.x, g_renderRect.y, g_renderRect.Right(), g_renderRect.Bottom());
		pDevice->SetTexture(0, NULL); //textura aiurea
		pDevice->SetTransform(D3DTS_WORLD, &g_matIdentity);
		DrawRectUP_TL1T(pDevice, rect, Vec2(0, 0), Vec2(0, 0), D3DCOLOR_XXXA(alpha));
		*/
}


void UT3D::DrawLineUP_TL1T( PDEVICE pDevice, Vec2 start, Vec2 end, DWORD color )
{
	VERT_TL1T verts[2];
	verts[0].pos = Vec4( start.x, start.y, 0.0f, 1.0f );
	verts[1].pos = Vec4( end.x, end.y, 0.0f, 1.0f );
	verts[0].tu = 0.0f; verts[1].tu = 0.0f;
	verts[0].tv = 0.0f; verts[1].tv = 0.0f;
	verts[0].color = verts[1].color = color;

	pDevice->SetFVF( VERT_TL1T::FVF );
	pDevice->DrawPrimitiveUP( D3DPT_LINELIST, 1, &verts, sizeof( VERT_TL1T ) );
}


OPRESULT UT3D::SetScissorClip( PDEVICE pDevice, int clipX, int clipY, int clipW, int clipH )
{
	assert( pDevice != nullptr );
	if ( UTApp().g_gfxFlags & K_UT_GFXFLAG_SCISSORTEST )
	{
		RECT rect_colorClip;
		SetRect( &rect_colorClip, clipX, clipY, clipX + clipW, clipY + clipH );
		pDevice->SetRenderState( D3DRS_SCISSORTESTENABLE, TRUE );
		pDevice->SetScissorRect( &rect_colorClip );

		return K_OP_OK;
	}

	return K_OP_FAILED;
}

OPRESULT UT3D::RemoveScissorClip( PDEVICE pDevice )
{
	assert( pDevice != nullptr );
	pDevice->SetRenderState( D3DRS_SCISSORTESTENABLE, FALSE );

	return K_OP_OK;
}


void UT3D::DeviceAdditiveON( PDEVICE pDevice )
{
	pDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
}

void UT3D::DeviceAdditiveOFF( PDEVICE pDevice )
{
	pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
}

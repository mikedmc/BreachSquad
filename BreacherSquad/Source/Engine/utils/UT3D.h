#pragma once

struct VERT_TL1TC
{
	Vec4 pos;
	DWORD color;

	static const DWORD FVF;
};

struct VERT_TL1T
{
	Vec4 pos;
	DWORD color;
	float tu, tv;

	static const DWORD FVF;
};

struct VERT_TL1TS
{
	Vec3 pos;
	DWORD color;
	float tu, tv;

	static const DWORD FVF;
};

struct VERT_TL2T
{
	Vec3 pos;
	float tu, tv;
	float lu, lv;

	static const DWORD FVF;
};


namespace UT3D
{
	void DrawRectUP_TL1T( PDEVICE pDevice, RectLTRB pos, RectLTRB uv, DWORD color = 0xffffffff );
	void DrawLineUP_TL1T( PDEVICE pDevice, Vec2 start, Vec2 end, DWORD color = 0xffffffff );
	void DrawFullscreenVignette( LPDIRECT3DDEVICE9 pDevice, float alpha );

	// Sets clip area on renderer (so you can't paint outside)
	OPRESULT SetScissorClip( PDEVICE pDevice, int clipX, int clipY, int clipW, int clipH );
	// Removes clip from renderer
	OPRESULT RemoveScissorClip( PDEVICE pDevice );
	// additive blending on
	void DeviceAdditiveON( PDEVICE pDevice );
	// additive blending off
	void DeviceAdditiveOFF( PDEVICE pDevice );
}

#pragma once

// structures
#define			Vec2			D3DXVECTOR2
#define			Vec3			D3DXVECTOR3
#define			Vec4			D3DXVECTOR4
#define			Quat			D3DXQUATERNION
#define			Mat				D3DXMATRIX
#define			MatA16			D3DXMATRIXA16

//functions
#define			MUMatIdentity			D3DXMatrixIdentity
#define			MUMatScaling			D3DXMatrixScaling
#define			MUMatTranslation		D3DXMatrixTranslation
#define			MUMatAffine2D			D3DXMatrixAffineTransformation2D
#define			MUOrthoOffCenterLH		D3DXMatrixOrthoOffCenterLH
#define			MUVec2Len				D3DXVec2Length
#define			MUVec2LenSq				D3DXVec2LengthSq

// macros
#define			Vec3ToVec2XY(vec)		Vec2(vec.x, vec.y)
#define			Vec2ToVec3XY0(vec)		Vec3(vec.x, vec.y, 0.0f)

#define			Vec2i					POINTXY_INT
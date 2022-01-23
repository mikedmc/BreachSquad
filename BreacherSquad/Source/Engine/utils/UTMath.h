#pragma once

// structures
#define			Vec2			D3DXVECTOR2
#define			Vec3			D3DXVECTOR3
#define			Vec4			D3DXVECTOR4
#define			Quat			D3DXQUATERNION
#define			Mat				D3DXMATRIX

//functions
#define			MUMatIdentity			D3DXMatrixIdentity
#define			MUMatScaling			D3DXMatrixScaling
#define			MUMatTranslation		D3DXMatrixTranslation
#define			MUMatAffine2D			D3DXMatrixAffineTransformation2D
#define			MUMatOrthoOffCenterLH	D3DXMatrixOrthoOffCenterLH
#define			MUMatRotX				D3DXMatrixRotationX
#define			MUMatRotY				D3DXMatrixRotationY
#define			MUMatRotZ				D3DXMatrixRotationZ

#define			MUVec2Len				D3DXVec2Length
#define			MUVec2LenSq				D3DXVec2LengthSq
#define			MUVec2Dot				D3DXVec2Dot
#define			MUVec2Cross				D3DXVec2CCW
#define			MUVec2Norm				D3DXVec2Normalize
#define			MUVec2TransformCoord	D3DXVec2TransformCoord

#define			MUVec3Len				D3DXVec3Length
#define			MUVec3LenSq				D3DXVec3LengthSq
#define			MUVec3Norm				D3DXVec3Normalize
#define			MUVec3Dot				D3DXVec3Dot
#define			MUVec3Cross				D3DXVec3Cross
#define			MUVec3Norm				D3DXVec3Normalize

// macros
#define			Vec3XY(vec)				Vec2(vec.x, vec.y)
#define			Vec2ToVec3XY0(vec)		Vec3(vec.x, vec.y, 0.0f)

#define			Vec2i					POINTXY_INT

// Is vector almost zero?
bool			MUVec2AlmostZero(Vec2 vec, float fThreshold = 0.00001f);
// Is vector almost zero?
bool			MUVec3AlmostZero(Vec3 vec, float fThreshold = 0.00001f);
// Zero vector
#define			g_Vec3Zero		D3DXVECTOR3(0.0f, 0.0f, 0.0f)
// Zero vector
#define			g_Vec2Zero		D3DXVECTOR2(0.0f, 0.0f)


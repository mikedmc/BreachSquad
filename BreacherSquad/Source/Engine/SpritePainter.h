#pragma once

///-------------------------------------------------------------
///	 Immediate mode sprite painter with transforms 
///  Uses ShaderManager to load the shaders
///-------------------------------------------------------------


#define K_BS_MAX_QUAD_CNT 4000
#define K_BS_MAX_MODECHANGES_CNT 50
//--- begin flags ---
//enables alpha bleding
#define K_BS_ALPHABLENDING 1
//enables alpha testing
#define K_BS_ALPHATEST 2

class CSpritePainter
{
private:
	PDEVICE					m_pDevice;
	PVERTEXSHADER			m_pVShader;										// Vertex shader to use when painting
	Mat						m_matProj;										// Projection matrix set on Begin

	UINT32					m_nVertexCursor;
	_VERTEX_PNCT4T4			*m_verts;										// All verts get written here before drawing them

	PVERTEXBUFFER			m_vb;
	PINDEXBUFFER			m_ib;
	
	UINT32					m_nTexChangesCursor;			
	PTEXTURE				m_texPtrs[K_BS_MAX_MODECHANGES_CNT];
	UINT32					m_nTrisPerTexture[K_BS_MAX_MODECHANGES_CNT];
	UINT32					m_nTrisOffsets[K_BS_MAX_MODECHANGES_CNT];
public:
	CSpritePainter(void);
	~CSpritePainter(void);

	// Call before painting anything
	OPRESULT				Begin(PVERTEXSHADER pVShader, Mat matProj, UINT32 flags = K_BS_ALPHABLENDING | K_BS_ALPHATEST);
	
	// Flushes remaining sprites and ends a scene
	OPRESULT				End();
	
	// Draws a non-transformed sprite
	// \param: pSrcUV - expects the rectangle in texture coordinates that will be drawn in pDestRect
	// \param: pDestRect - expects a rectangle where the srcUV will be painter. 
	// Use together with pPosition if you need rotations as rotations are applied before moving the pDestRect to pPosition 
	// allowing you to specify origin of rotation by defining pDestRect around the origin.
	OPRESULT				Draw(PTEXTURE pTexture, RECTLTRB_F &pSrcUV, RECTLTRB_F &pDestRect, Vec3 *pPosition, DWORD color = 0xffffffff, float fRotationZ = 0.0f, Vec2 vScale = { 1.0f, 1.0f });
	// Draw version with flip flags (a little slower)
	OPRESULT				DrawEx(PTEXTURE pTexture, RECTLTRB_F &pSrcUV, RECTLTRB_F &pDestRect, Vec3 *pPosition, DWORD color = 0xffffffff, float fRotationZ = 0.0f, Vec2 vScale = { 1.0f, 1.0f }, int flipFlags = 0);
	// #TODO: DRAW version with clip rect
	// #TODO: version with scissors for scene wide clip rects (setclip/remove clip)

	// Forces flushing of remaining sprites
	OPRESULT				Flush();


	OPRESULT OnCreateDevice(PDEVICE pDevice, const SURFACE_DESC* pBBDesc = NULL);
	OPRESULT OnResetDevice(PDEVICE pDevice, const SURFACE_DESC* pBBDesc = NULL);
	OPRESULT OnLostDevice();
	OPRESULT OnDestroyDevice();
};



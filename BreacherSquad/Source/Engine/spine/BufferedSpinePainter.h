#pragma once
#include "../BufferedPainter.h"


///----------------------------------------------------
/// Class that paints buffered meshes
/// Keeps track of texture and mode changes
/// Intended for small and fast meshes
///----------------------------------------------------
class CBufferedSpinePainter : private CBufferedPainter {
public:
	enum EBlendMode {
		BLEND_NORMAL = 0,
		BLEND_ADDITIVE = 1,
		BLEND_MULTIPLY,
		BLEND_SCREEN
	};

private:
	// It holds data about each pass (mode change) in the buffer
	struct CPaintPassData {
		EBlendMode			eMode;
		CSpineTex*			pTex;
		int					nMeshIdx;
		UINT				dwSkelUID;			// UID of current pass, usually corresponds to skeleton UID but it can be 0 if we don't need to split meshes by UID
	};

	int						passesCnt;								// total number of "passes" or "mode changes" needed to paint the meshes
	CPaintPassData			arrPasses[K_BP_MAX_MESHES_CNT]{};

public:
	CBufferedSpinePainter();
	~CBufferedSpinePainter();

	using				CBufferedPainter::Init;

	// Adds triangles to the paint buffer (CSpineTex can be replaced with texture pointer)
	// Returns pass index when starting a new mesh or -1 if using already started mesh
	int					BufferMesh(_VERTEX_PNCT4T4 *points, int trisCount, CSpineTex* pTex, EBlendMode eMode, UINT dwUID = 0);

	// Creates the actual vertex buffer with the data
	// Could be called inside Paint, left public for more granular control
	using				CBufferedPainter::BuildBuffers;

	// Clears all buffers
	void				Clear();

	// Paints all buffered meshes
	void				Paint(bool setFVF = true, ETexChannel eChannel = K_TEXCHAN_COLORMAP);
	// Paints one single pass from the passes array, by index
	OPRESULT			PaintPass(int nPassIdx, bool setFVF = true, ETexChannel eChannel = K_TEXCHAN_COLORMAP);

	//make parent framework methods public with "using"
	using				CBufferedPainter::OnCreateDevice;
	using				CBufferedPainter::OnResetDevice;
	using				CBufferedPainter::OnLostDevice;
	using				CBufferedPainter::OnDestroyDevice;
};



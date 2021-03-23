#include "dxstdafx.h"
#include "SpineManager.h"

CSpineManager::CSpineManager()
{
	m_Painter.Init(4000);
}

CSpineManager::~CSpineManager()
{
	Release();
}

CSpineManager::CAtlasContainer* CSpineManager::LoadAtlas(WCHAR * wcsFolder, WCHAR * wcsFile)
{
	WCHAR wcsPath[MAX_PATH];
	StringCchPrintf(wcsPath, MAX_PATH, L"%s%s", wcsFolder, wcsFile);

	// See if atlas is already loaded
	CAtlasContainer* m_atlas = GetAtlasByPath(wcsPath);
	if (m_atlas != null)
	{
		return m_atlas;
	}

	// Not loaded yet, load it now
	int buffSize = 0;
	char* buff = OS_readFileToBuffer(wcsPath, buffSize);
	if (buff == null)
	{
		ErrorBox(K_ERR_WARNING, L"[WARNING]CSpineManager::LoadAtlas - file not found: %s", wcsPath);
		return null;
	}
	//set prefix for the tex loader (avoid path operations from Spine as it doesn't support WCHAR)
	m_TexMgr.SetFilesPrefix(wcsFolder);
	
	//load with empty "dir" param so it doesn't process the folders (Spine is missing WCHAR support)
	m_atlas = new CAtlasContainer();
	//save atlas path to use id as ID
	m_atlas->shFilename.Init(wcsPath);
	m_atlas->pAtlas = new Atlas(buff, buffSize, "", &m_TexMgr);
	//release file buffer
	SAFE_DELETE_ARRAY(buff);
	// check atlas loaded
	if (m_atlas->pAtlas->getPages().size() == 0)
	{
		SAFE_DELETE(m_atlas->pAtlas);
		SAFE_DELETE(m_atlas);
		ErrorBox(K_ERR_WARNING, L"[WARNING]CSpineManager::LoadAtlas - couldn't load atlas: %s", wcsPath);
		return null;
	}

	arrAtlasses.Add(m_atlas);
	return m_atlas;
}

CSpineManager::CAtlasContainer* CSpineManager::GetAtlasByPath(WCHAR * wcsPath)
{
	CStringHash shPath(wcsPath);
	for (int kk = 0; kk < arrAtlasses.GetSize(); kk++)
	{
		if (arrAtlasses[kk]->shFilename == shPath)
			return arrAtlasses[kk];
	}
	return null;
}

CSpineManager::CSkeletonTemplate* CSpineManager::LoadSkeletonTemplateXML(WCHAR* wcsXMLpath)
{
	///--- check if already loaded ---
	CSkeletonTemplate* templ = GetSkeletonTemplateByPath(wcsXMLpath);
	if (templ != null)
		return templ;
	
	// not already loaded so load it now
	pugi::xml_document doc;
	if (!doc.load_file(wcsXMLpath))
	{
		ErrorBox(K_ERR_WARNING, L"[ERROR] CSpineManager: Unable to load skeleton template XML:%s\n", wcsXMLpath);
		return null;
	}

	CStringHash shSkelFile, shAtlasFile;
	pugi::xml_node bnode = doc.root().child(L"SKELETON");
	if (bnode == null)
	{
		ErrorBox(K_ERR_WARNING, L"[WARNING] CSpineManager: Unable to load SKELETON node. XML:%s\n", wcsXMLpath);
		return null;
	}

	if (!bnode.attribute(L"sSkeletonFile").empty())
		shSkelFile.Init(bnode.attribute(L"sSkeletonFile").value());
	else
	{
		ErrorBox(K_ERR_WARNING, L"[WARNING]CSpineManager::LoadSkeletonTemplates: Template without sSkeletonFile! [%s]", templ->shID.text);
		return null;
	}

	if (!bnode.attribute(L"sAtlasFile").empty())
		shAtlasFile.Init(bnode.attribute(L"sAtlasFile").value());
	else
	{
		ErrorBox(K_ERR_WARNING, L"[WARNING]CSpineManager::LoadSkeletonTemplates: Template without sAtlasFile! [%s]", templ->shID.text);
		return null;
	}

	templ = new CSkeletonTemplate();
	// init skeleton ID (file path)
	templ->shID.Init(wcsXMLpath);
	templ->shSkeletonFilename = shSkelFile;
	templ->shAtlasFilename = shAtlasFile;

	///--- try to load atlas and skeleton file ---
	int nErrors = 0;
	//get folder path (no file name)
	WCHAR wcsFullFolder[MAX_PATH];
	StringCchCopy(wcsFullFolder, MAX_PATH, wcsXMLpath);
	int nIdx = (int)wcslen(wcsFullFolder);
	while (--nIdx > 0 && wcsFullFolder[nIdx] != '\\' && wcsFullFolder[nIdx] != '/');
	wcsFullFolder[nIdx + 1] = '\0';

	CAtlasContainer* patlas = LoadAtlas(wcsFullFolder, templ->shAtlasFilename.text);
	if (patlas != null)
	{
		WCHAR wcsSkelPath[MAX_PATH];
		StringCchPrintf(wcsSkelPath, MAX_PATH, L"%s%s", wcsFullFolder, templ->shSkeletonFilename.text);
		if (FAILED(LoadSkeletonData(wcsSkelPath, patlas->pAtlas, templ)))
		{
			nErrors++;
		}
	}
	else
	{
		nErrors++;
	}

	//read special bones names
	pugi::xml_node nodebones = bnode.child(L"SPECIAL_BONES");
	if (nodebones != null)
	{
		for (int kk = 0; kk < K_SD_BONES_CNT; kk++)
		{
			pugi::xml_node nmnode = nodebones.child(ESpineSpecialBoneNames[kk].text);
			if (nmnode != NULL)
			{
				CHAR strVal[MAX_PATH];
				wcstombs(strVal, nmnode.attribute(L"sName").value(), MAX_PATH);
				templ->arrBonesNames[kk] = strVal;
			}
		}
	}

	//read slots names
	pugi::xml_node nodeslots = bnode.child(L"SLOTS");
	if (nodeslots != null)
	{
		for (int kk = 0; kk < K_SD_SLOTS_CNT; kk++)
		{
			pugi::xml_node nmnode = nodeslots.child(ESpineSlotNames[kk].text);
			if (nmnode != NULL)
			{
				CHAR strVal[MAX_PATH];
				wcstombs(strVal, nmnode.attribute(L"sName").value(), MAX_PATH);
				templ->arrSlotsNames[kk] = strVal;
			}
		}
	}

	//read events names
	pugi::xml_node nodeevents = bnode.child(L"EVENTS");
	if (nodeevents != null)
	{
		for (int kk = 0; kk < K_SD_EVENTS_CNT; kk++)
		{
			pugi::xml_node nmnode = nodeevents.child(ESpineEventNames[kk].text);
			if (nmnode != NULL)
			{
				CHAR strVal[MAX_PATH];
				wcstombs(strVal, nmnode.attribute(L"sName").value(), MAX_PATH);
				templ->arrEventsNames[kk] = strVal;
			}
		}
	}

	//errors? exit with null
	if (nErrors > 0)
	{
		SAFE_DELETE(templ->m_skeletonData);
		SAFE_DELETE(templ->m_animationStateData);
		SAFE_DELETE(templ);
		return null;
	}

	//add template to collection
	arrSkeletonTemplates.Add(templ);
	// return pointer to it
	return templ;
}

CSpineManager::CSkeletonTemplate* CSpineManager::GetSkeletonTemplateByPath(WCHAR* wcsXMLpath)
{
	CStringHash shInID(wcsXMLpath);
	for (int kk = 0; kk < arrSkeletonTemplates.GetSize(); kk++)
	{
		if (arrSkeletonTemplates[kk]->shID == shInID)
			return arrSkeletonTemplates[kk];
	}
	return null;
}

HRESULT CSpineManager::LoadSkeletonData(WCHAR * wcsPath, Atlas* pTargetAtlas, CSkeletonTemplate* pDestTemplate)
{
	int buffSize = 0;
	unsigned char* buff = OS_readFileToBufferUC(wcsPath, buffSize);

	assert(pDestTemplate != null);
	assert(pTargetAtlas != null);
	if (buff == null)
	{
		ErrorBox(K_ERR_WARNING, L"[WARNING]CSpineManager::LoadSkeletonData - file not found: %s", wcsPath);
		return E_FAIL;
	}

	SkeletonBinary skbin(pTargetAtlas);
	pDestTemplate->m_skeletonData = skbin.readSkeletonData(buff, buffSize);

	//release file buffer
	SAFE_DELETE_ARRAY(buff);
	// check skel data loaded
	if (pDestTemplate->m_skeletonData == null)
	{
		ErrorBox(K_ERR_WARNING, L"[WARNING]CSpineManager::LoadSkeletonData - couldn't load skeleton data: %s", wcsPath);
		LOG("[ERROR]readSkeletonData error: %s\n", skbin.getError().buffer());
		return E_FAIL;
	}

	//initialize animation state data for this skeleton
	pDestTemplate->m_animationStateData = new AnimationStateData(pDestTemplate->m_skeletonData);
	pDestTemplate->m_animationStateData->setDefaultMix(K_SM_DEFAULT_MIX_DURATION);

	return S_OK;
}

HRESULT CSpineManager::LoadSkeletonDataJSON(WCHAR * wcsPath, Atlas* pTargetAtlas, CSkeletonTemplate* pDestTemplate)
{
	int buffSize = 0;
	char* buff = OS_readFileToBuffer(wcsPath, buffSize);

	assert(pDestTemplate != null);
	assert(pTargetAtlas != null);
	if (buff == null)
	{
		ErrorBox(K_ERR_WARNING, L"[WARNING]CSpineManager::LoadSkeletonDataJSON - file not found: %s", wcsPath);
		return null;
	}

	SkeletonJson skjson(pTargetAtlas);
	pDestTemplate->m_skeletonData = skjson.readSkeletonData(buff);

	//release file buffer
	SAFE_DELETE_ARRAY(buff);
	// check skel data loaded
	if (pDestTemplate->m_skeletonData == null)
	{
		ErrorBox(K_ERR_WARNING, L"[WARNING]CSpineManager::LoadSkeletonDataJSON - couldn't load skeleton data: %s", wcsPath);
		LOG("[ERROR]readSkeletonData error: %s\n", skjson.getError().buffer());
		return E_FAIL;
	}

	//initialize animation state data for this skeleton
	pDestTemplate->m_animationStateData = new AnimationStateData(pDestTemplate->m_skeletonData);
	pDestTemplate->m_animationStateData->setDefaultMix(K_SM_DEFAULT_MIX_DURATION);

	return S_OK;
}

CSpineManager::CSkeletonInstance* CSpineManager::GetSkeletonInstance(CSkeletonTemplate* skelTemplate)
{
	if (skelTemplate == null)
		return null;

	CSkeletonInstance* nSkel = new CSkeletonInstance();

	// Create the skeleton and put it at a random position
	nSkel->skel = new Skeleton(skelTemplate->m_skeletonData);
	// Create the animation state and enqueue a random animation, looping
	nSkel->anim = new AnimationState(skelTemplate->m_animationStateData);

	//find special bones
	for (int kk = 0; kk < K_SD_BONES_CNT; kk++)
	{
		nSkel->arrBones[kk] = null;
		if (!skelTemplate->arrBonesNames[kk].empty())
			nSkel->arrBones[kk] = nSkel->skel->findBone(skelTemplate->arrBonesNames[kk].c_str());
	}
	//find slotsbones
	for (int kk = 0; kk < K_SD_SLOTS_CNT; kk++)
	{
		nSkel->arrSlots[kk] = null;
		if (!skelTemplate->arrSlotsNames[kk].empty())
			nSkel->arrSlots[kk] = nSkel->skel->findSlot(skelTemplate->arrSlotsNames[kk].c_str());
	}

	arrSkeletonInstances.Add(nSkel);
	return nSkel;
}

void CSpineManager::SetListenForEvents(CSkeletonInstance* skelInst, bool bListen)
{
	assert(skelInst!= null);
	if (bListen)
		skelInst->anim->setListener(Spine_AnimEventsCallback);
	else
		skelInst->anim->setListener((AnimationStateListener)null);
}

void CSpineManager::UpdateAnimationStates(float dTime, float fTimeLine)
{
	//update all animation states and apply them 
	for (int kk = 0; kk < arrSkeletonInstances.GetSize(); kk++)
	{																			
		CSkeletonInstance* ski = arrSkeletonInstances[kk];
		if (!ski->bEnabled)
			continue;
		// First update the animation state by the delta time
		ski->anim->update(dTime);

		// Next, apply the state to the skeleton
		ski->anim->apply(*ski->skel);
	}
}

void CSpineManager::Update(float dTime, float fTimeLine)
{
	//update all instances
	for (int kk = 0; kk < arrSkeletonInstances.GetSize(); kk++)
	{
		CSkeletonInstance* ski = arrSkeletonInstances[kk];
		if (!ski->bEnabled)
			continue;
		// Calculate world transforms for rendering
		ski->skel->updateWorldTransform();
	}

	//batch for rendering
	m_Painter.Clear();

	for (int kk = 0; kk < arrSkeletonInstances.GetSize(); kk++)
	{
		CSkeletonInstance* ski = arrSkeletonInstances[kk];
		
		if (!ski->bVisible)
		{
			ski->arrPassesIdx.Clear();
			continue;
		}

		BatchSkeleton(ski);
	}

	m_Painter.BuildBuffers();
}

void CSpineManager::Paint(ETexChannel eChannel)
{
	m_Painter.Paint(true, eChannel);
}

void CSpineManager::Paint(CSkeletonInstance* ski, ETexChannel eChannel /*= K_TEXCHAN_COLORMAP*/)
{
	if (!ski->bVisible)
		return;

	for (int kk = 0; kk < ski->arrPassesIdx.nCount; kk++)
	{
		m_Painter.PaintPass(ski->arrPassesIdx.m_pData[kk], true, eChannel);
	}
}

///--- scratch disk for building mesh triangles ---
#define	K_SPINE_SCRATCHDISKVERTS 900
static _VERTEX_PNCT4T4 arrScratchBuff[K_SPINE_SCRATCHDISKVERTS];
static int arrScratchCnt = 0;

inline void ScratchDisk_Reset() 
{
	arrScratchCnt = 0;
}

void ScratchDisk_AddVert(Vec2 vPos, float tU, float tV, DWORD dwColor)
{
	arrScratchBuff[arrScratchCnt].pos = D3DXVECTOR3(vPos.x, vPos.y, 0.0f);
	arrScratchBuff[arrScratchCnt].tex1.x = tU;
	arrScratchBuff[arrScratchCnt].tex1.y = tV;
	arrScratchBuff[arrScratchCnt].color = dwColor;

	arrScratchCnt++;
}

void CSpineManager::BatchSkeleton(CSkeletonInstance* ski)
{
	_ASSERT(ski != nullptr);
	Skeleton* skel = ski->skel;
	unsigned short quadIndices[] = { 0, 1, 2, 2, 3, 0 };
	spine::Vector<Vec2> arrVertsPos;

	ski->arrPassesIdx.Clear();
	// For each slot in the draw order array of the skeleton
	for (size_t i = 0, n = skel->getSlots().size(); i < n; ++i) 
	{
		Slot* slot = skel->getDrawOrder()[i];

		// Fetch the currently active attachment, continue
		// with the next slot in the draw order if no
		// attachment is active on the slot
		Attachment* attachment = slot->getAttachment();
		if (!attachment) 
			continue;

		// Fetch the blend mode from the slot and
		// translate it to the engine blend mode
		CBufferedSpinePainter::EBlendMode engineBlendMode;
		switch (slot->getData().getBlendMode()) 
		{
		case BlendMode_Normal:
			engineBlendMode = CBufferedSpinePainter::BLEND_NORMAL;
			break;
		case BlendMode_Additive:
			engineBlendMode = CBufferedSpinePainter::BLEND_ADDITIVE;
			break;
		case BlendMode_Multiply:
			engineBlendMode = CBufferedSpinePainter::BLEND_MULTIPLY;
			break;
		case BlendMode_Screen:
			engineBlendMode = CBufferedSpinePainter::BLEND_SCREEN;
			break;
		default:
			engineBlendMode = CBufferedSpinePainter::BLEND_NORMAL;
			break;
		}

		// Calculate the tinting color based on the skeleton's color
		// and the slot's color. Each color channel is given in the
		// range [0-1], you may have to multiply by 255 and cast to
		// and int if your engine uses integer ranges for color channels.
		Color skeletonColor = skel->getColor();
		Color slotColor = slot->getColor();
		Color tint = Color(skeletonColor.r * slotColor.r, skeletonColor.g * slotColor.g, skeletonColor.b * slotColor.b, skeletonColor.a * slotColor.a);
		DWORD dwSlotColor = D3DCOLOR_COLORVALUE(tint.r, tint.g, tint.b, tint.a);

		// Fill the vertices array, indices, and texture depending on the type of attachment
		CSpineTex* texture = NULL;
		if (attachment->getRTTI().isExactly(RegionAttachment::rtti)) 
		{
			// Cast to an spRegionAttachment so we can get the rendererObject
			// and compute the world vertices
			RegionAttachment* regionAttachment = (RegionAttachment*)attachment;

			// Our engine specific Texture is stored in the AtlasRegion which was
			// assigned to the attachment on load. It represents the texture atlas
			// page that contains the image the region attachment is mapped to.
			texture = (CSpineTex*)((AtlasRegion*)regionAttachment->getRendererObject())->page->getRendererObject();

			// Ensure there is enough room for vertices
			arrVertsPos.setSize(4, Vec2(0.0f, 0.0f));

			// Computed the world vertices positions for the 4 vertices that make up
			// the rectangular region attachment. This assumes the world transform of the
			// bone to which the slot (and hence attachment) is attached has been calculated
			// before rendering via Skeleton::updateWorldTransform(). The vertex positions
			// will be written directoy into the vertices array, with a stride of sizeof(Vertex)
			regionAttachment->computeWorldVertices(slot->getBone(), &arrVertsPos.buffer()->x, 0, 2);
			// Get UVs now
			spine::Vector<float>& arrUVs = regionAttachment->getUVs();
			
			ScratchDisk_Reset();
			int nIndicesCnt = 6; //2 triangles
			for (int kk = 0; kk < nIndicesCnt; kk++)
			{
				int nIdx = quadIndices[kk];
				ScratchDisk_AddVert(arrVertsPos[nIdx], arrUVs[nIdx * 2 + 0], arrUVs[nIdx * 2 + 1], dwSlotColor);
			}
			// BufferMesh only returns positive pass index when starting a new mesh so we add it to the passes array necessary for painting the skel
			int npassidx = m_Painter.BufferMesh(arrScratchBuff, arrScratchCnt / 3, texture, engineBlendMode, ski->UID);
			if (npassidx >= 0)
				ski->arrPassesIdx.Add(npassidx);
		}
		else if (attachment->getRTTI().isExactly(MeshAttachment::rtti)) 
		{
			// Cast to an MeshAttachment so we can get the rendererObject
			// and compute the world vertices
			MeshAttachment* mesh = (MeshAttachment*)attachment;

			// Ensure there is enough room for vertices
			size_t numVertices = mesh->getWorldVerticesLength() / 2;
			arrVertsPos.setSize(numVertices, Vec2(0.0f, 0.0f));

			// Our engine specific Texture is stored in the AtlasRegion which was
			// assigned to the attachment on load. It represents the texture atlas
			// page that contains the image the region attachment is mapped to.
			texture = (CSpineTex*)((AtlasRegion*)mesh->getRendererObject())->page->getRendererObject();

			// The vertex positions will be written directly into the vertices array, count in "floats", stride in "floats"
			// The vertex structure (Vec2) is 2 floats wide
			mesh->computeWorldVertices(*slot, 0, numVertices * 2, &arrVertsPos.buffer()->x, 0, 2);

			//get UVs
			spine::Vector<float>& arrUVs = mesh->getUVs();

			//access index buffer
			spine::Vector<unsigned short>& arrIndices = mesh->getTriangles();
			int numIndices = arrIndices.size();

			//#TODO: could be faster if we had an indexed painter

			ScratchDisk_Reset();
			// Copy color and UVs to the vertices
			for (int j = 0; j < numIndices; j++) 
			{
				int nIdx = arrIndices[j];
				ScratchDisk_AddVert(arrVertsPos[nIdx], arrUVs[nIdx * 2 + 0], arrUVs[nIdx * 2 + 1], dwSlotColor);
			}
			// BufferMesh only returns positive pass index when starting a new mesh so we add it to the passes array necessary for painting the skel
			int npassidx = m_Painter.BufferMesh(arrScratchBuff, arrScratchCnt / 3, texture, engineBlendMode, ski->UID);
			if (npassidx >= 0)
				ski->arrPassesIdx.Add(npassidx);
		}
	}
}

void CSpineManager::Release()
{
	//release skel templates
	for (int kk = 0; kk < arrSkeletonTemplates.GetSize(); kk++)
	{
		SAFE_DELETE(arrSkeletonTemplates[kk]->m_animationStateData);
		SAFE_DELETE(arrSkeletonTemplates[kk]->m_skeletonData);

		SAFE_DELETE(arrSkeletonTemplates[kk]);
	}
	arrSkeletonTemplates.RemoveAll();

	//release atlasses
	for (int kk = 0; kk < arrAtlasses.GetSize(); kk++)
	{
		SAFE_DELETE(arrAtlasses[kk]->pAtlas);
		
		SAFE_DELETE(arrAtlasses[kk]);
	}
	arrAtlasses.RemoveAll();

	//release skel instances
	for (int kk = 0; kk < arrSkeletonInstances.GetSize(); kk++)
	{
		SAFE_DELETE(arrSkeletonInstances[kk]->anim);
		SAFE_DELETE(arrSkeletonInstances[kk]->skel);

		SAFE_DELETE(arrSkeletonInstances[kk]);
	}
	arrSkeletonInstances.RemoveAll();
}

//-=-=-= SYSTEM / FRAMEWORK =-=-=-
HRESULT CSpineManager::OnCreateDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr = S_OK;
	m_pDevice = pd3dDevice;

	m_TexMgr.OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc);
	m_Painter.OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc);

	return hr;
}

HRESULT CSpineManager::OnResetDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc)
{
	m_pDevice = pd3dDevice;

	m_TexMgr.OnResetDevice(pd3dDevice, pBackBufferSurfaceDesc);
	m_Painter.OnResetDevice(pd3dDevice, pBackBufferSurfaceDesc);

	return S_OK;
}

HRESULT CSpineManager::OnLostDevice(void)
{
	HRESULT hr = S_OK;
	m_pDevice = NULL;

	m_TexMgr.OnLostDevice();
	m_Painter.OnLostDevice();

	return hr;
}

HRESULT CSpineManager::OnDestroyDevice(void)
{
	HRESULT hr = S_OK;
	m_pDevice = NULL;

	m_TexMgr.OnDestroyDevice();
	m_Painter.OnDestroyDevice();

	return hr;
}

CSpineManager::CSkeletonTemplate::CSkeletonTemplate() :
	m_skeletonData(null), m_animationStateData(null)
{
	for (int kk = 0; kk < K_SD_BONES_CNT; kk++)
		arrBonesNames[kk].clear();
	for (int kk = 0; kk < K_SD_EVENTS_CNT; kk++)
		arrEventsNames[kk].clear();
	for (int kk = 0; kk < K_SD_SLOTS_CNT; kk++)
		arrSlotsNames[kk].clear();
}

spine::Animation* CSpineManager::CSkeletonTemplate::GetAnimation(const char * strAnimName)
{
	//avoid assertion failed on empty strings and null
	if ((strAnimName == null) || (strAnimName[0] == 0))
	{
		ErrorBox(K_ERR_WARNING, L"[WARNING] CSpineManager::CSkeletonTemplate::GetAnimation called with empty animation name!");
		return NULL;
	}
	return m_skeletonData->findAnimation(strAnimName);
}

CSpineManager::CSkeletonInstance::CSkeletonInstance() :
	skel(null), anim(null), 
	bVisible(true), bEnabled(true),
	UID(0) //default id = not used
{
	arrPassesIdx.Clear();
	memset(arrBones, null, sizeof(Bone*));
	memset(arrSlots, null, sizeof(Slot*));
}

void CSpineManager::CSkeletonInstance::SetEnabled(bool bIsVisible, bool bIsUpdating)
{
	bVisible = bIsVisible;
	bEnabled = bIsUpdating;
}

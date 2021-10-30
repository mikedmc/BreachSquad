#include "dxstdafx.h"
#include "SpineAnimComp.h"

CSpineAnimComponent::CSpineAnimComponent()
{
	nAnimSet = 0;
	for (int kk = 0; kk < K_SACOMP_MAX_ANIM_TRACKS; kk++)
	{
		eLastAnim[kk] = K_SD_ANIM_EMPTY;
	}
}

CSpineAnimComponent::~CSpineAnimComponent()
{
	// remove used skeleton instance
	g_spineMgr.RemoveSkeletonInstance(pSkeleton);
	pSkeleton = nullptr;
}

void CSpineAnimComponent::Update(CActor& act, float dTime)
{
	//#TODO: ar trebui sa tina local aim vector sa nu te oblige sa il setezi pe fiecare frame din actor

	if ((pSkeleton != null) && (pSkeleton->skel != null))
	{
		pSkeleton->skel->setPosition(act.pos.xy_proj.x, act.pos.xy_proj.y);
		if (act.bAnimFlipX)
			this->pSkeleton->skel->setScaleX(-1.0f);
		else
			this->pSkeleton->skel->setScaleX(1.0f); 
	}
}

void CSpineAnimComponent::Paint(CActor& act, ETexChannel eChannel /*= K_TEXCHAN_COLORMAP*/)
{
	g_spineMgr.Paint(pSkeleton, eChannel);
}

void CSpineAnimComponent::UpdateTransform(CActor& act)
{
	//set skeleton data
	if ((pSkeleton != null) && (pSkeleton->skel != null))
	{
		pSkeleton->skel->setPosition(act.pos.xy_proj.x, act.pos.xy_proj.y);
		if (act.bAnimFlipX)
			this->pSkeleton->skel->setScaleX(-1.0f);
		else
			this->pSkeleton->skel->setScaleX(1.0f);
	}
}

void CSpineAnimComponent::SaveAnimationPointers(CActor& act)
{
	int nAnimsChanged = 0;
	for (int anm = 0; anm < K_SD_ANIMS_CNT; anm++)
	{
		for (int kk = 0; kk < K_SACOMP_ANIM_MAX_SETS; kk++)
		{
			this->arrAnimsPtr[anm].bLooping = act.actTemplate.arrAnims[anm].bLooping;
			// check for changes in the template
			if ((act.actTemplate.arrAnims[anm].animNamesA[kk].IsEmpty()) ||
				(arrAnimsPtr[anm].animNamesA[kk] != act.actTemplate.arrAnims[anm].animNamesA[kk]))
			{
				arrAnimsPtr[anm].animNamesA[kk] = act.actTemplate.arrAnims[anm].animNamesA[kk];
				// erase outdated anim pointers
				arrAnimsPtr[anm].pAnim[kk] = null;
			}

			if ((arrAnimsPtr[anm].pAnim[kk] == null) && (arrAnimsPtr[anm].animNamesA[kk].IsSet()))
			{
				// set new anim pointer
				spine::Animation* anim = pSkelTemplate->GetAnimation(this->arrAnimsPtr[anm].animNamesA[kk].text);
				if (anim != null)
				{
					this->arrAnimsPtr[anm].pAnim[kk] = anim;
					nAnimsChanged++;
				}
				else
				{
					LOG_DBG("[WARNING] CActor::UpdateAnimationPointers: Couldn't find animation [%s]", this->arrAnimsPtr[anm].animNamesA[kk].text);
				}
			}
		}
	}

	LOG_DBG(L"CActor::UpdateAnimationPointers: Updates %d animations", nAnimsChanged);

}

bool CSpineAnimComponent::SetSkin(const char * strSkinName)
{
	if ((pSkeleton == null) || (pSkeleton->skel == null) || (pSkelTemplate == null) || (strSkinName[0] == 0))
	{
		ErrorBox(K_ERR_WARNING, L"CActor::Spine_SetSkin: Empty skin name or skeleton data not loaded!");
		return false;
	}

	spine::Skin* pSkin = pSkelTemplate->m_skeletonData->findSkin(strSkinName);
	if (pSkin == null)
	{
		ErrorBox(K_ERR_WARNING, L"CActor::Spine_SetSkin: Skin not found!");
		return false;
	}

	pSkeleton->skel->setSkin(pSkin);
	LOG_DBG("Spine_SetSkin: setting skin: %s", strSkinName);

	return true;
}

bool CSpineAnimComponent::HasAnimation(ESpineAnim nAnimType, int nSet)
{
	if ((nSet < 0) || (nSet >= K_SACOMP_ANIM_MAX_SETS))
		return false;

	bool bHasIt = (this->arrAnimsPtr[nAnimType].pAnim[nSet] != null);
	return bHasIt;
}

OPRESULT CSpineAnimComponent::InitFromFile(CActor& act, WCHAR * Path)
{
	this->pSkelTemplate = g_spineMgr.LoadSkeletonTemplateXML(Path);
	if (pSkelTemplate == nullptr)
	{
		return OPRESULT(K_OP_FAILED, K_SEVERITY_WARNING, L"SpineAnimComponent::InitFromFile: Skeleton Template not found: %s", Path);
	}
	// save parent name
	shParentName = act.actTemplate.shID;

	float fScale = 1.0f;
	// Create skeleton instance
	pSkeleton = g_spineMgr.GetSkeletonInstance(pSkelTemplate);
	// set skeleton ID too so we get them batched in separate meshes:
	pSkeleton->UID = act.UID;
	pSkeleton->skel->setScaleY(-1.0f * fScale);
	pSkeleton->skel->setScaleX(fScale);

	// Set skin
	if (act.actTemplate.shSkinName.IsSet())
	{
		SetSkin(act.actTemplate.shSkinName.text);
	}

	// set pointers to spine animations for fast access
	SaveAnimationPointers(act);
	SetAnimSet(0);

	return K_OP_OK;
}

void CSpineAnimComponent::SetAnimSet(int newAnimSet)
{
	if (newAnimSet != nAnimSet)
	{
		nAnimSet = newAnimSet;
	}
}

spine::TrackEntry* CSpineAnimComponent::SetAnimOnce(int nTrack, ESpineAnim eAnim)
{
	assert((nTrack >= 0) || (nTrack < K_SACOMP_MAX_ANIM_TRACKS));
	assert((eAnim >= K_SD_ANIM_EMPTY) && (eAnim < K_SD_ANIMS_CNT));

	spine::TrackEntry* trk = null;
	if (eAnim != eLastAnim[nTrack])
	{
		eLastAnim[nTrack] = eAnim;

		if (eAnim <= K_SD_ANIM_EMPTY)
		{
			LOG_DBG(L"ACTOR[%s].SetAnimOnce: Clearing track %d.", shParentName.text, nTrack);
			pSkeleton->anim->setEmptyAnimation(nTrack, K_SM_DEFAULT_MIX_DURATION);
		}
		else
		{
			if (HasAnimation(eAnim))
			{
				LOG_DBG(L"ACTOR[%s]:SetAnimOnce:", shParentName.text);
				//LOG_DBG("track:%d anim:%s", nTrack, actTemplate.arrAnims[(int)eAnim].animNamesA[this->nAnimSet].text);
				trk = pSkeleton->anim->setAnimation(nTrack, arrAnimsPtr[(int)eAnim].pAnim[nAnimSet], arrAnimsPtr[(int)eAnim].bLooping);
			}
			else
			{
				LOG_DBG(L"ACTOR[%s]:SetAnimOnce: ANIM NOT FOUND: %s", shParentName.text, ESpineAnimNames[(int)eAnim]);
				//LOG_DBG("ANIM NOT FOUND! [%s]", actTemplate.arrAnims[(int)eAnim].animNamesA[this->nAnimSet].text);
			}
		}
	}

	return trk;

}

spine::TrackEntry* CSpineAnimComponent::AddAnimOnce(int nTrack, ESpineAnim eAnim, float fMixTime /*= K_SM_DEFAULT_MIX_DURATION*/, float fDelay /*= 0.0f*/)
{
	assert((nTrack >= 0) || (nTrack < K_SACOMP_MAX_ANIM_TRACKS));
	assert((eAnim >= K_SD_ANIM_EMPTY) && (eAnim < K_SD_ANIMS_CNT));

	spine::TrackEntry* trk = null;
	if (eAnim != eLastAnim[nTrack])
	{
		eLastAnim[nTrack] = eAnim;

		// set the anim on empty => clear the track
		if (eAnim <= K_SD_ANIM_EMPTY)
		{
			LOG_DBG(L"[WARNING] ACTOR[%s].AddAnimOnce: Empty anim on track %d.", shParentName.text, nTrack);
			pSkeleton->anim->addEmptyAnimation(nTrack, fMixTime, 0.0f);
		}
		else
		{
			if (HasAnimation(eAnim))
			{
				LOG_DBG(L"ACTOR[%s]:AddAnimOnce:", shParentName.text);
				//LOG_DBG("track:%d anim:%s fMixtime:%.2f fDelay:%.2f", nTrack, actTemplate.arrAnims[(int)eAnim].animNamesA[this->nAnimSet].text, fMixTime, fDelay);
				trk = pSkeleton->anim->addAnimation(nTrack, arrAnimsPtr[(int)eAnim].pAnim[nAnimSet], arrAnimsPtr[(int)eAnim].bLooping, fDelay);
				//set custom mix time
				trk->setMixTime(fMixTime);
			}
			else
			{
				LOG_DBG(L"ACTOR[%s]:AddAnimOnce: ANIM NOT FOUND: %s", shParentName.text, ESpineAnimNames[(int)eAnim]);
				//LOG_DBG("ANIM NOT FOUND! [%s]", actTemplate.arrAnims[eAnim].animNamesA[this->nAnimSet].text);
			}
		}
	}

	return trk;
}

void CSpineAnimComponent::SetAimVecLocal(Vec2 vLocalAim)
{
	//#TODO: vezi transformul asta ca sa muti din world space in skeleton space:
	//Vector2 ledgePointLocalSpace = skeletonAnimation.transform.InverseTransformPoint(ledgePoint); // your ledgePoint
	spine::Bone* b_aim = pSkeleton->arrBones[K_SD_BONE_AIM_IK];
	if (b_aim)
	{
		b_aim->setX(vLocalAim.x);
		b_aim->setY(vLocalAim.y);
	}
}

bool CSpineAnimComponent::GetGunPosWorld(Vec2 &retVec)
{
	spine::Bone* bone = pSkeleton->arrBones[K_SD_BONE_GUN_MOUNT];
	if (bone)
	{
		retVec = { bone->getWorldX(), bone->getWorldY() };
		return true;
	}
	return false;
}

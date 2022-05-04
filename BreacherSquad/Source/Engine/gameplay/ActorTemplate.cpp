#include "dxstdafx.h"
#include "ActorTemplate.h"


CActorTemplate::CActorTemplate() :
	arrSkinsCnt( 0 ),
	//generic params
	fLife( K_NOT_SET ), fArmor( K_NOT_SET ), fSpeedMove( K_NOT_SET ),
	actorClass( K_ACT_CLASS_NOT_SET ),
	//more important values
	eMaterial( K_LVL_MATERIAL_UNKNOWN ), eCaps( K_ACT_CAPS_NONE ),
	AItemplate( nullptr ), fMass( 100.0f ),
	heightZ( 32.0f ), heartZ ( 24.0f )
{
	//reset anim IDs
	for (int kk = 0; kk < K_ACT_ANIMS_CNT; kk++)
	{
		arrAnims[kk].Reset();
	}
	//reset verses
	for (int kk = 0; kk < K_LVL_ACT_VERSES_COUNT; kk++)
	{
		for (int jj = 0; jj < K_ACT_VERSES_MAX_SETS; jj++)
		{
			soundIDs[kk][jj] = -1; //default value for missing verse
		}
	}
	// set default aabb
	bbox.Set(-6.0f, -6.0f, 6.0f, 6.0f);
}

void CActorTemplate::FillDefaultValuesIfNotSet()
{
	if (eMaterial == K_LVL_MATERIAL_UNKNOWN) { eMaterial = K_LVL_MATERIAL_FLESH; }

	if (fLife == K_NOT_SET) { fLife = 100.0f; }
	if (fArmor == K_NOT_SET) { fArmor = 0.0f; }
	if (fSpeedMove == K_NOT_SET) { fSpeedMove = 64.0f; }
}

bool CActorTemplate::OverwriteAnimsFromTemplate(CActorTemplate* pTemplate, bool bEraseOldAnimations /*= false*/)
{
	if (pTemplate == NULL)
	{
		return false;
	}

	bool bChanged = false;
	for (int kk = 0; kk < K_ACT_ANIMS_CNT; kk++)
	{
		if (bEraseOldAnimations)
		{
			arrAnims[kk].Reset();
			bChanged = true;
		}

		for (int jj = 0; jj < K_ACT_ANIM_MAX_SETS; jj++)
		{
			for ( int ang = 0; ang < EDIR6S_CNT; ang++ )
			{
				//overwrite if existing
				if ( pTemplate->arrAnims[ kk ].animNamesA[ jj ][ ang ].IsSet() )
				{
					arrAnims[ kk ].animNamesA[ jj ][ ang ] = pTemplate->arrAnims[ kk ].animNamesA[ jj ][ ang ];
					bChanged = true;
				}
			}
		}
	}

	return bChanged;
}

void CActorTemplate::OverwriteGenericDataFromTemplate(CActorTemplate* pTemplate)
{
	if (pTemplate == null)
	{
		return;
	}
	//#TEMP: cand am templates aproape gata
	/*
	if (pTemplate->actorClass != K_ACT_CLASS_NOT_SET) { actorClass = pTemplate->actorClass; }
	if (pTemplate->foeClassFilter1 != K_ACT_CLASS_ANY) { foeClassFilter1 = pTemplate->foeClassFilter1; }
	if (pTemplate->foeClassFilter2 != K_ACT_CLASS_ANY) { foeClassFilter2 = pTemplate->foeClassFilter2; }
	if (pTemplate->eMaterial != K_LVL_MATERIAL_UNKNOWN) { eMaterial = pTemplate->eMaterial; }
	//add new caps (should XOR if need to disable them)
	if(pTemplate->eCaps != 0) eCaps = pTemplate->eCaps;

	if (pTemplate->nHUDportraitFrameIdx != K_NOT_SET) { nHUDportraitFrameIdx = pTemplate->nHUDportraitFrameIdx; }
	if (pTemplate->fLife != K_NOT_SET) { fLife = pTemplate->fLife; }
	if (pTemplate->fArmor != K_NOT_SET) { fArmor = pTemplate->fArmor; }
	if (pTemplate->nArmorDir != K_NOT_SET) { nArmorDir = pTemplate->nArmorDir; }
	if (pTemplate->nArmorRating != K_NOT_SET) { nArmorRating = pTemplate->nArmorRating; }
	if (pTemplate->jumpSpeed != K_NOT_SET) { jumpSpeed = pTemplate->jumpSpeed; }
	if (pTemplate->moveMaxSpeed != K_NOT_SET) { moveMaxSpeed = pTemplate->moveMaxSpeed; }
	if (pTemplate->moveMinSpeed != K_NOT_SET) { moveMinSpeed = pTemplate->moveMinSpeed; }
	if (pTemplate->moveBackSpeed != K_NOT_SET) { moveBackSpeed = pTemplate->moveBackSpeed; }
	if (pTemplate->fDexterity != K_NOT_SET) { fDexterity = pTemplate->fDexterity; }
	if (pTemplate->fRecoilModifier != K_NOT_SET) { fRecoilModifier = pTemplate->fRecoilModifier; }
	if (pTemplate->climbSpeed != K_NOT_SET) { climbSpeed = pTemplate->climbSpeed; }
	if (pTemplate->distSee != K_NOT_SET) { distSee = pTemplate->distSee; }
	if (pTemplate->distHear != K_NOT_SET) { distHear = pTemplate->distHear; }
	if (pTemplate->distAttackMax != K_NOT_SET) { distAttackMax = pTemplate->distAttackMax; }
	if (pTemplate->distAttackMin != K_NOT_SET) { distAttackMin = pTemplate->distAttackMin; }
	if (pTemplate->fMass != K_NOT_SET) { fMass = pTemplate->fMass; }
	if (pTemplate->fFOVpercent != K_NOT_SET) { fFOVpercent = pTemplate->fFOVpercent; }
	if (pTemplate->fArmorMPP != K_NOT_SET) { fArmorMPP = pTemplate->fArmorMPP; }
	//scripts
	if (!pTemplate->shScript_OnSpawn.IsEmpty()) { shScript_OnSpawn = pTemplate->shScript_OnSpawn; }
	*/
}

void CActorTemplate::AddGenericDataFromTemplate(CActorTemplate* pTemplate)
{
	if (pTemplate == null)
	{
		return;
	}

	//#TEMP: cand am templates aproape gata
	/*
	if (pTemplate->actorClass != K_ACT_CLASS_NOT_SET) { actorClass = pTemplate->actorClass; }
	if (pTemplate->foeClassFilter1 != K_ACT_CLASS_ANY) { foeClassFilter1 = pTemplate->foeClassFilter1; }
	if (pTemplate->foeClassFilter2 != K_ACT_CLASS_ANY) { foeClassFilter2 = pTemplate->foeClassFilter2; }
	if (pTemplate->eMaterial != K_LVL_MATERIAL_UNKNOWN) { eMaterial = pTemplate->eMaterial; }
	//XOR in new caps:
	eCaps ^= pTemplate->eCaps;

	if (pTemplate->nHUDportraitFrameIdx != K_NOT_SET) { nHUDportraitFrameIdx = pTemplate->nHUDportraitFrameIdx; }
	if (pTemplate->fLife != K_NOT_SET) { fLife += pTemplate->fLife; }
	if (pTemplate->fArmor != K_NOT_SET) { fArmor += pTemplate->fArmor; }
	if (pTemplate->nArmorDir != K_NOT_SET) { nArmorDir = pTemplate->nArmorDir; }  //exception: armor dir can't be added so it gets written over
	if (pTemplate->nArmorRating != K_NOT_SET) { nArmorRating += pTemplate->nArmorRating; }
	if (pTemplate->jumpSpeed != K_NOT_SET) { jumpSpeed += pTemplate->jumpSpeed; }
	if (pTemplate->moveMaxSpeed != K_NOT_SET) { moveMaxSpeed += pTemplate->moveMaxSpeed; }
	if (pTemplate->moveMinSpeed != K_NOT_SET) { moveMinSpeed += pTemplate->moveMinSpeed; }
	if (pTemplate->moveBackSpeed != K_NOT_SET) { moveBackSpeed += pTemplate->moveBackSpeed; }
	if (pTemplate->fDexterity != K_NOT_SET) { fDexterity += pTemplate->fDexterity; }
	if (pTemplate->fRecoilModifier != K_NOT_SET) { fRecoilModifier += pTemplate->fRecoilModifier; }
	if (pTemplate->climbSpeed != K_NOT_SET) { climbSpeed += pTemplate->climbSpeed; }
	if (pTemplate->distSee != K_NOT_SET) { distSee += pTemplate->distSee; }
	if (pTemplate->distHear != K_NOT_SET) { distHear += pTemplate->distHear; }
	if (pTemplate->distAttackMax != K_NOT_SET) { distAttackMax += pTemplate->distAttackMax; }
	if (pTemplate->distAttackMin != K_NOT_SET) { distAttackMin += pTemplate->distAttackMin; }
	if (pTemplate->fMass != K_NOT_SET) { fMass += pTemplate->fMass; }
	if (pTemplate->fFOVpercent != K_NOT_SET) { fFOVpercent += pTemplate->fFOVpercent; }
	if (pTemplate->fArmorMPP != K_NOT_SET) { fArmorMPP += pTemplate->fArmorMPP; }
	//weapon scripts
	if (!pTemplate->shScript_OnSpawn.IsEmpty()) { shScript_OnSpawn = pTemplate->shScript_OnSpawn; }

	//normalize some values
	if (fLife < 0.0f) fLife = 0.0f;
	if (fArmor < 0.0f) fArmor = 0.0f;
	CLAMP(fArmorMPP, 0.0f, 1.0f);
	if (nArmorRating < 0) nArmorRating = 0;
	if (fDexterity < 0.0f) fDexterity = 0.0f;
	if (fRecoilModifier < 0.0f) fRecoilModifier = 0.0f;
	if (fMass < 0.1f) fMass = 0.1f;
	if (moveMinSpeed < 0.0f) moveMinSpeed = 0.0f;
	if (moveMaxSpeed < 0.0f) moveMaxSpeed = 0.0f;
	*/
}


UINT32 CActorTemplate::GetSkinMaskValue( WCHAR* skinName )
{
	for ( int kk = 0; kk < K_ACT_SKINS_MAX_SETS; kk++ )
	{
		if ( arrSkins[ kk ].name.IsEqual( skinName ) )
			return arrSkins[kk].layersVisMask;
	}
	return 0xffffffff;
}


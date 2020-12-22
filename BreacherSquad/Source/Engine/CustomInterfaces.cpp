#include "dxstdafx.h"

#pragma region IGM_INTERFACE

// animation names to paint the controller buttons from (PC Default is XBox)
int K_CI_ANIMIDX_BUTICONS_UP = ANM_CONTROLS_SPR_CTRLR_XBOX_UP;
int K_CI_ANIMIDX_BUTICONS_DN = ANM_CONTROLS_SPR_CTRLR_XBOX_DOWN;
// PS
//int K_CI_ANIMIDX_BUTICONS_UP = ANM_CONTROLS_SPR_CTRLR_PS_UP;
//int K_CI_ANIMIDX_BUTICONS_DN = ANM_CONTROLS_SPR_CTRLR_PS_DOWN;
// NINTENDO
//int K_CI_ANIMIDX_BUTICONS_UP = ANM_CONTROLS_SPR_CTRLR_NINTENDO_UP;
//int K_CI_ANIMIDX_BUTICONS_DN = ANM_CONTROLS_SPR_CTRLR_NINTENDO_DOWN;
// NINTENDO - separate controllers
//int K_CI_ANIMIDX_BUTICONS_UP = ANM_CONTROLS_SPR_CTRLR_NINTENDO_UP2;
//int K_CI_ANIMIDX_BUTICONS_DN = ANM_CONTROLS_SPR_CTRLR_NINTENDO_DOWN2;


void CCustomInterfaceIGM::UpdateInterfaceForPlayer(int nPlayerOrdinal, D3DXVECTOR2 vPos, bool bFlipped)
{
	if ((nPlayerOrdinal < 0) || (nPlayerOrdinal >= K_MAX_PLAYERS_CNT) || (playerAct[nPlayerOrdinal] == null))
		return;
	//skip update for dead players
	if (playerAct[nPlayerOrdinal]->fLife <= 0.0f)
		return;

	D3DXVECTOR2 pos = vPos;
	RECTXYWH facerect = sprCol->GetAFrameBBox(ANM_IGM_INTERFACE_SPR_PORTRAITS, nPortraitFrame[nPlayerOrdinal]);
	CWeapon* wpn = playerAct[nPlayerOrdinal]->pSelectedWeapon[K_LVL_ACT_WEAPON_PRIMARY];

	RECTXYWH wpnrect(0, 0, 0, 0);
	if(wpn->WeaponTemplate.nHUD_AnimIdx >= 0)
		wpnrect = sprCol->GetAFrameBBox(wpn->WeaponTemplate.nHUD_AnimIdx, 0);

	int algny = pos.y - facerect.h + 2 + 8;
	int algnx = pos.x + facerect.Right() - 2 + wpnrect.w + 8;
	if (bFlipped)
		algnx = pos.x - facerect.Right() + 3 - wpnrect.w - 8;

	///--- alt fire cooldown ---
	//frames 0 si 1 sunt iconurile pentru alt fire, din animatia armei de alt fire
	CWeapon* altwpn = playerAct[nPlayerOrdinal]->pSelectedWeapon[K_LVL_ACT_WEAPON_SECONDARY];
	if ((altwpn->status != K_LVL_WPN_STATUS_UNKNOWN) && (altwpn->WeaponTemplate.nHUD_AnimIdxALT >= 0))
	{
		//anunt umplerea doar cand dureaza mai mult de o secunda ca sa nu deranjeze
		if ((altwpn->WeaponTemplate.fFireRateWait > 1.0f) && (altwpn->status == K_LVL_WPN_STATUS_READY) && (altwpn->statusOld != altwpn->status))
		{
//			SND_PLAY(SNDIDX_PWUP_FULL);
			for (int kk = 0; kk < 3; kk++)
			{
				g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_INTERFACE_ELEMENTS, false, 1, &D3DXVECTOR2(algnx, algny), NULL, NULL, 0.6f, 1.0f, 2.0f, 0.0f, 0.0f, 0.0f, 0.3f, 0xffffffff, K_PART_LAYER_INTERFACE_LIGHT, 0.0f, kk * 0.2f);
			}
		}

		if (!bFlipped)
			algnx += 15.0f; //move cursor to the right
		else
			algnx -= 15.0f;
	}

	///--- gear icon and cooldown ---
	//frames 0 si 1 sunt iconurile pentru gear icon
	CWeapon* gearwpn = playerAct[nPlayerOrdinal]->pSelectedWeapon[K_LVL_ACT_WEAPON_GEAR];
	if (gearwpn != null)
	{
		if ((gearwpn->status == K_LVL_WPN_STATUS_READY) && (gearwpn->statusOld != gearwpn->status))
		{
//			SND_PLAY(SNDIDX_PWUP_FULL);
			for (int kk = 0; kk < 3; kk++)
			{
				g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_INTERFACE_ELEMENTS, false, 1, &D3DXVECTOR2(algnx, algny), NULL, NULL, 0.6f, 1.0f, 2.0f, 0.0f, 0.0f, 0.0f, 0.3f, 0xffffffff, K_PART_LAYER_INTERFACE_LIGHT, 0.0f, kk * 0.2f);
			}
		}
	}
}

void CCustomInterfaceIGM::PaintInterfaceForPlayer(LPDIRECT3DDEVICE9 pDevice, ID3DXSprite* pSprite, int nPlayerOrdinal, RECTXYWH_F scrRect, bool bFlipped, int arrStrategic[], int arrStrategicNames[])
{
	if ((nPlayerOrdinal < 0) || (nPlayerOrdinal >= K_MAX_PLAYERS_CNT))
		return;

	D3DXVECTOR2 pos;
	D3DXMATRIXA16 matFlip;

	D3DXVECTOR2 vPos;
	if (nPlayerOrdinal == 0)
		vPos = D3DXVECTOR2(scrRect.x, scrRect.Bottom());
	else
		vPos = D3DXVECTOR2(scrRect.Right(), scrRect.Bottom());

	if (bFlipped)
	{
		pos = vPos;
		//flipped for player 2
		D3DXMATRIXA16 mat2;
		D3DXMatrixScaling(&matFlip, -1.0f, 1.0f, 1.0f);
		D3DXMatrixAffineTransformation2D(&mat2, 1.0f, NULL, 0.0f, &D3DXVECTOR2(2.0f * vPos.x, 0.0f));
		matFlip *= mat2;
		pSprite->SetTransform(&matFlip);
	}
	else
	{
		pSprite->SetTransform(&g_matIdentity);
		pos = vPos;
	}

	//different skins
	int nPortraitsAnim = (nPlayerOrdinal == 0) ? ANM_IGM_INTERFACE_SPR_PORTRAITS : ANM_IGM_INTERFACE_SPR_PORTRAITS_R;

	//hot join and dead portraits
	if (playerAct[nPlayerOrdinal] == null)
	{
		if (nPortraitFrameHotJoin[nPlayerOrdinal] != -1)
		{
			//selection arrows
			RECTXYWH facerect1 = sprCol->GetAFrameBBox(nPortraitsAnim, nPortraitFrameHotJoin[nPlayerOrdinal]);
			CSprite::paintFrame(sprCol, pos.x, pos.y - facerect1.h, nPortraitsAnim, nPortraitFrameHotJoin[nPlayerOrdinal]);
			
			//left-right selection arrows
			CSprite::paintFrame(sprCol, pos.x + 8.0f - sin(fLocalTimeline * 8.0f), pos.y - facerect1.h + facerect1.CenterY(), ANM_IGM_INTERFACE_SPR_HOT_JOIN_CTRLS, 0);
			CSprite::paintFrame(sprCol, pos.x - 8.0f + facerect1.Right() + sin(fLocalTimeline * 8.0f), pos.y - facerect1.h + facerect1.CenterY(), ANM_IGM_INTERFACE_SPR_HOT_JOIN_CTRLS, 2);
			
			//Class name
			if (bFlipped)
			{
				pSprite->SetTransform(&g_matIdentity);
				g_font8bs1->DrawString(STR_PLAYER_CLASS_ASSAULTER + nPortraitFrameHotJoin[nPlayerOrdinal], vPos.x - facerect1.Right(), vPos.y - 3, FONTFLAG_ANCHOR_BOTTOMRIGHT, K_COLOR_SELECTED_TEXT);
				pSprite->SetTransform(&matFlip);
			}
			else
			{
				g_font8bs1->DrawString(STR_PLAYER_CLASS_ASSAULTER + nPortraitFrameHotJoin[nPlayerOrdinal], vPos.x + facerect1.Right(), vPos.y - 3, FONTFLAG_ANCHOR_BOTTOMLEFT, K_COLOR_SELECTED_TEXT);
			}
		}
		else //dead man portraits
		{
			if (bPlayerPlayedBefore[nPlayerOrdinal])
			{
				RECTXYWH facerect = sprCol->GetAFrameBBox(nPortraitsAnim, nPortraitFrame[nPlayerOrdinal]);
				CSprite::paintFrame(sprCol, pos.x, pos.y - facerect.h, nPortraitsAnim, nPortraitFrame[nPlayerOrdinal], 0xbbff6666);
			}
		}
	}
	//text hot join pentru cei care au mai jucat

	if (playerAct[nPlayerOrdinal] == null)
	{
		pSprite->SetTransform(&g_matIdentity);
		return;
	}
	else //--- daca playerul e mort dar nu e dezalocat poti face continue ---
	{
		CActor* pPlayer = playerAct[nPlayerOrdinal];
		if (pPlayer->GetCurrentBehavior() == AI_BEHAVIOR_DEAD)
		{
			//portretul
			RECTXYWH facerect = sprCol->GetAFrameBBox(nPortraitsAnim, nPortraitFrame[nPlayerOrdinal]);
			CSprite::paintFrame(sprCol, pos.x, pos.y - facerect.h, nPortraitsAnim, nPortraitFrame[nPlayerOrdinal]);

			float fTimer = pPlayer->AItimer1;
			CStringDesc strdesc;
			g_stringsMgr.ReplaceTokenInt(&strdesc, STR_RESPAWNING_IN_N, 1, (int)ceil(fTimer));
			//ca sa licare
			if ((FLOAT_FRAC(fTimer) > 0.2f) && (fTimer > 0.0f))
			{
				///--- lives left ---
				int nHearts = nLivesLeft[nPlayerOrdinal];
				int nHeartFrame = 0;
				if (nHearts <= 0)
				{
					nHearts = 1;
					nHeartFrame = 1; //empty heart
				}
				for (int jj = 0; jj < nHearts; jj++)
				{
					D3DXVECTOR2 vStrPos(pos.x + 7.0f, pos.y - facerect.h + 4.0f);
					CSprite::paintFrame(sprCol, vStrPos.x + 8.0f * jj, vStrPos.y, ANM_IGM_INTERFACE_SPR_LIVES, nHeartFrame);
				}
				//text: continue?
				if (nLivesLeft[nPlayerOrdinal] > 0)
				{
					if (bFlipped)
					{
						pSprite->SetTransform(&g_matIdentity);
						if (nLivesLeft[nPlayerOrdinal] > 0)
						{
							g_font8bs1->DrawString(&strdesc, vPos.x - 2 - facerect.w, vPos.y - 3, FONTFLAG_ANCHOR_BOTTOMRIGHT, K_COLOR_SELECTED_TEXT);
							g_font6ns1->DrawString(STR_FIRE_TO_RESPAWN, vPos.x - 2 - facerect.w, vPos.y - 13, FONTFLAG_ANCHOR_BOTTOMRIGHT, K_COLOR_DEFAULT_TEXT);
						}
						pSprite->SetTransform(&matFlip);
					}
					else
					{
						g_font8bs1->DrawString(&strdesc, pos.x + facerect.w, pos.y - 3, FONTFLAG_ANCHOR_BOTTOMLEFT, K_COLOR_SELECTED_TEXT);
						g_font6ns1->DrawString(STR_FIRE_TO_RESPAWN, pos.x + facerect.w, pos.y - 13, FONTFLAG_ANCHOR_BOTTOMLEFT, K_COLOR_DEFAULT_TEXT);
					}
				}
			}
			return;
		}
	}
	
	DWORD dwIconHoverCol = D3DCOLOR_FFFA(0.5f + 0.5f * sin(fLocalTimeline * 5.0f));
	//get player controller ptr
	CController* ctrlr = UTGetCtrlrMgr().GetControllerByInstanceID(playerAct[nPlayerOrdinal]->nControllerInstanceID);

	//--- player is alive ---
	CSprite spr;
	//character face
	RECTXYWH facerect = sprCol->GetAFrameBBox(nPortraitsAnim, nPortraitFrame[nPlayerOrdinal]);
	if(ctrlr != null)
		CSprite::paintFrame(sprCol, pos.x, pos.y - facerect.h, nPortraitsAnim, nPortraitFrame[nPlayerOrdinal]);
	else
		CSprite::paintFrame(sprCol, pos.x, pos.y - facerect.h, nPortraitsAnim, nPortraitFrame[nPlayerOrdinal], 0x88ffffff);
	///--- lives left ---
	int nHearts = nLivesLeft[nPlayerOrdinal];
	int nHeartFrame = 0;
	if (nHearts <= 0)
	{
		nHearts = 1;
		nHeartFrame = 1; //empty heart
	}
	CLAMP(nHearts, 0, 4);

	for (int jj = 0; jj < nHearts; jj++)
	{
		D3DXVECTOR2 vStrPos(pos.x + 7.0f, pos.y - facerect.h + 4.0f);
		CSprite::paintFrame(sprCol, vStrPos.x + 8.0f * jj, vStrPos.y, ANM_IGM_INTERFACE_SPR_LIVES, nHeartFrame);
	}

	/// --- get weapon animation ---
	//weapon HUD animation format (frames): weapon icon, large bullet bar placeholder, bullet bar cap, magazine bullet placeholder, many bullets bar, single bullet
	CWeapon* wpn = playerAct[nPlayerOrdinal]->pSelectedWeapon[K_LVL_ACT_WEAPON_PRIMARY];

	//#HACK: show weapon cooldown
#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
	if (UTGetAppClass().m_Settings.bShowInterfaceHelp)
	{
		if (playerAct[nPlayerOrdinal]->pCurrentWeapon != null)
		{
			CWeapon* weapon = playerAct[nPlayerOrdinal]->pCurrentWeapon;
			float fPercent = weapon->fAimErrorFOV / weapon->WeaponTemplate.fAimErrorMaxFOV;

			RECTXYWH rect(pos.x + 5, pos.y - 200, 60, 5);
			CtrlMgrDrawProgress_HeadsOutside(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_PROGRESS, rect, fPercent, 0xffffffff);
		}
	}
#endif
	
	if (wpn->WeaponTemplate.nHUD_AnimIdx < 0)
	{
		LOG( L"IGMInterface::couldn't find weapon HUD animation!" );
		return;
	}

	//variabile afisare taste folosite
	/* offsets are relative to the draw pos: x y x y etc */
	/* daca offsetul ramane pe 0 nu se deseneaza tasta pentru el */
	int arr_offsets[50];
	memset(arr_offsets, 0, sizeof(int) * 50);

	RECTXYWH butrectwpn = sprCol->GetAFrameBBox(wpn->WeaponTemplate.nHUD_AnimIdx, 0);

	RECTXYWH cliprect(0, -10000, 10000, 20000);
	int algnx = pos.x + facerect.Right() - 2;
	int algny = pos.y - facerect.h + 2;

	///--- paint weapon icon ---
	CSprite::paintFrame(sprCol, algnx, algny, wpn->WeaponTemplate.nHUD_AnimIdx, 0);
	//set offset for first command
	arr_offsets[0] = facerect.w - 2 + butrectwpn.w / 2; arr_offsets[1] = -facerect.h - 5;

	algnx += butrectwpn.w;
	///--- paint alt fire cooldown ---
	//frames 0 si 1 sunt iconurile pentru alt fire, din animatia armei de alt fire
	CWeapon* altwpn = playerAct[nPlayerOrdinal]->pSelectedWeapon[K_LVL_ACT_WEAPON_SECONDARY];
	if (altwpn->status != K_LVL_WPN_STATUS_UNKNOWN)
	{
		if (altwpn->WeaponTemplate.nHUD_AnimIdxALT >= 0)
		{
			float fCooldown = altwpn->fireRateTimer / altwpn->WeaponTemplate.fFireRateWait;
			RECTXYWH iconrect = sprCol->GetAFrameBBox_real(altwpn->WeaponTemplate.nHUD_AnimIdxALT, 0);

			spr.Init(altwpn->WeaponTemplate.nHUD_AnimIdxALT, algnx, algny, 0);
			spr.paint(sprCol);
			//clipped version
			cliprect = iconrect;
			cliprect.x += algnx; cliprect.y += algny;
			cliprect.h = (int)floor(cliprect.h * (1.0f - fCooldown));
			spr.currentFrame = 1;
			spr.paint(sprCol, &cliprect);

			/*
			//additive blink
			if (altwpn->status == K_LVL_WPN_STATUS_READY)
			{
				AdditiveBlendingON(pDevice, pSprite);
				spr.color = dwIconHoverCol;
				spr.paint(sprCol, &cliprect);
				AdditiveBlendingOFF(pDevice, pSprite);
			}
			*/

			//set offset for second command
			arr_offsets[2] = arr_offsets[0] + butrectwpn.w / 2 + 8; arr_offsets[3] = arr_offsets[1];

			algnx += 15.0f; //move cursor to the right
		}
	}
	///--- paint gear icon and cooldown ---
	//frames 0 si 1 sunt iconurile pentru gear icon
	CWeapon* gearwpn = playerAct[nPlayerOrdinal]->pSelectedWeapon[K_LVL_ACT_WEAPON_GEAR];
	if ((gearwpn != null) && (!gearwpn->WeaponTemplate.name.IsEmpty()))
	{
		bool bCanShoot = g_level.CanShootWeapon(gearwpn);

		int gearanmidx = -1;
		if (gearwpn->WeaponTemplate.nHUD_AnimIdxALT >= 0)
		{
			gearanmidx = gearwpn->WeaponTemplate.nHUD_AnimIdxALT;
			//if gear animation found
			float fCooldown = gearwpn->fireRateTimer / gearwpn->WeaponTemplate.fFireRateWait;
			RECTXYWH iconrect = sprCol->GetAFrameBBox_real(gearanmidx, 0);

			spr.Init(gearanmidx, algnx, algny, 0);
			if (!bCanShoot)
				spr.color = 0xff888888;

			spr.paint(sprCol);
			//clipped version (colored)
			if (gearwpn->ammoLeft != 0)
			{
				cliprect = iconrect;
				cliprect.x += algnx; cliprect.y += algny;
				cliprect.h = (int)floor(cliprect.h * (1.0f - fCooldown));
				spr.currentFrame = 1;
				spr.paint(sprCol, &cliprect);
				/*
				//additive blink
				if (gearwpn->status == K_LVL_WPN_STATUS_READY)
				{
					AdditiveBlendingON(pDevice, pSprite);
					spr.color = dwIconHoverCol;
					spr.paint(sprCol, &cliprect);
					AdditiveBlendingOFF(pDevice, pSprite);
				}
				*/
			}
			//ammo left
			if (gearwpn->ammoLeft >= 0)
			{
				CStringDesc sdAmmo;
				g_stringsMgr.SetStringDesc(&sdAmmo, L"%d", gearwpn->ammoLeft);
				if (bFlipped)
				{
					pSprite->SetTransform(&g_matIdentity);
					g_font6ns1->DrawString(&sdAmmo, vPos.x + (vPos.x - algnx - 1), algny + iconrect.h, FONTFLAG_ANCHOR_BOTTOMRIGHT, K_COLOR_SELECTED_TEXT);
					pSprite->SetTransform(&matFlip);
				}
				else
				{
					g_font6ns1->DrawString(&sdAmmo, algnx + iconrect.w, algny + iconrect.h, FONTFLAG_ANCHOR_BOTTOMRIGHT, K_COLOR_SELECTED_TEXT);
				}
			}

			//set X offset for third command (depending on the second one, if shown or not)
			if(arr_offsets[2] != 0)
				arr_offsets[4] = arr_offsets[2] + 16;
			else
				arr_offsets[4] = arr_offsets[0] + butrectwpn.w / 2 + 8; 
			//and Y
			arr_offsets[5] = arr_offsets[1];

		}
	}

	///--- paint ammo ---
	algnx = pos.x + facerect.Right();
	algny = pos.y - 15;
	cliprect.Set(0, -10000, 10000, 20000);																									 
	int nBulletW = sprCol->GetAFrameBBox_real(wpn->WeaponTemplate.nHUD_AnimIdx, 5).w - 1; //bullet frame (are un pixel in plus de contur)
	int nTotalBullets = wpn->WeaponTemplate.nClipSize + wpn->WeaponTemplate.nBulletChamberSize;
	bool bHasBulletChamber = wpn->WeaponTemplate.nBulletChamberSize > 0;
	int nBulletsLeft = wpn->ammoLeft;
	//special cases:
	if (wpn->WeaponTemplate.nClipSize < 0) //infinite bullets paint one
	{
		bHasBulletChamber = false;
		nBulletsLeft = 1;
		nTotalBullets = 1;
	}
	//draw placeholder
	spr.Init(wpn->WeaponTemplate.nHUD_AnimIdx, algnx, algny, 1);
	cliprect.w = algnx + nTotalBullets * nBulletW;
	spr.paint(sprCol, &cliprect);
	spr.currentFrame = 2; //bullets holder cap
	spr.pos.x = cliprect.w;
	spr.paint(sprCol);

	//set offset for reload command
	if ((!wpn->WeaponTemplate.nReloadUnitSize == 0) && (nBulletsLeft < 0.4f * nTotalBullets) && 
		(playerAct[nPlayerOrdinal]->nAttackStatus != K_LVL_ACT_ATTACK_RELOADING) && (wpn->WeaponTemplate.nClipSize > 0))
	{
		arr_offsets[6] = facerect.w + nTotalBullets * nBulletW + 6; arr_offsets[7] = -18;
	}

	//draw bullet chamber
	if (bHasBulletChamber)
	{
		CSprite::paintFrame(sprCol, algnx, algny, wpn->WeaponTemplate.nHUD_AnimIdx, 3); //bullet chamber frame
	}
	//paint bullets
	if (nBulletsLeft > 0)
	{
		cliprect.w = algnx + nBulletsLeft * nBulletW + 1; //nu tai ultimul pixel de contur
		spr.pos.x = algnx;
		spr.currentFrame = 4; //bullets array frame
		spr.paint(sprCol, &cliprect);
	}

	///--- paint life ---
	const int lifeBarW = (int)(K_CI_IGM_LIFE_BAR_MULTIPLIER * playerAct[nPlayerOrdinal]->templateActor.fLife);
	algnx = pos.x + facerect.Right() + 2; //offset hardcodat datorita graficii
	algny = pos.y - 5;
	spr.Init(ANM_IGM_INTERFACE_SPR_LIFE_BAR, algnx, algny, 0);
	cliprect.Set(0, -10000, 10000, 20000);
	cliprect.w = algnx + lifeBarW;
	spr.paint(sprCol, &cliprect);
	spr.currentFrame = 1; //life holder cap
	spr.pos.x += lifeBarW;
	spr.paint(sprCol);

	//fill life
	int lifeperc = (int)ceil(lifeBarW * (playerAct[nPlayerOrdinal]->fLife / playerAct[nPlayerOrdinal]->templateActor.fLife));
	spr.currentFrame = 2; //red bar to be clipped
	spr.pos.x = algnx;
	cliprect.w = algnx + lifeperc;
	spr.paint(sprCol, &cliprect);
	//life cap
	spr.pos.x += lifeperc;
	spr.currentFrame = 3;
	spr.paint(sprCol);

	///--- paint shield ---
	if (playerAct[nPlayerOrdinal]->templateActor.fArmor > 0.0f)
	{
		const int shieldBarW = (int)(K_CI_IGM_SHIELD_BAR_MULTIPLIER * playerAct[nPlayerOrdinal]->templateActor.fArmor);
		algnx = pos.x + facerect.Right() + 9 + lifeBarW; //offset hardcodat datorita graficii
		algny = pos.y - 5;
		spr.Init(ANM_IGM_INTERFACE_SPR_LIFE_BAR, algnx, algny, 0);
		cliprect.Set(0, -10000, 10000, 20000);
		cliprect.w = algnx + shieldBarW;
		spr.paint(sprCol, &cliprect);
		spr.currentFrame = 1; //life holder cap
		spr.pos.x += shieldBarW;
		spr.paint(sprCol);
		//fill life
		int shieldperc = (int)ceil(shieldBarW * (playerAct[nPlayerOrdinal]->fArmor / playerAct[nPlayerOrdinal]->templateActor.fArmor));
		if (shieldperc > 0.0f)
		{
			spr.currentFrame = 4; //blue bar to be clipped
			spr.pos.x = algnx;
			cliprect.w = algnx + shieldperc;
			spr.paint(sprCol, &cliprect);
			//shield cap
			spr.pos.x += shieldperc;
			spr.currentFrame = 5;
			spr.paint(sprCol);
		}
	}

	///--- paint strategic bar ---
	//se deseneaza mereu fara flip
	if (bFlipped)
	{
		pSprite->SetTransform(&g_matIdentity);
	}

	RECTXYWH barrect = sprCol->GetAFrameBBox(ANM_IGM_INTERFACE_SPR_STRATEGIC_BAR, 0);
	//one filled SP slot width, including spacing
	int nSlotW = sprCol->GetAFrameBBox(ANM_IGM_INTERFACE_SPR_STRATEGIC_BAR, 2).w;

	if (nPlayerOrdinal == 0)
	{
		algnx = scrRect.x;
		algny = scrRect.y;
	}
	else
	{
		algnx = scrRect.Right() - barrect.w;
		algny = scrRect.y;
		//move bar a little to the left
		if (bHasExtraSPSlots[nPlayerOrdinal])
			algnx -= K_LVL_STRATEGIC_POINTS_ADDED_BY_PERK * nSlotW;
	}
	//base bar
	CSprite::paintFrame(sprCol, algnx, algny, ANM_IGM_INTERFACE_SPR_STRATEGIC_BAR, 0);
	//extra long SP bar
	if (bHasExtraSPSlots[nPlayerOrdinal])
		CSprite::paintFrame(sprCol, algnx, algny, ANM_IGM_INTERFACE_SPR_STRATEGIC_BAR, 6);
	//daca a mai jucat playerul celelalt dar acum e mort dau optiunea de respawn
	bool bShowRespawnOther = false;
	int nOtherPlOrdinal = (nPlayerOrdinal + 1) % K_MAX_PLAYERS_CNT;
	if ((playerAct[nOtherPlOrdinal] == null) && (nPortraitFrameHotJoin[nOtherPlOrdinal] == -1) && (bPlayerPlayedBefore[nOtherPlOrdinal] == true))
		bShowRespawnOther = true;
	bool bShowGet1UP = (nLivesLeft[nPlayerOrdinal] == 0);

	//float fPerc = LIMIT((fStrategicPoints[nPlayerOrdinal] / (float)K_LVL_MAX_STRATEGIC_POINTS), 0.0f, 1.0f);
	int nFullSegments = (int)floor(fStrategicPoints[nPlayerOrdinal]);
	int nSelection = nStrategicSelection[nPlayerOrdinal];
	int nPrice = nSelection + 1;
	DWORD dwColFull = D3DCOLOR_FFFA(0.8f + 0.2f * sin(fLocalTimeline * 5.0f));
	//paint filler
	barrect = sprCol->GetAFrameBBox(ANM_IGM_INTERFACE_SPR_STRATEGIC_BAR, 1);
	//show button if needed
	if ((nFullSegments >= K_LVL_MAX_STRATEGIC_POINTS) && (nSelection < 0))
	{
		arr_offsets[10] = barrect.w + 10;
		arr_offsets[11] = -scrRect.h + 5;
		
		if (!bHasExtraSPSlots[nPlayerOrdinal])
			arr_offsets[10] -= K_LVL_STRATEGIC_POINTS_ADDED_BY_PERK * nSlotW;
	}
	//#HACK: tutorial - paint help using special ability
	if ((nFullSegments >= K_LVL_MAX_STRATEGIC_POINTS) && (nSelection < 0) &&
		(nPlayerOrdinal == 0) && (g_level.m_nPlayers == 1) && (g_userData[K_MEMID_TUT_INTERFACE_STRATEGIC] == 0))
	{
		RECTXYWH tmprect;
		tmprect.Set(vPos.x + barrect.w + 50, vPos.y - scrRect.h + 5, 120, 24);
		SIZEWH txtsz = g_font6n1->MeasureString(STR_TUTORIAL_INTERFACE_STRATEGIC, tmprect.w);
		tmprect.h = txtsz.h;
		CtrlMgrDrawFrame(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_FRAME1, tmprect, 0xffffffff);
		g_font6n1->DrawString(STR_TUTORIAL_INTERFACE_STRATEGIC, tmprect, FONTFLAG_ANCHOR_VCENTERHCENTER | FONTFLAG_WRAPTEXT, 0xffffffff);
		//arrow too
		float foff = max(0.0f, 5.0f * sin(fLocalTimeline * 6.0f));
		CSprite::paintFrame(&UTGetGUI().m_sprCol, vPos.x + barrect.w + 45.0f + foff, vPos.y - scrRect.h + 9, ANM_CONTROLS_SPR_TUTORIAL_L_ARR, 0, 0xffffffff);
	}


	barrect.x += algnx;
	barrect.y += algny;
	barrect.w = nSlotW * fStrategicPoints[nPlayerOrdinal];// barrect.w * fPerc;
	spr.Init(ANM_IGM_INTERFACE_SPR_STRATEGIC_BAR, algnx, algny);
	spr.currentFrame = 1;
	spr.paint(sprCol, &barrect);

	//lighten up bar!
	bool bBlinkOn = ((int(fStrategicHighlightTimer * 8.0f) % 2) == 1);
	if ((fStrategicHighlightTimer > 0.0f) && (bBlinkOn))
	{
		AdditiveBlendingON(pDevice, pSprite);
		CSprite localspr = spr;
		//fade out color
		//localspr.color = D3DCOLOR_FFFA(fStrategicHighlightTimer);
		localspr.paint(sprCol, &barrect);
		AdditiveBlendingOFF(pDevice, pSprite);
	}

	//dreptunghi plin (si pt aliniere icons)
	barrect = sprCol->GetAFrameBBox(ANM_IGM_INTERFACE_SPR_STRATEGIC_BAR, 2);

	//paint integer bars when not selecting
	//if (nSelection < 0)
	//{
	//	AdditiveBlendingON(pDevice, pSprite);
	//	spr.currentFrame = 2;
	//	spr.color = dwColFull;
	//	for (int kk = 0; kk < nFullSegments; kk++)
	//	{
	//		spr.pos.x = algnx + kk * barrect.w;
	//		spr.pos.y = algny;
	//		spr.paint(sprCol);
	//	}
	//	AdditiveBlendingOFF(pDevice, pSprite);
	//}

	//paint icons
	spr.Init(ANM_IGM_INTERFACE_SPR_STRATEGIC_BAR_ICONS, algnx, algny);
	for (int kk = 0; kk < K_LVL_MAX_STRATEGIC_POINTS; kk++)
	{
		if (arrStrategic[kk] < 0)
			continue;

		bool bDisabled = false;
		if ((arrStrategic[kk] == K_CI_STRATEGIC_REINFORCEMENT) && (!bShowRespawnOther))
			bDisabled = true;
		if ((arrStrategic[kk] == K_CI_STRATEGIC_EXTRA_LIFE) && (!bShowGet1UP))
			bDisabled = true;

		spr.pos.x = algnx + 9 + kk * barrect.w; spr.pos.y = algny + 9;
		//sunt cate 3 frames per icon (mic disabled, mic, mai mare pt selectie)
		spr.currentFrame = arrStrategic[kk] * 3;
		if ((nFullSegments >= (kk + 1)) && (!bDisabled))
			spr.currentFrame++;
		spr.paint(sprCol);
		//blinkers when not selected but has enough points
		if (!bDisabled)
		{
			//respawn peer blinks
			if ((arrStrategic[kk] == K_CI_STRATEGIC_REINFORCEMENT) && (g_timers.GetTimerValue(600) < 0.4f) && (nFullSegments >= (kk + 1)))
			{
				AdditiveBlendingON(pDevice, pSprite);
				spr.paint(sprCol);
				AdditiveBlendingOFF(pDevice, pSprite);
			}
		}

		//#TODO: sa iti dea un feedback de activare a abilitatii (ramane deschisa interfata si licare iconul sau ceva), 

		if (kk == nStrategicSelection[nPlayerOrdinal])
		{
			for (int ll = 0; ll <= kk; ll++)
			{
				//dreptunghi plin (si pt aliniere icons)
				RECTXYWH localrect = sprCol->GetAFrameBBox(ANM_IGM_INTERFACE_SPR_STRATEGIC_BAR, 2);
				if(nPrice > nFullSegments)
					CSprite::paintFrame(sprCol, algnx + ll * localrect.w, algny, ANM_IGM_INTERFACE_SPR_STRATEGIC_BAR, 3, 0xffffffff); //red
				else
					CSprite::paintFrame(sprCol, algnx + ll * localrect.w, algny, ANM_IGM_INTERFACE_SPR_STRATEGIC_BAR, 2, dwColFull); //blue
			}
			//cursor selectie
			CSprite spr2(ANM_IGM_INTERFACE_SPR_STRATEGIC_BAR, 0, 0);
			spr2.color = dwIconHoverCol;
			spr2.currentFrame = 4;
			//disabled or not enough points - red cursor
			if ((bDisabled) || (nPrice > nFullSegments))
				spr2.currentFrame = 5;

			RECTXYWH currect = sprCol->GetAFrameBBox(ANM_IGM_INTERFACE_SPR_STRATEGIC_BAR, 4);
			spr2.color = 0xffffffff;
			spr2.pos.x = algnx + kk * currect.w;
			spr2.pos.y = algny;
			spr2.paint(sprCol);
			//icon mare
			if (!bDisabled)
			{
				spr.currentFrame = arrStrategic[kk] * 3 + 2;
				spr.pos = spr2.pos;
			}
			else
			{
				spr.currentFrame = arrStrategic[kk] * 3;
			}
			spr.paint(sprCol);
			//nume icon
			CStringDesc * strDesc = g_stringsMgr.strings[arrStrategicNames[kk]];
			CTexturedFont* pFont = g_font5n1;
			D3DXVECTOR2 vButCenter(spr2.pos.x + 8, spr2.pos.y + 33);
			SIZEWH fsz = pFont->MeasureString(strDesc);
			RECTXYWH butrect(vButCenter.x - fsz.w / 2, vButCenter.y, fsz.w, fsz.h);
			if (butrect.x < 4)
				butrect.x = 4;
			int nRoff = butrect.Right() - (scrRect.Right() - 4);
			if (nRoff > 0)
				butrect.x -= nRoff;

			CtrlMgrDrawHTilingAnim_HeadsOutside(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_BUT_SM_DARKBLUE, 0, butrect, 0xffffffff);
			pFont->DrawString(strDesc, butrect.CenterX(), vButCenter.y, FONTFLAG_ANCHOR_VCENTERHCENTER, 0xffffffff);

			//show helper key
			if ((!bDisabled) && (nPrice <= nFullSegments))
			{
				if (!bFlipped)
				{
					arr_offsets[12] = butrect.Right() + 5;
					arr_offsets[13] = -scrRect.h + butrect.y - 2;
				}
				else
				{
					arr_offsets[12] = scrRect.Right() - butrect.x + 5;
					arr_offsets[13] = -scrRect.h + butrect.y - 2;
				}
			}
		}
	}
	//re-flip
	if (bFlipped)
	{
		pSprite->SetTransform(&matFlip);
	}

	//final flush
	pSprite->SetTransform(&g_matIdentity);
	///--- paint helper keys ---
	EControllerCommand arr_commands[] = { 
		K_CM_COMMAND_FIRE1, K_CM_COMMAND_FIRE2, K_CM_COMMAND_USE_GEAR, K_CM_COMMAND_RELOAD, K_CM_COMMAND_MELEE, //main controls
		K_CM_COMMAND_STRATEGIC_MENU, K_CM_COMMAND_FIRE1 //strategic menu open, strategic option activate
	}; 
	
	bool bDrawKeys = true;
	if ((ctrlr != null) && (ctrlr->eType == K_CM_CT_NET_FRAMELOCK))
		bDrawKeys = false;

	//draw buttons for local players
	if (bDrawKeys)
	{
		for (int kk = 0; kk < ARRAY_SIZE(arr_commands); kk++)
		{
			//if not set, don't draw
			if ((arr_offsets[kk * 2] == 0) && (arr_offsets[kk * 2 + 1] == 0))
				continue;

			D3DXVECTOR2 vBP;
			if (bFlipped)
				vBP = D3DXVECTOR2(vPos.x - arr_offsets[kk * 2], vPos.y + arr_offsets[kk * 2 + 1]);
			else
				vBP = D3DXVECTOR2(vPos.x + arr_offsets[kk * 2], vPos.y + arr_offsets[kk * 2 + 1]);
			//key icons not set, defaults on key names
			if (arrKeyIcons[nPlayerOrdinal][kk] < 0)
			{
				int anmIdx = ANM_CONTROLS_SPR_BUT_SM_GREY2;
				bool bPressed = false;
				int nAlign = 0;
				if ((arr_commands[kk] == K_CM_COMMAND_RELOAD) || (arr_commands[kk] == K_CM_COMMAND_STRATEGIC_MENU) || (kk == 6))
				{
					bPressed = (g_timers.GetTimerValue(400) < 0.2f) ? true : false;

					nAlign = -1;
					if (bFlipped)
						nAlign = 1;
				}

				if ((UTGetAppClass().m_Settings.bShowInterfaceHelp) || (arr_commands[kk] == K_CM_COMMAND_RELOAD) || (arr_commands[kk] == K_CM_COMMAND_STRATEGIC_MENU) || (kk == 6))
				{
					CtrlMgrDrawButtonFromText(&UTGetGUI().m_sprCol, anmIdx, bPressed, &arrKeyNames[nPlayerOrdinal][kk], g_font5n2, vBP, 0xffffffff, nAlign);
				}
			}
			else //key icons set
			{
				int anmIdx = K_CI_ANIMIDX_BUTICONS_UP;
				bool bPressed = false;
				int nAlign = 0;

				if ((arr_commands[kk] == K_CM_COMMAND_RELOAD) || (arr_commands[kk] == K_CM_COMMAND_STRATEGIC_MENU) || (kk == 6))
				{
					bPressed = (g_timers.GetTimerValue(400) < 0.2f) ? true : false;

					nAlign = -1;
					if (bFlipped)
						nAlign = 1;
				}
				if (bPressed)
					anmIdx = K_CI_ANIMIDX_BUTICONS_DN;

				if ((UTGetAppClass().m_Settings.bShowInterfaceHelp) || (arr_commands[kk] == K_CM_COMMAND_RELOAD) || (arr_commands[kk] == K_CM_COMMAND_STRATEGIC_MENU) || (kk == 6))
				{
					int butw = UTGetGUI().m_sprCol.GetAFrameBBox(anmIdx, arrKeyIcons[nPlayerOrdinal][kk]).w;
					int algnoffx = (-nAlign * butw) / 2;
					CSprite::paintFrame(&UTGetGUI().m_sprCol, vBP.x + algnoffx, vBP.y, anmIdx, arrKeyIcons[nPlayerOrdinal][kk]);
				}
			}
		}
	}

	pSprite->Flush();
}

CCustomInterfaceIGM::CCustomInterfaceIGM()
{
	fLocalTimeline = 0.0f;

	Reset();
}

void CCustomInterfaceIGM::Reset()
{
	m_fBombTimer = -1.0f;

	for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
	{
		playerAct[kk] = null;
		nPortraitFrame[kk] = 0;
		nPortraitFrameHotJoin[kk] = -1; //no hot join
		bPlayerPlayedBefore[kk] = false;
		bHasExtraSPSlots[kk] = false;

		nLivesLeft[kk] = 0;

		fStrategicHighlightTimer = 0.0f;
		fStrategicPoints[kk] = 0.0f;
		fStrategicPoints_old[kk] = 0.0f;
		nStrategicSelection[kk] = -1;

		for (int jj = 0; jj < ARRAY_SIZE(arrKeyIcons[0]); jj++)
		{
			arrKeyIcons[kk][jj] = -1;//reset icons on DON'T SHOW
		}
	}
}

void CCustomInterfaceIGM::Init(CSpriteCollection* sprCollection, CActor * player1, CActor * player2)
{
	sprCol = sprCollection;
	playerAct[0] = player1;
	playerAct[1] = player2;

	if(playerAct[0] != null)
		bPlayerPlayedBefore[0] = true;
	if (playerAct[1] != null)
		bPlayerPlayedBefore[1] = true;

	for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
	{
		if (playerAct[kk] != null)
		{
			nPortraitFrame[kk] = playerAct[kk]->templateActor.nHUDportraitFrameIdx;
			//set same portrait on spawned players
			nPortraitFrameHotJoin[kk] = nPortraitFrame[kk];

			//#PERK: EXTRA SP SLOTS - gives you 2 additionsl SP slots
			bHasExtraSPSlots[kk] = false;
			if (g_playerSelScr.IsPerkEnabled(playerAct[kk]->nPlayerOrdinal, &shPerk_EXTRA_SP_SLOTS))
				bHasExtraSPSlots[kk] = true;

			///--- save key names ---
			//get player controller ptr
			CController* ctrlr = UTGetCtrlrMgr().GetControllerByInstanceID(playerAct[kk]->nControllerInstanceID);

			if (ctrlr == null)
			{
				ErrorBox(K_ERR_WARNING, L"Illegal controller InstanceID:%d", playerAct[kk]->nControllerInstanceID);
				continue;
			}

			EControllerCommand arr_commands[] = {
				K_CM_COMMAND_FIRE1, K_CM_COMMAND_FIRE2, K_CM_COMMAND_USE_GEAR, K_CM_COMMAND_RELOAD, K_CM_COMMAND_MELEE, //main controls
				K_CM_COMMAND_STRATEGIC_MENU, K_CM_COMMAND_FIRE1 //strategic menu open, strategic option activate
			};
			WCHAR strKey[MAX_PATH];
			//save command names
			if (ctrlr->eType == K_CM_CT_NET_FRAMELOCK)
			{
				for (int ii = 0; ii < ARRAY_SIZE(arr_commands); ii++)
				{
					//reset 
					arrKeyIcons[kk][ii] = -1;//reset icons on DON'T SHOW
					arrKeyNames[kk][ii].Reset();
				}
			}
			else if (ctrlr->eType == K_CM_CT_KBM_SDL)
			{
				for (int ii = 0; ii < ARRAY_SIZE(arr_commands); ii++)
				{
					arrKeyIcons[kk][ii] = -1;//reset icons on DON'T SHOW

					SDL_Scancode commandscan = (SDL_Scancode)ctrlr->GetKeyMappingForCommand(arr_commands[ii]);
					//mbstowcs_s(null, strKey, SDL_GetScancodeName(commandscan), MAX_PATH);
					mbstowcs_s(null, strKey, UTGetCtrlrMgr().GetSDLScancodeName(commandscan), MAX_PATH);
					g_stringsMgr.SetStringDesc(&arrKeyNames[kk][ii], strKey);
				}
			}
			else if (ctrlr->eType == K_CM_CT_JOYSTICK_SDL)
			{
				for (int ii = 0; ii < ARRAY_SIZE(arr_commands); ii++)
				{
					CControllerTrigger* trigger = ctrlr->GetTriggerForCommand(arr_commands[ii]);

					if (trigger->eType == K_CM_BUTTON)
					{
						//set icon from CTRLR_XBOX_UP/DOWN from controls.bsx
						arrKeyIcons[kk][ii] = K_CI_ARR_BUTICONS_FRAMES[trigger->keyMapping];
						//save string name as a fallback
						CHAR sName[MAX_PATH];
						sprintf(sName, SDL_GameControllerGetStringForButton((SDL_GameControllerButton)trigger->keyMapping));
						for (int ll = 0; ll < (int)strlen(sName); ll++)
							sName[ll] = toupper(sName[ll]);

						mbstowcs_s(null, strKey, sName, MAX_PATH);
						g_stringsMgr.SetStringDesc(&arrKeyNames[kk][ii], strKey);
					}
					else if (trigger->eType == K_CM_HALF_AXIS)
					{
						arrKeyIcons[kk][ii] = K_CI_ARR_AXISICONS_FRAMES[trigger->keyMapping];
						//save string name as a fallback
						mbstowcs_s(null, strKey, SDL_GameControllerGetStringForAxis((SDL_GameControllerAxis)trigger->keyMapping), MAX_PATH);
						g_stringsMgr.SetStringDesc(&arrKeyNames[kk][ii], strKey);
					}
				}
			}
		}
	}
}

void CCustomInterfaceIGM::SetHotJoinSelection(int nPlayerOrdinal, int nSelectedType)
{
	nPortraitFrameHotJoin[nPlayerOrdinal] = nSelectedType;
}

void CCustomInterfaceIGM::SetStrategicPoints(float fPl1newVal, float fPl2newVal)
{
	float fSetVal = fPl1newVal;
	if (fSetVal < 0.0f)
		fSetVal = 0.0f;
	fStrategicPoints_old[0] = fStrategicPoints[0];
	fStrategicPoints[0] = fSetVal;
	//se reseteaza ambele cand se duce pe 0.0f
	if (fSetVal <= 0.0f)
		fStrategicPoints_old[0] = fStrategicPoints[0];

	fSetVal = fPl2newVal;
	if (fSetVal < 0.0f)
		fSetVal = 0.0f;
	fStrategicPoints_old[1] = fStrategicPoints[1];
	fStrategicPoints[1] = fSetVal;
	//se reseteaza ambele cand se duce pe 0.0f
	if (fSetVal <= 0.0f)
		fStrategicPoints_old[1] = fStrategicPoints[1];
	//highlight strategic points when gaining)
	if ((fStrategicPoints_old[0] < fStrategicPoints[0]) || (fStrategicPoints_old[1] < fStrategicPoints[1]))
		fStrategicHighlightTimer = 1.0f;
}

void CCustomInterfaceIGM::SetLivesLeft(int nPl1Lives, int nPl2Lives)
{
	nLivesLeft[0] = nPl1Lives;
	nLivesLeft[1] = nPl2Lives;
}

void CCustomInterfaceIGM::SetStrategicSelection(int nPlayerOrdinal, int nSelection)
{
	nStrategicSelection[nPlayerOrdinal] = nSelection;
}

void CCustomInterfaceIGM::SetBombTimer(float fTimer)
{
	m_fBombTimer = fTimer;
}

bool CCustomInterfaceIGM::Update(float dTime)
{
	fLocalTimeline += dTime;

	dec_limit(fStrategicHighlightTimer, dTime * 1.5f, 0.0f);

	RECTXYWH_F camAABB = UTGetAppClass().g_cam240hScreen.GetCamWorldAABB();

	UpdateInterfaceForPlayer(0, D3DXVECTOR2(camAABB.x, camAABB.Bottom()), false);
	UpdateInterfaceForPlayer(1, D3DXVECTOR2(camAABB.Right(), camAABB.Bottom()), true);

	return false;
}

void CCustomInterfaceIGM::Paint(LPDIRECT3DDEVICE9 pDevice, ID3DXSprite* pSprite)
{
	//get current camera screen rect
	CCameraTransform::SetActiveCamera(pDevice, &UTGetAppClass().g_cam240hScreen);
	RECTXYWH_F camAABB = CCameraTransform::GetActiveCamera()->GetCamWorldAABB();
	pSprite->SetTransform(&g_matIdentity);
	//#TODO: #HACK: nu imi place ca acceseaza direct date din g_level
	PaintInterfaceForPlayer(pDevice, pSprite, 0, camAABB, false, g_level.m_arrStrategicAbilities[0], g_level.m_arrStrategicAbilitiesNames[0]);
	PaintInterfaceForPlayer(pDevice, pSprite, 1, camAABB, true, g_level.m_arrStrategicAbilities[1], g_level.m_arrStrategicAbilitiesNames[1]);

	///--- bomb timer ---
	if (m_fBombTimer >= 0.0f)
	{
		pSprite->SetTransform(&g_matIdentity);
		DWORD wcol = 0xffaa2222;
		if ((m_fBombTimer <= 15.0f) && (FLOAT_FRAC(m_fBombTimer) < 0.4f))
			wcol = 0xffff2222;

		D3DXVECTOR2 vTimerPos(camAABB.CenterX(), camAABB.y + 14.0f);
		int nFrame = 0;
		if (FLOAT_FRAC(m_fBombTimer) < 0.4f)
			nFrame = 1;
		CSprite::paintFrame(sprCol, vTimerPos.x, vTimerPos.y, ANM_IGM_INTERFACE_SPR_BOMB_TIMER, nFrame);

		WCHAR txt[MAX_PATH];
		OS_FormatTime(txt, MAX_PATH, m_fBombTimer);
		CStringDesc strdesc;
		g_stringsMgr.SetStringDesc(&strdesc, txt);
		g_font8bs1->DrawString(&strdesc, vTimerPos.x, vTimerPos.y, FONTFLAG_ANCHOR_BOTTOMCENTER, wcol);
	}

	///--- inventory ---
	//keeps inventory items to draw
	CFixedArray<int, 10> arrInvAnmIdx;
	CVariantComplex* vcYellowCard = UTGetScriptManager().GetGlobalVar(L"inventory_goldKey");
	if ((vcYellowCard->m_type != CVariantComplex::K_ARGTYPE_NONE) && (vcYellowCard->m_asINT32 != 0))
	{
		arrInvAnmIdx.Add(1);
	}
	CVariantComplex* vcRedCard = UTGetScriptManager().GetGlobalVar(L"inventory_redKey");
	if ((vcRedCard->m_type != CVariantComplex::K_ARGTYPE_NONE) && (vcRedCard->m_asINT32 != 0))
	{
		arrInvAnmIdx.Add(0);
	}

	//now paint inventory items
	pSprite->SetTransform(&g_matIdentity);
	int nInvOffx = 8 * (arrInvAnmIdx.Count() - 1);
	int nInvOffy = (m_fBombTimer >= 0.0f) ? 24 : 10;
	for (int kk = 0; kk < arrInvAnmIdx.Count(); kk++)
	{
		CSprite::paintFrame(sprCol, camAABB.CenterX() - nInvOffx + kk * 18, camAABB.y + nInvOffy, ANM_IGM_INTERFACE_SPR_INVENTORY, arrInvAnmIdx.m_pData[kk]);
	}

	pSprite->Flush();
}

void CCustomInterfaceIGM::Release()
{
	for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
	{
		playerAct[kk] = null;
		bPlayerPlayedBefore[kk] = false;
	}
	sprCol = null;
}

#pragma endregion

#pragma region TEXT_BUBBLE

CCustomInterfaceTextBubble::CCustomInterfaceTextBubble()
{
	eType = K_TYPE_NONE;
	pos = D3DXVECTOR2(0.0f, 0.0f);
	bbox.Set(0.0f, 0.0f, 0.0f, 0.0f);
	fAlpha = 0.0f;
	bActive = false;
	nTextIDX = -1;
	fShowTimer = 0.0f;
	
	nParam1 = 0;
	shString.Reset();

	nFontID = FONTIDX_6_NS1;
}

void CCustomInterfaceTextBubble::ShowLevelHint(CCameraTransform* pCamera, int nnTextIndex, int nnFontID, D3DXVECTOR2 vPos, float fTimer)
{
	eType = K_TYPE_LEVEL_HINT;

	pos = vPos;
	bActive = true;
	nTextIDX = nnTextIndex;
	fShowTimer = fTimer;

	nFontID = nnFontID;
	m_pCamera = pCamera;
}

void CCustomInterfaceTextBubble::Hide(bool bForced /*= false*/)
{
	bActive = false;
	if (bForced)
		fAlpha = 0.0f;
}

void CCustomInterfaceTextBubble::Init(CSpriteCollection* sprCollection)
{
	sprCol = sprCollection;
	fAlpha = 0.0f;
	bActive = false;
	fShowTimer = 0.0f;
	m_pCamera = null;
}

bool CCustomInterfaceTextBubble::Update(float dTime)
{
	if (m_pCamera == null)
		return false;

	//handle timer
	if (fShowTimer > 0.0f)
	{
		fShowTimer -= dTime;
		if (fShowTimer <= 0.0f)
		{
			bActive = false;
			fShowTimer = 0.0f;
		}
	}

	RECTXYWH_F scrrect = m_pCamera->GetCamWorldAABB();
	//ajustam scrrect
	scrrect.Inflate(-16.0f);
	//cand sursa mesajului iese din ecran il opreste
	if (!PointInRect(pos, scrrect))
		bActive = false;
	//transparency
	if (bActive)
	{
		inc_limit(fAlpha, dTime * K_CI_TB_FADE_SPEED_PERSEC, 1.0f);
	}
	else
	{
		dec_limit(fAlpha, dTime * K_CI_TB_FADE_SPEED_PERSEC, 0.0f);
	}

	return false;
}

void CCustomInterfaceTextBubble::Paint(LPDIRECT3DDEVICE9 pDevice, ID3DXSprite* pSprite)
{
	if ((fAlpha <= 0.0f) || (m_pCamera == null))
		return;

	pSprite->Flush();
	CCameraTransform* pcam = CCameraTransform::GetActiveCamera();
	RECTXYWH_F scrrect = CCameraTransform::GetActiveCamera()->GetCamWorldAABB();
	//bring point to render camera space
	D3DXVECTOR2 vPos = pos;

	switch (eType)
	{
		case K_TYPE_LEVEL_HINT:
		{
			//compute bbox
			SIZEWH textsz = UTGetFontsManager().fonts[nFontID]->MeasureString(nTextIDX, K_CI_TB_MAX_WIDTH);
			bbox.Set(vPos.x - textsz.w / 2.0f, vPos.y - textsz.h - 5, textsz.w, textsz.h);
			//fac sa nu iasa textul din ecran
			if (bbox.x < scrrect.x + 10)
				bbox.x = scrrect.x + 10;
			else if (bbox.Right() > (scrrect.Right() - 10))
				bbox.x -= bbox.Right() - (scrrect.Right() - 10);
			if (bbox.y < scrrect.y + 10)
				bbox.y = scrrect.y + 10;

			//desenez
			DWORD dwCol = D3DCOLOR_FFFA(fAlpha);
			RECTXYWH localbox(bbox.x, bbox.y, bbox.w, bbox.h);
			D3DXMATRIXA16 mattr;
			D3DXMatrixAffineTransformation2D(&mattr, 1.0f, NULL, 0.0f, &vPos);

			CtrlMgrDrawFrameF(sprCol, ANM_CONTROLS_SPR_FRAME_BUBBLE, localbox, dwCol);
			//sageata catre sursa mesajului
			float arrposx = vPos.x;
			CLAMP(arrposx, bbox.x + 5, bbox.Right() - 5);
			if (sprCol->GetAFramesCnt(ANM_CONTROLS_SPR_FRAME_BUBBLE) >= 10)
			{
				CSprite::paintFrame(sprCol, arrposx, bbox.Bottom(), ANM_CONTROLS_SPR_FRAME_BUBBLE, 9, dwCol);
			}

			CTexturedFont* pFont = UTGetFontsManager()[nFontID];
			if ((pFont->pFontReplacementTTF != null) && (pFont->bFontReplacementOn))
			{
				pSprite->Flush();

				D3DXVECTOR2 vPosUL = m_pCamera->WorldToWorld(D3DXVECTOR2(bbox.x, bbox.y), UTGetAppClass().g_cam480hScreen);
				D3DXVECTOR2 vPosDR = m_pCamera->WorldToWorld(D3DXVECTOR2(bbox.Right(), bbox.Bottom()), UTGetAppClass().g_cam480hScreen);


				RECT trct;
				SetRect(&trct, vPosUL.x, vPosUL.y, vPosDR.x, vPosDR.y);
				CCameraTransform::SetActiveCamera(pDevice, &UTGetAppClass().g_cam480hScreen);
				pFont->pFontReplacementTTF->pFont->DrawTextW(pSprite, g_stringsMgr.strings[nTextIDX]->sText, -1, &trct, DT_CENTER | DT_VCENTER, dwCol);
				pSprite->Flush();
				CCameraTransform::SetActiveCamera(pDevice, pcam);
			}
			else
			{
				pFont->DrawString(nTextIDX, localbox, FONTFLAG_ANCHOR_VCENTERHCENTER | FONTFLAG_WRAPTEXT, dwCol);
			}
		}
		break;
		case K_TYPE_LOCKED_DOOR_HINT:
		{
		}
		break;
	}

	pSprite->Flush();
}

void CCustomInterfaceTextBubble::Release()
{
	m_pCamera = null;
	sprCol = null;
}

#pragma endregion

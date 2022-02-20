#include "dxstdafx.h"

//static fn
CSpriteCollection* CControl::m_pSprCol = nullptr;

//controls types
//params default ID,X,Y,W,H,inflate. 
enum EControlType {
	CCTRL_TYPE_WINDOW = 0,
	CCTRL_TYPE_PANEL,
	CCTRL_TYPE_PANELSM,						// panel without focus rect and icon
	CCTRL_TYPE_BUTTON,						//params: fHoverPercent, nMsgParam - nMsgParam is sent to handler for processing
	CCTRL_TYPE_LABEL,
	CCTRL_TYPE_BLINKING_LABEL,
	CCTRL_TYPE_CHECKBOX,
	CCTRL_TYPE_SLIDER,
	CCTRL_TYPE_INPUTBOX,
	CCTRL_TYPE_TEXTBOX,
	CCTRL_TYPE_ANIMATION,
	CCTRL_TYPE_DROPDOWN,					//single string, formatted with \n
	CCTRL_TYPE_LISTBOX,						//(special for usernames)
	CCTRL_TYPE_PROGRESS_BAR,
	CCTRL_TYPE_VIGNETTE,
	CCTRL_TYPE_SCROLLMENU,					//menu with scroll. On selection sends INDEX STRING for selected string
	CCTRL_TYPE_WIDEBAR,						//screen wide band for level name (2 frames h tiling: border and filler)
	CCTRL_TYPE_FILLING_STARS,				//3 filling stars. nStars = number of filled stars

	CCTRL_TYPE_SDL_KEYREADER,				//control that reads scancode SDL and shows the name of the pressed key
	CCTRL_TYPE_LIST_SELECTOR,				// Shows a list (no pages) (\n formatted string). second string is right aligned. permits selection
	CCTRL_TYPE_LIST_SELECTOR_TRUETYPE,		// Paged list, TT fonts. params: nSelectedIdx, nOptionsCnt

	CCTRL_TYPE_NET_VOTE,					//shows a list with peers selections on networked games (near buttons that need player cooperation)

	CCTRL_TYPE_XP_BAR,						//experience bar (player XP upgrades)
	CCTRL_TYPE_PLAYER_UPGRADE_CONTROL,		//game specific class upgrade stuff
	CCTRL_TYPE_SCORESLIST_TRUETYPE,			//truetype leaderboards list
	CCTRL_TYPE_TASKS_LIST,					//shows 2 level tasks
	CCTRL_TYPE_SLIDER_PAGES,				//only shows left right arrows, no center (uses slider animations)

	CCTRL_TYPES_COUNT,
};

// ordinea de aici trebuie sa corespunda cu ordinea din CCTRL_TYPE
CStringHash EControlTypeNames[] =
{
	L"Window",
	L"Panel",
	L"PanelSM",
	L"Button",
	L"Label",
	L"BlinkingLabel",
	L"Checkbox",
	L"Slider",
	L"Inputbox",
	L"Textbox",
	L"Animation",
	L"Dropdown",
	L"Listbox",
	L"ProgressBar",
	L"Vignette",
	L"ScrollMenu",
	L"Widebar",
	L"FillingStars",
	L"SDL_KeyReader",
	L"ListSelector",
	L"ListSelectorTT",
	L"NetVote",
	L"XPBar",
	L"PlayerUpgradeControl",
	L"ScoresListTT",
	L"TasksList",
	L"SliderPages"
};

void CControl::SetManagersPtr( CSpriteCollection* sprCol )
{
	m_pSprCol = sprCol;
}

CControl::CControl( const WCHAR* typeName )
{
	statusFlags = 0;
	bDisabled = false;
	fDisabledPercent = 0.0f;
	bVisible = true;
	bCanHaveFocus = false;
	bShowFocusCursor = true;
	fFocusPercent = 0.0f;
	paramsDict.DeleteAll();
	bbox.Set( 0, 0, 0, 0 );
	type = ( EControlType ) GetListIndexByName( typeName, EControlTypeNames, CCTRL_TYPES_COUNT );
}

CControl::CControl( CControl* ctrl )
{
	*this = *ctrl;
	this->paramsDict = ctrl->paramsDict;
}

CControl::~CControl()
{
}

RectXYWHi CControl::GetBBox()
{
	return bbox;
}

void CControl::OnFocused( int nFocusDirection )
{
	switch ( type )
	{
		case CCTRL_TYPE_LIST_SELECTOR_TRUETYPE:
		case CCTRL_TYPE_LIST_SELECTOR:
		{
			if ( nFocusDirection == 0 )
				return;

			bool bUserCanSelect = paramsDict.GetVariantByName( L"bUserCanSelect" )->m_asBool;
			int nOptionsCnt = paramsDict.GetVariantByName( L"nOptionsCnt" )->m_asINT32;

			if ( ( bUserCanSelect ) && ( nOptionsCnt > 0 ) )
			{
				if ( nFocusDirection > 0 )
				{
					paramsDict.SetNamedVarINT32( L"nSelectedIdx", 0 );
				}
				else
				{
					paramsDict.SetNamedVarINT32( L"nSelectedIdx", nOptionsCnt - 1 );
				}
			}
		}
		break;
		case CCTRL_TYPE_PLAYER_UPGRADE_CONTROL:
		{
			//too hard and not so important
		}
		break;
	}
}

void CControl::Reset()
{
	//generic params
	paramsDict.DeleteAll();

	bbox.Set( 0, 0, 0, 0 );

	statusFlags = 0;
	bDisabled = false;
	fDisabledPercent = 0.0f;
	bVisible = true;
	bCanHaveFocus = false;
	bShowFocusCursor = true;

	switch ( type )
	{
		case CCTRL_TYPE_DROPDOWN:
		{
			bCanHaveFocus = true;
			bShowFocusCursor = true;

			paramsDict.SetNamedVarINT32( L"nSelectedIdx", 0 );
			paramsDict.SetNamedVarINT32( L"nItemsCnt", 0 );
		}
		break;
		case CCTRL_TYPE_FILLING_STARS:
		{
			paramsDict.SetNamedVarFloat( L"fTimerStars", 0.0f );
		}
		break;
		case CCTRL_TYPE_LIST_SELECTOR_TRUETYPE:
		case CCTRL_TYPE_LIST_SELECTOR:
		{
			bCanHaveFocus = true;
			bShowFocusCursor = true;

			paramsDict.SetNamedVarINT32( L"nPage", 0 );
			paramsDict.SetNamedVarINT32( L"nSelectedIdx", -1 );
			paramsDict.SetNamedVarINT32( L"nOptionsCnt", 0 );
			//can user select rows? on by default
			paramsDict.SetNamedVarBool( L"bUserCanSelect", true );
		}
		break;
		case CCTRL_TYPE_SCORESLIST_TRUETYPE:
		{
			bCanHaveFocus = false;
			bShowFocusCursor = false;
		}
		break;
		case CCTRL_TYPE_PLAYER_UPGRADE_CONTROL:
		{
			bCanHaveFocus = true;
			bShowFocusCursor = false;

			paramsDict.SetNamedVarINT32( L"nPlayerOrdinal", -1 );   //player ordinal (0 or 1)

			paramsDict.SetNamedVarINT32( L"nSelectedLine", 0 );
			paramsDict.SetNamedVarINT32( L"nSelectedPoint", 0 );

			//XP points
			for ( int ll = 0; ll < K_PSS_UPGRADE_BARS_CNT; ll++ )
			{
				WCHAR strParamName[ MAX_PATH ];
				StringCchPrintf( strParamName, MAX_PATH, L"spent_bar%d", ll ); //values can be negative too
				paramsDict.SetNamedVarINT32( strParamName, 0 );
			}
		}
		break;
		case CCTRL_TYPE_SCROLLMENU:
		{
			bCanHaveFocus = true;
			bShowFocusCursor = false;

			paramsDict.SetNamedVarFloat( L"fSelectionCursor", 0.0f );
			paramsDict.SetNamedVarINT32( L"nSelectedIdx", 0 );
			paramsDict.SetNamedVarINT32( L"nTextAlignFlags", 0 );
			paramsDict.SetNamedVarINT32( L"Vspacing", 20 );
			paramsDict.SetNamedVarINT32( L"disabledFlags", 0 );
		}
		break;
		case CCTRL_TYPE_BUTTON:
		{
			bCanHaveFocus = true;

			paramsDict.SetNamedVarFloat( L"fHoverPercent", 0.0f );
			paramsDict.SetNamedVarINT32( L"nMsgParam", 0 );
		}
		break;
		case CCTRL_TYPE_BLINKING_LABEL:
		{
			paramsDict.SetNamedVarINT32( L"nTextAlignFlags", 0 );
			paramsDict.SetNamedVarFloat( L"fTimer", 0.0f );
		}
		break;
		case CCTRL_TYPE_SDL_KEYREADER:
		{
			paramsDict.SetNamedVarString( L"sKeyName", L"?" );
			//tinem minte selectia anterioara ca sa vedem cand se schimba
			paramsDict.SetNamedVarINT32( L"nSDLscancode_old", -1 );
		}
		break;
		case CCTRL_TYPE_LABEL:
		{
			paramsDict.SetNamedVarINT32( L"nTextAlignFlags", 0 );
		}
		break;
		case CCTRL_TYPE_NET_VOTE:
		{
		}
		break;
		case CCTRL_TYPE_ANIMATION:
		{
		}
		break;
		case CCTRL_TYPE_SLIDER:
		{
			bCanHaveFocus = true;

			paramsDict.SetNamedVarFloat( L"fSlidePercent", 0.0f );
		}
		break;
		case CCTRL_TYPE_SLIDER_PAGES:
		{
			bCanHaveFocus = true;

			paramsDict.SetNamedVarINT32( L"nPage", 0 );
		}
		break;
		case CCTRL_TYPE_CHECKBOX:
		{
			bCanHaveFocus = true;

			paramsDict.SetNamedVarBool( L"bChecked", false );
			paramsDict.SetNamedVarFloat( L"fHoverPercent", 0.0f );
		}
		break;
		case CCTRL_TYPE_PROGRESS_BAR:
		{
			paramsDict.SetNamedVarFloat( L"fProgress", 100.0f );
		}
		break;
		case CCTRL_TYPE_XP_BAR:
		{
			//fProgress is used to interpolate from oldXP to newXP (from negative to 0 it just waits)
			paramsDict.SetNamedVarFloat( L"fProgress", -1.0f );
		}
		break;
		case CCTRL_TYPE_INPUTBOX:
		{
			bCanHaveFocus = true;

			paramsDict.SetNamedVarINT32( L"nTextLen", 5 );
			paramsDict.SetNamedVarString( L"sInputText", L"Player" );
		}
		break;
		case CCTRL_TYPE_LISTBOX:
		{
			paramsDict.SetNamedVarINT32( L"nSelIdx", 0 );
			paramsDict.SetNamedVarINT32( L"nSelIdxMax", 0 );
		}
		break;
	}
}

void CControl::Initialize()
{
	switch ( type )
	{
		case CCTRL_TYPE_SCORESLIST_TRUETYPE:
		{
		}
		break;
		case CCTRL_TYPE_LIST_SELECTOR:
		{
			//save number of options now
			int stringIdx = -1;
			int fontIdx = -1;
			CVariantComplex * var = paramsDict.GetVariantByName( L"stringID" );
			if ( var->m_type != CVariantComplex::K_ARGTYPE_NONE )
			{
				stringIdx = var->m_asINT32;
			}
			var = paramsDict.GetVariantByName( L"fontID" );
			if ( var->m_type != CVariantComplex::K_ARGTYPE_NONE )
			{
				fontIdx = var->m_asINT32;
			}

			if ( ( stringIdx >= 0 ) && ( fontIdx >= 0 ) )
			{
				int optcnt = __Texts().GetSubstringsCount( stringIdx, L'\n' );
				int rowh = ( __TexFonts().fonts[ fontIdx ]->rowHeight + __TexFonts().fonts[ fontIdx ]->rowSpacing );
				//save options count
				paramsDict.SetNamedVarINT32( L"nRowHeight", rowh );
				paramsDict.SetNamedVarINT32( L"nOptionsCnt", optcnt );
			}
			else
			{
				paramsDict.SetNamedVarINT32( L"nRowHeight", 10 );
				paramsDict.SetNamedVarINT32( L"nOptionsCnt", 1 );
			}
			paramsDict.SetNamedVarINT32( L"nSelectedIdx", -1 );
			paramsDict.SetNamedVarBool( L"bUserCanSelect", true );
		}
		break;
		case CCTRL_TYPE_LIST_SELECTOR_TRUETYPE:
		{
			//save number of options now
			int stringIdx = -1;
			int fontIdx = -1;
			CVariantComplex * var = paramsDict.GetVariantByName( L"stringID" );
			if ( var->m_type != CVariantComplex::K_ARGTYPE_NONE )
			{
				stringIdx = var->m_asINT32;
			}

			if ( stringIdx >= 0 )
			{
				CTTFont* pTTFont = UTGetTTFManager().GetFont( shTTFID_SZ20.textHash );
				int rowh = 10;
				if ( ( pTTFont != null ) && ( pTTFont->pFont ) )
				{
					D3DXFONT_DESCW descW;
					pTTFont->pFont->GetDesc( &descW );
					rowh = descW.Height;
				}

				int optcnt = __Texts().GetSubstringsCount( stringIdx, L'\n' );
				paramsDict.SetNamedVarINT32( L"nRowHeight", rowh );
				//save options count
				paramsDict.SetNamedVarINT32( L"nOptionsCnt", optcnt );
			}
			else
			{
				paramsDict.SetNamedVarINT32( L"nRowHeight", 10 );
				paramsDict.SetNamedVarINT32( L"nOptionsCnt", 0 );
			}
			paramsDict.SetNamedVarINT32( L"nSelectedIdx", -1 );
			paramsDict.SetNamedVarBool( L"bUserCanSelect", true );
		}
		break;

		case CCTRL_TYPE_PLAYER_UPGRADE_CONTROL:
		{
		}
		break;

	}
}

void CControl::Update( float dTime, float fTimeline )
{
	RectXYWHi BBox, BBox_inflated;
	int inflate = 0; 
	int animIdx = -1;
	int stringIdx = -1;
	int fontIdx = -1;

	BBox = bbox;
	BBox_inflated = bbox;

	CVariantComplex* varInflate = paramsDict.GetVariantByName( L"inflate" );
	if ( varInflate->m_type != CVariantComplex::K_ARGTYPE_NONE )
	{
		inflate = varInflate->m_asINT32;
		if ( inflate != 0 )
		{
			BBox_inflated.x += inflate;
			BBox_inflated.y += inflate;
			BBox_inflated.w -= 2 * inflate;
			BBox_inflated.h -= 2 * inflate;
		}
	}

	CVariantComplex* var = paramsDict.GetVariantByName( L"animID" );
	if ( var->m_type != CVariantComplex::K_ARGTYPE_NONE )
	{
		animIdx = var->m_asINT32;
	}
	var = paramsDict.GetVariantByName( L"fontID" );
	if ( var->m_type != CVariantComplex::K_ARGTYPE_NONE )
	{
		fontIdx = var->m_asINT32;
	}
	var = paramsDict.GetVariantByName( L"stringID" );
	if ( var->m_type != CVariantComplex::K_ARGTYPE_NONE )
	{
		stringIdx = var->m_asINT32;
	}
	// disabled percentage
	if ( bDisabled ) 
	{
		inc_limit( fDisabledPercent, 10.0f * dTime, 1.0f );
	}
	else
	{
		dec_limit( fDisabledPercent, 10.0f * dTime, 0.0f );
	}

	switch ( type )
	{
		case CCTRL_TYPE_ANIMATION:
		{
		}
		break;
		case CCTRL_TYPE_DROPDOWN:
		{
			if ( bDisabled )
				break;

			int nSelectedIdx = paramsDict.GetVariantByName( L"nSelectedIdx" )->m_asINT32;
			int nItemsCnt = 0;
			CVariantComplex* vc = paramsDict.GetVariantByName( L"stringID_list" );
			if ( (vc->m_type == CVariantComplex::K_ARGTYPE_STRING) && (!vc->m_strArg.IsEmpty()) )
			{
				int nStringIdx_list = __Texts().GetStrIdx( vc->m_strArg.textHash );
				nItemsCnt = __Texts().GetSubstringsCount( nStringIdx_list, L'\n' );
			}
			//left button
			RectXYWHi bbL = m_pSprCol->GetAFrameBBox( animIdx, 1 );
			bbL.x = BBox.x; bbL.y = BBox.Bottom() - bbL.h;
			//right button
			RectXYWHi bbR = m_pSprCol->GetAFrameBBox( animIdx, 2 );
			bbR.x = BBox.Right() - bbR.w; bbR.y = BBox.Bottom() - bbR.h;
			RectXYWHi bbB( BBox.x + bbL.w, BBox.Bottom() - bbL.h, BBox.w - bbL.w - bbR.w, bbL.h );

			//tratam mai intai inputurile pe flaguri in cazul in care vin din handleCommand
			int nSelectedIdxNew = nSelectedIdx;

			//setam flaguri noi pentru frame-ul urmator
			if ( g_mouse.Lbut == K_MOUSE_BUTT_JUSTPRESSED )
			{
				if ( ( Rects::PointInRect( &layer->mouseRelPos, &bbL ) ) & ( nSelectedIdx > 0 ) )
				{
					statusFlags |= CCTRL_STATUS_FLAG_CLICKEDLEFT;
				}
				else if ( ( Rects::PointInRect( &layer->mouseRelPos, &bbR ) ) && ( nSelectedIdx < nItemsCnt - 1 ) )
				{
					statusFlags |= CCTRL_STATUS_FLAG_CLICKEDRIGHT;
				}
			}

			//--- efectuam actiunea in fn de flaguri si facem clear la flags ---
			if ( statusFlags & CCTRL_STATUS_FLAG_CLICKEDLEFT )
			{
				nSelectedIdxNew--;
				statusFlags &= ~CCTRL_STATUS_FLAG_CLICKEDLEFT;
				SND_PLAY( SNDIDX_CLICK );
			}
			if ( statusFlags & CCTRL_STATUS_FLAG_CLICKEDRIGHT )
			{
				nSelectedIdxNew++;
				statusFlags &= ~CCTRL_STATUS_FLAG_CLICKEDRIGHT;
				SND_PLAY( SNDIDX_CLICK );
			}


			CLAMP( nSelectedIdxNew, 0, nItemsCnt - 1 );

			if ( nSelectedIdxNew != nSelectedIdx )
			{
				layer->FocusControl( this );
				nSelectedIdx = nSelectedIdxNew;

				CEvent *nevent = new CEvent( CEventTypes::evtT_CONTROLS, CEventCommands::evtC_CONTROLS_SELECTION_CHANGED );
				nevent->AddNamedArgUINT32( L"layerID", layer->ID.getHash() );
				CVariantComplex* lvar = paramsDict.GetVariantByName( L"ID" );
				if ( lvar->m_type != CVariantComplex::K_ARGTYPE_NONE )
					nevent->AddNamedArgUINT32( L"ctrlID", lvar->m_strArg.getHash() );
				else
					nevent->AddNamedArgUINT32( L"ctrlID", 0 );

				//add new selection param
				nevent->AddNamedArgINT32( L"nSelectedIdx", nSelectedIdx );
				UTGetEventManager().QueueEvent( nevent );
			}

			//refresh params
			paramsDict.SetNamedVarINT32( L"nSelectedIdx", nSelectedIdx );
			paramsDict.SetNamedVarINT32( L"nItemsCnt", nItemsCnt );
		}
		break;
		case CCTRL_TYPE_FILLING_STARS:
		{
			int nStars = paramsDict.GetVariantByName( L"nStars" )->m_asINT32;
			float fTimer = paramsDict.GetVariantByName( L"fTimerStars" )->m_asFloat;

			float fTimerOld = fTimer;
			inc_limit( fTimer, dTime * 2.0f, ( float ) nStars );

			if ( floor( fTimerOld ) != floor( fTimer ) )
			{
				SND_PLAY( SNDIDX_STARHIT );
				//generate particles
				RectXYWHi starrect = m_pSprCol->GetAFrameBBox( animIdx, 0 );
				Vec2 vStartPos( bbox.CenterX(), bbox.CenterY() );
				vStartPos.x -= starrect.w;
				vStartPos.x += starrect.w * floor( fTimerOld );
				g_particlesMgr.GenerateStarEffect( vStartPos, K_PART_LAYER_CONTROLS_LIGHT );
			}

			//save timer
			paramsDict.SetNamedVarFloat( L"fTimerStars", fTimer );
		}
		break;
		case CCTRL_TYPE_XP_BAR:
		{
			int nOldVal = paramsDict.GetVariantByName( L"nOldValue" )->m_asINT32;
			int nNewVal = paramsDict.GetVariantByName( L"nNewValue" )->m_asINT32;
			float fProgress = paramsDict.GetVariantByName( L"fProgress" )->m_asFloat;
			if ( nOldVal == nNewVal )
				fProgress = 1.0f;

			float fProgressOld = fProgress;
			int nCurrentVal_old = nOldVal + ( int ) floor( fProgressOld * ( nNewVal - nOldVal ) );
			int nCurrentLevel_old = App_GetXPLevel( nCurrentVal_old );

			inc_limit( fProgress, dTime, 1.0f );
			//save back timer
			paramsDict.SetNamedVarFloat( L"fProgress", fProgress );
			if ( fProgress < 0.0f )
				break;

			int nCurrentVal = nOldVal + ( int ) floor( fProgress * ( nNewVal - nOldVal ) );
			int nCurrentLevel = App_GetXPLevel( nCurrentVal );

			//generate particles
			if ( nCurrentLevel > nCurrentLevel_old )
			{
				//chevron bbox
				RectXYWHi rct = m_pSprCol->GetAFrameBBox( animIdx, 8 );
				Vec2 vStartPos( bbox.x, bbox.y + bbox.h / 2 );
				vStartPos.x = vStartPos.x + rct.x + rct.w / 2;
				vStartPos.y = vStartPos.y + rct.y + rct.h / 2;
				g_particlesMgr.GenerateStarEffect( vStartPos, K_PART_LAYER_CONTROLS_LIGHT );

				//				SND_PLAY(SNDIDX_UI_LEVELUP);
			}
		}
		break;
		case CCTRL_TYPE_SDL_KEYREADER:
		{
			int nKeyOld = paramsDict.GetVariantByName( L"nSDLscancode_old" )->m_asINT32;
			int nKeycode = paramsDict.GetVariantByName( L"nSDLscancode" )->m_asINT32;
			if ( nKeycode != nKeyOld )
			{
				//update inner string
				CHAR strKeys[ MAX_PATH ];
				WCHAR wstrKeys[ MAX_PATH ];
				StringCchPrintfA( strKeys, MAX_PATH, "%s", SDL_GetScancodeName( ( SDL_Scancode ) nKeycode ) );
				mbstowcs( wstrKeys, strKeys, MAX_PATH );
				paramsDict.SetNamedVarString( L"sKeyName", wstrKeys );

				//update old scancode
				paramsDict.SetNamedVarINT32( L"nSDLscancode_old", nKeycode );

				//send message selection changed
				CEvent *nevent = new CEvent( CEventTypes::evtT_CONTROLS, CEventCommands::evtC_CONTROLS_SELECTION_CHANGED );
				nevent->AddNamedArgUINT32( L"layerID", layer->ID.getHash() );
				CVariantComplex* lvar = paramsDict.GetVariantByName( L"ID" );
				if ( lvar->m_type != CVariantComplex::K_ARGTYPE_NONE )
					nevent->AddNamedArgUINT32( L"ctrlID", lvar->m_strArg.getHash() );
				else
					nevent->AddNamedArgUINT32( L"ctrlID", 0 );
				//add custom data - trimit scancode
				nevent->AddNamedArgINT32( L"nSDLscancode", nKeycode );
				//pass on received params (added when showing the control)
				nevent->AddNamedArgINT32( L"nKeyboardOrdinal", paramsDict.GetVariantByName( L"nKeyboardOrdinal" )->m_asINT32 );
				nevent->AddNamedArgINT32( L"nSDLcommand", paramsDict.GetVariantByName( L"nSDLcommand" )->m_asINT32 );

				UTGetEventManager().QueueEvent( nevent );

			}
		}
		break;
		case CCTRL_TYPE_BLINKING_LABEL:
		{
			float fTimer = paramsDict.GetVariantByName( L"fTimer" )->m_asFloat;
			float fLoopTimer = paramsDict.GetVariantByName( L"timerLoop" )->m_asFloat;

			fTimer += dTime;
			if ( fTimer >= fLoopTimer )
				fTimer = 0.0f;

			paramsDict.SetNamedVarFloat( L"fTimer", fTimer );
		}
		break;
		case CCTRL_TYPE_LIST_SELECTOR_TRUETYPE:
		case CCTRL_TYPE_LIST_SELECTOR:
		{
			if ( bDisabled )
				break;

			bool bUserCanSelect = paramsDict.GetVariantByName( L"bUserCanSelect" )->m_asBool;
			int	selectedIdx = paramsDict.GetVariantByName( L"nSelectedIdx" )->m_asINT32;
			int optcnt = paramsDict.GetVariantByName( L"nOptionsCnt" )->m_asINT32;
			int nMinPage = paramsDict.GetVariantByName( L"nMinPage" )->m_asINT32;
			int nMaxPage = paramsDict.GetVariantByName( L"nMaxPage" )->m_asINT32;
			int nPage = paramsDict.GetVariantByName( L"nPage" )->m_asINT32;
			int nOldPage = nPage;

			int rowh = 10;
			if ( fontIdx >= 0 )
				rowh = ( __TexFonts().fonts[ fontIdx ]->rowHeight + __TexFonts().fonts[ fontIdx ]->rowSpacing );
			//treat controller input
			if ( bUserCanSelect )
			{
				if ( statusFlags & CCTRL_STATUS_FLAG_CLICKEDUP )
				{
					if ( selectedIdx > 0 )
					{
						selectedIdx--;
						statusFlags &= ~CCTRL_STATUS_FLAG_CLICKEDUP;
					}
				}
				if ( statusFlags & CCTRL_STATUS_FLAG_CLICKEDDOWN )
				{
					if ( selectedIdx < optcnt - 1 )
					{
						selectedIdx++;
						statusFlags &= ~CCTRL_STATUS_FLAG_CLICKEDDOWN;
					}
				}
			}
			//page shuffling
			if ( statusFlags & CCTRL_STATUS_FLAG_CLICKEDLEFT )
			{
				nPage--;
				statusFlags &= ~CCTRL_STATUS_FLAG_CLICKEDLEFT;
			}
			if ( statusFlags & CCTRL_STATUS_FLAG_CLICKEDRIGHT )
			{
				nPage++;
				statusFlags &= ~CCTRL_STATUS_FLAG_CLICKEDRIGHT;
			}

			RectXYWHi bbL = m_pSprCol->GetAFrameBBox( ANM_CONTROLS_SPR_ARROWS3, 0 ); //left
			bbL.x += BBox.x; bbL.y += BBox.CenterY();

			RectXYWHi bbR = m_pSprCol->GetAFrameBBox( ANM_CONTROLS_SPR_ARROWS3, 2 ); //right
			bbR.x += BBox.Right(); bbR.y += BBox.CenterY();

			if ( g_mouse.Lbut == K_MOUSE_BUTT_JUSTPRESSED )
			{
				if ( Rects::PointInRect( &layer->mouseRelPos, &BBox_inflated ) )
				{
					//request focus
					layer->FocusControl( this );

					if ( bUserCanSelect )
					{
						int selMouse = ( layer->mouseRelPos.y - BBox_inflated.y ) / rowh;
						if ( selectedIdx == selMouse ) //already selected, engage
						{
							statusFlags |= CCTRL_STATUS_FLAG_CLICKED;
						}
						//reinforce selection
						selectedIdx = selMouse;
						CLAMP( selectedIdx, 0, optcnt - 1 );
					}
				}
				//scroll arrows
				if ( Rects::PointInRect( &layer->mouseRelPos, &bbL ) )
				{
					SND_PLAY( SNDIDX_CLICK );
					statusFlags |= CCTRL_STATUS_FLAG_CLICKEDLEFT;
				}
				else if ( Rects::PointInRect( &layer->mouseRelPos, &bbR ) )
				{
					SND_PLAY( SNDIDX_CLICK );
					statusFlags |= CCTRL_STATUS_FLAG_CLICKEDRIGHT;
				}
			}

			CLAMP( nPage, nMinPage, nMaxPage );

			//changed page
			if ( nPage != nOldPage )
			{
				CEvent *nevent = new CEvent( CEventTypes::evtT_CONTROLS, CEventCommands::evtC_CONTROLS_PAGE_CHANGED );
				nevent->AddNamedArgUINT32( L"layerID", layer->ID.getHash() );
				CVariantComplex* lvar = paramsDict.GetVariantByName( L"ID" );
				if ( lvar->m_type != CVariantComplex::K_ARGTYPE_NONE )
					nevent->AddNamedArgUINT32( L"ctrlID", lvar->m_strArg.getHash() );
				else
					nevent->AddNamedArgUINT32( L"ctrlID", 0 );

				//add new selection param
				nevent->AddNamedArgINT32( L"nPageIdx", nPage );
				nevent->AddNamedArgINT32( L"nOldIdx", nOldPage );
				UTGetEventManager().QueueEvent( nevent );
			}

			//selecting something with fire control 
			if ( ( statusFlags & CCTRL_STATUS_FLAG_CLICKED ) && ( bUserCanSelect ) )
			{
				int	baseIndex = paramsDict.GetVariantByName( L"nBaseIndex" )->m_asINT32;
				statusFlags &= ~CCTRL_STATUS_FLAG_CLICKED;

				CEvent *nevent = new CEvent( CEventTypes::evtT_CONTROLS, CEventCommands::evtC_CONTROLS_SELECTION_CHANGED );
				nevent->AddNamedArgUINT32( L"layerID", layer->ID.getHash() );
				CVariantComplex* lvar = paramsDict.GetVariantByName( L"ID" );
				if ( lvar->m_type != CVariantComplex::K_ARGTYPE_NONE )
					nevent->AddNamedArgUINT32( L"ctrlID", lvar->m_strArg.getHash() );
				else
					nevent->AddNamedArgUINT32( L"ctrlID", 0 );

				//add new selection param
				nevent->AddNamedArgINT32( L"nSelectedIdx", selectedIdx + baseIndex );
				nevent->AddNamedArgINT32( L"nPageIdx", nPage );
				UTGetEventManager().QueueEvent( nevent );
			}

			paramsDict.SetNamedVarINT32( L"nSelectedIdx", selectedIdx );
			paramsDict.SetNamedVarINT32( L"nPage", nPage );
		}
		break;

		case CCTRL_TYPE_SCORESLIST_TRUETYPE:
		{
		}
		break;

		case CCTRL_TYPE_PLAYER_UPGRADE_CONTROL:
		{
			int nPlayerOrdinal = paramsDict.GetVariantByName( L"nPlayerOrdinal" )->m_asINT32;
			if ( ( nPlayerOrdinal < 0 ) || ( nPlayerOrdinal > 1 ) )
				break;
			int nPlayerClass = ( int ) g_playerSelScr.m_arrPlayers[ nPlayerOrdinal ].eType;

			//compute local XP points
			int nXPPointsReal = App_GetAvailableXPPoints( ( EPSSPlayerClass ) nPlayerClass ); //real number of XP points to spend
			int nXPPointsLocal = nXPPointsReal;
			int nSpentPoints[ K_PSS_UPGRADE_BARS_CNT ];
			for ( int ll = 0; ll < K_PSS_UPGRADE_BARS_CNT; ll++ )
			{
				WCHAR strParamName[ MAX_PATH ];
				StringCchPrintf( strParamName, MAX_PATH, L"spent_bar%d", ll );
				nSpentPoints[ ll ] = paramsDict.GetVariantByName( strParamName )->m_asINT32;
				//compute local XP points
				nXPPointsLocal -= nSpentPoints[ ll ];
			}

			//compute number of points spent on own bars (first 2 are the team bars, always)
			int nOwnBarsSpent = 0;
			for ( int ll = 2; ll < K_PSS_UPGRADE_BARS_CNT; ll++ )
			{
				int nBarIdx = g_playerSelScr.arrItemsByClass[ nPlayerClass ].arrUpgradeBarsIdx[ ll ];
				int nFilledReal = g_playerSelScr.m_arrPlayers[ nPlayerOrdinal ].arrUpgradeBarsPts[ ll ];
				nOwnBarsSpent += nFilledReal;
			}

			int nCurrentLevel = App_GetXPLevel( g_playerSelScr.m_arrPlayers[ nPlayerOrdinal ].nPlayerXPPts );
			int nTotalPoints = K_GAME_XPPOINTS_PER_XPLEVEL * nCurrentLevel;
			int nTeamSpentPoints = nTotalPoints - nXPPointsReal - nOwnBarsSpent;
			int nTeamSpentPointsLocal = nTeamSpentPoints + ( nSpentPoints[ 0 ] + nSpentPoints[ 1 ] );

			//update available points
			__Texts().SetString( STR_UNUSED_POINTS_VAL, L"%d", nXPPointsLocal );

			int	nSelectedLine = paramsDict.GetVariantByName( L"nSelectedLine" )->m_asINT32;
			int	nSelectedPoint = paramsDict.GetVariantByName( L"nSelectedPoint" )->m_asINT32;
			int nSelectedLine_old = nSelectedLine;
			int nSelectedPoint_old = nSelectedPoint;

			int nBarIdx = g_playerSelScr.arrItemsByClass[ nPlayerClass ].arrUpgradeBarsIdx[ nSelectedLine ];
			int nDots = g_playerSelScr.m_arrUpgradeBars[ nBarIdx ]->nTotalPoints;
			//int nFilledReal = g_userData[K_MEMID_UPGRADE_BAR_POINTS_START + g_playerSelScr.m_arrUpgradeBars[nBarIdx]->nMemSlot];
			int nFilledReal = g_playerSelScr.m_arrPlayers[ nPlayerOrdinal ].arrUpgradeBarsPts[ nSelectedLine ];
			int nFilledLocal = nFilledReal + nSpentPoints[ nSelectedLine ];

			//tratare statusuri setate in receive input
			if ( statusFlags & CCTRL_STATUS_FLAG_CLICKEDUP )
			{
				nSelectedLine--;
				CLAMP( nSelectedLine, 0, K_PSS_UPGRADE_BARS_CNT - 1 );
				nBarIdx = g_playerSelScr.arrItemsByClass[ nPlayerClass ].arrUpgradeBarsIdx[ nSelectedLine ];
				//nFilledReal = g_userData[K_MEMID_UPGRADE_BAR_POINTS_START + g_playerSelScr.m_arrUpgradeBars[nBarIdx]->nMemSlot];
				nFilledReal = g_playerSelScr.m_arrPlayers[ nPlayerOrdinal ].arrUpgradeBarsPts[ nSelectedLine ];
				nFilledLocal = nFilledReal + nSpentPoints[ nSelectedLine ];
				nSelectedPoint = nFilledLocal - 1;

				statusFlags &= ~CCTRL_STATUS_FLAG_CLICKEDUP;
			}
			if ( statusFlags & CCTRL_STATUS_FLAG_CLICKEDDOWN )
			{
				nSelectedLine++;
				CLAMP( nSelectedLine, 0, K_PSS_UPGRADE_BARS_CNT - 1 );
				nBarIdx = g_playerSelScr.arrItemsByClass[ nPlayerClass ].arrUpgradeBarsIdx[ nSelectedLine ];
				//nFilledReal = g_userData[K_MEMID_UPGRADE_BAR_POINTS_START + g_playerSelScr.m_arrUpgradeBars[nBarIdx]->nMemSlot];
				nFilledReal = g_playerSelScr.m_arrPlayers[ nPlayerOrdinal ].arrUpgradeBarsPts[ nSelectedLine ];
				nFilledLocal = nFilledReal + nSpentPoints[ nSelectedLine ];
				nSelectedPoint = nFilledLocal - 1;

				statusFlags &= ~CCTRL_STATUS_FLAG_CLICKEDDOWN;
			}
			if ( statusFlags & CCTRL_STATUS_FLAG_CLICKEDLEFT )
			{
				nSelectedPoint--;
				statusFlags &= ~CCTRL_STATUS_FLAG_CLICKEDLEFT;
			}
			if ( statusFlags & CCTRL_STATUS_FLAG_CLICKEDRIGHT )
			{
				nSelectedPoint++;
				statusFlags &= ~CCTRL_STATUS_FLAG_CLICKEDRIGHT;
			}
			//mouse selection
			if ( g_mouse.Lbut == K_MOUSE_BUTT_JUSTPRESSED )
			{
				RectXYWHi rectSz = m_pSprCol->GetAFrameBBox( animIdx, 0 );
				int nClassRows = 2, nTeamRows = 3; //how many rows for class properties (const)	vs team properties
				int nGroupSpacing = 9; //spacing after class rows
				int nTotalRows = nClassRows + nTeamRows;
				assert( nClassRows + nTeamRows <= K_PSS_UPGRADE_BARS_CNT );
				//team group
				RectXYWHi bbgroup( BBox_inflated.x, BBox_inflated.y, BBox_inflated.w, ( rectSz.h + 1 ) * nClassRows + 1 );

				for ( int ll = 0; ll < nTotalRows; ll++ )
				{
					RectXYWHi bbline = bbgroup;
					bbline.h = rectSz.h;
					bbline.y += ll * ( rectSz.h + 1 );
					if ( ll >= nClassRows )
						bbline.y += nGroupSpacing - 1;
					bbline.Inflate( -2, -2 );
					//start of bar
					Vec2 vPos( bbline.Right() - rectSz.w * nDots + rectSz.w / 2, bbline.CenterY() );

					if ( Rects::PointInRect( &layer->mouseRelPos, &bbline ) )
					{
						//request focus
						layer->FocusControl( this );

						int selMouseLine = ll;
						int selMouseCol = int( layer->mouseRelPos.x + rectSz.w / 2.0f - vPos.x ) / rectSz.w;
						if ( ( nSelectedLine == selMouseLine ) && ( nSelectedPoint == selMouseCol ) ) //already selected, engage
						{
							statusFlags |= CCTRL_STATUS_FLAG_CLICKED;
						}
						//reinforce selection
						nSelectedLine = selMouseLine;
						nSelectedPoint = selMouseCol;
						break;
					}
				}
			}

			CLAMP( nSelectedLine, 0, K_PSS_UPGRADE_BARS_CNT - 1 );
			CLAMP( nSelectedPoint, 0, nDots - 1 );

			//selection changed. Write text description:
			if ( ( nSelectedLine != nSelectedLine_old ) || ( nSelectedPoint != nSelectedPoint_old ) )
			{
				int nlBarIdx = g_playerSelScr.arrItemsByClass[ nPlayerClass ].arrUpgradeBarsIdx[ nSelectedLine ];
				int nStrIdx = g_playerSelScr.m_arrUpgradeBars[ nlBarIdx ]->m_arrPerks[ nSelectedPoint ].nStrIdx_desc;
				if ( nStrIdx == -1 )
					nStrIdx = g_playerSelScr.m_arrUpgradeBars[ nlBarIdx ]->nStrIdx_desc;
				//update perk description
				if ( nStrIdx >= 0 )
					__Texts().SetString_NoParse( STR_SELECTED_PERK_DESC_VAL, __Texts().strings[ nStrIdx ]->sText );
				else
					__Texts().SetString_NoParse( STR_SELECTED_PERK_DESC_VAL, L" " );
			}

			//activated (by mouse or SELECT button)
			if ( statusFlags & CCTRL_STATUS_FLAG_CLICKED )
			{
				statusFlags &= ~CCTRL_STATUS_FLAG_CLICKED;
				//ACTION!
				int nDeltaPointsLocal = nSelectedPoint + 1 - nFilledLocal;
				if ( nDeltaPointsLocal > nXPPointsLocal )
					nDeltaPointsLocal = nXPPointsLocal;
				//click on top of stack, remove top (so you can remove the first dot too)
				if ( ( nDeltaPointsLocal == 0 ) && ( nFilledLocal > 0 ) )
					nDeltaPointsLocal = -1;
				//don't remove more than what you added on the team bars
				if ( nSelectedLine <= 1 )
				{
					if ( ( nDeltaPointsLocal < 0 ) && ( -nDeltaPointsLocal > nTeamSpentPointsLocal ) )
						nDeltaPointsLocal = -max( 0, nTeamSpentPointsLocal );
				}

				//change spent points
				nSpentPoints[ nSelectedLine ] = nFilledLocal + nDeltaPointsLocal - nFilledReal;
				SND_PLAY( SNDIDX_CLICK );
				/*
				//Can't delete points from the TEAM bars if they're already commited
				if ((nFilledLocal + nDeltaPointsLocal - nFilledReal < 0) && (nSelectedLine <= 1))
				{
					SND_PLAY(SNDIDX_DENIED);
					//tutorial for trying to remove from the team bars
					UTGetControlsManager().MessageBoxOK(STR_WARNING, STR_TUTORIAL_XPPOINTS_TEAM_REMOVE);
				}
				else
				{
					//tutorial for adding on the TEAM bars
					if ((g_userData[K_MEMID_TUT_XP_TEAM_ADD] == 0) && (nSelectedLine <= 1))
					{
						UTGetControlsManager().MessageBoxOK(STR_WARNING, STR_TUTORIAL_XPPOINTS_TEAM_ADD);
						g_userData[K_MEMID_TUT_XP_TEAM_ADD] = 1;
					}
					else
					{
						nSpentPoints[nSelectedLine] = nFilledLocal + nDeltaPointsLocal - nFilledReal;
						SND_PLAY(SNDIDX_CLICK);
					}
				}
				*/
			}

			paramsDict.SetNamedVarINT32( L"nSelectedLine", nSelectedLine );
			paramsDict.SetNamedVarINT32( L"nSelectedPoint", nSelectedPoint );
			for ( int ll = 0; ll < K_PSS_UPGRADE_BARS_CNT; ll++ )
			{
				WCHAR strParamName[ MAX_PATH ];
				StringCchPrintf( strParamName, MAX_PATH, L"spent_bar%d", ll ); //values can be negative too
				paramsDict.SetNamedVarINT32( strParamName, nSpentPoints[ ll ] );
			}
		}
		break;

		case CCTRL_TYPE_SCROLLMENU:
		{
			float fSelectionCursor = paramsDict.GetVariantByName( L"fSelectionCursor" )->m_asFloat;
			int	selectedIdx = paramsDict.GetVariantByName( L"nSelectedIdx" )->m_asINT32;
			int vSpacing = paramsDict.GetVariantByName( L"Vspacing" )->m_asINT32;
			int nDisabledFlags = paramsDict.GetVariantByName( L"disabledFlags" )->m_asINT32;

			//tratare statusuri setate in receive input
			if ( statusFlags & CCTRL_STATUS_FLAG_CLICKEDDOWN )
			{
				selectedIdx++;
				statusFlags &= ~CCTRL_STATUS_FLAG_CLICKEDDOWN;
			}
			if ( statusFlags & CCTRL_STATUS_FLAG_CLICKEDUP )
			{
				selectedIdx--;
				statusFlags &= ~CCTRL_STATUS_FLAG_CLICKEDUP;
			}

			// get number of options
			int optcnt = 0;
			for ( int ll = 0; ll < 10; ll++ )
			{
				WCHAR varname[ MAX_PATH ];
				StringCchPrintf( varname, MAX_PATH, L"StringID%d", ll );
				CVariantComplex* vc = paramsDict.GetVariantByName( varname );
				if ( ( vc->m_type == CVariantComplex::K_ARGTYPE_STRING ) && ( !vc->m_strArg.IsEmpty() ) )
				{
					if ( g_mouse.Lbut == K_MOUSE_BUTT_JUSTPRESSED )
					{
						int rowh = vSpacing;
						if ( fontIdx >= 0 )
							rowh = __TexFonts().fonts[ fontIdx ]->rowHeight;

						RectXYWHi optbb( bbox.x, bbox.y + vSpacing * optcnt, bbox.w, vSpacing );
						if ( Rects::PointInRect( &layer->mouseRelPos, &optbb ) )
						{
							//if (selectedIdx == optcnt) //already selected, engage
							{
								statusFlags |= CCTRL_STATUS_FLAG_CLICKED;
							}
							//reinforce selection
							selectedIdx = optcnt;
							fSelectionCursor = selectedIdx;
						}
					}

					optcnt++;
				}
				else
				{
					break; //exit for
				}
			}
			//limitez selectie
			CLAMP( selectedIdx, 0, optcnt - 1 );
			//modific cursor selectie sa se duca spre selectia curenta
			if ( fSelectionCursor < ( float ) selectedIdx )
			{
				fSelectionCursor += dTime * 10.0f;
				if ( fSelectionCursor >= ( float ) selectedIdx )
					fSelectionCursor = ( float ) selectedIdx;
			}
			else if ( fSelectionCursor > ( float )selectedIdx )
			{
				fSelectionCursor -= dTime * 10.0f;
				if ( fSelectionCursor <= ( float ) selectedIdx )
					fSelectionCursor = ( float ) selectedIdx;
			}
			//clicked? - deocamdata mesajul de clicked se da direct din ReceiveInput
			if ( statusFlags & CCTRL_STATUS_FLAG_CLICKED )
			{
				//see if option is disabled
				int nOptFlag = 1 << selectedIdx;
				if ( ( nDisabledFlags & nOptFlag ) == 0 )
				{
					CEvent *nevent = new CEvent( CEventTypes::evtT_CONTROLS, CEventCommands::evtC_CONTROLS_SELECTION_CHANGED );
					nevent->AddNamedArgUINT32( L"layerID", layer->ID.getHash() );
					CVariantComplex* lvar = paramsDict.GetVariantByName( L"ID" );
					if ( lvar->m_type != CVariantComplex::K_ARGTYPE_NONE )
						nevent->AddNamedArgUINT32( L"ctrlID", lvar->m_strArg.getHash() );
					else
						nevent->AddNamedArgUINT32( L"ctrlID", 0 );
					//add custom data - trimit index string selectat
					int nStringIdx = -1;
					//incercam sa citim string idx selectat
					WCHAR varname[ MAX_PATH ];
					StringCchPrintf( varname, MAX_PATH, L"StringID%d", selectedIdx );
					CVariantComplex* vc = paramsDict.GetVariantByName( varname );
					if ( ( vc->m_type == CVariantComplex::K_ARGTYPE_STRING ) && ( !vc->m_strArg.IsEmpty() ) )
					{
						nStringIdx = __Texts().GetStrIdx( vc->m_strArg.textHash );
					}
					//trimitem string idx
					nevent->AddNamedArgINT32( L"nSelectedIdx", nStringIdx );
					UTGetEventManager().QueueEvent( nevent );

					SND_PLAY( SNDIDX_CLICK );
				}
				else
				{
					SND_PLAY( SNDIDX_DENIED );
				}

				//remove clicked flag
				statusFlags &= ~CCTRL_STATUS_FLAG_CLICKED;
			}

			//save back updated variants 
			paramsDict.SetNamedVarFloat( L"fSelectionCursor", fSelectionCursor );
			paramsDict.SetNamedVarINT32( L"nSelectedIdx", selectedIdx );
		}
		break;
		case CCTRL_TYPE_BUTTON:
		{
			float	hoverPercent = paramsDict.GetVariantByName( L"fHoverPercent" )->m_asFloat;

			RectXYWHi movedBB = BBox;

			if ( ( !bDisabled ) && ( !GameState::isTransitioning() ) )
			{
				bool bExecuteClick = false;
				statusFlags &= ~CCTRL_STATUS_FLAG_HOVER;

				if ( Rects::PointInRect( &layer->mouseRelPos, &movedBB ) )
				{
					statusFlags |= CCTRL_STATUS_FLAG_HOVER;
					if ( ( ( statusFlags & CCTRL_STATUS_FLAG_CLICKED ) == 0 ) && ( g_mouse.Lbut == K_MOUSE_BUTT_JUSTPRESSED ) )
					{
						SND_PLAY( SNDIDX_CLICK );
						statusFlags |= CCTRL_STATUS_FLAG_CLICKED;
					}
				}

				if ( ( statusFlags & CCTRL_STATUS_FLAG_HOVER ) == 0 )
				{
					statusFlags &= ~CCTRL_STATUS_FLAG_CLICKED;
				}

				if ( ( ( g_mouse.Lbut == K_MOUSE_BUTT_JUSTRELEASED ) || ( g_mouse.Lbut == K_MOUSE_BUTT_NOTPRESSED ) ) && ( statusFlags & CCTRL_STATUS_FLAG_CLICKED ) )
				{
					statusFlags &= ~CCTRL_STATUS_FLAG_CLICKED;

					bExecuteClick = true;
				}
				// for controller click
				if ( statusFlags & CCTRL_STATUS_FLAG_CLICKED_ALT )
				{
					statusFlags &= ~CCTRL_STATUS_FLAG_CLICKED_ALT;
					statusFlags |= CCTRL_STATUS_FLAG_CLICKED;

					SND_PLAY( SNDIDX_CLICK );
					bExecuteClick = true;
				}

				if ( bExecuteClick )
				{
					layer->FocusControl( this );

					CEvent *nevent = new CEvent( CEventTypes::evtT_CONTROLS, CEventCommands::evtC_CONTROLS_CLICK );
					nevent->AddNamedArgUINT32( L"layerID", layer->ID.getHash() );
					CVariantComplex* lvar = paramsDict.GetVariantByName( L"ID" );
					if ( lvar->m_type != CVariantComplex::K_ARGTYPE_NONE )
						nevent->AddNamedArgUINT32( L"ctrlID", lvar->m_strArg.getHash() );
					else
						nevent->AddNamedArgUINT32( L"ctrlID", 0 );

					//add custom message from XML/interfaces editor
					int		msgParam = paramsDict.GetVariantByName( L"nMsgParamINT32" )->m_asINT32;
					nevent->AddNamedArgINT32( L"nMsgParamINT32", msgParam );
					UINT32	msgParamU = paramsDict.GetVariantByName( L"nMsgParamUINT32" )->m_asUINT32;
					nevent->AddNamedArgUINT32( L"nMsgParamUINT32", msgParamU );

					UTGetEventManager().QueueEvent( nevent );
				}
			}
			else
			{
				statusFlags = 0;
			}
			//hover
			if ( statusFlags & CCTRL_STATUS_FLAG_HOVER )
			{
				inc_limit( hoverPercent, 10.0f * dTime, 1.0f );
			}
			else
			{
				dec_limit( hoverPercent, 10.0f * dTime, 0.0f );
			}

			//save back updated variants 
			paramsDict.SetNamedVarFloat( L"fHoverPercent", hoverPercent );
		}
		break;
		case CCTRL_TYPE_CHECKBOX:
		{
			bool bChecked = paramsDict.GetVariantByName( L"bChecked" )->m_asBool;
			float hoverPercent = paramsDict.GetVariantByName( L"fHoverPercent" )->m_asFloat;

			RectXYWHi frameBB = m_pSprCol->GetAFrameBBox( animIdx, 0 );
			frameBB.x += BBox.x; frameBB.y += BBox.y;

			bool bCheckChanged = false;

			if ( Rects::PointInRect( &layer->mouseRelPos, &frameBB ) )
			{
				statusFlags |= CCTRL_STATUS_FLAG_HOVER;
				if ( g_mouse.Lbut == K_MOUSE_BUTT_JUSTPRESSED )
				{
					statusFlags &= ~CCTRL_STATUS_FLAG_HOVER;
					bCheckChanged = true;
				}
			}
			else
			{
				statusFlags &= ~CCTRL_STATUS_FLAG_HOVER;
			}
			//trateaza activarea din taste
			if ( statusFlags & CCTRL_STATUS_FLAG_CLICKED )
			{
				bCheckChanged = true;
				statusFlags &= ~CCTRL_STATUS_FLAG_CLICKED;
			}

			if ( statusFlags & CCTRL_STATUS_FLAG_HOVER )
			{
				inc_limit( hoverPercent, dTime * 8.0f, 1.0f );
			}
			else
			{
				dec_limit( hoverPercent, dTime * 8.0f, 0.0f );
			}

			//--- send messaje on check change ---
			if ( bCheckChanged )
			{
				layer->FocusControl( this );
				SND_PLAY( SNDIDX_CLICK );
				//toggle check
				bChecked = !bChecked;

				CEvent *nevent = new CEvent( CEventTypes::evtT_CONTROLS, CEventCommands::evtC_CONTROLS_CHECK_CHANGED );
				nevent->AddNamedArgUINT32( L"layerID", layer->ID.getHash() );
				CVariantComplex* lvar = paramsDict.GetVariantByName( L"ID" );
				if ( lvar != NULL )
					nevent->AddNamedArgUINT32( L"ctrlID", lvar->m_strArg.getHash() );
				else
					nevent->AddNamedArgUINT32( L"ctrlID", 0 );
				nevent->AddNamedArgBool( L"bChecked", bChecked );
				UTGetEventManager().QueueEvent( nevent );
			}

			paramsDict.SetNamedVarFloat( L"fHoverPercent", hoverPercent );
			paramsDict.SetNamedVarBool( L"bChecked", bChecked );
		}
		break;
		case CCTRL_TYPE_SLIDER:
		{
			float slidePercent = paramsDict.GetVariantByName( L"fSlidePercent" )->m_asFloat;
			bool hasArrows = paramsDict.GetVariantByName( L"hasArrows" )->m_asINT32;
			int nTicks = paramsDict.GetVariantByName( L"Steps" )->m_asINT32;

			float fTickSize = 0.1f;
			if ( nTicks > 0 )
				fTickSize = 1.0f / nTicks;

			//left button
			RectXYWHi bbL = m_pSprCol->GetAFrameBBox( animIdx, 3 ); 
			bbL.x = BBox.x; bbL.y = BBox.Bottom() - bbL.h;
			//right button
			RectXYWHi bbR = m_pSprCol->GetAFrameBBox( animIdx, 4 ); 
			bbR.x = BBox.Right() - bbR.w; bbR.y = BBox.Bottom() - bbR.h;
			RectXYWHi bbB( BBox.x + bbL.w, BBox.Bottom() - bbL.h, BBox.w - bbL.w - bbR.w, bbL.h );

			// treat flag input first maybe they come from handleCommand
			float newSlidePercent = slidePercent;

			if ( g_mouse.Lbut == K_MOUSE_BUTT_JUSTPRESSED )
			{
				//clickuri
				if ( hasArrows )
				{
					if ( Rects::PointInRect( &layer->mouseRelPos, &bbL ) )
					{
						SND_PLAY( SNDIDX_CLICK );
						statusFlags |= CCTRL_STATUS_FLAG_CLICKEDLEFT;
					}
					else if ( Rects::PointInRect( &layer->mouseRelPos, &bbR ) )
					{
						SND_PLAY( SNDIDX_CLICK );
						statusFlags |= CCTRL_STATUS_FLAG_CLICKEDRIGHT;
					}
				}
			}

			if ( ( g_mouse.Lbut == K_MOUSE_BUTT_JUSTPRESSED ) || ( g_mouse.Lbut == K_MOUSE_BUTT_DRAG ) )
			{
				if ( Rects::PointInRect( &layer->mouseRelPos, &bbB ) )
				{
					statusFlags |= CCTRL_STATUS_FLAG_CLICKED;

					if ( g_mouse.Lbut == K_MOUSE_BUTT_JUSTPRESSED )
						SND_PLAY( SNDIDX_CLICK );
				}
			}

			//--- efectuam actiunea in fn de flaguri si facem clear la flags ---
			if ( statusFlags & CCTRL_STATUS_FLAG_CLICKEDLEFT )
			{
				newSlidePercent -= fTickSize;
				statusFlags &= ~CCTRL_STATUS_FLAG_CLICKEDLEFT;
			}
			if ( statusFlags & CCTRL_STATUS_FLAG_CLICKEDRIGHT )
			{
				newSlidePercent += fTickSize;
				statusFlags &= ~CCTRL_STATUS_FLAG_CLICKEDRIGHT;
			}
			// cursor moved
			if ( statusFlags & CCTRL_STATUS_FLAG_CLICKED )
			{
				newSlidePercent = ( float ) ( layer->mouseRelPos.x - bbB.x ) / ( float ) ( bbB.w );
				statusFlags &= ~CCTRL_STATUS_FLAG_CLICKED;
			}


			CLAMP( newSlidePercent, 0.0f, 1.0f );
			//daca avem ticks ne limitam la ele
			if ( nTicks > 0 )
			{
				int full = ( int ) ROUND_FLOAT( newSlidePercent / fTickSize );
				newSlidePercent = full * fTickSize;
				CLAMP( newSlidePercent, 0.0f, 1.0f );
			}

			if ( newSlidePercent != slidePercent )
			{
				layer->FocusControl( this );

				slidePercent = newSlidePercent;

				CEvent *nevent = new CEvent( CEventTypes::evtT_CONTROLS, CEventCommands::evtC_CONTROLS_SLIDER_CHANGED );
				nevent->AddNamedArgUINT32( L"layerID", layer->ID.getHash() );
				CVariantComplex* lvar = paramsDict.GetVariantByName( L"ID" );
				if ( lvar != NULL )
					nevent->AddNamedArgUINT32( L"ctrlID", lvar->m_strArg.getHash() );
				else
					nevent->AddNamedArgUINT32( L"ctrlID", 0 );
				nevent->AddNamedArgFloat( L"fSlidePercent", slidePercent );
				UTGetEventManager().QueueEvent( nevent );
			}

			//refresh params
			paramsDict.SetNamedVarFloat( L"fSlidePercent", slidePercent );
		}
		break;

		case CCTRL_TYPE_SLIDER_PAGES:
		{
			int nPage = paramsDict.GetVariantByName( L"nPage" )->m_asINT32;
			int nMinPage = paramsDict.GetVariantByName( L"nMinPage" )->m_asINT32;
			int nMaxPage = paramsDict.GetVariantByName( L"nMaxPage" )->m_asINT32;


			RectXYWHi bbB = BBox; //bbox bar
			//heads sizes
			int w1 = m_pSprCol->GetAFrameBBox( animIdx, 0 ).w;
			int w2 = m_pSprCol->GetAFrameBBox( animIdx, 2 ).w;
			bbB.x += w1; bbB.w -= w1 + w2;

			RectXYWHi bbL = m_pSprCol->GetAFrameBBox( animIdx, 5 ); //buton stanga
			bbL.x += BBox.x; bbL.y += BBox.CenterY();

			RectXYWHi bbR = m_pSprCol->GetAFrameBBox( animIdx, 6 ); //buton dreapta
			bbR.x += BBox.Right(); bbR.y += BBox.CenterY();

			//setam flaguri noi pentru frame-ul urmator
			if ( g_mouse.Lbut == K_MOUSE_BUTT_JUSTPRESSED )
			{
				if ( Rects::PointInRect( &layer->mouseRelPos, &bbL ) )
				{
					SND_PLAY( SNDIDX_CLICK );
					statusFlags |= CCTRL_STATUS_FLAG_CLICKEDLEFT;
				}
				else if ( Rects::PointInRect( &layer->mouseRelPos, &bbR ) )
				{
					SND_PLAY( SNDIDX_CLICK );
					statusFlags |= CCTRL_STATUS_FLAG_CLICKEDRIGHT;
				}
			}

			//--- efectuam actiunea in fn de flaguri si facem clear la flags ---
			int nNewPage = nPage;
			if ( statusFlags & CCTRL_STATUS_FLAG_CLICKEDLEFT )
			{
				nNewPage--;
				statusFlags &= ~CCTRL_STATUS_FLAG_CLICKEDLEFT;
			}
			if ( statusFlags & CCTRL_STATUS_FLAG_CLICKEDRIGHT )
			{
				nNewPage++;
				statusFlags &= ~CCTRL_STATUS_FLAG_CLICKEDRIGHT;
			}

			if ( ( nMinPage >= 0 ) && ( nNewPage < nMinPage ) )
				nNewPage = nMinPage;
			if ( ( nMaxPage >= 0 ) && ( nNewPage > nMaxPage ) )
				nNewPage = nMaxPage;

			if ( nNewPage != nPage )
			{

				CEvent *nevent = new CEvent( CEventTypes::evtT_CONTROLS, CEventCommands::evtC_CONTROLS_PAGE_CHANGED );
				nevent->AddNamedArgUINT32( L"layerID", layer->ID.getHash() );
				CVariantComplex* lvar = paramsDict.GetVariantByName( L"ID" );
				if ( lvar != NULL )
					nevent->AddNamedArgUINT32( L"ctrlID", lvar->m_strArg.getHash() );
				else
					nevent->AddNamedArgUINT32( L"ctrlID", 0 );
				nevent->AddNamedArgINT32( L"nPageIdx", nNewPage );
				nevent->AddNamedArgINT32( L"nOldIdx", nPage );
				UTGetEventManager().QueueEvent( nevent );

				nPage = nNewPage;
			}

			//refresh params
			paramsDict.SetNamedVarINT32( L"nPage", nPage );
		}
		break;
		case CCTRL_TYPE_LISTBOX:
		{
			if ( fontIdx < 0 ) return;

			int selIdx = _wtoi( paramsDict.GetVariantByName( L"selIdx" )->m_strArg.text );
			int selIdxMax = _wtoi( paramsDict.GetVariantByName( L"selIdxMax" )->m_strArg.text );

			if ( g_mouse.Lbut == K_MOUSE_BUTT_JUSTPRESSED && Rects::PointInRect( &layer->mouseRelPos, &BBox ) )
			{
				int dy = layer->mouseRelPos.y - BBox.y;
				int worldY = dy;

				int rowh = __TexFonts().fonts[ fontIdx ]->rowHeight + __TexFonts().fonts[ fontIdx ]->rowSpacing;
				int clickIdx = worldY / rowh;

				if ( ( clickIdx >= 0 ) && ( clickIdx <= selIdxMax ) )
					selIdx = clickIdx;
			}

			WCHAR val[ MAX_PATH ];
			StringCchPrintf( val, MAX_PATH, L"%d", selIdx );
			paramsDict.SetNamedVarString( L"selIdx", val );
		}
		break;
	}
}

void CControl::Paint( CCameraTransform *pCamera, Mat * matWorld )
{
	if ( !bVisible )
		return;

	RectXYWHi BBox, BBox_inflated;
	int animIdx = -1;
	int stringIdx = -1;
	int fontIdx = -1;
	int inflate = 0; 
	DWORD dwFontColor = 0xffffffff;
	DWORD dwColor = 0xffffffff;

	BBox = bbox;
	BBox_inflated = bbox;

	CVariantComplex* varInflate = paramsDict.GetVariantByName( L"inflate" );
	if ( varInflate->m_type != CVariantComplex::K_ARGTYPE_NONE )
	{
		inflate = varInflate->m_asINT32;
		if ( inflate != 0 )
		{
			BBox_inflated.x += inflate;
			BBox_inflated.y += inflate;
			BBox_inflated.w -= 2 * inflate;
			BBox_inflated.h -= 2 * inflate;
		}
	}


	CVariantComplex* var = paramsDict.GetVariantByName( L"animID" );
	if ( var->m_type != CVariantComplex::K_ARGTYPE_NONE )
	{
		animIdx = var->m_asINT32;
	}

	var = paramsDict.GetVariantByName( L"fontID" );
	if ( var->m_type != CVariantComplex::K_ARGTYPE_NONE )
	{
		fontIdx = var->m_asINT32;
	}

	var = paramsDict.GetVariantByName( L"stringID" );
	if ( var->m_type != CVariantComplex::K_ARGTYPE_NONE )
	{
		stringIdx = var->m_asINT32;
	}

	var = paramsDict.GetVariantByName( L"fontColor" );
	if ( var->m_type == CVariantComplex::K_ARGTYPE_HEXCOLOR )
	{
		dwFontColor = var->m_asUINT32;
	}
	//set font color
	float fColAlpha = DW_GETFALPHA( dwFontColor );
	dwFontColor = DW_COLORALPHA( dwFontColor, layer->alpha * fColAlpha );

	var = paramsDict.GetVariantByName( L"color" );
	if ( var->m_type == CVariantComplex::K_ARGTYPE_HEXCOLOR )
	{
		dwColor = var->m_asUINT32;
	}
	else
	{
		// mandatory param
		paramsDict.SetNamedVarHEXCOLOR( L"color", 0xffffffff );
	}
	//set font color
	fColAlpha = DW_GETFALPHA( dwColor );
	dwColor = DW_COLORALPHA( dwColor, layer->alpha * fColAlpha );


	switch ( type )
	{
		case CCTRL_TYPE_DROPDOWN:
		{
			if ( ( animIdx < 0 ) || ( animIdx >= m_pSprCol->animationNo ) || ( m_pSprCol->Animations[ animIdx ]->aframesNo < 3 ) )
			{
				drawDebugText( BBox.x, BBox.y, L"Invalid animIdx or frames count" );
				return;
			}

			int nSelectedIdx = paramsDict.GetVariantByName( L"nSelectedIdx" )->m_asINT32;
			int nItemsCnt = paramsDict.GetVariantByName( L"nItemsCnt" )->m_asINT32;
			int nIconFrame = paramsDict.GetVariantByName( L"iconFrame" )->m_asINT32;

			float dCol = 1.0f - 0.4f * fDisabledPercent;
			DWORD wcol = D3DCOLOR_COLORVALUE( dCol, dCol, dCol, layer->alpha );

			// containing panel
			GUIUtils::DrawPanel( m_pSprCol, BBox_inflated, fFocusPercent, layer->alpha, ANM_CONTROLS_SPR_PANELICONS, nIconFrame );
			if ( (stringIdx >= 0) && (fontIdx >= 0) )
			{
				__TexFonts().fonts[ fontIdx ]->DrawString( stringIdx, BBox.CenterX(), BBox.y + 1, FONTFLAG_ANCHOR_TOPCENTER, wcol );
			}
			// get arrows sizes (left arrow)
			RectXYWHi arrRect = m_pSprCol->GetAFrameBBox( animIdx, 1 );
			RectXYWHi bboxBar( BBox.x + arrRect.w, BBox.Bottom() - arrRect.h, BBox.w - arrRect.w * 2, arrRect.h );
			UTSprite::PaintFModuleStretched( m_pSprCol, Vec2( bboxBar.x, bboxBar.CenterY() ), animIdx, 0, 0, DW_COLOR_FFFA( layer->alpha ), bboxBar.w );

			int frame;
			// left button
			if ( nSelectedIdx > 0 )
			{
				frame = 1;
				if ( statusFlags & CCTRL_STATUS_FLAG_CLICKEDLEFT )
					frame = 1;
				UTSprite::PaintFrame( m_pSprCol, Vec2( bboxBar.x, bboxBar.CenterY() ), animIdx, frame, wcol );
			}
			// right button
			if ( nSelectedIdx < nItemsCnt - 1 )
			{
				frame = 2;
				if ( statusFlags & CCTRL_STATUS_FLAG_CLICKEDRIGHT )
					frame = 2;
				UTSprite::PaintFrame( m_pSprCol, Vec2( bboxBar.Right(), bboxBar.CenterY() ), animIdx, frame, wcol );
			}
			// text
			CVariantComplex* vc = paramsDict.GetVariantByName( L"stringID_list" );
			if ( (vc->m_type == CVariantComplex::K_ARGTYPE_STRING) && (!vc->m_strArg.IsEmpty()) && (fontIdx >= 0) )
			{
				int nStringIdx_list = __Texts().GetStrIdx( vc->m_strArg.textHash );
				CStringDesc sdSelection;
				__Texts().GetSubstring( &sdSelection, nStringIdx_list, nSelectedIdx, L'\n' );
				__TexFonts().fonts[ fontIdx ]->DrawString( &sdSelection, bboxBar.CenterX(), bboxBar.CenterY(), FONTFLAG_ANCHOR_VCENTERHCENTER, DW_COLORALPHA( dwFontColor, layer->alpha * (1.0f - fDisabledPercent * 0.8f) ) );
			}
			else
			{
				drawDebugText( bboxBar.x, bboxBar.y, L"Missing list font or stringID_list", 0xffff0000 );
			}
		}
		break;

		case CCTRL_TYPE_FILLING_STARS:
		{
			if ( animIdx < 0 )
				break;

			int nStars = paramsDict.GetVariantByName( L"nStars" )->m_asINT32;
			float fTimer = paramsDict.GetVariantByName( L"fTimerStars" )->m_asFloat;
			float fcoeff = floor( fTimer );
			float ffrac = FLOAT_FRAC( fTimer );
			float ffracinv = 1.0f - ffrac;

			DWORD wcol = DW_COLOR_FFFA( layer->alpha );
			RectXYWHi starrect = m_pSprCol->GetAFrameBBox( animIdx, 0 );

			Vec2 vStartPos( BBox_inflated.CenterX(), BBox_inflated.CenterY() );
			vStartPos.x -= starrect.w;
			for ( int kk = 0; kk < 3; kk++ )
			{
				//suport stea
				UTSprite::PaintFrame( m_pSprCol, vStartPos.x + kk * starrect.w, vStartPos.y, animIdx, 0, wcol );
				if ( ( kk < nStars ) && ( kk < fcoeff ) )
				{
					UTSprite::PaintFrame( m_pSprCol, vStartPos.x + kk * starrect.w, vStartPos.y, animIdx, 1, wcol );
				}
			}
			//deseneaza steaua care se scaleaza
			//partea de fractie creste intre 0 si 1 pentru fiecare stea
			if ( ffrac != 0.0f )
			{
				/*
				D3DXMATRIXA16 mattrans;
				D3DXMatrixAffineTransformation2D(&mattrans, 1.0f + ffracinv * 2.0f, NULL, 0.0f, &Vec2(vStartPos.x + fcoeff * starrect.w, vStartPos.y - ffracinv * 20.0f));
				layer->pControlsManager->m_pSprite->SetTransform(&mattrans);
				*/
				UTSprite::PaintFrame( m_pSprCol, 0.0f, 0.0f, animIdx, 1, DW_COLOR_FFFA( layer->alpha * ffrac ) );
				//layer->pControlsManager->m_pSprite->SetTransform(&g_matIdentity);
			}
		}
		break;
		case CCTRL_TYPE_LIST_SELECTOR:
		{
			if ( fontIdx < 0 )
			{
				drawDebugText( BBox.x, BBox.y, L"Invalid font ID!" );
				return;
			}
			//desenam
			DWORD wcol = DW_COLOR_FFFA( layer->alpha );
			//background
			GUIUtils::DrawWindowFrame( m_pSprCol, ANM_CONTROLS_SPR_FRAME_BLACK3, BBox_inflated, wcol );
			//selection
			int	selectedIdx = paramsDict.GetVariantByName( L"nSelectedIdx" )->m_asINT32;
			int rowH = paramsDict.GetVariantByName( L"nRowHeight" )->m_asINT32;
			int nOptionsCnt = paramsDict.GetVariantByName( L"nOptionsCnt" )->m_asINT32;

			//paint cursor
			if ( ( rowH > 0 ) && ( !bDisabled ) )
			{
				RectXYWHi selbb( BBox_inflated.x, BBox_inflated.y + rowH * selectedIdx, BBox_inflated.w, rowH - 3 );
				GUIUtils::DrawWindowFrame( m_pSprCol, ANM_CONTROLS_SPR_FRAME3, selbb, wcol );
			}
			//draw strings (left string); row by row as to keep old spacing
			for ( int kk = 0; kk < nOptionsCnt; kk++ )
			{
				CStringDesc strRow;
				__Texts().GetSubstring( &strRow, stringIdx, kk, '\n' );
				__TexFonts().fonts[ fontIdx ]->DrawString( &strRow, BBox_inflated.x, BBox_inflated.y + rowH * kk, FONTFLAG_ANCHOR_TOPLEFT, dwFontColor );
			}
			//right string
			CVariantComplex* vc = paramsDict.GetVariantByName( L"StringID_right" );
			if ( ( vc->m_type == CVariantComplex::K_ARGTYPE_STRING ) && ( !vc->m_strArg.IsEmpty() ) )
			{
				int nStringIdx_right = __Texts().GetStrIdx( vc->m_strArg.textHash );
				if ( nStringIdx_right >= 0 )
				{
					DWORD dwColorRight = 0xffffffff;
					var = paramsDict.GetVariantByName( L"FontColor_right" );
					if ( var->m_type == CVariantComplex::K_ARGTYPE_HEXCOLOR )
					{
						dwColorRight = var->m_asUINT32;
					}
					dwColorRight = DW_COLORALPHA( dwColorRight, layer->alpha );
					//draw string
					for ( int kk = 0; kk < nOptionsCnt; kk++ )
					{
						CStringDesc strRow;
						__Texts().GetSubstring( &strRow, nStringIdx_right, kk, '\n' );
						__TexFonts().fonts[ fontIdx ]->DrawString( &strRow, BBox_inflated.Right(), BBox_inflated.y + rowH * kk, FONTFLAG_ANCHOR_TOPRIGHT, dwColorRight );
					}
				}
			}
		}
		break;

		case CCTRL_TYPE_LIST_SELECTOR_TRUETYPE:
		{
			/*
			CTTFont* pTTFont = UTGetTTFManager().GetFont(shTTFID_SZ20.textHash);
			if ((pTTFont == null) || (pTTFont->pFont == null))
			{
				drawDebugText(BBox.x, BBox.y, L"TrueType font missing!");
				return;
			}
			*/
			if ( stringIdx < 0 )
			{
				drawDebugText( BBox.x, BBox.y, L"Left list stringIdx missing!" );
			}

			DWORD wcol = DW_COLOR_FFFA( layer->alpha );
			int nPage = paramsDict.GetVariantByName( L"nPage" )->m_asINT32;
			int nMinPage = paramsDict.GetVariantByName( L"nMinPage" )->m_asINT32;
			int nMaxPage = paramsDict.GetVariantByName( L"nMaxPage" )->m_asINT32;
			int optcnt = paramsDict.GetVariantByName( L"nOptionsCnt" )->m_asINT32;

			//background
			GUIUtils::DrawWindowFrame( m_pSprCol, ANM_CONTROLS_SPR_FRAME_BLACK3, BBox_inflated, wcol );
			//paging arrows
			if ( nMinPage != nMaxPage )
			{
				float offx = 1.0f;
				if ( statusFlags & CCTRL_STATUS_FLAG_HAS_FOCUS )
					offx = 1.0f + 1.0f * sin( layer->pControlsManager->fLocalTimeline * 3.0f );
				//left but
				if ( nPage > nMinPage )
					UTSprite::PaintFrame( m_pSprCol, BBox.x - offx, BBox.CenterY(), ANM_CONTROLS_SPR_ARROWS3, 0, wcol );
				//right but
				if ( nPage < nMaxPage )
				{
					UTSprite::PaintFrame( m_pSprCol, BBox.Right() + offx, BBox.CenterY(), ANM_CONTROLS_SPR_ARROWS3, 2, wcol );
				}
			}
			//selection
			int	selectedIdx = paramsDict.GetVariantByName( L"nSelectedIdx" )->m_asINT32;
			int rowH = paramsDict.GetVariantByName( L"nRowHeight" )->m_asINT32;
			rowH /= 2; //truetype text is in double the resolution

			if ( ( rowH > 0 ) && ( optcnt > 0 ) && ( selectedIdx >= 0 ) && ( selectedIdx < optcnt ) )
			{
				RectXYWHi selbb( BBox_inflated.x, BBox_inflated.y + rowH * selectedIdx, BBox_inflated.w, rowH );
				GUIUtils::DrawWindowFrame( m_pSprCol, ANM_CONTROLS_SPR_FRAME3, selbb, wcol );
			}
			/*
			//draw strings (left string)
			//set new transform, bigger resolution
			layer->pControlsManager->m_pSprite->Flush();
			layer->pControlsManager->SetCameraTransform(&UTApp().g_cam480hScreen);
			CCameraTransform::SetActiveCamera(layer->pControlsManager->m_pDevice, &UTApp().g_cam480hScreen);
			layer->pControlsManager->m_cameraScreenRect = layer->pControlsManager->m_pCamera->GetCamWorldAABB();

			RECTXYWH_F realrect = BBox_inflated;
			//#HACK: the new transform is 2 times the old interface transform:
			realrect.x *= 2.0f;
			realrect.y *= 2.0f;
			realrect.w *= 2.0f;
			realrect.h *= 2.0f;
			D3DXMATRIXA16 matlocal = *matWorld;
			//multiply layer/render positions too
			matlocal._41 *= 2.0f; matlocal._42 *= 2.0f;

			App_SetWorldTransform(layer->pControlsManager->m_pDevice, &matlocal);

			RECT rc, rc_shadow;
			SetRect(&rc, realrect.x, realrect.y, realrect.Right(), realrect.Bottom());
			rc_shadow = rc;
			rc_shadow.top += 2;

			if (stringIdx >= 0)
			{
				pTTFont->pFont->DrawTextW(layer->pControlsManager->m_pSprite, __Texts().strings[stringIdx]->sText, -1, &rc, DT_NOCLIP, dwFontColor);
			}
			//right string
			CVariantComplex* vc = paramsDict.GetVariantByName(L"StringID_right");
			if ((vc->m_type == CVariantComplex::K_ARGTYPE_STRING) && (!vc->m_strArg.IsEmpty()))
			{
				int nStringIdx_right = __Texts().GetStrIdx(vc->m_strArg.textHash);
				if (nStringIdx_right >= 0)
				{
					DWORD dwColorRight = 0xffffffff;
					var = paramsDict.GetVariantByName(L"FontColor_right");
					if (var->m_type == CVariantComplex::K_ARGTYPE_HEXCOLOR)
					{
						dwColorRight = var->m_asUINT32;
					}
					dwColorRight = DW_COLORALPHA(dwColorRight, layer->alpha);
					//draw string
					pTTFont->pFont->DrawTextW(layer->pControlsManager->m_pSprite, __Texts().strings[nStringIdx_right]->sText, -1, &rc, DT_NOCLIP | DT_RIGHT, dwColorRight);
				}
			}

			//restore camera
			layer->pControlsManager->m_pSprite->Flush();
			layer->pControlsManager->SetCameraTransform(&UTApp().g_cam360hScreen);
			CCameraTransform::SetActiveCamera(layer->pControlsManager->m_pDevice, &UTApp().g_cam360hScreen);
			layer->pControlsManager->m_cameraScreenRect = layer->pControlsManager->m_pCamera->GetCamWorldAABB();

			App_SetWorldTransform(layer->pControlsManager->m_pDevice, matWorld);
			*/
		}
		break;

		case CCTRL_TYPE_SCORESLIST_TRUETYPE:
		{
			/*
			CTTFont* pTTFont = UTGetTTFManager().GetFont(shTTFID_SZ20.textHash);
			if ((pTTFont == null) || (pTTFont->pFont == null))
			{
				drawDebugText(BBox.x, BBox.y, L"TrueType font missing!");
				return;
			}
			if (stringIdx < 0)
			{
				drawDebugText(BBox.x, BBox.y, L"Left list stringIdx missing!");
			}

			//get right string idx
			int nStringIdx_right = -1;
			CVariantComplex* vc = paramsDict.GetVariantByName(L"StringID_right");
			if ((vc->m_type == CVariantComplex::K_ARGTYPE_STRING) && (!vc->m_strArg.IsEmpty()))
			{
				nStringIdx_right = __Texts().GetStrIdx(vc->m_strArg.textHash);
				if (nStringIdx_right < 0)
				{
					drawDebugText(BBox.x, BBox.y + 20, L"Right list stringIdx missing!");
				}
			}

			//set new transform, bigger resolution
			layer->pControlsManager->m_pSprite->Flush();
			layer->pControlsManager->SetCameraTransform(&UTApp().g_cam480hScreen);
			CCameraTransform::SetActiveCamera(layer->pControlsManager->m_pDevice, &UTApp().g_cam480hScreen);
			layer->pControlsManager->m_cameraScreenRect = layer->pControlsManager->m_pCamera->GetCamWorldAABB();

			RECTXYWH_F realrect = BBox_inflated;
			//#HACK: the new transform is 2 times the old interface transform:
			realrect.x *= 2.0f;
			realrect.y *= 2.0f;
			realrect.w *= 2.0f;
			realrect.h *= 2.0f;
			D3DXMATRIXA16 matlocal = *matWorld;
			//multiply layer/render positions too
			matlocal._41 *= 2.0f; matlocal._42 *= 2.0f;

			App_SetWorldTransform(layer->pControlsManager->m_pDevice, &matlocal);

			RECT rc, rc_shadow;
			SetRect(&rc, realrect.x, realrect.y, realrect.Right(), realrect.Bottom());
			rc_shadow = rc;
			rc_shadow.top += 2;

			//paint arrow towards player name
			int nSelectedIdx = -1;
			vc = paramsDict.GetVariantByName(L"nSelectedIdx");
			if (vc->m_type == CVariantComplex::K_ARGTYPE_INT32)
				nSelectedIdx = vc->m_asINT32;
			if (nSelectedIdx >= 0)
			{
				D3DXFONT_DESCW descW;
				pTTFont->pFont->GetDesc(&descW);
				RECTXYWH box(realrect.x - 5, realrect.y + descW.Height * nSelectedIdx, realrect.w + 10, descW.Height);
				DrawFrame(m_pSprCol, ANM_CONTROLS_SPR_FRAME3, box, DW_COLOR_FFFA(layer->alpha));
			}

			//paint text
			if (stringIdx >= 0)
			{
				pTTFont->pFont->DrawTextW(layer->pControlsManager->m_pSprite, __Texts().strings[stringIdx]->sText, -1, &rc_shadow, DT_NOCLIP, DW_COLOR_XXXA(layer->alpha * 0.6f));
				pTTFont->pFont->DrawTextW(layer->pControlsManager->m_pSprite, __Texts().strings[stringIdx]->sText, -1, &rc, DT_NOCLIP, dwColor);
			}
			if (nStringIdx_right >= 0)
			{
				pTTFont->pFont->DrawTextW(layer->pControlsManager->m_pSprite, __Texts().strings[nStringIdx_right]->sText, -1, &rc_shadow, DT_NOCLIP | DT_RIGHT, DW_COLOR_XXXA(layer->alpha * 0.6f));
				pTTFont->pFont->DrawTextW(layer->pControlsManager->m_pSprite, __Texts().strings[nStringIdx_right]->sText, -1, &rc, DT_NOCLIP | DT_RIGHT, dwFontColor);
			}


			//restore camera
			layer->pControlsManager->m_pSprite->Flush();
			layer->pControlsManager->SetCameraTransform(&UTApp().g_cam360hScreen);
			CCameraTransform::SetActiveCamera(layer->pControlsManager->m_pDevice, &UTApp().g_cam360hScreen);
			layer->pControlsManager->m_cameraScreenRect = layer->pControlsManager->m_pCamera->GetCamWorldAABB();

			App_SetWorldTransform(layer->pControlsManager->m_pDevice, matWorld);
			*/
		}
		break;

		case CCTRL_TYPE_PLAYER_UPGRADE_CONTROL:
		{
			if ( ( fontIdx < 0 ) || ( animIdx < 0 ) )
			{
				drawDebugText( BBox.x, BBox.y, L"Invalid font ID or AnimID!" );
				return;

			}
			int nPlayerOrdinal = paramsDict.GetVariantByName( L"nPlayerOrdinal" )->m_asINT32;
			CLAMP( nPlayerOrdinal, 0, 1 );
			int nPlayerClass = ( int ) g_playerSelScr.m_arrPlayers[ nPlayerOrdinal ].eType;
			int nSelectedLine = paramsDict.GetVariantByName( L"nSelectedLine" )->m_asINT32;
			int	nSelectedPoint = paramsDict.GetVariantByName( L"nSelectedPoint" )->m_asINT32;

			int nXPPointsReal = App_GetAvailableXPPoints( ( EPSSPlayerClass ) nPlayerClass ); //real number of XP points to spend
			int nXPPointsLocal = nXPPointsReal;
			int nSpentPoints[ K_PSS_UPGRADE_BARS_CNT ];
			for ( int ll = 0; ll < K_PSS_UPGRADE_BARS_CNT; ll++ )
			{
				WCHAR strParamName[ MAX_PATH ];
				StringCchPrintf( strParamName, MAX_PATH, L"spent_bar%d", ll );
				nSpentPoints[ ll ] = paramsDict.GetVariantByName( strParamName )->m_asINT32;
				//update local points
				nXPPointsLocal -= nSpentPoints[ ll ];
			}

			//compute number of points spent on own bars (first 2 are the team bars, always)
			int nOwnBarsSpent = 0;
			for ( int ll = 2; ll < K_PSS_UPGRADE_BARS_CNT; ll++ )
			{
				int nBarIdx = g_playerSelScr.arrItemsByClass[ nPlayerClass ].arrUpgradeBarsIdx[ ll ];
				int nFilledReal = g_playerSelScr.m_arrPlayers[ nPlayerOrdinal ].arrUpgradeBarsPts[ ll ];
				nOwnBarsSpent += nFilledReal;
			}
			int nCurrentLevel = App_GetXPLevel( g_playerSelScr.m_arrPlayers[ nPlayerOrdinal ].nPlayerXPPts );
			int nTotalPoints = K_GAME_XPPOINTS_PER_XPLEVEL * nCurrentLevel;
			int nTeamSpentPoints = nTotalPoints - nXPPointsReal - nOwnBarsSpent;
			int nTeamSpentPointsLocal = nTeamSpentPoints + ( nSpentPoints[ 0 ] + nSpentPoints[ 1 ] );

			//desenam
			DWORD wcol = DW_COLOR_FFFA( layer->alpha );
			DWORD wcoldenied = DW_COLORALPHA( 0xffff8888, layer->alpha );

			//row height from blob
			RectXYWHi rectSz = m_pSprCol->GetAFrameBBox( animIdx, 0 );
			int nClassRows = 2, nTeamRows = 3; //how many rows for class properties (const)	vs team properties
			int nGroupSpacing = 9; //spacing after class rows
			int nTotalRows = nClassRows + nTeamRows;
			assert( nClassRows + nTeamRows <= K_PSS_UPGRADE_BARS_CNT );
			//team group
			RectXYWHi bbgroup( BBox_inflated.x, BBox_inflated.y, BBox_inflated.w, ( rectSz.h + 1 ) * nClassRows + 1 );
			GUIUtils::DrawWindowFrame( m_pSprCol, ANM_CONTROLS_SPR_FRAME_WHITE2, bbgroup, DW_COLORALPHA( 0xff0f1f2f, layer->alpha ) );
			bbgroup.Inflate( -6, -1 ); bbgroup.x += 5;
			GUIUtils::DrawWindowFrame( m_pSprCol, ANM_CONTROLS_SPR_FRAME_WHITE1, bbgroup, DW_COLORALPHA( 0xff102436, layer->alpha ) );
			//group name
			//__TexFonts().fonts[fontIdx]->DrawStringTransformed(__Texts().strings[STR_TEAM_UPC], bbgroup.x - 4, bbgroup.CenterY(), 1.0f, -HALF_PI, FONTFLAG_ANCHOR_BOTTOMCENTER, DW_COLORALPHA(K_COLOR_DEFAULT_TEXT, layer->alpha * 0.5f));

			//class group
			RectXYWHi bbteam( BBox_inflated.x, BBox_inflated.y + bbgroup.h + nGroupSpacing, BBox_inflated.w, ( rectSz.h + 1 ) * nTeamRows + 1 );
			GUIUtils::DrawWindowFrame( m_pSprCol, ANM_CONTROLS_SPR_FRAME_WHITE2, bbteam, DW_COLORALPHA( 0xff122426, layer->alpha ) );
			bbteam.Inflate( -6, -1 ); bbteam.x += 5;
			GUIUtils::DrawWindowFrame( m_pSprCol, ANM_CONTROLS_SPR_FRAME_WHITE1, bbteam, DW_COLORALPHA( 0xff1E3538, layer->alpha ) );
			//group name
			//__TexFonts().fonts[fontIdx]->DrawStringTransformed(__Texts().strings[STR_CLASS_UPC], bbteam.x - 4, bbteam.CenterY(), 1.0f, -HALF_PI, FONTFLAG_ANCHOR_BOTTOMCENTER, DW_COLORALPHA(K_COLOR_DEFAULT_TEXT, layer->alpha * 0.5f));

			//enforce player class
			if ( ( nPlayerClass < 0 ) || ( nPlayerClass >= K_PSS_CLASSES_COUNT ) )
			{
				drawDebugText( BBox.x, BBox.y, L"Player class not specified!" );
				return;
			}
			//individual bars
			bool bDeniedOperation = false;
			for ( int ll = 0; ll < nTotalRows; ll++ )
			{
				RectXYWHi bbline = bbgroup;
				bbline.h = rectSz.h;
				bbline.y += ll * ( rectSz.h + 1 );
				if ( ll >= nClassRows )
					bbline.y += nGroupSpacing - 1;
				///paint bar:
				bbline.Inflate( -2, -2 );
				if ( ll < nClassRows )
					GUIUtils::DrawWindowFrame( m_pSprCol, ANM_CONTROLS_SPR_FRAME_WHITE1, bbline, DW_COLORALPHA( 0xff0f1f2f, layer->alpha ) );
				else
					GUIUtils::DrawWindowFrame( m_pSprCol, ANM_CONTROLS_SPR_FRAME_WHITE1, bbline, DW_COLORALPHA( 0xff122426, layer->alpha ) );

				//paint upgrade bar (right aligned)
				int nBarIdx = g_playerSelScr.arrItemsByClass[ nPlayerClass ].arrUpgradeBarsIdx[ ll ];
				int nDots = g_playerSelScr.m_arrUpgradeBars[ nBarIdx ]->nTotalPoints;

				//int nFilledReal = g_userData[K_MEMID_UPGRADE_BAR_POINTS_START + g_playerSelScr.m_arrUpgradeBars[nBarIdx]->nMemSlot];
				int nFilledReal = g_playerSelScr.m_arrPlayers[ nPlayerOrdinal ].arrUpgradeBarsPts[ ll ];
				int nFilledLocal = nFilledReal + nSpentPoints[ ll ];
				//limit fillers to available points
				int tmpdelta = ( nSelectedPoint + 1 ) - nFilledLocal;
				if ( tmpdelta > nXPPointsLocal )
					tmpdelta = nXPPointsLocal;
				//don't remove more than what you added on the team bars
				if ( nSelectedLine <= 1 )
				{
					if ( ( tmpdelta < 0 ) && ( -tmpdelta > nTeamSpentPointsLocal ) )
						tmpdelta = -max( 0, nTeamSpentPointsLocal );
				}
				//show filled points
				int nFilledWished = nFilledLocal + tmpdelta;
				int nFilled = nFilledLocal;

				//denied operations: can't subtract from team bars
				/*
				if ((nSelectedLine == ll) && (ll <= 1) && (nFilledWished < nFilledReal))
				{
					bDeniedOperation = true;
					nFilledWished = nFilledLocal; //don't blink
				}
				*/
				//blink points
				if ( ( statusFlags & CCTRL_STATUS_FLAG_HAS_FOCUS ) && ( ll == nSelectedLine ) )
				{
					if ( g_timers.GetTimerValue( 600 ) < 0.3f )
						nFilled = nFilledWished;
				}

				Vec2 vPos( bbline.Right() - rectSz.w * nDots + rectSz.w / 2, bbline.CenterY() );
				//bar text
				int nStrIdx = g_playerSelScr.m_arrUpgradeBars[ nBarIdx ]->nStrIdx_name;
				__TexFonts().fonts[ fontIdx ]->DrawStringClamped( nStrIdx, bbgroup.x + 1, vPos.y, vPos.x - bbteam.x - 5, FONTFLAG_ANCHOR_VCENTERLEFT, DW_COLORALPHA( K_COLOR_DEFAULT_TEXT, layer->alpha ) );

				//bar points
				for ( int xx = 0; xx < nDots; xx++ )
				{
					int nPointPrice = g_playerSelScr.m_arrUpgradeBars[ nBarIdx ]->m_arrPerks[ xx ].nPointPrice;
					//skip perks (painted later)
					if ( nPointPrice >= 0 )
						continue;
					int nframe = 0;
					if ( xx < nFilled )
					{
						nframe = 5;
						if ( ll >= nClassRows )
							nframe = 8;
					}

					DWORD pcol = wcol;
					//paint in different color what you can't remove
					if ( ( nSelectedLine <= 1 ) && ( ll == nSelectedLine ) )
					{
						int nMinPos = nFilledLocal - nTeamSpentPointsLocal;
						if ( xx < nMinPos )
							pcol = wcoldenied;
					}

					UTSprite::PaintFrame( m_pSprCol, vPos.x + xx * rectSz.w, vPos.y, animIdx, nframe, pcol );
				}
				//links
				for ( int xx = 0; xx < nDots; xx++ )
				{
					if ( xx < nDots - 1 )
					{
						int nframe = 1;
						if ( ( xx < nDots - 1 ) && ( xx + 1 < nFilled ) )
						{
							nframe = 6;
							if ( ll >= nClassRows )
								nframe = 9;
						}
						DWORD pcol = wcol;
						//paint in different color what you can't remove
						if ( ( nSelectedLine <= 1 ) && ( ll == nSelectedLine ) )
						{
							int nMinPos = nFilledLocal - nTeamSpentPointsLocal;
							if ( xx < nMinPos - 1 )
								pcol = wcoldenied;
						}
						UTSprite::PaintFrame( m_pSprCol, vPos.x + xx * rectSz.w + rectSz.w / 2, vPos.y, animIdx, nframe, pcol );
					}
				}
				//large panels and icons
				for ( int xx = 0; xx < nDots; xx++ )
				{
					int nPointPrice = g_playerSelScr.m_arrUpgradeBars[ nBarIdx ]->m_arrPerks[ xx ].nPointPrice;
					if ( nPointPrice < 0 )
						continue;
					int nframe = 3;
					if ( xx == 0 ) nframe = 2;
					if ( xx == nDots - 1 ) nframe = 4;
					UTSprite::PaintFrame( m_pSprCol, vPos.x + xx * rectSz.w, vPos.y, animIdx, nframe, wcol );
					//paint color filled rectangle
					if ( xx < nFilled )
					{
						nframe = 7;
						if ( ll >= nClassRows )
							nframe = 10;

						DWORD pcol = wcol;
						//paint in different color what you can't remove
						if ( ( nSelectedLine <= 1 ) && ( ll == nSelectedLine ) )
						{
							int nMinPos = nFilledLocal - nTeamSpentPointsLocal;
							if ( xx < nMinPos )
								pcol = wcoldenied;
						}
						UTSprite::PaintFrame( m_pSprCol, vPos.x + xx * rectSz.w, vPos.y, animIdx, nframe, pcol );
					}
					//paint icons:
					int nIcon = g_playerSelScr.m_arrUpgradeBars[ nBarIdx ]->m_arrPerks[ xx ].nIconIdx;
					if ( nIcon >= 0 )
					{
						DWORD dwIconCol = 0xff95c7f0;
						if ( ll >= nClassRows )
							dwIconCol = 0xff8ee6e2;
						if ( xx >= nFilled )
							dwIconCol = 0xff384652;
						UTSprite::PaintFrame( m_pSprCol, vPos.x + xx * rectSz.w, vPos.y, ANM_CONTROLS_SPR_UPGRADE_ICONS, nIcon, DW_COLORALPHA( dwIconCol, layer->alpha ) );
					}
				}
			}

			//paint selected line
			if ( statusFlags & CCTRL_STATUS_FLAG_HAS_FOCUS )
			{
				RectXYWHi bbline = bbgroup;
				bbline.h = rectSz.h;
				bbline.y += nSelectedLine * ( rectSz.h + 1 );
				if ( nSelectedLine >= nClassRows )
					bbline.y += nGroupSpacing - 1;
				//paint selection
				RectXYWHi bbsel = bbline;
				bbsel.Inflate( -1, -1 );
				GUIUtils::DrawWindowFrame( m_pSprCol, ANM_CONTROLS_SPR_FRAME_HOLLOW1, bbsel, DW_COLORALPHA( 0xff254159, layer->alpha ) );
				//paint selected point
				int nBarIdx = g_playerSelScr.arrItemsByClass[ nPlayerClass ].arrUpgradeBarsIdx[ nSelectedLine ];
				int nPointPrice = g_playerSelScr.m_arrUpgradeBars[ nBarIdx ]->m_arrPerks[ nSelectedPoint ].nPointPrice;
				int nDots = g_playerSelScr.m_arrUpgradeBars[ nBarIdx ]->nTotalPoints;
				bbline.Inflate( -2, -2 );
				Vec2 vPos( bbline.Right() - rectSz.w * nDots + rectSz.w / 2, bbline.CenterY() );
				DWORD dwLocalCol = wcol;
				//Can't delete points from the TEAM bars
				if ( bDeniedOperation )
					dwLocalCol = DW_COLORALPHA( 0xffff2222, layer->alpha );

				if ( nPointPrice < 0 )
					UTSprite::PaintFrame( m_pSprCol, vPos.x + nSelectedPoint * rectSz.w, vPos.y, animIdx, 11, dwLocalCol );
				else
					UTSprite::PaintFrame( m_pSprCol, vPos.x + nSelectedPoint * rectSz.w, vPos.y, animIdx, 12, dwLocalCol );
			}
		}
		break;

		case CCTRL_TYPE_SCROLLMENU:
		{
			if ( fontIdx < 0 )
			{
				drawDebugText( BBox.x, BBox.y, L"Invalid font ID!" );
				return;
			}
			if ( ( animIdx >= 0 ) && ( m_pSprCol->Animations[ animIdx ]->aframesNo < 3 ) )
			{
				drawDebugText( BBox.x, BBox.y, L"Invalid animIdx. Needs 3 frames." );
				return;
			}

			float fSelectionCursor = paramsDict.GetVariantByName( L"fSelectionCursor" )->m_asFloat;
			int	selectedIdx = paramsDict.GetVariantByName( L"nSelectedIdx" )->m_asINT32;
			int textAlignFlags = paramsDict.GetVariantByName( L"nTextAlignFlags" )->m_asINT32;
			int vSpacing = paramsDict.GetVariantByName( L"Vspacing" )->m_asINT32;
			int nDisabledFlags = paramsDict.GetVariantByName( L"disabledFlags" )->m_asINT32;
			//desenam meniul
			DWORD wcol = DW_COLOR_FFFA( layer->alpha );

			int curidx = 0;
			for ( int ll = 0; ll < 8; ll++ ) //max 8 string options
			{
				float selperc = max( 0.0f, 1.0f - fabs( ( float ) ll - fSelectionCursor ) );
				float invselperc = 1.0f - selperc;
				int nDisFlag = 1 << ll;
				bool bDisabledLocal = ( ( nDisabledFlags & nDisFlag ) != 0 );

				WCHAR varname[ MAX_PATH ];
				StringCchPrintf( varname, MAX_PATH, L"StringID%d", ll );
				CVariantComplex* vc = paramsDict.GetVariantByName( varname );
				if ( ( vc->m_type == CVariantComplex::K_ARGTYPE_STRING ) && ( !vc->m_strArg.IsEmpty() ) )
				{
					int nStringIdx = __Texts().GetStrIdx( vc->m_strArg.textHash );
					SizeWHi strSz = __TexFonts().fonts[ fontIdx ]->MeasureString( nStringIdx, BBox.w );
					RectXYWHi currRect( BBox.CenterX() - strSz.w / 2 - 20, BBox.y + vSpacing * curidx - 1, strSz.w + 40, vSpacing );

					//paint cursor
					if ( animIdx >= 0 )
					{
						if ( selperc > 0.0f )
						{
							// paint filler under the selection
							DWORD bgcol = DW_COLORALPHA( GUIUtils::colPanelIdle, layer->alpha * selperc * 0.7f );
							UTSprite::PaintFModuleStretched( m_pSprCol, Vec2( currRect.x, currRect.y ), animIdx, 0, 0, bgcol, currRect.w, currRect.h );
							//paint selected cursor
							UTSprite::PaintFrame( m_pSprCol, currRect.x - invselperc * 15.0f, currRect.CenterY(), animIdx, 1, DW_COLOR_FFFA( selperc ) );
							UTSprite::PaintFrame( m_pSprCol, currRect.Right() + invselperc * 15.0f, currRect.CenterY(), animIdx, 2, DW_COLOR_FFFA( selperc ) );
						}
					}

					//and string
					DWORD exitcol;
					exitcol = DW_COLOR_LERP( wcol, dwFontColor, selperc );
					if ( bDisabledLocal )
						exitcol = DW_COLORALPHA( exitcol, 0.5f );

					Vec2 vTextOffset( selperc * 15.0f, 0.0f );
					if ( textAlignFlags & FONTFLAG_ANCHOR_CENTER )
						vTextOffset = Vec2( 0.0f, -1.0f * selperc );

					RectXYWHi drawrect( BBox.x + vTextOffset.x, BBox.y + vSpacing * curidx + vTextOffset.y, BBox.w, currRect.h );
					Vec2 vTextOrigin( drawrect.x, drawrect.CenterY() );
					if ( textAlignFlags & FONTFLAG_ANCHOR_CENTER )
						vTextOrigin = Vec2( drawrect.CenterX(), drawrect.CenterY() );
					else if ( textAlignFlags & FONTFLAG_ANCHOR_RIGHT )
						vTextOrigin = Vec2( drawrect.Right(), drawrect.CenterY() );
					//text rect
					RectXYWHi butr( BBox_inflated.x, BBox_inflated.y + vSpacing * curidx - ceil( selperc ), BBox_inflated.w, vSpacing );

					__TexFonts().fonts[ fontIdx ]->DrawString( nStringIdx, vTextOrigin.x, vTextOrigin.y, textAlignFlags | FONTFLAG_ANCHOR_VCENTER, ( DWORD ) exitcol );
					if ( ( ll == selectedIdx ) && ( !bDisabled ) )
					{
						float fcursor = FLOAT_MOD( layer->pControlsManager->fLocalTimeline * 2.0f, 3 ) - 1.0f;
						__TexFonts().fonts[ fontIdx ]->DrawString( nStringIdx, vTextOrigin.x, vTextOrigin.y, textAlignFlags | FONTFLAG_ANCHOR_VCENTER, ( DWORD ) exitcol );
					}

					curidx++;
				}
				else
				{
					break;
				}
			}
		}
		break;

		case CCTRL_TYPE_WIDEBAR:
		{
			if ( animIdx < 0 )
			{
				drawDebugText( BBox.x, BBox.y, L"Invalid Animation ID!" );
				return;
			}

			//deseneaza fundal (frames: 0-top bar, 1-bg filler, 2-bottom bar)
			RectXYWH camrect = pCamera->GetCamWorldAABB();
			RectXYWH barrect;
			barrect.x = floor( camrect.x - camrect.w / 2.0f - 10.0f ); barrect.w = ceil( camrect.w + 20.0f );
			barrect.y = BBox_inflated.CenterY() - ( BBox_inflated.h / 2.0f ) * layer->alpha;
			barrect.h = BBox_inflated.h * layer->alpha;

			CSpr bar( m_pSprCol, animIdx, barrect.x, ROUND_FLOAT( barrect.y ) );
			//back
			bar.frameIdx = 1;
			bar.color = dwColor;
			//bar.paintTiled( m_pSprCol, barrect.w, barrect.h );
			//bars
			//top
			bar.frameIdx = 0;
			//bar.paintTiled( m_pSprCol, barrect.w );
			//bottom
			bar.frameIdx = 2;
			bar.pos.y = ROUND_FLOAT( barrect.Bottom() );
			//bar.paintTiled( m_pSprCol, barrect.w );
		}
		break;


		case CCTRL_TYPE_BUTTON:
		{
			if ( ( animIdx < 0 ) || ( animIdx >= m_pSprCol->animationNo ) || ( m_pSprCol->Animations[ animIdx ]->aframesNo < 5 ) )
			{
				drawDebugText( BBox.x, BBox.y, L"Invalid animIdx or frame count" );
				return;
			}

			float dCol = 1.0f - 0.3f * fDisabledPercent;
			DWORD wcol = DW_COLORVALUE( dCol, dCol, dCol, layer->alpha );

			Vec2 butC( ( int ) ( BBox.x + BBox.w / 2.0f ), ( int ) ( BBox.y + BBox.h / 2.0f ) );
			bool bPressed = ((statusFlags & CCTRL_STATUS_FLAG_CLICKED) != 0);
			bool bHover = ((statusFlags & CCTRL_STATUS_FLAG_HOVER) != 0);
			float hoverPerc = 0.0f;
			if ( bHover )
			{
				hoverPerc = paramsDict.GetVariantByName( L"fHoverPercent" )->m_asFloat;
			}
			GUIUtils::DrawButton( m_pSprCol, animIdx, BBox, bPressed, hoverPerc, fFocusPercent, layer->alpha );

			//text
			if ( stringIdx >= 0 )
			{
				if ( fontIdx >= 0 )
				{
					int offy = 0;
					if ( ( statusFlags & CCTRL_STATUS_FLAG_CLICKED ) != 0 )
						offy = 1;

					__TexFonts().fonts[ fontIdx ]->DrawString( stringIdx, butC.x, butC.y + offy, FONTFLAG_ANCHOR_VCENTERHCENTER, dwFontColor );
				}
				else
				{
					drawDebugText( butC.x, butC.y, __Texts().strings[ stringIdx ]->sText, wcol );
				}
			}
		}
		break;


		case CCTRL_TYPE_WINDOW:
		{
			if ( ( animIdx < 0 ) || ( animIdx >= m_pSprCol->animationNo ) || ( m_pSprCol->Animations[ animIdx ]->aframesNo < 2 ) )
			{
				drawDebugText( BBox.x, BBox.y, L"Invalid window animIdx" );
				return;
			}

			//back frame
			GUIUtils::DrawWindow( m_pSprCol, animIdx, BBox_inflated, layer->alpha, fontIdx, stringIdx, dwFontColor );
		}
		break;

		case CCTRL_TYPE_PANEL:
		{
			if ( ( animIdx < 0 ) || ( animIdx >= m_pSprCol->animationNo ) || ( m_pSprCol->Animations[ animIdx ]->aframesNo < 2 ) )
			{
				drawDebugText( BBox.x, BBox.y, L"Invalid animIdx" );
				return;
			}

			int iconFrame = paramsDict.GetVariantByName( L"iconFrame" )->m_asINT32;
			GUIUtils::DrawPanel( m_pSprCol, BBox_inflated, 1.0f, layer->alpha, ANM_CONTROLS_SPR_PANELICONS, iconFrame );
		}
		break;

		case CCTRL_TYPE_PANELSM:
		{
			if ( (animIdx < 0) || (animIdx >= m_pSprCol->animationNo) || (m_pSprCol->Animations[ animIdx ]->aframesNo < 2) )
			{
				drawDebugText( BBox.x, BBox.y, L"Invalid animIdx" );
				return;
			}

			int iconFrame = paramsDict.GetVariantByName( L"iconFrame" )->m_asINT32;
			GUIUtils::DrawPanelSM( m_pSprCol, BBox_inflated, 1.0f, layer->alpha );
		}
		break;

		case CCTRL_TYPE_SDL_KEYREADER:
		{
			RectXYWHi movedB = BBox;
			if ( fontIdx >= 0 )
			{
				CStringHash* strh = &paramsDict.GetVariantByName( L"sKeyName" )->m_strArg;
				CStringDesc strdesc;
				__Texts().SetStringDesc( &strdesc, strh->text );

				__TexFonts().fonts[ fontIdx ]->DrawString( &strdesc, movedB, FONTFLAG_ANCHOR_VCENTERHCENTER, dwFontColor );
			}
		}
		break;
		case CCTRL_TYPE_TASKS_LIST:
		{
			int nIconFrame1 = -1, nIconFrame2 = -1;
			if ( animIdx >= 0 )
			{
				nIconFrame1 = paramsDict.GetVariantByName( L"nIconFrame1" )->m_asINT32;
				nIconFrame2 = paramsDict.GetVariantByName( L"nIconFrame2" )->m_asINT32;
			}
			DWORD dwCol = DW_COLOR_FFFA( layer->alpha );
			//now paint
			if ( fontIdx >= 0 )
			{
				int nStrIdx1 = -1, nStrIdx2 = -1;
				CVariantComplex* vc = paramsDict.GetVariantByName( L"stringID1" );
				if ( ( vc->m_type == CVariantComplex::K_ARGTYPE_STRING ) && ( !vc->m_strArg.IsEmpty() ) )
					nStrIdx1 = __Texts().GetStrIdx( vc->m_strArg.textHash );
				vc = paramsDict.GetVariantByName( L"stringID2" );
				if ( ( vc->m_type == CVariantComplex::K_ARGTYPE_STRING ) && ( !vc->m_strArg.IsEmpty() ) )
					nStrIdx2 = __Texts().GetStrIdx( vc->m_strArg.textHash );

				int nCount = 0;
				if ( nStrIdx1 >= 0 )
					nCount++;
				if ( nStrIdx2 >= 0 )
					nCount++;

				RectXYWHi iconbox( 0.0f, 0.0f, 0.0f, __TexFonts().fonts[ fontIdx ]->rowHeight );
				Vec2 vPos( BBox.x, BBox.Bottom() - iconbox.h / 2.0f - ( nCount - 1 ) * iconbox.h );
				bool bActive = false;
				if ( nIconFrame1 >= 0 )
				{
					if ( nIconFrame1 > 0 )
						bActive = true;

					iconbox = m_pSprCol->GetAFrameBBox( animIdx, nIconFrame1 );
					vPos.x = BBox.x + iconbox.w;
					//top align:
					vPos.y = BBox.y + iconbox.h / 2;
					//bottom align:
					//vPos.y = BBox.Bottom() - iconbox.h / 2 - (nCount - 1) * iconbox.h;

					RectXYWHi frrct( vPos.x - iconbox.w, vPos.y - iconbox.h / 2, BBox.w, iconbox.h );
					frrct.Inflate( -1, -1 );
					if ( bActive )
						GUIUtils::DrawWindowFrame( m_pSprCol, ANM_CONTROLS_SPR_FRAME_BLACK4, frrct, dwCol );
					else
						GUIUtils::DrawWindowFrame( m_pSprCol, ANM_CONTROLS_SPR_FRAME6_DARK, frrct, dwCol );

					UTSprite::PaintFrame( m_pSprCol, Vec2(vPos.x - iconbox.w, vPos.y), animIdx, nIconFrame1 );
				}
				if ( nStrIdx1 >= 0 )
				{
					if ( bActive )
						__TexFonts().fonts[ fontIdx ]->DrawString( nStrIdx1, vPos.x, vPos.y, FONTFLAG_ANCHOR_VCENTERLEFT, dwFontColor );
					else
						__TexFonts().fonts[ fontIdx ]->DrawString( nStrIdx1, vPos.x, vPos.y, FONTFLAG_ANCHOR_VCENTERLEFT, DW_COLORALPHA( dwFontColor, layer->alpha * 0.5f ) );
				}

				vPos.y += iconbox.h + 5;
				bActive = false;
				if ( nIconFrame2 >= 0 )
				{
					if ( nIconFrame2 > 0 )
						bActive = true;
					RectXYWHi frrct( vPos.x - iconbox.w, vPos.y - iconbox.h / 2, BBox.w, iconbox.h );
					frrct.Inflate( -1, -1 );
					if ( bActive )
						GUIUtils::DrawWindowFrame( m_pSprCol, ANM_CONTROLS_SPR_FRAME_BLACK4, frrct, dwCol );
					else
						GUIUtils::DrawWindowFrame( m_pSprCol, ANM_CONTROLS_SPR_FRAME6_DARK, frrct, dwCol );

					UTSprite::PaintFrame( m_pSprCol, vPos.x - iconbox.w, vPos.y, animIdx, nIconFrame2 );
				}
				if ( nStrIdx2 >= 0 )
				{
					if ( bActive )
						__TexFonts().fonts[ fontIdx ]->DrawString( nStrIdx2, vPos.x, vPos.y, FONTFLAG_ANCHOR_VCENTERLEFT, dwFontColor );
					else
						__TexFonts().fonts[ fontIdx ]->DrawString( nStrIdx2, vPos.x, vPos.y, FONTFLAG_ANCHOR_VCENTERLEFT, DW_COLORALPHA( dwFontColor, layer->alpha * 0.5f ) );
				}
			}
		}
		break;
		case CCTRL_TYPE_LABEL:
		{
			int textAlignFlags = paramsDict.GetVariantByName( L"nTextAlignFlags" )->m_asINT32;

			RectXYWHi movedB = BBox;
			if ( fontIdx >= 0 )
			{
				int noscaleFlags = FONTFLAG_WRAPTEXT | FONTFLAG_CLIPTEXT | FONTFLAG_JUSTIFY;
				if ( ( textAlignFlags & noscaleFlags ) != 0 )
					__TexFonts().fonts[ fontIdx ]->DrawString( stringIdx, movedB, textAlignFlags, dwFontColor );
				else
					__TexFonts().fonts[ fontIdx ]->DrawStringScaleW( stringIdx, movedB, textAlignFlags, dwFontColor );
			}
			else
			{
				drawDebugText( BBox.x + BBox.w / 2, BBox.y + BBox.h / 2, __Texts().strings[ stringIdx ]->sText, dwFontColor );
			}
		}
		break;
		case CCTRL_TYPE_BLINKING_LABEL:
		{
			float fTimer = paramsDict.GetVariantByName( L"fTimer" )->m_asFloat;
			float fBlinkTimer = paramsDict.GetVariantByName( L"timerBlink" )->m_asFloat;

			DWORD textcol = dwFontColor;

			if ( fTimer < fBlinkTimer )
			{
				CVariantComplex* varc = paramsDict.GetVariantByName( L"blinkColor" );
				if ( varc->m_type == CVariantComplex::K_ARGTYPE_HEXCOLOR )
				{
					textcol = var->m_asUINT32;
				}
			}

			int textAlignFlags = paramsDict.GetVariantByName( L"nTextAlignFlags" )->m_asINT32;

			RectXYWHi movedB = BBox;
			if ( fontIdx >= 0 )
			{
				int noscaleFlags = FONTFLAG_WRAPTEXT | FONTFLAG_CLIPTEXT | FONTFLAG_JUSTIFY;
				if ( ( textAlignFlags & noscaleFlags ) != 0 )
					__TexFonts().fonts[ fontIdx ]->DrawString( stringIdx, movedB, textAlignFlags, textcol );
				else
					__TexFonts().fonts[ fontIdx ]->DrawStringScaleW( stringIdx, movedB, textAlignFlags, textcol );
			}
			else
			{
				drawDebugText( BBox.x + BBox.w / 2, BBox.y + BBox.h / 2, __Texts().strings[ stringIdx ]->sText, textcol );
			}
		}
		break;
		case CCTRL_TYPE_ANIMATION:
		{
			if ( ( animIdx < 0 ) || ( animIdx >= m_pSprCol->animationNo ) )
			{
				drawDebugText( BBox.x, BBox.y, L"Invalid animIdx" );
				return;
			}

			int frameIdx = paramsDict.GetVariantByName( L"setFrame" )->m_asINT32;

			if ( ( frameIdx < 0 ) || ( frameIdx >= m_pSprCol->Animations[ animIdx ]->aframesNo ) )
			{
				drawDebugText( BBox.x, BBox.y, L"Invalid frameIdx" );
				return;
			}
			//scale the frame?
			bool bScale = ( bool ) paramsDict.GetVariantByName( L"scaleImage" )->m_asINT32;
			bool bAnimated = ( bool ) paramsDict.GetVariantByName( L"Animated" )->m_asINT32;

			DWORD wcol = DW_COLOR_FFFA( layer->alpha );
			//anim hack for tut arrow
			float foff = 0.0f;
			if ( bAnimated )
			{
				foff = max( 0.0f, 5.0f * sin( layer->pControlsManager->fLocalTimeline * 6.0f ) );
			}

			RectXYWHi animR = m_pSprCol->GetAFrameBBox_real( animIdx, frameIdx );

			float fScaleX = 1.0f, fScaleY = 1.0f;
			if ( bScale )
			{
				fScaleX = BBox.w / ( float ) animR.w;
				fScaleY = BBox.h / ( float ) animR.h;
			}
			float fScale = min( fScaleX, fScaleY );

			/*
			D3DXMATRIX mat;
			D3DXMatrixAffineTransformation2D(&mat, fScale, NULL, 0.0f, &Vec2((int)BBox.x + foff, (int)BBox.y));
			layer->pControlsManager->m_pSprite->SetTransform(&mat);
			*/
			UTSprite::PaintFrame( m_pSprCol, Vec2( (int)BBox.x + foff, (int)BBox.y ), animIdx, frameIdx, wcol );
			//layer->pControlsManager->m_pSprite->SetTransform(&g_matIdentity);
		}
		break;
		case CCTRL_TYPE_NET_VOTE:
		{
			if ( ( animIdx < 0 ) || ( animIdx >= m_pSprCol->animationNo ) )
			{
				drawDebugText( BBox.x, BBox.y, L"Invalid animIdx" );
				return;
			}

			int leftVote = paramsDict.GetVariantByName( L"leftVote" )->m_asINT32;
			int rightVote = paramsDict.GetVariantByName( L"rightVote" )->m_asINT32;

			if ( ( UTApp().IsGameNetworked() ) || ( GameState::state == GAME_STATE_CONTROLSED ) )
			{
				DWORD wcol = DW_COLOR_FFFA( layer->alpha );
				int nFrame = 0;
				if ( ( leftVote > 0 ) && ( rightVote <= 0 ) )
					nFrame = 1;
				else if ( ( leftVote <= 0 ) && ( rightVote > 0 ) )
					nFrame = 2;
				else if ( ( leftVote > 0 ) && ( rightVote > 0 ) )
					nFrame = 3;
				//now paint
				UTSprite::PaintFrame( m_pSprCol, BBox.CenterX(), BBox.CenterY(), animIdx, nFrame, wcol );
			}
		}
		break;

		case CCTRL_TYPE_SLIDER:
		{
			if ( ( animIdx < 0 ) || ( animIdx >= m_pSprCol->animationNo ) || ( m_pSprCol->Animations[ animIdx ]->aframesNo < 5 ) )
			{
				drawDebugText( BBox.x, BBox.y, L"Invalid animIdx(needs 5 frames)" );
				return;
			}

			bool hasArrows = ( bool ) paramsDict.GetVariantByName( L"hasArrows" )->m_asINT32;
			float slidePercent = paramsDict.GetVariantByName( L"fSlidePercent" )->m_asFloat;
			int nSteps = paramsDict.GetVariantByName( L"steps" )->m_asINT32;
			int nIconFrame = paramsDict.GetVariantByName( L"iconFrame" )->m_asINT32;
			DWORD wcol = DW_COLOR_FFFA( layer->alpha );

			// containing panel
			GUIUtils::DrawPanel( m_pSprCol, BBox_inflated, fFocusPercent, layer->alpha, ANM_CONTROLS_SPR_PANELICONS, nIconFrame);
			if ( (stringIdx >= 0) && (fontIdx >= 0) ) 
			{
				__TexFonts().fonts[ fontIdx ]->DrawString( stringIdx, BBox.CenterX(), BBox.y + 1, FONTFLAG_ANCHOR_TOPCENTER, wcol);
			}
			// get arrows sizes (left arrow)
			RectXYWHi arrRect = m_pSprCol->GetAFrameBBox( animIdx, 3 );
			RectXYWHi bboxBar( BBox.x + arrRect.w, BBox.Bottom() - arrRect.h, BBox.w - arrRect.w * 2, arrRect.h );
			GUIUtils::DrawProgress( m_pSprCol, animIdx, bboxBar, slidePercent, fFocusPercent, layer->alpha, nSteps );
			
			if ( hasArrows )
			{
				int frame;
				//left but
				frame = 3;
				if ( statusFlags & CCTRL_STATUS_FLAG_CLICKEDLEFT )
					frame = 3;
				UTSprite::PaintFrame( m_pSprCol, Vec2(bboxBar.x, bboxBar.CenterY()), animIdx, frame, wcol );

				//right but
				frame = 4;
				if ( statusFlags & CCTRL_STATUS_FLAG_CLICKEDRIGHT )
					frame = 4;
				UTSprite::PaintFrame( m_pSprCol, Vec2(bboxBar.Right(), bboxBar.CenterY()), animIdx, frame, wcol );
			}
		}
		break;
		case CCTRL_TYPE_SLIDER_PAGES:
		{
			//pt desenare: head left, center, head right, filler, tick, arr left, arr right
			if ( ( animIdx < 0 ) || ( animIdx >= m_pSprCol->animationNo ) || ( m_pSprCol->Animations[ animIdx ]->aframesNo < 7 ) )
			{
				drawDebugText( BBox.x, BBox.y, L"Invalid animIdx(needs 7 frames)" );
				return;
			}

			int nPage = paramsDict.GetVariantByName( L"nPage" )->m_asINT32;
			int nMinPage = paramsDict.GetVariantByName( L"nMinPage" )->m_asINT32;
			int nMaxPage = paramsDict.GetVariantByName( L"nMaxPage" )->m_asINT32;

			RectXYWHi bboxBar = BBox;

			DWORD wcol = DW_COLOR_FFFA( layer->alpha );

			float offx = 0.0f;
			if ( statusFlags & CCTRL_STATUS_FLAG_HAS_FOCUS )
				offx = 1.0f + 1.0f * sin( layer->pControlsManager->fLocalTimeline * 3.0f );

			int frame;
			//left but
			if ( ( nMinPage < 0 ) || ( nPage > nMinPage ) )
			{
				frame = 5;
				if ( statusFlags & CCTRL_STATUS_FLAG_CLICKEDLEFT )
					frame = 5;
				UTSprite::PaintFrame( m_pSprCol, BBox.x - offx, BBox.CenterY(), animIdx, frame, wcol );
			}

			//right but
			if ( ( nMaxPage < 0 ) || ( nPage < nMaxPage ) )
			{
				frame = 6;
				if ( statusFlags & CCTRL_STATUS_FLAG_CLICKEDRIGHT )
					frame = 6;
				UTSprite::PaintFrame( m_pSprCol, BBox.Right() + offx, BBox.CenterY(), animIdx, frame, wcol );
			}
		}
		break;
		case CCTRL_TYPE_CHECKBOX:
		{
			if ( ( animIdx < 0 ) || ( animIdx >= m_pSprCol->animationNo ) )
			{
				drawDebugText( BBox.x, BBox.y, L"Invalid animIdx" );
				return;
			}

			bool bChecked = paramsDict.GetVariantByName( L"bChecked" )->m_asBool;
			float hoverPercent = paramsDict.GetVariantByName( L"fHoverPercent" )->m_asFloat;

			int frame = 0;
			if ( bChecked )
				frame = 1;

			// panel
			//RectXYWHi checkrct = m_pSprCol->GetAFrameBBox( animIdx, 0 );
			RectXYWHi panelrect( BBox_inflated );
			//panelrect.x += checkrct.w + 2; 
			//panelrect.w -= checkrct.w + 2;
			GUIUtils::DrawPanel( m_pSprCol, panelrect, fFocusPercent, layer->alpha );
			// actual checkbox
			UTSprite::PaintFrame( m_pSprCol, BBox.x, BBox.CenterY(), animIdx, frame, dwColor );

			if ( fontIdx >= 0 )
			{
				__TexFonts().fonts[ fontIdx ]->DrawString( stringIdx, panelrect.x + 5, panelrect.CenterY(), FONTFLAG_ANCHOR_VCENTERLEFT, dwFontColor );
			}
			else
			{
				drawDebugText( BBox.x + BBox.w / 2, BBox.y + BBox.h / 2, __Texts().strings[ stringIdx ]->sText, 0xffffffff );
			}
		}
		break;
		case CCTRL_TYPE_VIGNETTE:
		{
			if ( ( animIdx < 0 ) || ( animIdx >= m_pSprCol->animationNo ) )
			{
				drawDebugText( BBox.x, BBox.y, L"Invalid animIdx" );
				return;
			}

			int frameIdx = paramsDict.GetVariantByName( L"frameIdx" )->m_asINT32;
			if ( ( frameIdx < 0 ) || ( frameIdx > m_pSprCol->GetAFramesCnt( animIdx ) ) )
			{
				drawDebugText( BBox.x, BBox.y, L"Invalid frameIdx" );
				return;
			}

			float fAlpha = DW_GETFALPHA( dwColor );

			// scale to screen
			RectXYWH camRect = pCamera->GetWorldAABB();
			camRect.Inflate( 20.0f );
			UTSprite::PaintFModuleStretched( m_pSprCol, Vec2(-camRect.w/2.0f, -camRect.h/2.0f), animIdx, frameIdx, 0, DW_COLORALPHA( dwColor, fAlpha * layer->alpha ), camRect.w, camRect.h );
		}
		break;
		case CCTRL_TYPE_PROGRESS_BAR:
		{
			if ( ( animIdx < 0 ) || ( animIdx >= m_pSprCol->animationNo ) || ( m_pSprCol->Animations[ animIdx ]->aframesNo < 8 ) )
			{
				drawDebugText( BBox.x, BBox.y, L"Invalid animIdx" );
				return;
			}

			int progress = _wtoi( paramsDict.GetVariantByName( L"progress" )->m_strArg.text );

			DWORD col = DW_COLOR_FFFA( layer->alpha );
			if ( bDisabled )
			{
				col = D3DCOLOR_COLORVALUE( 0.6f, 0.6f, 0.6f, layer->alpha );
			}

			int barH = 0;

			// back
			barH = m_pSprCol->GetAFrameBBox( animIdx, 1 ).h;
			UTSprite::PaintFrame( m_pSprCol, BBox.x, BBox.y, animIdx, 0, col );
			//UTSprite::PaintFrameModuleTiled( m_pSprCol, BBox.x, BBox.y, animIdx, 1, 0, col, BBox.w, barH );
			UTSprite::PaintFrame( m_pSprCol, BBox.x + BBox.w, BBox.y, animIdx, 2, col );

			// fill
			float fProg = progress * 0.01f; // progress este maxim 100
			int tileW = fProg * BBox.w;
			barH = m_pSprCol->GetAFrameBBox( animIdx, 4 ).h;
			UTSprite::PaintFrame( m_pSprCol, BBox.x, BBox.y, animIdx, 3, col );
			//UTSprite::PaintFrameModuleTiled( m_pSprCol, BBox.x, BBox.y, animIdx, 4, 0, col, tileW, barH );
			UTSprite::PaintFrame( m_pSprCol, BBox.x + tileW, BBox.y, animIdx, 5, col );

			// border
			barH = m_pSprCol->GetAFrameBBox( animIdx, 7 ).h;
			UTSprite::PaintFrame( m_pSprCol, BBox.x, BBox.y, animIdx, 6, col );
			//UTSprite::PaintFrameModuleTiled( m_pSprCol, BBox.x, BBox.y, animIdx, 7, 0, col, BBox.w, barH );
			UTSprite::PaintFrame( m_pSprCol, BBox.x + BBox.w, BBox.y, animIdx, 8, col );
		}
		break;

		case CCTRL_TYPE_XP_BAR:
		{
			if ( ( animIdx < 0 ) || ( animIdx >= m_pSprCol->animationNo ) || ( m_pSprCol->Animations[ animIdx ]->aframesNo < 6 ) )
			{
				drawDebugText( BBox.x, BBox.y, L"Invalid animIdx (min 6 frames)" );
				return;
			}
			DWORD col = DW_COLOR_FFFA( layer->alpha );
			if ( bDisabled )
				col = D3DCOLOR_COLORVALUE( 0.6f, 0.6f, 0.6f, layer->alpha );

			int nOldVal = paramsDict.GetVariantByName( L"nOldValue" )->m_asINT32;
			int nNewVal = paramsDict.GetVariantByName( L"nNewValue" )->m_asINT32;
			float fProgress = paramsDict.GetVariantByName( L"fProgress" )->m_asFloat;
			if ( fProgress < 0.0f )
				fProgress = 0.0f;

			int nCurrentVal = nOldVal + ( int ) floor( fProgress * ( nNewVal - nOldVal ) );
			int nCurrentLevel = App_GetXPLevel( nCurrentVal );
			int nMaxXP = App_GetMaxXP( nCurrentLevel );
			int nMinXP = App_GetMaxXP( nCurrentLevel - 1 );
			if ( nNewVal > nMaxXP )
				nNewVal = nMaxXP;
			if ( nOldVal < nMinXP )
				nOldVal = nMinXP;

			int barH = 0;
			int centerY = BBox.CenterY();
			// back
			barH = m_pSprCol->GetAFrameBBox( animIdx, 1 ).h;
			UTSprite::PaintFrame( m_pSprCol, BBox.x, centerY, animIdx, 0, col );
			//UTSprite::PaintFrameModuleTiled( m_pSprCol, BBox.x, centerY, animIdx, 1, 0, col, BBox.w, barH );
			UTSprite::PaintFrame( m_pSprCol, BBox.x + BBox.w, centerY, animIdx, 2, col );

			if ( fProgress < 1.0f )
			{
				bool bFlicker = false;
				if ( ( g_timers.GetTimerValue( 200 ) < 0.1f ) && ( fProgress <= 0.0f ) )
					bFlicker = true;
				// fill blinking teaser
				float fTease = ( ( float ) ( nNewVal - nMinXP ) / ( float ) ( nMaxXP - nMinXP ) );
				int tileW = fTease * BBox.w;
				barH = m_pSprCol->GetAFrameBBox( animIdx, 3 ).h;
				// yellow teaser bar
				//UTSprite::PaintFrameModuleTiled(m_pSprMgr, BBox.x, centerY, animIdx, ((bFlicker)?4:3), 0, col, tileW, barH);
				float fcol = 0.2f;
				if ( bFlicker )
					fcol = 0.4f;
				//UTSprite::PaintFrameModuleTiled( m_pSprCol, BBox.x, centerY, animIdx, 5, 0, DW_COLOR_FFFA( fcol * layer->alpha ), tileW, barH );
				// plus sign
				UTSprite::PaintFrame( m_pSprCol, BBox.Right() + 2, centerY, animIdx, ( ( bFlicker ) ? 7 : 6 ), col );
			}

			//actual filler
			float fProg = 1.0f; //filled when full level
			if ( ( nMaxXP - nMinXP ) > 0 )
				fProg = ( ( float ) ( nCurrentVal - nMinXP ) / ( float ) ( nMaxXP - nMinXP ) );

			int tileW = fProg * BBox.w;
			barH = m_pSprCol->GetAFrameBBox( animIdx, 5 ).h;
			//UTSprite::PaintFrameModuleTiled( m_pSprCol, BBox.x, centerY, animIdx, 5, 0, col, tileW, barH );

			//level shield
			UTSprite::PaintFrame( m_pSprCol, BBox.x, centerY, animIdx, 9, col );
			RectXYWHi prct = m_pSprCol->GetAFrameBBox( animIdx, 9 );
			Vec2 vposLvl( BBox.x + prct.x + prct.w / 2, BBox.CenterY() + prct.y + prct.h / 2 );
			//current level
			if ( fontIdx >= 0 )
			{
				CStringDesc stringDesc;
				__Texts().SetStringDesc( &stringDesc, L"%d", nCurrentLevel + 1 );
				__TexFonts().fonts[ fontIdx ]->DrawString( &stringDesc, vposLvl.x, vposLvl.y, FONTFLAG_ANCHOR_VCENTERHCENTER, dwFontColor );
				//points
				if ( ( nCurrentLevel >= K_GAME_MAX_UPGRADE_LEVELS ) && ( fProgress >= 1.0f ) )
				{
					__TexFonts().fonts[ fontIdx ]->DrawString( STR_MAX_LEVEL, BBox.CenterX(), centerY, FONTFLAG_ANCHOR_VCENTERHCENTER, DW_COLORALPHA( K_COLOR_SELECTED_TEXT, layer->alpha ) );
				}
				else
				{
					__Texts().SetStringDesc( &stringDesc, L"%d/%d", nCurrentVal - nMinXP, nMaxXP - nMinXP );
					__TexFonts().fonts[ fontIdx ]->DrawString( &stringDesc, BBox.CenterX(), centerY, FONTFLAG_ANCHOR_VCENTERHCENTER, DW_COLORALPHA( K_COLOR_SELECTED_TEXT, layer->alpha ) );
				}
			}
			//chevron when gaining a level
			int nInitialLevel = App_GetXPLevel( nOldVal );
			if ( nInitialLevel < nCurrentLevel )
			{
				UTSprite::PaintFrame( m_pSprCol, BBox.x, centerY, animIdx, 8, col );
			}
		}
		break;

		case CCTRL_TYPE_INPUTBOX:
		{
			if ( ( animIdx < 0 ) || ( animIdx >= m_pSprCol->animationNo ) )
			{
				drawDebugText( BBox.x, BBox.y, L"Invalid animIdx" );
				return;
			}
			if ( m_pSprCol->Animations[ animIdx ]->aframesNo < 3 )
			{
				drawDebugText( BBox.x, BBox.y, L"Needs 3 frames!" );
				return;
			}

			WCHAR inputText[ MAX_PATH ];
			StringCchPrintf( inputText, MAX_PATH, L"%s", paramsDict.GetVariantByName( L"inputText" )->m_strArg.text );

			DWORD wcol = DW_COLOR_FFFA( layer->alpha );
			GUIUtils::DrawHTilingAnim_HeadsOutside( m_pSprCol, animIdx, 0, BBox_inflated, wcol );

			// text
			if ( fontIdx >= 0 )
			{
				CStringDesc stringDesc;
				__Texts().SetStringDesc( &stringDesc, L"%s", inputText );
				int strw = __TexFonts().fonts[ fontIdx ]->DrawStringScaleW( &stringDesc, BBox.x + BBox.w / 2, BBox.y + BBox.h / 2, BBox.w, FONTFLAG_ANCHOR_VCENTERHCENTER, dwFontColor );

				// paint cursor
				//if (g_timers.GetTimerValue(500) < 0.25f)
				//{
				//	UTSprite::PaintFrame(m_pSprMgr, BBox.x + BBox.w / 2 + strw / 2 + 3, BBox.y + BBox.h / 2 + 8, ANM_CONTROLS_SPR_CTRL_UTILS, 1, wcol);
				//}
			}
		}
		break;
	}
}

bool CControl::HandleCommand( ECtrlMgrCommandType cmd, int nSDLinstanceID )
{
	switch ( type )
	{
		case CCTRL_TYPE_LIST_SELECTOR_TRUETYPE:
		case CCTRL_TYPE_LIST_SELECTOR:
		{
			bool bUserCanSelect = paramsDict.GetVariantByName( L"bUserCanSelect" )->m_asBool;
			int	selectedIdx = paramsDict.GetVariantByName( L"nSelectedIdx" )->m_asINT32;
			int optcnt = paramsDict.GetVariantByName( L"nOptionsCnt" )->m_asINT32;
			int nMinPage = paramsDict.GetVariantByName( L"nMinPage" )->m_asINT32;
			int nMaxPage = paramsDict.GetVariantByName( L"nMaxPage" )->m_asINT32;

			int stringIdx = -1;
			CVariantComplex* var = paramsDict.GetVariantByName( L"stringID" );
			if ( var->m_type != CVariantComplex::K_ARGTYPE_NONE )
			{
				stringIdx = var->m_asINT32;
			}

			//control has focus, scroll selection
			if ( cmd == K_CCTRLMGR_COMMAND_LEFT )
			{
				if ( nMinPage == nMaxPage )
					return false;

				statusFlags |= CCTRL_STATUS_FLAG_CLICKEDLEFT;
				return true;
			}
			else if ( cmd == K_CCTRLMGR_COMMAND_RIGHT )
			{
				if ( nMinPage == nMaxPage )
					return false;

				statusFlags |= CCTRL_STATUS_FLAG_CLICKEDRIGHT;
				return true;
			}
			//allow up/dn/select only if selectable is on
			if ( bUserCanSelect )
			{
				if ( cmd == K_CCTRLMGR_COMMAND_UP )
				{
					if ( ( optcnt <= 0 ) || ( selectedIdx == 0 ) )
						return false;

					statusFlags |= CCTRL_STATUS_FLAG_CLICKEDUP;
					return true;
				}
				else if ( cmd == K_CCTRLMGR_COMMAND_DOWN )
				{
					if ( ( optcnt <= 0 ) || ( selectedIdx >= optcnt - 1 ) )
						return false;

					statusFlags |= CCTRL_STATUS_FLAG_CLICKEDDOWN;
					return true;
				}
				else if ( cmd == K_CCTRLMGR_COMMAND_SELECT )
				{
					if ( optcnt <= 0 )
						return false;
					statusFlags |= CCTRL_STATUS_FLAG_CLICKED;
					return true;
				}
			}
		}
		break;
		case CCTRL_TYPE_PLAYER_UPGRADE_CONTROL:
		{
			int	nSelectedLine = paramsDict.GetVariantByName( L"nSelectedLine" )->m_asINT32;

			//control has focus, scroll selection
			if ( cmd == K_CCTRLMGR_COMMAND_LEFT )
			{
				statusFlags |= CCTRL_STATUS_FLAG_CLICKEDLEFT;
				return true;
			}
			else if ( cmd == K_CCTRLMGR_COMMAND_RIGHT )
			{
				statusFlags |= CCTRL_STATUS_FLAG_CLICKEDRIGHT;
				return true;
			}
			else if ( cmd == K_CCTRLMGR_COMMAND_UP )
			{
				if ( nSelectedLine == 0 )
					return false;

				statusFlags |= CCTRL_STATUS_FLAG_CLICKEDUP;
				return true;
			}
			if ( cmd == K_CCTRLMGR_COMMAND_DOWN )
			{
				if ( nSelectedLine >= K_PSS_UPGRADE_BARS_CNT - 1 )
					return false;

				statusFlags |= CCTRL_STATUS_FLAG_CLICKEDDOWN;
				return true;
			}
			else if ( cmd == K_CCTRLMGR_COMMAND_SELECT )
			{
				statusFlags |= CCTRL_STATUS_FLAG_CLICKED;
				return true;
			}
		}
		break;
		case CCTRL_TYPE_DROPDOWN:
		case CCTRL_TYPE_SLIDER:
		case CCTRL_TYPE_SLIDER_PAGES:
		{
			if ( cmd == K_CCTRLMGR_COMMAND_LEFT )
			{
				statusFlags |= CCTRL_STATUS_FLAG_CLICKEDLEFT;
				return true;
			}
			else if ( cmd == K_CCTRLMGR_COMMAND_RIGHT )
			{
				statusFlags |= CCTRL_STATUS_FLAG_CLICKEDRIGHT;
				return true;
			}
		}
		break;
		case CCTRL_TYPE_BUTTON:
		{
			if ( cmd == K_CCTRLMGR_COMMAND_SELECT )
			{
				statusFlags |= CCTRL_STATUS_FLAG_CLICKED_ALT;
				return true;
			}
		}
		break;
		case CCTRL_TYPE_CHECKBOX:
		{
			if ( cmd == K_CCTRLMGR_COMMAND_SELECT )
			{
				statusFlags |= CCTRL_STATUS_FLAG_CLICKED;
				return true;
			}
		}
		break;
		case CCTRL_TYPE_SCROLLMENU:
		{
			if ( cmd == K_CCTRLMGR_COMMAND_DOWN )
			{
				statusFlags |= CCTRL_STATUS_FLAG_CLICKEDDOWN;
				return true;
			}
			else if ( cmd == K_CCTRLMGR_COMMAND_UP )
			{
				statusFlags |= CCTRL_STATUS_FLAG_CLICKEDUP;
				return true;
			}
			else if ( cmd == K_CCTRLMGR_COMMAND_SELECT )
			{
				statusFlags |= CCTRL_STATUS_FLAG_CLICKED;
				return true;
			}
		}
		break;
	}

	return false;
}

void CControlsManager::SetParamValue( CControl * pCtrl, const WCHAR * sParamName, WCHAR * sParamValue, bool bIgnoreWarnings )
{
	if ( NULL == pCtrl )
		return;

	UINT32 paramNameHash = FastHash( sParamName );
	//verifica o serie de variabile generale care au nevoie de traducere
	if ( paramNameHash == FastHash( L"X" ) )
	{
		pCtrl->bbox.x = _wtoi( sParamValue );
		//adaug si parametru in dictionar ca sa apara in editor
		pCtrl->paramsDict.SetNamedVarAUTO( sParamName, sParamValue );
	}
	else if ( paramNameHash == FastHash( L"Y" ) )
	{
		pCtrl->bbox.y = _wtoi( sParamValue );
		//adaug si parametru in dictionar ca sa apara in editor
		pCtrl->paramsDict.SetNamedVarAUTO( sParamName, sParamValue );
	}
	else if ( paramNameHash == FastHash( L"W" ) )
	{
		pCtrl->bbox.w = _wtoi( sParamValue );
		//adaug si parametru in dictionar ca sa apara in editor
		pCtrl->paramsDict.SetNamedVarAUTO( sParamName, sParamValue );
	}
	else if ( paramNameHash == FastHash( L"H" ) )
	{
		pCtrl->bbox.h = _wtoi( sParamValue );
		//adaug si parametru in dictionar ca sa apara in editor
		pCtrl->paramsDict.SetNamedVarAUTO( sParamName, sParamValue );
	}
	else if ( paramNameHash == FastHash( L"animID" ) )
	{
		int anmIdx = m_sprCol.GetAnimationIdxByName( sParamValue );
		if ( ( anmIdx < 0 ) && ( !bIgnoreWarnings ) )
			ErrorBox( K_ERR_WARNING, L"CControlsManager::SetParamValue - couldn't find animation [%s]", sParamValue );

		pCtrl->paramsDict.SetNamedVarINT32( sParamName, anmIdx );
	}
	else if ( paramNameHash == FastHash( L"fontID" ) )
	{
		int fontIdx = __TexFonts().GetFontIdx( sParamValue );
		if ( ( fontIdx < 0 ) && ( !bIgnoreWarnings ) )
			ErrorBox( K_ERR_WARNING, L"CControlsManager::SetParamValue - couldn't find font [%s]", sParamValue );

		pCtrl->paramsDict.SetNamedVarINT32( sParamName, fontIdx );
	}
	else if ( paramNameHash == FastHash( L"stringID" ) )
	{
		int stringIdx = __Texts().GetStrIdx( sParamValue );
		if ( ( stringIdx < 0 ) && ( !bIgnoreWarnings ) )
			ErrorBox( K_ERR_WARNING, L"CControlsManager::SetParamValue - couldn't find string [%s]", sParamValue );

		pCtrl->paramsDict.SetNamedVarINT32( sParamName, stringIdx );
	}
	else if ( paramNameHash == FastHash( L"wrapText" ) )
	{
		int textAlignFlags = pCtrl->paramsDict.GetVariantByName( L"nTextAlignFlags" )->m_asINT32;
		bool wrap = false;
		if ( wcscmp( sParamValue, L"true" ) == 0 )
			wrap = true;

		if ( wrap )
			textAlignFlags |= FONTFLAG_WRAPTEXT;
		else
			textAlignFlags &= ~FONTFLAG_WRAPTEXT;

		pCtrl->paramsDict.SetNamedVarINT32( L"nTextAlignFlags", textAlignFlags );
		//add it in the dictionary so it shows up in the editor
		pCtrl->paramsDict.SetNamedVarBool( sParamName, wrap );
	}
	else if ( paramNameHash == FastHash( L"justify" ) )
	{
		int textAlignFlags = pCtrl->paramsDict.GetVariantByName( L"nTextAlignFlags" )->m_asINT32;

		bool justify = false;
		if ( wcscmp( sParamValue, L"true" ) == 0 )
			justify = true;

		if ( justify )
			textAlignFlags |= FONTFLAG_JUSTIFY;
		else
			textAlignFlags &= ~FONTFLAG_JUSTIFY;

		pCtrl->paramsDict.SetNamedVarINT32( L"nTextAlignFlags", textAlignFlags );
		//adaug si parametru in dictionar ca sa apara in editor
		pCtrl->paramsDict.SetNamedVarBool( sParamName, justify );
	}
	else if ( paramNameHash == FastHash( L"align" ) )
	{
		int textAlignFlags = pCtrl->paramsDict.GetVariantByName( L"nTextAlignFlags" )->m_asINT32;

		textAlignFlags &= ( ~( FONTFLAG_ANCHOR_RIGHT | FONTFLAG_ANCHOR_CENTER | FONTFLAG_ANCHOR_LEFT ) );
		int align = _wtoi( sParamValue );
		if ( align == 1 )
			textAlignFlags |= FONTFLAG_ANCHOR_RIGHT;
		else if ( align == -1 )
			textAlignFlags |= FONTFLAG_ANCHOR_LEFT;
		else
			textAlignFlags |= FONTFLAG_ANCHOR_CENTER;

		pCtrl->paramsDict.SetNamedVarINT32( L"nTextAlignFlags", textAlignFlags );
		//adaug si parametru in dictionar ca sa apara in editor
		pCtrl->paramsDict.SetNamedVarINT32( sParamName, align );
	}
	else if ( paramNameHash == FastHash( L"valign" ) )
	{
		int textAlignFlags = pCtrl->paramsDict.GetVariantByName( L"nTextAlignFlags" )->m_asINT32;

		textAlignFlags &= ( ~( FONTFLAG_ANCHOR_TOP | FONTFLAG_ANCHOR_VCENTER | FONTFLAG_ANCHOR_BOTTOM ) );
		int valign = _wtoi( sParamValue );
		if ( valign == 1 )
			textAlignFlags |= FONTFLAG_ANCHOR_BOTTOM;
		else if ( valign == -1 )
			textAlignFlags |= FONTFLAG_ANCHOR_TOP;
		else
			textAlignFlags |= FONTFLAG_ANCHOR_VCENTER;

		pCtrl->paramsDict.SetNamedVarINT32( L"nTextAlignFlags", textAlignFlags );
		//adaug si parametru in dictionar ca sa apara in editor
		pCtrl->paramsDict.SetNamedVarINT32( sParamName, valign );
	}
	else  //auto type converts based on value
	{
		pCtrl->paramsDict.SetNamedVarAUTO( sParamName, sParamValue );
	}
}

//**************************************************************************
//		CCtrlLayer
//**************************************************************************
CCtrlLayer::CCtrlLayer()
{
	alpha = 1.0f;
	statusFlags = 0;
	//set anchors
	anchorX = K_CCTRL_LAYER_ANCHOR_CENTER;
	anchorY = K_CCTRL_LAYER_ANCHOR_CENTER;

	X = Y = 0;
	bBlocking = bGetsInput = false;
	shFocusedControlID.Reset();
	fDestroyTimer = 0.0f;

	ID.Reset();

	bAnimate = true;

	pControlsManager = NULL;

	for ( int kk = 0; kk < controls.GetSize(); kk++ )
		SAFE_DELETE( controls[ kk ] );
	controls.RemoveAll();
}

CCtrlLayer::~CCtrlLayer()
{
	for ( int kk = 0; kk < controls.GetSize(); kk++ )
	{
		SAFE_DELETE( controls[ kk ] );
	}
	controls.RemoveAll();
}

void CCtrlLayer::FocusInitialize()
{
	nFocusFirstFocusableIdx = -1; //save first focusable control
	nFocusedControlIdx = -1;

	for ( int kk = 0; kk < controls.GetSize(); kk++ )
	{
		if ( ( controls[ kk ]->bCanHaveFocus ) && ( controls[ kk ]->bDisabled == false ) )
		{
			if ( nFocusFirstFocusableIdx < 0 )
			{
				nFocusFirstFocusableIdx = kk;
				nFocusedControlIdx = kk;
			}
			//--- selectez controlul specificat ca fiind focusat sau primul buton daca exista ---
			bool bFocusIt = false;
			CVariantComplex* lvar = controls[ kk ]->paramsDict.GetVariantByName( L"ID" );
			if ( !shFocusedControlID.IsEmpty() )
			{
				if ( lvar->m_strArg.textHash == shFocusedControlID.textHash )
					bFocusIt = true;
			}
			else //no control ID specified in layer
			{
				if ( ( nFocusedControlIdx >= 0 ) && ( controls[ kk ]->type == CCTRL_TYPE_BUTTON ) && ( nFocusedControlIdx != kk ) )
				{
					bFocusIt = true;
				}
			}

			if ( bFocusIt )
			{
				controls[ nFocusedControlIdx ]->statusFlags &= ~CCTRL_STATUS_FLAG_HAS_FOCUS;
				nFocusedControlIdx = kk;
				controls[ nFocusedControlIdx ]->statusFlags |= CCTRL_STATUS_FLAG_HAS_FOCUS;
				controls[ nFocusedControlIdx ]->OnFocused();
				return;
			}
		}
	}
}

void CCtrlLayer::FocusNextControl()
{
	//no focus made
	if ( nFocusedControlIdx < 0 )
	{
		for ( int kk = 0; kk < controls.GetSize(); kk++ )
		{
			if ( ( controls[ kk ]->bCanHaveFocus ) && ( controls[ kk ]->bDisabled == false ) )
			{
				nFocusFirstFocusableIdx = kk; //save first focusable control

				nFocusedControlIdx = kk;
				//and set focus
				controls[ nFocusedControlIdx ]->statusFlags |= CCTRL_STATUS_FLAG_HAS_FOCUS;
				controls[ nFocusedControlIdx ]->OnFocused();
				return;
			}
		}
	}
	else //focus next control
	{
		int nNextFocusable = nFocusedControlIdx;
		for ( int kk = 1; kk < controls.GetSize(); kk++ )
		{
			int ctrlidx = ( kk + nFocusedControlIdx ) % controls.GetSize();
			if ( ( controls[ ctrlidx ]->bCanHaveFocus ) && ( controls[ ctrlidx ]->bDisabled == false ) )
			{
				nNextFocusable = ctrlidx;
				break;
			}
		}
		// found another control? focus it
		if ( nNextFocusable != nFocusedControlIdx )
		{
			int nOldFocusIdx = nFocusedControlIdx;
			controls[ nFocusedControlIdx ]->statusFlags &= ~CCTRL_STATUS_FLAG_HAS_FOCUS;
			nFocusedControlIdx = nNextFocusable;
			//and set focus
			controls[ nFocusedControlIdx ]->statusFlags |= CCTRL_STATUS_FLAG_HAS_FOCUS;
			controls[ nFocusedControlIdx ]->OnFocused( 1 );
		}
	}
}

void CCtrlLayer::FocusPreviousControl()
{
	//no focus made
	if ( nFocusedControlIdx < 0 )
	{
		for ( int kk = 0; kk < controls.GetSize(); kk++ )
		{
			if ( ( controls[ kk ]->bCanHaveFocus ) && ( controls[ kk ]->bDisabled == false ) )
			{
				nFocusedControlIdx = kk;
				//save first focusable control
				nFocusFirstFocusableIdx = kk;
				//and set focus
				controls[ nFocusedControlIdx ]->statusFlags |= CCTRL_STATUS_FLAG_HAS_FOCUS;
				controls[ nFocusedControlIdx ]->OnFocused();
				return;
			}
		}
	}
	else //focus previous control
	{
		int nPrevFocusable = nFocusedControlIdx;
		for ( int kk = controls.GetSize() - 1; kk > 0; kk-- )
		{
			int ctrlidx = ( kk + nFocusedControlIdx ) % controls.GetSize();
			if ( ( controls[ ctrlidx ]->bCanHaveFocus ) && ( controls[ ctrlidx ]->bDisabled == false ) )
			{
				nPrevFocusable = ctrlidx;
				break;
			}
		}
		//found another control? focus that too
		if ( nPrevFocusable != nFocusedControlIdx )
		{
			int nOldFocusIdx = nFocusedControlIdx;
			controls[ nFocusedControlIdx ]->statusFlags &= ~CCTRL_STATUS_FLAG_HAS_FOCUS;
			nFocusedControlIdx = nPrevFocusable;
			//and set focus
			controls[ nFocusedControlIdx ]->statusFlags |= CCTRL_STATUS_FLAG_HAS_FOCUS;
			controls[ nFocusedControlIdx ]->OnFocused( -1 );
		}
	}
}

bool CCtrlLayer::FocusControl( CControl* pCtrl )
{
	if ( ( pCtrl == null ) || ( pCtrl->bCanHaveFocus == false ) )
		return false;

	int nCtrlIdx = controls.IndexOf( pCtrl );
	if ( nCtrlIdx < 0 )
		return false;

	// found another control? focus it
	if ( nFocusedControlIdx == nCtrlIdx )
		return false;

	int nOldFocusIdx = nFocusedControlIdx;
	controls[ nFocusedControlIdx ]->statusFlags &= ~CCTRL_STATUS_FLAG_HAS_FOCUS;
	nFocusedControlIdx = nCtrlIdx;
	//and set focus
	controls[ nFocusedControlIdx ]->statusFlags |= CCTRL_STATUS_FLAG_HAS_FOCUS;
	controls[ nFocusedControlIdx ]->OnFocused( SIGN( nFocusedControlIdx - nOldFocusIdx ) );
	return true;
}

void CCtrlLayer::SetAnchor( ECtrlAnchor nAnchorX, ECtrlAnchor nAnchorY )
{
	anchorX = nAnchorX;
	anchorY = nAnchorY;
}

Vec2i CCtrlLayer::GetPos()
{
	return Vec2i( X, Y );
}


CControl* CCtrlLayer::GetControlByIdx( int nIdx )
{
	if ( ( nIdx < 0 ) || ( nIdx >= controls.GetSize() ) )
		return nullptr;
	return controls[ nIdx ];
}

CControl* CCtrlLayer::GetControlByName( char* ctrlName )
{
	UINT32 chash = FastHash( ctrlName, strlen( ctrlName ) );
	for ( int kk = 0; kk < controls.GetSize(); kk++ )
	{
		if ( controls[ kk ]->paramsDict.GetVariantByName( L"ID" ) == NULL )
			continue;

		if ( controls[ kk ]->paramsDict.GetVariantByName( L"ID" )->m_strArg.getHash() == chash )
			return controls[ kk ];
	}
	return nullptr;
}

bool CCtrlLayer::ControlSetDisableByName( bool bDisabledValue, char* ctrlName )
{
	UINT32 chash = FastHash( ctrlName, strlen( ctrlName ) );
	for ( int kk = 0; kk < controls.GetSize(); kk++ )
	{
		if ( controls[ kk ]->paramsDict.GetVariantByName( L"ID" )->m_strArg.getHash() == chash )
		{
			controls[ kk ]->bDisabled = bDisabledValue;
			return true;
		}
	}
	return false;
}

CCtrlLayer* CCtrlLayer::Clone()
{
	CCtrlLayer* nlay = new CCtrlLayer();
	nlay->X = X; nlay->Y = Y;
	nlay->bBlocking = bBlocking;
	nlay->bGetsInput = bGetsInput;
	nlay->bAnimate = bAnimate;
	nlay->ID = ID;
	nlay->anchorX = anchorX;
	nlay->anchorY = anchorY;
	nlay->fDestroyTimer = fDestroyTimer;

	nlay->statusFlags = 0;
	nlay->nFocusedControlIdx = -1;
	nlay->nFocusFirstFocusableIdx = -1;
	nlay->shFocusedControlID = shFocusedControlID;

	for ( int kk = 0; kk < controls.GetSize(); kk++ )
	{
		CControl* nctrl = new CControl( controls[ kk ]->paramsDict.GetVariantByName( L"Type" )->m_strArg.text );
		nctrl->layer = nlay;
		//copy necessary data from templates
		nctrl->paramsDict = controls[ kk ]->paramsDict;
		nctrl->bbox = controls[ kk ]->bbox;
		nctrl->bCanHaveFocus = controls[ kk ]->bCanHaveFocus;
		nctrl->bShowFocusCursor = controls[ kk ]->bShowFocusCursor;

		//initialize control variables
		nctrl->Initialize();

		nlay->controls.Add( nctrl );
	}

	nlay->FocusInitialize();

	return nlay;
}

void GUIUtils::DrawButtonFromText( CSpriteCollection *sprCol, int animIdx, bool bPressed, CStringDesc *strDesc, CTexFont* pFont, Vec2 vButCenter, DWORD color, int nAlignHsign )
{
	//no text, don't draw
	if ( strDesc->len == 0 )
		return;

	SizeWHi fsz = pFont->MeasureString( strDesc );
	RectXYWHi sprrect = sprCol->GetAFrameBBox( animIdx, 1 );
	RectXYWHi butrect( vButCenter.x - fsz.w / 2, vButCenter.y - 2, fsz.w, sprrect.h );
	if ( nAlignHsign < 0 )
	{
		butrect.x = vButCenter.x;
	}
	else if ( nAlignHsign > 0 )
	{
		butrect.x = vButCenter.x - fsz.w;
	}

	if ( bPressed )
	{
		GUIUtils::DrawHTilingAnim_HeadsOutside( sprCol, animIdx, 6, butrect, color );
		pFont->DrawString( strDesc, butrect.CenterX(), vButCenter.y + 1.0f, FONTFLAG_ANCHOR_VCENTERHCENTER, color );
	}
	else
	{
		GUIUtils::DrawHTilingAnim_HeadsOutside( sprCol, animIdx, 0, butrect, color );
		pFont->DrawString( strDesc, butrect.CenterX(), vButCenter.y, FONTFLAG_ANCHOR_VCENTERHCENTER, color );
	}
}

void GUIUtils::DrawHTilingAnim( CSpriteCollection *sprCol, int animIdx, int nStartFrame, RectXYWHi BBox, DWORD color )
{
	RectXYWHi leftheadbb = sprCol->GetAFrameBBox( animIdx, nStartFrame );
	RectXYWHi rightheadbb = sprCol->GetAFrameBBox( animIdx, nStartFrame + 2 );
	int centerw = BBox.w - leftheadbb.w - rightheadbb.w;
	//UTSprite::PaintFrameModuleTiled( sprCol, BBox.x + leftheadbb.w, ( int ) ( BBox.y + BBox.h / 2.0f ), animIdx, nStartFrame + 1, 0, color, centerw, -1 );
	UTSprite::PaintFrame( sprCol, BBox.x, ( int ) ( BBox.y + BBox.h / 2.0f ), animIdx, nStartFrame, color );
	UTSprite::PaintFrame( sprCol, BBox.x + BBox.w - rightheadbb.w, ( int ) ( BBox.y + BBox.h / 2.0f ), animIdx, nStartFrame + 2, color );
}

void GUIUtils::DrawHTilingAnim_HeadsOutside( CSpriteCollection *sprCol, int animIdx, int nStartFrame, RectXYWHi BBox, DWORD color )
{
	//UTSprite::PaintFrameModuleTiled( sprCol, BBox.x, ( int ) ( BBox.y + BBox.h / 2.0f ), animIdx, nStartFrame + 1, 0, color, BBox.w, -1 );
	UTSprite::PaintFrame( sprCol, BBox.x, ( int ) ( BBox.y + BBox.h / 2.0f ), animIdx, nStartFrame, color );
	UTSprite::PaintFrame( sprCol, BBox.x + BBox.w, ( int ) ( BBox.y + BBox.h / 2.0f ), animIdx, nStartFrame + 2, color );
}

void GUIUtils::DrawProgress( CSpriteCollection *sprCol, int animIdx, RectXYWHi BBox, float fPercentFull, float fFocus, float fAlpha, int nSteps )
{
	UTSprite::PaintFModuleStretched( sprCol, Vec2( BBox.x, BBox.CenterY() ), animIdx, 0, 0, DW_COLOR_FFFA( fAlpha ), BBox.w );
	RectXYWHi cliprect = BBox;
	cliprect.w = ( int ) ceil( fPercentFull * cliprect.w );
	UTSprite::PaintFModuleStretched( sprCol, Vec2( BBox.x, BBox.CenterY() ), animIdx, 2, 0, DW_COLOR_FFFA( fAlpha ), cliprect.w );
	UTSprite::PaintFModuleStretched( sprCol, Vec2( BBox.x, BBox.CenterY() ), animIdx, 1, 0, DW_COLOR_FFFA( fAlpha * fFocus ), cliprect.w );
}


void GUIUtils::DrawPanelSM( CSpriteCollection *sprCol, RectXYWHi BBox, float fFocusPercent, float fAlpha /*= 1.0f */ )
{
	// default panel anim
	int animIdx = ANM_CONTROLS_SPR_PANEL1;
	// get thin bar width
	RectXYWHi leftSz1 = sprCol->GetAFrameBBox( animIdx, 1 );
	// extend box to the left
	int nLeftSize = leftSz1.w;
	RectLTRB clipwin( BBox );
	clipwin.left -= nLeftSize;
	Vec2 vUL( clipwin.left, clipwin.top );

	DWORD dwPanelColor = DW_COLOR_LERP( colPanelIdle, colPanelFocused, fFocusPercent );
	DWORD dwFocusAlpha = DW_COLOR_FFFA( fFocusPercent * fAlpha );
	DWORD dwAlpha = DW_COLOR_FFFA( fAlpha );
	// paint base
	UTSprite::PaintFModuleStretched( sprCol, Vec2(BBox.x, BBox.y), animIdx, 0, 0, DW_COLORALPHA( dwPanelColor, fAlpha ), BBox.w, BBox.h );
	// paint thin bar
	UTSprite::PaintFModuleClipped( sprCol, vUL, animIdx, 2, 0, clipwin, dwAlpha );
	UTSprite::PaintFModuleClipped( sprCol, vUL, animIdx, 1, 0, clipwin, dwFocusAlpha );
}

void GUIUtils::DrawButton( CSpriteCollection *sprCol, int animIdx, RectXYWHi BBox, bool bPressed, float fHoverPercent, float fFocusPercent, float fAlpha /*= 1.0f*/ )
{
	// get thin bar width
	RectXYWHi leftSz1 = sprCol->GetAFrameBBox_real( animIdx, 3 );
	// extend box to the left
	int nLeftSize = leftSz1.w;
	RectLTRB clipwin( BBox );
	clipwin.left -= nLeftSize; 
	Vec2 vUL( clipwin.left, clipwin.top );

	DWORD dwFocusAlpha = DW_COLOR_FFFA( fFocusPercent * fAlpha );
	float fHC = LIMIT( (fHoverPercent * 0.6f + fFocusPercent * 0.9f), 0.0f, 1.0f );
	DWORD dwHoverAlpha = DW_COLOR_FFFA( fHC * fAlpha );
	DWORD dwAlpha = DW_COLOR_FFFA( fAlpha );
	// paint base
	int nframe = 0;
	if ( bPressed )
		nframe = 2;
	RectLTRB clipbut( BBox );
	UTSprite::PaintFModuleClipped( sprCol, Vec2(BBox.x, BBox.y), animIdx, 0, 0, clipbut, dwAlpha );
	if(!bPressed)
		UTSprite::PaintFModuleClipped( sprCol, Vec2( BBox.x, BBox.y ), animIdx, 1, 0, clipbut, dwHoverAlpha );
	// paint thin bar
	UTSprite::PaintFModuleClipped( sprCol, Vec2( vUL.x, vUL.y ), animIdx, 4, 0, clipwin, dwAlpha );
	if(fFocusPercent > 0.0f)
		UTSprite::PaintFModuleClipped( sprCol, Vec2( vUL.x, vUL.y ), animIdx, 3, 0, clipwin, dwFocusAlpha );
}

void GUIUtils::DrawProgress_HeadsOutside( CSpriteCollection *sprCol, int animIdx, RectXYWHi BBox, float fPercentFull, DWORD color /*= 0xffffffff*/, int nTicks /*= 0*/ )
{
	GUIUtils::DrawHTilingAnim_HeadsOutside( sprCol, animIdx, 0, BBox, color );
	RectXYWHi cliprect = BBox;

	if ( nTicks > 1 )
	{
		float ticksz = ( float ) cliprect.w / ( float ) nTicks;
		for ( int kk = 1; kk < nTicks; kk++ )
		{
			UTSprite::PaintFModule( sprCol, Vec2(cliprect.x + ceil( kk * ticksz ), cliprect.CenterY()), animIdx, 4, 0, color );
		}
	}

	cliprect.w = ( int ) ceil( fPercentFull * cliprect.w );
	//UTSprite::PaintFrameModuleTiled( sprCol, BBox.x, BBox.CenterY(), animIdx, 3, 0, color, cliprect.w );
}

void GUIUtils::DrawPageSelector( CSpriteCollection *sprCol, int animIdx, RectXYWHi BBox, int nPagesCnt, int nSelectedPage, DWORD color /*= 0xffffffff*/, int nAlign /*= 0*/ )
{
	//int selpage = LIMIT(nSelectedPage, 0, nPagesCnt - 1);
	RectXYWHi ptrect = sprCol->GetAFrameBBox( animIdx, 0 ); //unselected page tick
	int width = nPagesCnt * ptrect.w - 1; //subtract dot spacing (1)
	int startposx = BBox.CenterX() - width / 2 + ptrect.w / 2;
	if ( nAlign < 0 )
		startposx = BBox.x;
	else if ( nAlign > 0 )
		startposx = BBox.Right() - width;

	for ( int kk = 0; kk < nPagesCnt; kk++ )
	{
		int curframe = 0;
		if ( kk == nSelectedPage )
			curframe = 1;

		UTSprite::PaintFrame( sprCol, startposx + kk * ptrect.w, BBox.Bottom(), animIdx, curframe, color );
	}
}

void GUIUtils::DrawWindowFrameF( CSpriteCollection *sprCol, int animIdx, RectXYWH BBox, DWORD color, float fInflate )
{
	RectXYWH BBox_local = BBox;
	BBox_local.Inflate( fInflate );
	/*
	RECTXYWH frrect;
	CSpr spr( m_pSprCol, animIdx, BBox_local.x, BBox_local.y );
	spr.color = color;

	//tiling centers
	//center
	spr.currentFrame = 4;
	spr.paintTiled( sprCol, BBox_local.w, BBox_local.h );
	//top
	spr.currentFrame = 1;
	spr.paintTiled( sprCol, BBox_local.w, -1 );
	//bottom
	spr.currentFrame = 7;
	spr.pos.y += BBox_local.h;
	spr.paintTiled( sprCol, BBox_local.w, -1 );
	//colturile (pot fi desenate peste centru)
	//cl
	spr.pos.y = BBox_local.y;
	spr.currentFrame = 3;
	spr.paintTiled( sprCol, -1, BBox_local.h );
	//ul
	spr.currentFrame = 0;
	spr.paint( sprCol );
	//cr
	spr.pos.x += BBox_local.w;
	spr.currentFrame = 5;
	spr.paintTiled( sprCol, -1, BBox_local.h );
	//ur
	spr.currentFrame = 2;
	spr.paint( sprCol );
	//dl
	spr.currentFrame = 6;
	spr.pos.x = BBox_local.x;
	spr.pos.y += BBox_local.h;
	spr.paint( sprCol );
	//dr
	spr.currentFrame = 8;
	spr.pos.x += BBox_local.w;
	spr.paint( sprCol );
	*/
}

//deseneaza fereastra in exteriorul BBOX, zona BBox fiind in totalitate folosibila
void GUIUtils::DrawWindowFrame( CSpriteCollection *sprCol, int animIdx, RectXYWHi BBox, DWORD color, int nInflate )
{
	RectXYWHi BBox_local = BBox;
	BBox_local.Inflate( nInflate, nInflate );
	/*
	RECTXYWH frrect;
	CSpr spr( animIdx, BBox_local.x, BBox_local.y );
	spr.color = color;

	//tiling centers
	//center
	spr.currentFrame = 4;
	spr.paintTiled( sprCol, BBox_local.w, BBox_local.h );
	//top
	spr.currentFrame = 1;
	spr.paintTiled( sprCol, BBox_local.w, -1 );
	//bottom
	spr.currentFrame = 7;
	spr.pos.y += BBox_local.h;
	spr.paintTiled( sprCol, BBox_local.w, -1 );
	//colturile (pot fi desenate peste centru)
	//cl
	spr.pos.y = BBox_local.y;
	spr.currentFrame = 3;
	spr.paintTiled( sprCol, -1, BBox_local.h );
	//ul
	spr.currentFrame = 0;
	spr.paint( sprCol );
	//cr
	spr.pos.x += BBox_local.w;
	spr.currentFrame = 5;
	spr.paintTiled( sprCol, -1, BBox_local.h );
	//ur
	spr.currentFrame = 2;
	spr.paint( sprCol );
	//dl
	spr.currentFrame = 6;
	spr.pos.x = BBox_local.x;
	spr.pos.y += BBox_local.h;
	spr.paint( sprCol );
	//dr
	spr.currentFrame = 8;
	spr.pos.x += BBox_local.w;
	spr.paint( sprCol );
	*/
}

void GUIUtils::DrawWindow( CSpriteCollection *sprCol, int animIdx, RectXYWHi BBox, float alpha, int nFontIdx, CStringDesc* strTitle, DWORD dwTitleColor )
{
	DWORD wndBaseColor = DW_COLORALPHA( 0xff000000, 0.8f * alpha );

	Vec2 vUL( BBox.x, BBox.y );
	RectLTRB clipwin( BBox );
	// paint base
	UTSprite::PaintFModuleStretched( sprCol, vUL, animIdx, 0, 0, wndBaseColor, BBox.w, BBox.h );
	// paint titlebar
	UTSprite::PaintFModuleClipped( sprCol, vUL, animIdx, 1, 0, clipwin, DW_COLOR_FFFA(alpha));
	//paint title
	if ( strTitle != nullptr && nFontIdx >= 0 )
	{
		// get titlebar width for text centering
		RectXYWHi barSz = sprCol->GetAFrameBBox( animIdx, 1 ); 
		Vec2 titleBarCenter( BBox.x + barSz.w / 2.0f, BBox.y + 5 );
		Mat matTitle;
		MUMatAffine2D( &matTitle, 1.0f, NULL, -HALF_PI, &titleBarCenter );
		__Painter().SetTransform( matTitle );
		__TexFonts().fonts[ nFontIdx ]->DrawString( strTitle, 0.0f, 0.0f, FONTFLAG_ANCHOR_VCENTERRIGHT, dwTitleColor );
		__Painter().SetTransform( g_matIdentity );
	}
}

void GUIUtils::DrawWindow( CSpriteCollection *sprCol, int animIdx, RectXYWHi BBox, float alpha, int nFontIdx, int nStrIdxTitle, DWORD dwTitleColor )
{
	CStringDesc* sdTitle = __Texts().GetStringDescByIdx( nStrIdxTitle );
	DrawWindow( sprCol, animIdx, BBox, alpha, nFontIdx, sdTitle, dwTitleColor );
}

void GUIUtils::DrawPanel( CSpriteCollection *sprCol, RectXYWHi BBox, float fFocusPercent, float fAlpha, int nIconAnimIdx /*= -1*/, int nIconFrame /*= -1 */ )
{
	// default panel anim
	int animIdx = ANM_CONTROLS_SPR_PANEL1;
	// get wide bar size
	RectXYWHi leftSz1 = sprCol->GetAFrameBBox_real( animIdx, 3 );
	// get thin bar width
	RectXYWHi leftSz2 = sprCol->GetAFrameBBox_real( animIdx, 1 );
	// extend box to the left
	int nLeftSize = leftSz1.w + leftSz2.w;
	Vec2 vUL( BBox.x - nLeftSize, BBox.y );
	RectLTRB clipwin( BBox );
	clipwin.left -= nLeftSize;
	
	DWORD dwPanelColor = DW_COLOR_LERP( colPanelIdle, colPanelFocused, fFocusPercent );
	DWORD dwFocusAlpha = DW_COLOR_FFFA( fFocusPercent * fAlpha );
	DWORD dwAlpha = DW_COLOR_FFFA( fAlpha );
	// paint base
	UTSprite::PaintFModuleStretched( sprCol, vUL, animIdx, 0, 0, DW_COLORALPHA(dwPanelColor, fAlpha), clipwin.Width(), clipwin.Height() );
	// paint side color
	if ( fFocusPercent > 0.0f )
	{
		UTSprite::PaintFModuleClipped( sprCol, vUL, animIdx, 3, 0, clipwin, dwFocusAlpha );
	}
	// paint thin bar
	UTSprite::PaintFModuleClipped( sprCol, Vec2( vUL.x + leftSz1.w, vUL.y ), animIdx, 2, 0, clipwin, dwAlpha );
	UTSprite::PaintFModuleClipped( sprCol, Vec2( vUL.x + leftSz1.w, vUL.y ), animIdx, 1, 0, clipwin, dwFocusAlpha );
	// paint icon
	if (( nIconAnimIdx >= 0 ) && (nIconFrame >= 0) && (nIconFrame < sprCol->GetAFramesCnt(nIconAnimIdx)) )
	{
		DWORD dwIconColor = DW_COLOR_LERP( colPanelIconIdle, colPanelIconFocused, fFocusPercent );
		UTSprite::PaintFModule( sprCol, Vec2( vUL.x + leftSz1.w / 2, vUL.y ), nIconAnimIdx, nIconFrame, 0, DW_COLORALPHA(dwIconColor, fAlpha));
	}
}


//**************************************************************************
//		Controls Manager
//**************************************************************************

CControlsManager::CControlsManager()
{
	m_pCamera = nullptr;
	m_cameraScreenRect.Set( 0, 0, 0, 0 );

	bIsBlocking = false;

	bLoaded = false;

	//local timeline
	fLocalTimeline = 0.0f;

}

CControlsManager::~CControlsManager()
{
	Release();
}

void CControlsManager::SetCameraTransform( CCameraTransform* pCamera )
{
	m_pCamera = pCamera;
	if ( m_pCamera != null )
	{
		m_cameraScreenRect = m_pCamera->GetCamWorldAABB();
	}
}

CCtrlLayer* CControlsManager::GetTopmostLayer()
{
	if ( Layers.GetSize() <= 0 )
		return nullptr;
	return Layers[ Layers.GetSize() - 1 ];
}


// INPUT FILE FORMAT:
//<?xml version="1.0"?>
//<Interfaces Version="1.0" SpriteCollection="controls.bsx">
//  <Layer ID="LAYER_ID_QUIT" X="0" Y="0"  isBlocking="true" getsInput="false">
//    <Control Type="Window" animID="ANM_WINDOWTITLE" FontID="FONT_ID_LARGE" stringID="STR_TITLE1" X="-105" Y="-105" W="210" H="210" />
//    <Control Type="Button" ID="CTRL_BUT_CANCEL" animID="ANM_BUTTON" FontID="FONT_ID_MED" stringID="STR_CANCEL" X="-90" Y="56" W="180" H="32" />
//    <Control Type="Label" FontID="FONT_ID_SMALL" stringID="STR_QUITSURE" wrapText="1" align="center" valign="top" X="-93" Y="-74" W="187" H="83" />
//  </Layer>
//</Interfaces>
OPRESULT CControlsManager::LoadControlsXML( WCHAR* XMLpath )
{
	if ( bLoaded )
	{
		Release();
	}

	pugi::xml_document doc;
	if ( !doc.load_file( XMLpath ) )
	{
		ErrorBox( K_ERR_WARNING, L"Unable to load Interfaces XML:%s\n", XMLpath );
		return( E_FAIL );
	}

	pugi::xml_attribute ver = doc.root().child( L"Interfaces" ).attribute( L"Version" );
	if ( ver.as_float() != INTERFACES_VERSION )
	{
		ErrorBox( K_ERR_WARNING, L"Interfaces XML wrong version:%s\n", XMLpath );
		return E_FAIL;
	}
	pugi::xml_attribute sprfile = doc.root().child( L"Interfaces" ).attribute( L"SpriteCollection" );
	if ( sprfile.empty() )
	{
		ErrorBox( K_ERR_WARNING, L"Interfaces XML didn't specify SpriteCollection!\n" );
		return E_FAIL;
	}

	const WCHAR* sprName = doc.root().child( L"Interfaces" ).attribute( L"SpriteCollection" ).value();
	///--- incarca sprite collection ---
	// finds path of sprite collection
	WCHAR szwPath[ MAX_PATH ];
	StringCchCopy( szwPath, MAX_PATH, XMLpath );
	int nIdx = ( int ) wcslen( szwPath );
	while ( --nIdx > 0 && szwPath[ nIdx ] != '\\' && szwPath[ nIdx ] != '/' );
	szwPath[ nIdx + 1 ] = '\0';
	StringCchCat( szwPath, MAX_PATH, sprName );

	V_OP_RETHR( m_sprCol.LoadSprites( szwPath ) );
	// save control sprite ptr (static)
	CControl::SetManagersPtr( &m_sprCol );

	// loads list of controls
	pugi::xml_node layernodes = doc.root().child( L"Interfaces" );
	for ( pugi::xml_node layerdata = layernodes.first_child(); layerdata; layerdata = layerdata.next_sibling() )
	{
		CCtrlLayer *nlayer = new CCtrlLayer();
		if ( !layerdata.attribute( L"getsInput" ).empty() )
			nlayer->bGetsInput = layerdata.attribute( L"getsInput" ).as_bool();
		nlayer->bBlocking = layerdata.attribute( L"isBlocking" ).as_bool();
		if ( !layerdata.attribute( L"isAnimated" ).empty() )
			nlayer->bAnimate = layerdata.attribute( L"isAnimated" ).as_bool();
		int lX = layerdata.attribute( L"X" ).as_int();
		int lY = layerdata.attribute( L"Y" ).as_int();
		nlayer->SetPos( lX, lY );

		nlayer->shFocusedControlID.Reset();
		if ( !layerdata.attribute( L"focusedControlID" ).empty() )
		{
			nlayer->shFocusedControlID.Init( layerdata.attribute( L"focusedControlID" ).value() );
		}

		nlayer->fDestroyTimer = layerdata.attribute( L"fTimer" ).as_float();
		//layer anchors
		WCHAR sAnchorValue[ MAX_PATH ];
		//X anchor
		nlayer->anchorX = ( ECtrlAnchor ) layerdata.attribute( L"anchorX" ).as_int();
		//Y anchor
		nlayer->anchorY = ( ECtrlAnchor ) layerdata.attribute( L"anchorY" ).as_int();

		if ( !layerdata.attribute( L"ID" ).empty() )
		{
			WCHAR strID[ MAX_PATH ];
			StringCchCopy( strID, MAX_PATH, layerdata.attribute( L"ID" ).value() );

			nlayer->ID.Init( strID );

		}
		else
		{
			ErrorBox( K_ERR_WARNING, L"Layer ID empty in file: %s", XMLpath );
			nlayer->ID.Reset();
		}
		///--- reads controls ---
		for ( pugi::xml_node controldata = layerdata.first_child(); controldata; controldata = controldata.next_sibling() )
		{
			const WCHAR* cType = controldata.attribute( L"Type" ).value();

			CControl* nctrl = new CControl( cType );

			nctrl->Reset();

			for ( pugi::xml_attribute attData = controldata.first_attribute(); attData; attData = attData.next_attribute() )
			{
				WCHAR attValue[ MAX_PATH ];
				StringCchPrintf( attValue, MAX_PATH, L"%s", attData.value() );
				//set param - translates data from string to binary
				SetParamValue( nctrl, attData.name(), attValue, true );
			}

			nctrl->layer = nlayer;

			nlayer->controls.Add( nctrl );
		}
		// now add layer into list of layer templates
		layersDefinitions.Add( nlayer );
	}

	bLoaded = true;
	return S_OK;
}

//release all resources
void CControlsManager::Release()
{
	if ( !bLoaded )
		return;

	for ( int kk = 0; kk < layersDefinitions.GetSize(); kk++ )
	{
		SAFE_DELETE( layersDefinitions[ kk ] );
	}
	layersDefinitions.RemoveAll();

	for ( int kk = 0; kk < Layers.GetSize(); kk++ )
	{
		SAFE_DELETE( Layers[ kk ] );
	}
	Layers.RemoveAll();

	m_sprCol.Release();

	bIsBlocking = false;
	bLoaded = false;
}

void CControlsManager::ReceiveInput( ECtrlMgrInputType eCommandType, UINT32 nCommand, int nCommandParam/*=-1*/ )
{
	switch ( eCommandType )
	{
		case K_CCTRLMGR_INPUT_SDL_KEY:
		{
			// non blocking layers don't get input
			if ( !bIsBlocking )
				break;
			// nCommand: 1 keypressed, 0 key released
			// nCommandParam: -1 not set or SDLscancode for key
			CCtrlLayer *lay = GetTopmostInputLayer();
			if ( ( lay != NULL ) && ( lay->alpha >= 1.0f ) ) // only read if fully visible
			{
				for ( int j = 0; j < lay->controls.GetSize(); j++ )
				{
					CControl *ctrl = lay->controls[ j ];
					//send code on keypress
					if ( ( ctrl->type == CCTRL_TYPE_SDL_KEYREADER ) && ( nCommand != 0 ) )
					{
						ctrl->paramsDict.SetNamedVarINT32( L"nSDLscancode", nCommandParam );
					}
				}
			}
		}
		break;
		case K_CCTRLMGR_INPUT_CHAR:
		{
			WCHAR c = ( WCHAR ) nCommand;

			CCtrlLayer *lay = GetTopmostInputLayer();
			if ( ( lay != NULL ) && ( lay->alpha >= 1.0f ) )
			{
				for ( int j = 0; j < lay->controls.GetSize(); j++ )
				{
					CControl *ctrl = lay->controls[ j ];
					if ( ctrl->type == CCTRL_TYPE_INPUTBOX )
					{
						_locale_t localt = _create_locale( LC_ALL, "" );
						if ( !_iswalnum_l( c, localt ) )
							continue;

						int textLen = _wtoi( ctrl->paramsDict.GetVariantByName( L"textLen" )->m_strArg.text );
						int maxLen = _wtoi( ctrl->paramsDict.GetVariantByName( L"maxLen" )->m_strArg.text );

						if ( ( __Texts().GetLetterIdx( c ) != K_STRMGR_SPACE ) && ( textLen < maxLen ) && ( c >= '0' ) )
						{
							WCHAR inputText[ MAX_PATH ];
							StringCchPrintf( inputText, MAX_PATH, L"%s", ctrl->paramsDict.GetVariantByName( L"inputText" )->m_strArg.text );
							inputText[ textLen ] = c;
							textLen++;
							//asigura caracterul de end of string
							inputText[ textLen ] = 0;

							WCHAR val[ MAX_PATH ];
							StringCchPrintf( val, MAX_PATH, L"%d", textLen );
							ctrl->paramsDict.SetNamedVarString( L"textLen", val );
							ctrl->paramsDict.SetNamedVarString( L"inputText", inputText );
							SND_PLAY( SNDIDX_CLICK );
						}
					}
				}
			}
		}
		break;
		case K_CCTRLMGR_INPUT_KEY:
		{
			UINT vk = ( UINT ) nCommand;

			CCtrlLayer *lay = GetTopmostInputLayer();
			if ( ( lay != NULL ) && ( lay->alpha >= 1.0f ) )
			{
				for ( int j = 0; j < lay->controls.GetSize(); j++ )
				{
					CControl *ctrl = lay->controls[ j ];
					if ( ctrl->type == CCTRL_TYPE_INPUTBOX )
					{
						int textLen = _wtoi( ctrl->paramsDict.GetVariantByName( L"textLen" )->m_strArg.text );
						//int maxLen = _wtoi(ctrl->paramsDict.GetVariantByName(L"maxLen")->m_strArg.text);
						WCHAR inputText[ MAX_PATH ];
						StringCchPrintf( inputText, MAX_PATH, L"%s", ctrl->paramsDict.GetVariantByName( L"inputText" )->m_strArg.text );
						if ( vk == VK_BACK )
						{
							if ( textLen > 0 )
							{
								textLen--;
								inputText[ textLen ] = 0;
								SND_PLAY( SNDIDX_CLICK );
							}
						}
						else if ( vk == VK_RETURN )
						{
							CEvent *nevent = new CEvent( CEventTypes::evtT_CONTROLS, CEventCommands::evtC_CONTROLS_CLICK );
							nevent->AddNamedArgUINT32( L"ctrlID", GET_FAST_HASH( "BUT_NEW_USER" ) );
							UTGetEventManager().QueueEvent( nevent );

							//TODO: ce comanda trimite cand faci enter pe inputbox. Poate mesaj de click pe input box?
							//ProcessInterfaceMessages(lay->ID, GET_FAST_HASH("CTRL_BUT_NEWPLAYER"), CCTRL_MESSAGE_CLICK);
						}

						WCHAR val[ MAX_PATH ];
						StringCchPrintf( val, MAX_PATH, L"%d", textLen );
						ctrl->paramsDict.SetNamedVarString( L"textLen", val );
						ctrl->paramsDict.SetNamedVarString( L"inputText", inputText );
					}
				}
			}
		}
		break;
		case K_CCTRLMGR_INPUT_COMMAND:
		{
			ECtrlMgrCommandType cmd = ( ECtrlMgrCommandType ) nCommand;
			//#TODO: aici ar trebui ca controlul selectat sa preia inputul si sa-l trateze singur in loc sa caut un control anume
			CCtrlLayer *lay = GetTopmostInputLayer();
			if ( (lay != NULL) && (lay->alpha >= 1.0f) )
			{
				//change focused control (if it has any)
				if ( lay->nFocusFirstFocusableIdx >= 0 )
				{
					bool bCommandProcessed = false;
					if ( lay->nFocusedControlIdx >= 0 )
					{
						bCommandProcessed = lay->controls[ lay->nFocusedControlIdx ]->HandleCommand( cmd, nCommandParam );
					}
					//not processed by focused control? process it by the layer
					if ( !bCommandProcessed )
					{
						if ( cmd == K_CCTRLMGR_COMMAND_DOWN )
						{
							lay->FocusNextControl();
						}
						else if ( cmd == K_CCTRLMGR_COMMAND_UP )
						{
							lay->FocusPreviousControl();
						}
						else if ( cmd == K_CCTRLMGR_COMMAND_BACK )
						{
							//handle back command (close window) - default close IDs
							CControl *ctrl = lay->GetControlByName( "BUT_CLOSE" );
							if ( ctrl == null )
								ctrl = lay->GetControlByName( "BUT_CLOSE_FORCED" );
							if ( ctrl == null )
								ctrl = lay->GetControlByName( "BUT_CLOSE_SETTINGS" );
							if ( ctrl == null )
								ctrl = lay->GetControlByName( "BUT_CANCEL_LOBBY" );
							if ( ctrl == null )
								ctrl = lay->GetControlByName( "BUT_CLOSE_KEYDEF" );
							// daca avem buton clasic de close dam comanda de click pe el
							if ( ctrl != nullptr )
							{
								ctrl->statusFlags |= CCTRL_STATUS_FLAG_CLICKED_ALT;
							}
						}
					}
				}
			}
		}
		break;
	}

}

void CControlsManager::MessageBoxOK( int titleStringId, int textStringId )
{
	CCtrlLayer* tmpLayer = ShowLayerOnce( "LAYER_ID_MSGBOX_OK" );

	if ( tmpLayer != NULL )
	{
		CControl* tmpCb = tmpLayer->GetControlByName( "WINDOW" );
		if ( tmpCb != NULL )
		{
			tmpCb->paramsDict.SetNamedVarINT32( L"stringID", titleStringId );
		}
		tmpCb = tmpLayer->GetControlByName( "TEXT_LABEL" );
		if ( tmpCb != NULL )
		{
			tmpCb->paramsDict.SetNamedVarINT32( L"stringID", textStringId );
		}
	}
}

void CControlsManager::Update( float dTime )
{
	///--- update local timeline ---
	fLocalTimeline += dTime;

	bIsBlocking = false;

	if ( Layers.GetSize() <= 0 )
	{
		return;
	}
	if ( m_pCamera != nullptr )
	{
		m_cameraScreenRect = m_pCamera->GetCamWorldAABB();
	}

	// check input from all connected controllers and translate to local commands
	for ( CController * ctrlr : UTGetCtrlrMgr().m_arrControllers )
	{
		//always skip network controllers
		if ( ctrlr->eType == K_CM_CT_NET_FRAMELOCK )
			continue;

		if ( ( ctrlr->GetButState( K_CM_COMMAND_MOVE_X ) == K_CM_BUTSTATE_JUSTPRESSED ) && ( ctrlr->GetAxisVal( K_CM_COMMAND_MOVE_X ) < 0.0f ) )
			ReceiveInput( K_CCTRLMGR_INPUT_COMMAND, K_CCTRLMGR_COMMAND_LEFT, ctrlr->nSDLInstanceId );
		if ( ( ctrlr->GetButState( K_CM_COMMAND_MOVE_X ) == K_CM_BUTSTATE_JUSTPRESSED ) && ( ctrlr->GetAxisVal( K_CM_COMMAND_MOVE_X ) > 0.0f ) )
			ReceiveInput( K_CCTRLMGR_INPUT_COMMAND, K_CCTRLMGR_COMMAND_RIGHT, ctrlr->nSDLInstanceId );
		if ( ( ctrlr->GetButState( K_CM_COMMAND_MOVE_Y ) == K_CM_BUTSTATE_JUSTPRESSED ) && ( ctrlr->GetAxisVal( K_CM_COMMAND_MOVE_Y ) < 0.0f ) )
			ReceiveInput( K_CCTRLMGR_INPUT_COMMAND, K_CCTRLMGR_COMMAND_UP, ctrlr->nSDLInstanceId );
		if ( ( ctrlr->GetButState( K_CM_COMMAND_MOVE_Y ) == K_CM_BUTSTATE_JUSTPRESSED ) && ( ctrlr->GetAxisVal( K_CM_COMMAND_MOVE_Y ) > 0.0f ) )
			ReceiveInput( K_CCTRLMGR_INPUT_COMMAND, K_CCTRLMGR_COMMAND_DOWN, ctrlr->nSDLInstanceId );

		// send select commands
		EControllerCommand arrSelectCommands[] = { K_CM_COMMAND_FIRE1, K_CM_COMMAND_JUMP, K_CM_COMMAND_SELECT };
		for each(auto a in arrSelectCommands)
		{
			if ( ctrlr->sCommands.keyState[ a ] == K_CM_BUTSTATE_JUSTPRESSED )
			{
				// always ignore pointer buttons as they are treated directly inside the controls and allowing them would break things
				CControllerTrigger* trig = ctrlr->GetTriggerForCommand( a );
				if ( (trig != nullptr) && (trig->eType == K_CM_POINTER_BUTTON) )
					continue;

				ReceiveInput( K_CCTRLMGR_INPUT_COMMAND, K_CCTRLMGR_COMMAND_SELECT, ctrlr->nSDLInstanceId );
				break;
			}
		}

		if ( ( ctrlr->sCommands.keyState[ K_CM_COMMAND_BACK ] == K_CM_BUTSTATE_JUSTPRESSED ) ||
			( ctrlr->sCommands.keyState[ K_CM_COMMAND_RELOAD ] == K_CM_BUTSTATE_JUSTPRESSED ) ||
			( ctrlr->sCommands.keyState[ K_CM_COMMAND_MELEE ] == K_CM_BUTSTATE_JUSTPRESSED ) )
		{
			ReceiveInput( K_CCTRLMGR_INPUT_COMMAND, K_CCTRLMGR_COMMAND_BACK, ctrlr->nSDLInstanceId );
		}
	}

	for ( int kk = 0; kk < Layers.GetSize(); kk++ )
	{
		CCtrlLayer *lay = Layers[ kk ];
		//set relative mouse pos
		Vec2 localMousePt = g_mouse.pos;
		if ( m_pCamera != null )
		{
			localMousePt = m_pCamera->ScreenToWorld( g_mouse.pos );
		}
		Vec2i lPos = lay->GetPos();
		Vec2i anchor;
		anchor.x = m_cameraScreenRect.CenterX() + lay->anchorX * ( m_cameraScreenRect.w / 2 );
		anchor.y = m_cameraScreenRect.CenterY() + lay->anchorY * ( m_cameraScreenRect.h / 2 );

		lay->mouseRelPos.x = ( int ) localMousePt.x;
		lay->mouseRelPos.y = ( int ) localMousePt.y;

		lay->mouseRelPos.x -= lPos.x + ( int ) anchor.x;
		lay->mouseRelPos.y -= lPos.y + ( int ) anchor.y;
		// updates destroy timer
		if ( lay->fDestroyTimer > 0.0f )
		{
			lay->fDestroyTimer -= dTime;
			if ( lay->fDestroyTimer <= 0.0f )
			{
				lay->fDestroyTimer = 0.0f;
				lay->statusFlags |= CCTRL_STATUS_FLAG_REMOVED;
			}
		}
		// was it removed?
		if ( lay->statusFlags & CCTRL_STATUS_FLAG_REMOVED )
		{
			dec_limit( lay->alpha, dTime * 6.0f, 0.0f );
			if ( lay->statusFlags & CCTRL_STATUS_FLAG_FORCED )
				lay->alpha = 0.0f;
		}
		else
		{
			inc_limit( lay->alpha, dTime * 6.0f, 1.0f );
		}
		//set blocking flag
		if ( lay->bBlocking )
			bIsBlocking = true;
	}

	// updates all layers and controls, bottop to top until blocking
	for ( int kk = Layers.GetSize() - 1; kk >= 0; kk-- )
	{
		CCtrlLayer* layer = Layers[ kk ];
		// was it removed? don't update anymore
		if ( ( layer->statusFlags & CCTRL_STATUS_FLAG_REMOVED ) != 0 )
			continue;
		// update all controls
		for ( int ll = 0; ll < layer->controls.GetSize(); ll++ )
		{
			// update visual focus percent in each control
			CControl* ctrl = layer->controls[ ll ];
			if ( !ctrl->bCanHaveFocus || ctrl->bDisabled || layer->nFocusedControlIdx != ll )
			{
				dec_limit( ctrl->fFocusPercent, 6.0f * dTime, 0.0f );
			}
			else if ( layer->nFocusedControlIdx == ll )
			{
				inc_limit( ctrl->fFocusPercent, 6.0f * dTime, 1.0f );
			}
			// update control now
			ctrl->Update( dTime, fLocalTimeline );
		}
		// was this layer blocking? stop updating layers
		if ( layer->bBlocking )
			break;
	}

	// controls were updated, see if layer needs removing
	for ( int kk = Layers.GetSize() - 1; kk >= 0; kk-- )
	{
		if ( ( Layers[ kk ]->statusFlags & CCTRL_STATUS_FLAG_REMOVED ) && ( Layers[ kk ]->alpha <= 0.0f ) )
		{
			SAFE_DELETE( Layers[ kk ] );
			Layers.Remove( kk );
		}
	}


	//--- update particles ---
	g_particlesMgr.UpdateLayer( K_PART_LAYER_CONTROLS_LIGHT, dTime );
}

void CControlsManager::Paint()
{
	Mat matTransform, mats;

	__Painter().Flush();

	if ( m_pCamera )
	{
		CCameraTransform::SetActiveCamera( m_pDevice, m_pCamera );
		m_cameraScreenRect = m_pCamera->GetCamWorldAABB();
	}

	//get the layer that gets the input for the focused controls
	CCtrlLayer* pInputLayer = GetTopmostInputLayer();

	for ( int ii = 0; ii < Layers.GetSize(); ii++ )
	{
		CCtrlLayer* lay = Layers[ ii ];

		float perc = TimeEasing( lay->alpha );
		if ( !lay->bAnimate )
			perc = 1.0f;

		Vec2i lpos = lay->GetPos();
		Vec2i anchor;
		anchor.x = m_cameraScreenRect.CenterX() + lay->anchorX * ( m_cameraScreenRect.w / 2 );
		anchor.y = m_cameraScreenRect.CenterY() + lay->anchorY * ( m_cameraScreenRect.h / 2 );
		//add anchor
		lpos.x += anchor.x;
		lpos.y += anchor.y;

		MUMatScaling( &matTransform, 0.9f + 0.1f * perc, 0.9f + 0.1f * perc, 1.0f );
		MUMatTranslation( &mats, lpos.x, lpos.y, 0.0f );
		matTransform *= mats;
		if ( m_pCamera )
		{
			matTransform *= m_pCamera->GetViewTransform();
		}
		__Painter().SetViewTransform( matTransform );

		bool bHideTabstop = false;
		if ( ( lay->nFocusedControlIdx >= 0 ) && ( lay->controls[ lay->nFocusedControlIdx ]->bShowFocusCursor == false ) )
			bHideTabstop = true;

		for ( int kk = 0; kk < lay->controls.Count(); kk++ )
		{
			//paint control
			lay->controls[ kk ]->Paint( lay->pControlsManager->m_pCamera, &matTransform );
		}
	}

#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
	//paints test points for pixel alignment
	//if (m_pCamera)
	//{
	//	UTSprite::PaintFrame(&m_sprCol, m_cameraScreenRect.x, m_cameraScreenRect.y, ANM_CONTROLS_SPR_ICONS_MISC, 2);
	//	UTSprite::PaintFrame(&m_sprCol, m_cameraScreenRect.Right(), m_cameraScreenRect.Bottom(), ANM_CONTROLS_SPR_ICONS_MISC, 2);
	//}
#endif
	//--- particles ---
	g_particlesMgr.PaintLayer( K_PART_LAYER_CONTROLS_LIGHT, true );

	__Painter().SetViewTransform( g_matIdentity );
}

CCtrlLayer* CControlsManager::ShowLayerOnce( CHAR* layerName, float fAlpha, int posX, int posY )
{
	if ( GetLayerByName( layerName ) == NULL )
		return ShowLayer( layerName, fAlpha, posX, posY );

	return nullptr;
}

CCtrlLayer* CControlsManager::ShowLayer( CHAR* layerName, float fAlpha, int posX, int posY )
{
	UINT32 layID = FastHash( layerName, strlen( layerName ) );
	int layIdx = -1;
	for ( int kk = 0; kk < layersDefinitions.GetSize(); kk++ )
	{
		if ( layersDefinitions[ kk ]->ID.textHash == layID )
		{
			layIdx = kk;
			break;
		}
	}
	if ( layIdx < 0 )
	{
		WCHAR txtline[ MAX_PATH ];
		mbstowcs( txtline, layerName, MAX_PATH );
		ErrorBox( K_ERR_WARNING, L"Layer definition not found! %s", txtline );
		return nullptr;
	}
	//daca am gasit layerul, deci il clonez
	CCtrlLayer *srclay = layersDefinitions[ layIdx ]->Clone();
	srclay->alpha = fAlpha;
	srclay->MoveLayer( posX, posY );
	srclay->statusFlags = 0;
	srclay->pControlsManager = this;
	Layers.Add( srclay );

	return srclay;
}

void CControlsManager::RemoveTopmostLayer( bool forced )
{
	if ( Layers.GetSize() == 0 )
		return;

	CCtrlLayer* layer = Layers[ Layers.GetSize() - 1 ];
	layer->statusFlags |= CCTRL_STATUS_FLAG_REMOVED;

	if ( forced )
		layer->statusFlags |= CCTRL_STATUS_FLAG_FORCED;
}

void CControlsManager::RemoveLayer( UINT32 layerID, bool forced )
{
	for ( int kk = Layers.GetSize() - 1; kk >= 0; kk-- )
	{
		if ( Layers[ kk ]->ID.textHash == layerID )
		{
			Layers[ kk ]->statusFlags |= CCTRL_STATUS_FLAG_REMOVED;
			if ( forced )
				Layers[ kk ]->statusFlags |= CCTRL_STATUS_FLAG_FORCED;

			return;
		}
	}
}

void CControlsManager::RemoveLayer( CHAR* layerName, bool forced )
{
	UINT32 dwLayerHash = FastHash( layerName );
	for ( int kk = Layers.GetSize() - 1; kk >= 0; kk-- )
	{
		if ( Layers[ kk ]->ID.textHash == dwLayerHash )
		{
			Layers[ kk ]->statusFlags |= CCTRL_STATUS_FLAG_REMOVED;
			if ( forced )
				Layers[ kk ]->statusFlags |= CCTRL_STATUS_FLAG_FORCED;

			return;
		}
	}
}


void CControlsManager::RemoveAllLayers( bool forced )
{
	//daca forced==true scade alpha imediat si le da remove imediat
	for ( int kk = 0; kk < Layers.GetSize(); kk++ )
	{
		Layers[ kk ]->statusFlags |= CCTRL_STATUS_FLAG_REMOVED;
		if ( forced )
			Layers[ kk ]->statusFlags |= CCTRL_STATUS_FLAG_FORCED;
	}

	return;
}

CCtrlLayer* CControlsManager::GetLayerByNameHash( UINT32 layerNameHash )
{
	for ( int kk = 0; kk < Layers.GetSize(); kk++ )
	{
		if ( Layers[ kk ]->ID.textHash == layerNameHash )
			return Layers[ kk ];
	}
	return nullptr;
}

CCtrlLayer* CControlsManager::GetLayerByName( CHAR* layerName )
{
	UINT32 layID = FastHash( layerName, strlen( layerName ) );
	for ( int kk = 0; kk < Layers.GetSize(); kk++ )
	{
		if ( Layers[ kk ]->ID.textHash == layID )
			return Layers[ kk ];
	}
	return nullptr;
}

CCtrlLayer* CControlsManager::GetLayerByIdx( int layerIdx )
{
	if ( ( layerIdx < 0 ) || ( layerIdx >= Layers.GetSize() ) )
		return nullptr;
	return Layers[ layerIdx ];
}

CCtrlLayer* CControlsManager::GetTopmostInputLayer()
{
	for ( int kk = Layers.GetSize() - 1; kk >= 0; kk-- )
	{
		if ( Layers[ kk ]->bGetsInput == true )
			return Layers[ kk ];
	}
	return nullptr;
}

/*----------------------------------*\
*  System/Framework
\*----------------------------------*/

OPRESULT CControlsManager::OnCreateDevice( PDEVICE pDevice, const SURFACE_DESC * pBBDesc )
{
	m_pDevice = pDevice;
	V_OP_RET( m_sprCol.OnCreateDevice( pDevice ) );
	return K_OP_OK;
}

OPRESULT CControlsManager::OnResetDevice( PDEVICE pDevice, const SURFACE_DESC * pBBDesc )
{
	m_pDevice = pDevice;
	V_OP_RET( m_sprCol.OnResetDevice( pDevice ) );
	return K_OP_OK;
}

OPRESULT CControlsManager::OnLostDevice()
{
	m_pDevice = nullptr;
	V_OP_RET( m_sprCol.OnLostDevice() );
	return K_OP_OK;
}

OPRESULT CControlsManager::OnDestroyDevice()
{
	m_pDevice = nullptr;
	V_OP_RET( m_sprCol.OnDestroyDevice() );
	return K_OP_OK;
}

void CControl::drawDebugText( int x, int y, const wchar_t* text, DWORD color )
{
#if defined(_DEBUG) || defined(DEBUG)
	if ( g_font6ns1 != null )
	{
		CStringDesc strdesc;
		__Texts().SetStringDesc( &strdesc, L"%s", text );
		g_font6ns1->DrawString( &strdesc, x, y, FONTFLAG_ANCHOR_TOPLEFT, color );
	}
#endif
}


///**************************************************************************************
/// SINGLETON
///**************************************************************************************

CControlsManager& UTGetGUI()
{
	static CControlsManager g_ControlsManager;
	return g_ControlsManager;
}


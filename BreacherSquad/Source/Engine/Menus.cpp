#include "dxstdafx.h"
#include "Menus.h"

CMenus::CMenus()
{
	m_pDevice = nullptr;
	m_state = GAME_STATE_EMPTY;
	m_nSubstate = 0;
	fLocalTimeline = 0.0f;
}

CMenus::~CMenus()
{
	Release();
}

OPRESULT CMenus::Init()
{
	WCHAR xmlpath[ MAX_PATH ];
	FileManager::GetMediaPath( L"media/interfaces/menus0.bsx", xmlpath );
	V_OP_RET(m_sprCol.LoadSprites( xmlpath ));

	return K_OP_OK;
}

void CMenus::Update( float dTime )
{
	fLocalTimeline += dTime;

	RectXYWH worldrect = UTApp().g_rect360hWorld;

	//generare particule bokeh
	if ( g_timers.Tick(400) )
	{
		__Particles().AddParticle(ANM_PARTICLES_SPR_BOKEH_L, false, randint(2), &D3DXVECTOR2(worldrect.Right() + randfloatsgn(50.0f), worldrect.h + 30.0f),
			&D3DXVECTOR2(-10.0f, -5.0f), &D3DXVECTOR2(-randfloat(20.0f), -20.0f - randfloat(10.0f)), 3.0f + randfloat(2.0f), 0.6f, 0.0f, 0.0f, 0.0f, 0.1f, 1.0f, 0x88ffffff, K_PART_LAYER_FRONT_LIGHT);
	}
	//particule foc
	if ( g_timers.Tick(90) )
	{
		__Particles().AddParticle(ANM_PARTICLES_SPR_FIRE_REAL, false, randint(12), &D3DXVECTOR2(worldrect.Right() + 10.0f, randfloat(worldrect.h / 2.0f) + worldrect.h * 0.25f),
			&D3DXVECTOR2(-4.0f, -8.0f), &D3DXVECTOR2(-40.0f - randfloat(20.0f), -10.0f - randfloat(10.0f)), 3.0f + randfloat(2.0f), 0.6f - randfloat(0.2f), -0.05f, 0.0f, randfloatsgn(PI), 0.1f, 1.0f, 0xffffffff, K_PART_LAYER_FRONT_LIGHT);
	}
	//particule foc spate
	if ( g_timers.Tick(60) )
	{
		__Particles().AddParticle(ANM_PARTICLES_SPR_FIRE_REAL, false, randint(12), &D3DXVECTOR2(worldrect.Right() + 10.0f, randfloat(worldrect.h / 2.0f) + worldrect.h * 0.25f),
			&D3DXVECTOR2(-8.0f, -4.0f), &D3DXVECTOR2(-40.0f - randfloat(20.0f), -10.0f - randfloat(10.0f)), 3.0f + randfloat(2.0f), 0.5f - randfloat(0.2f), -0.05f, 0.0f, randfloatsgn(PI), 0.1f, 1.0f, 0xaaffffff, K_PART_LAYER_NORMAL_LIGHT);
	}
}

void CMenus::Paint()
{
	RectXYWH camrect = UTApp().g_cam360hScreen.GetCamWorldAABB();

	PaintBackground( UTApp().g_rect360hWorld, 0xffffffff, true, true );

	__Painter().Flush();
}

void CMenus::PaintBackground( RectXYWH worldRect, DWORD dwColor, bool bPaintParticles /*= false*/, bool bPaintTitle /*= false*/ )
{
	const Vec2 vLogoPos( 110.0f, 60.0f );
	//background
	UTSprite::PaintFrame( &m_sprCol, Vec2( worldRect.x - 18.0f * sin( fLocalTimeline * 0.2f ), worldRect.y ), ANM_MENUS0_SPR_BACKGROUND, 0, dwColor );
	// chars back layer
	UTSprite::PaintFrame( &m_sprCol, Vec2( worldRect.x + 18.0f * sin( fLocalTimeline * 0.2f ), worldRect.y ), ANM_MENUS0_SPR_BACKGROUND, 1, dwColor );
	// chars front layer
	UTSprite::PaintFrame( &m_sprCol, Vec2( worldRect.x + 40.0f * sin( fLocalTimeline * 0.2f ), worldRect.y ), ANM_MENUS0_SPR_BACKGROUND, 2, dwColor );

	//particles
	if ( bPaintParticles )
	{
		//linear sampling
		m_pDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
		m_pDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );

		//particule foc
		__Particles().PaintLayer( K_PART_LAYER_NORMAL_LIGHT, true );

		m_pDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
		m_pDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
	}

	//paint character flickering orange light
	__Painter().AdditiveBlendingOn();
	float alpha = 0.4f + UTPerlin::PerlinNoise1D( fLocalTimeline, 3.0f, 2.0f, 0.4f, 0.5f, 2 );
	CLAMP( alpha, 0.0f, 1.0f );
	//paint flickering right glow
	UTSprite::PaintFrame( &m_sprCol, Vec2( worldRect.Right(), worldRect.Bottom() ), ANM_MENUS0_SPR_BACKGROUND, 3, DW_COLORALPHA( dwColor, alpha ) );
	// paint logo bg
	float alphatitle = 0.8f + sin( fLocalTimeline * 2.0f ) * 0.2f;
	DWORD dwTitleCol = DW_COLORALPHA( dwColor, alphatitle );
	if ( bPaintTitle )
	{
		// rays
		UTSprite::PaintFrame( &m_sprCol, vLogoPos, ANM_MENUS0_SPR_LOGO_MM, 2, dwTitleCol );
		// logo glow
		UTSprite::PaintFrame( &m_sprCol, vLogoPos, ANM_MENUS0_SPR_LOGO_MM, 1, dwTitleCol );
	}
	__Painter().AdditiveBlendingOff();
	// logo normal
	if ( bPaintTitle )
		UTSprite::PaintFrame( &m_sprCol, vLogoPos, ANM_MENUS0_SPR_LOGO_MM, 0 );
}

void CMenus::Release()
{
	m_sprCol.Release();
}

bool CMenus::HandleEvent( CEvent &nEvent )
{
	if ( nEvent.m_eventType == CEventTypes::evtT_INFO)
	{
		if ( nEvent.m_eventCommand == CEventCommands::evtC_GAMESTATE_CHANGE )
		{
			EGameState gameState = ( EGameState ) nEvent.GetArgumentByName( L"newGameState" )->m_asUINT32;
			// set internal state
			SetState( gameState );
		}
	}
	// don't consume event, just listen in
	return false;
}

void CMenus::SetState( EGameState neState )
{
	if ( neState == m_state )
		return;
	//#TODO: we can do stuff based on old state like deallocationg if necessary
		
	// reset some data on state change
	m_state = neState;
	m_nSubstate = 0;
	fLocalTimeline = 0.0f;

	switch ( neState )
	{
		case GAME_STATE_EMPTY:
			break;
		case GAME_STATE_PRELOAD:
			break;
		case GAME_STATE_DEVELOPER:
			break;
		case GAME_STATE_LOADING:
			break;
		case GAME_STATE_MAINMENU:
			break;
		case GAME_STATE_NET_LOBBY:
			break;
		case GAME_STATE_JOIN_COOP_LIST:
			break;
		case GAME_STATE_GAME_MODE_SELECTION:
			break;
		case GAME_STATE_CHAPTER_SELECTION:
			break;
		case GAME_STATE_LEVEL_SELECTION:
			break;
		case GAME_STATE_PLAYER_SELECTION:
			break;
		case GAME_STATE_GAME:
			break;
		case GAME_STATE_WORKSHOP:
			break;
		case GAME_STATE_CONTROLSED:
			break;
		case GAME_STATE_UPLOAD_MOD:
			break;
		default:
			break;
	}
}

OPRESULT CMenus::OnCreateDevice( PDEVICE pDevice, const SURFACE_DESC* pBBDesc /*= nullptr*/ )
{
	m_pDevice = pDevice;
	V_OP_RET( m_sprCol.OnCreateDevice( pDevice, pBBDesc ) );
	return K_OP_OK;
}

OPRESULT CMenus::OnResetDevice( PDEVICE pDevice, const SURFACE_DESC* pBBDesc /*= nullptr*/ )
{
	m_pDevice = pDevice;
	V_OP_RET( m_sprCol.OnResetDevice( pDevice, pBBDesc ) );
	return K_OP_OK;
}

OPRESULT CMenus::OnLostDevice()
{
	m_pDevice = nullptr;
	m_sprCol.OnLostDevice();
	return K_OP_OK;
}

OPRESULT CMenus::OnDestroyDevice()
{
	m_pDevice = nullptr;
	m_sprCol.OnDestroyDevice();
	return K_OP_OK;
}

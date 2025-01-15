#include "dxstdafx.h"
#include "LevelEditor.h"


#include <vector>
#include <string>
using namespace std;

#define K_BBOX_SCALE_BOX_SIZE 10


// light types by name, mostly used in the editor
// must correspond to eLightType
const char* K_LIGHT_TYPES_NAMES_ARR[] =
{
	"Ambient",
	"Projected",
	"Point",
	"Directional",
	"IES",
};



CLevelEditor::CLevelEditor() :
	m_pLevel( nullptr ), m_pDevice( nullptr ), m_pCam( nullptr ),
	eTool( K_LED_TILE ), eMod( K_LEM_NONE ), fTimeline( 0.0 ), pSelected( nullptr )
{
	m_vCamPos = { 0.0f, 0.0f };
	m_vCamPos_ini = m_vCamPos;

	vMouseWorld = Vec2( 0.0f, 0.0f );
	vMouseWorld_last = vMouseWorld;
}


CLevelEditor::~CLevelEditor()
{
	Close();
	Release();
}

void CLevelEditor::Release()
{
	m_sprCol.Release();
}


OPRESULT CLevelEditor::Launch( CLevel* level )
{
	//#TODO: load sprites here
	_ASSERT( level != nullptr );
	// only if level already loaded
	if ( !level->m_bLoaded )
		return K_OP_INVALIDARGS;

	// load necessary sprites from file
	WCHAR wsPath[MAX_PATH];
	swprintf_s( wsPath, MAX_PATH, L"%s/interfaces/lvled.bsx", UTApp().g_wszAppResDir );
	V_OP_RET( m_sprCol.LoadSprites( wsPath ) );

	// save pointer to current level
	m_pLevel = level;
	// select proper camera here for converting from level space to screen space
	m_pCam = &m_pLevel->m_camLevelToScr;
	m_vCamPos = Vec3XY( m_pCam->GetCamPos() );
	m_vCamPos_ini = m_vCamPos;

	eTool = K_LED_TILE;
	eMod = K_LEM_NONE;

	bLaunched = true;
	return K_OP_OK;
}


void CLevelEditor::Close()
{
	bLaunched = false;
	pSelected = nullptr;
	m_pLevel = nullptr;
	m_sprCol.Release();
}


void CLevelEditor::Update( float dTime )
{
	if ( !IsLaunched() )
		return;

	fTimeline += dTime;

	// don't do any processing if clicked on imgui
	if ( __ImGui().GetWantCaptureMouse() )
		return;

	/// mouse pos in level world
	vMouseWorld_last = vMouseWorld;
	vMouseWorld = m_pCam->ScreenToWorld( g_mouse.pos, &UTApp().g_rectRenderPP );
	Vec2 v_mouse_delta = vMouseWorld - vMouseWorld_last;

	// left mouse button
	if ( g_mouse.Lbut == K_MOUSE_BUTT_JUSTPRESSED )
	{
		switch ( eTool )
		{
			case K_LED_LIGHT:
			{
				if ( ( pSelected != nullptr ) && ( MUVec2Len( &( pSelected->pos.xy_proj - vMouseWorld ) ) < K_TILE_SIZE_F * 2.0f ) )
				{
					// move it
					eMod = K_LEM_MOVE;
				}
				else
				{
					pSelected = m_pLevel->SpawnLight( Vec3( vMouseWorld.x, vMouseWorld.y, 32.0f ), K_LVL_LT_POINT, 0xffffffff, 128.0f );
				}
			}
			break;
			case K_LED_PROP:
			{
				if ( pSelected != nullptr )
				{
					// move it
					eMod = K_LEM_MOVE;
				}
			}
			break;
		}
	}

	// right mouse button
	if ( g_mouse.Rbut == K_MOUSE_BUTT_JUSTPRESSED )
	{
		pSelected = SelectClosest( vMouseWorld );
	}

	/// Mouse moved ?
	if ( !UTMath::Vec2IsZero( v_mouse_delta ) )
	{
		bool bShiftDown = DXUTIsKeyDown( VK_LSHIFT ) || DXUTIsKeyDown( VK_RSHIFT );

		//#TODO: move this to ProcessMouseMove function
		switch ( eTool )
		{
			case K_LED_TILE:
				break;
			case K_LED_LIGHT:
				if ( eMod == K_LEM_MOVE )
				{
					if ( !bShiftDown )
						pSelected->Move( Vec3( v_mouse_delta.x, v_mouse_delta.y, 0.0f ) );
					else
						pSelected->Move( Vec3( 0.0f, 0.0f, -v_mouse_delta.y ) );
				}
				break;
			case K_LED_PROP:
				if ( eMod == K_LEM_MOVE )
				{
					if ( !bShiftDown )
						pSelected->Move( Vec3( v_mouse_delta.x, v_mouse_delta.y, 0.0f ) );
					else
						pSelected->Move( Vec3( 0.0f, 0.0f, -v_mouse_delta.y ) );
				}
				break;
			case K_LED_ACTOR:
				break;
			case K_LED_COLBOX:
				break;
			case K_LED_TOOLS_CNT:
				break;
			default:
				break;
		}
	}

	/// reset modifier on mouse up
	if ( g_mouse.Lbut == K_MOUSE_BUTT_JUSTRELEASED )
	{
		eMod = K_LEM_NONE;
	}

	/// Process realtime keys
	if ( DXUTIsKeyDown( 'A' ) )
	{
		m_vCamPos.x -= K_LED_CAMSPEED * dTime;
		if ( m_vCamPos.x < m_pLevel->m_levelAABB.x )
			m_vCamPos.x = m_pLevel->m_levelAABB.x;
	}
	if ( DXUTIsKeyDown( 'D' ) )
	{
		m_vCamPos.x += K_LED_CAMSPEED * dTime;
		if ( m_vCamPos.x > m_pLevel->m_levelAABB.Right() )
			m_vCamPos.x = m_pLevel->m_levelAABB.Right();
	}
	if ( DXUTIsKeyDown( 'W' ) )
	{
		m_vCamPos.y -= K_LED_CAMSPEED * dTime;
		if ( m_vCamPos.y < m_pLevel->m_levelAABB.y )
			m_vCamPos.y = m_pLevel->m_levelAABB.y;
	}
	if ( DXUTIsKeyDown( 'S' ) )
	{
		m_vCamPos.y += K_LED_CAMSPEED * dTime;
		if ( m_vCamPos.y > m_pLevel->m_levelAABB.Bottom() )
			m_vCamPos.y = m_pLevel->m_levelAABB.Bottom();
	}
}


void CLevelEditor::ReceiveKeys( UINT key )
{
	if ( !IsLaunched() )
		return;

	if ( __ImGui().GetWantCaptureKeyboard() )
		return;

	switch ( key )
	{
		case VK_HOME:
		{
			m_vCamPos = m_vCamPos_ini;
		}
		break;
		case VK_DELETE:
		{
			switch ( eTool )
			{
				case K_LED_TILE:
					break;
				case K_LED_LIGHT:
					if ( ( pSelected ) && ( pSelected->GetClassType() == K_LVL_IAI_TYPE_LIGHT ) )
					{
						pSelected->Kill();
						pSelected = nullptr;
					}
					break;
				case K_LED_PROP:
					break;
				case K_LED_ACTOR:
					break;
				case K_LED_COLBOX:
					break;
				case K_LED_TOOLS_CNT:
					break;
				default:
					break;
			}
		}
		break;
		case VK_UP:
		{
			int dY = 1;
			if ( DXUTIsKeyDown( VK_CONTROL ) )
				dY = 5;
		}
		break;
	}
}


OPRESULT CLevelEditor::SaveLevel( WCHAR* strPath )
{
	return K_OP_FAILED;
}


void CLevelEditor::SetTool( eLvlEdTool nTool )
{
	pSelected = nullptr;
	eTool = nTool;
	eMod = K_LEM_NONE;
}

void CLevelEditor::Paint()
{
	if ( m_pLevel == nullptr )
		return;
	_ASSERT( m_pDevice != nullptr );

	RectXYWH	camrect = UTApp().g_camScreen.GetCamWorldAABB();
	Mat			matCam = UTApp().g_camScreen.GetViewTransform();
	CAABB		camAABB( camrect );

	__Painter().SetViewTransform( matCam );

	switch ( eTool )
	{
		case K_LED_LIGHT:
		{
			for ( int kk = 0; kk < m_pLevel->m_visibleList.visible_lights.Count(); kk++ )
			{
				CLight* lg = m_pLevel->m_visibleList.visible_lights[kk];
				Vec2 lgproj = lg->pos.xy_proj;
				Vec2 vpos = m_pCam->WorldToScreen( lgproj, &UTApp().g_rectRenderPP );
				Vec2 vposprj = m_pCam->WorldToScreen( lg->pos.xy, &UTApp().g_rectRenderPP );

				DWORD lcol = ( pSelected == lg ) ? 0xffff2222 : 0xff22ff22;
				DrawVRuler( vposprj, vposprj.y - vpos.y, lcol );

				int anm = ( pSelected == lg ) ? ANM_LVLED_SPR_ICONS_BASE_SEL : ANM_LVLED_SPR_ICONS_BASE;
				int iconIdx = (int)lg->type;
				CLAMP( iconIdx, 0, m_sprCol.GetAFramesCnt( anm ) );
				UTSprite::PaintFrame( &m_sprCol, vpos.x, vpos.y, anm, iconIdx, 0xffffffff );
			}
		}
		break;

		case K_LED_PROP:
		{
			if ( pSelected )
			{
				// paint bbox
				CProp *pp = static_cast<CProp*>( pSelected );
				RectXYWH bb( pp->bbox.vMin.x, pp->bbox.vMin.y, pp->bbox.vSize.x, pp->bbox.vSize.y );
				RectXYWH bbfloor( pp->bbox_floor.vMin.x, pp->bbox_floor.vMin.y, pp->bbox_floor.vSize.x, pp->bbox_floor.vSize.y );

				RectXYWH prjrct_floor = m_pCam->WorldToScreen( bbfloor, &UTApp().g_rectRenderPP );
				DrawBBox( prjrct_floor, 0xffff8888 );
				RectXYWH prjrct = m_pCam->WorldToScreen( bb, &UTApp().g_rectRenderPP );
				DrawBBox( prjrct, 0xffffffff );

				// paint origin
				Vec2 vposprj = m_pCam->WorldToScreen( pp->pos.xy, &UTApp().g_rectRenderPP );
				UTSprite::PaintFrame( &m_sprCol, vposprj.x, vposprj.y, ANM_LVLED_SPR_CROSSHAIRS, 0, 0xffff2222 );

				// paint elevation
				if ( pp->pos.xyz.z != 0.0f )
				{
					DrawVRuler(vposprj, pp->pos.xyz.z, 0xffff2222);
				}
			}
		}
		break;
	}

	__Painter().Flush();
}


void CLevelEditor::IMGUI_ShowInterfaces()
{
	if ( !IsLaunched() )
		return;

	{
		//ImGuiViewport * vp = ImGui::GetWindowViewport();

		///--- TOOLS WINDOW
		ImGui::Begin( "Tools", null, ImGuiWindowFlags_NoNavInputs );
		const char* arrtools[] = { "Tiles", "Lights", "Props", "Actors", "CollBoxes" };

		for ( int kk = 0; kk < K_LED_TOOLS_CNT; kk++ )
		{
			if ( (eLvlEdTool)kk == eTool )
				ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.2f, 0.6f, 0.2f, 1.0f ) );
			else
				ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.2f, 0.3f, 0.2f, 1.0f ) );
			// tool selection here
			if ( ImGui::Button( arrtools[kk], ImVec2( 80, 0 ) ) )
			{
				SetTool( (eLvlEdTool)kk );
			}

			ImGui::PopStyleColor( 1 );
		}

		ImGui::Separator();
		ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.6f, 0.2f, 0.2f, 1.0f ) );
		ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4( 1.0f, 0.2f, 0.2f, 1.0f ) );
		if ( ImGui::Button( "Save All", ImVec2( 80, 0 ) ) )
		{
			//SaveLevel();
		}
		ImGui::PopStyleColor( 2 );
		ImGui::End();

		// get type of selected element
		EActiveInterfaceType selType = K_LVL_IAI_TYPE_UNKNOWN;
		if ( pSelected )
		{
			selType = (EActiveInterfaceType)pSelected->GetClassType();
		}

		///--- CONTROLS TEMPLATES
		ImGui::Begin( "Properties", null, ImGuiWindowFlags_NoNavInputs );

		switch ( selType )
		{
			case K_LVL_IAI_TYPE_LIGHT:
				IMGUI_AddLightProps( static_cast<CLight*>( pSelected ) );
				break;
			case K_LVL_IAI_TYPE_PROP:
				IMGUI_AddPropProps( static_cast<CProp*>( pSelected ) );
				break;
			case K_LVL_IAI_TYPE_ACTOR:
				break;
			case K_LVL_IAI_TYPE_COLSHAPE:
				break;
			case K_LVL_IAI_TYPE_BASE:
				// should never get here
				break;
			default:
				// nothing is selected
				break;
		}

		ImGui::End();
	}
}


IActiveInterface* CLevelEditor::SelectClosest( Vec2 vPoint, float fMaxRadius )
{
	if ( ( eTool < K_LED_TOOL_SELECTABLES_START ) || ( eTool >= K_LED_TOOLS_CNT ) )
		return nullptr;

	IActiveInterface* pSel = nullptr;
	float mindist = 100000.0f;
	switch ( eTool )
	{
		case K_LED_LIGHT:
		{
			for ( int kk = 0; kk < m_pLevel->m_visibleList.visible_lights.Count(); kk++ )
			{
				CLight* lg = m_pLevel->m_visibleList.visible_lights[kk];
				float dst = MUVec2Len( &( lg->pos.xy_proj - vPoint ) );
				if ( ( dst < fMaxRadius ) && ( dst < mindist ) )
				{
					mindist = dst;
					pSel = lg;
				}
			}
		}
		break;
		case K_LED_PROP:
		{
			for ( int kk = 0; kk < m_pLevel->m_visibleList.visible_props.Count(); kk++ )
			{
				CProp* lg = m_pLevel->m_visibleList.visible_props[kk];
				if ( lg->bbox.PointIn( vPoint ) )
				{
					pSel = lg;
					break;
				}
			}
		}
		break;
	}

	return pSel;
}


void CLevelEditor::IMGUI_AddLightProps( CLight* light )
{
	CSpriteLib* spr_lights = m_pLevel->m_sprLib.GetLibByNick( K_LIBNICK_LIGHTS );
	_ASSERT(spr_lights != nullptr && "Failed to get LIBNICK_LIGHTS");

	if ( light == nullptr )
	{
		ImGui::Text( "RMB to select" );
		return;
	}
	// type of light
	int ltype = (int)light->type;
	if ( ImGui::Combo( "Type", &ltype, K_LIGHT_TYPES_NAMES_ARR, IM_ARRAYSIZE( K_LIGHT_TYPES_NAMES_ARR ), IM_ARRAYSIZE( K_LIGHT_TYPES_NAMES_ARR ) ) )
	{
		light->type = (eLightType)ltype;
		light->UpdateInternalData( spr_lights );
	}

	ImGui::Separator();
	// custom data for each light type	
	switch ( light->type )
	{
		case K_LVL_LT_POINT:
		{
			// position
			float f3[3] = { light->pos.xyz.x, light->pos.xyz.y, light->pos.xyz.z };
			if ( ImGui::DragFloat3( "Pos", f3, 1.0f, -128.0f, 100000.0f, "%.2f" ) )
			{
				light->SetPos( Vec3( f3[0], f3[1], f3[2] ) );
			}
			// radius
			if ( ImGui::DragFloat( "Radius", &light->fRadius, 1.0f, 16.0f, 1000.0f, "%.2f" ) )
			{
				light->UpdateInternalData();
			}
			// intensity
			ImGui::DragFloat( "Intensity", &light->fIntensity, 0.01f, 0.1f, 5.0f, "%.2f" );
			// color
			ImVec4 color;
			DW_COLOR_GETARGB( light->color, color.w, color.x, color.y, color.z );
			ImGui::ColorEdit4( "Color", (float*)&color, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_DisplayHex );
			if ( ImGui::IsItemEdited() )
			{
				light->color = D3DCOLOR_COLORVALUE( color.x, color.y, color.z, 1.0f );
			}
			// cast shadows
			bool casts_shadows = light->GetCastShadows();
			if ( ImGui::Checkbox( "Shadows", &casts_shadows ) )
			{
				light->SetCastShadows( casts_shadows );
			}
		}
		break;

		case K_LVL_LT_PROJECTED_DIR:
		{
			// position
			float f3[3] = { light->pos.xyz.x, light->pos.xyz.y, light->pos.xyz.z };
			if ( ImGui::DragFloat3( "Pos", f3, 1.0f, 0.0f, 100000.0f, "%.2f" ) )
			{
				light->SetPos( Vec3( f3[0], f3[1], 0.0f ) );
			}
			// direction
			float d3[3] = { light->vnDir.x, light->vnDir.y, light->vnDir.z };
			if ( ImGui::DragFloat3( "Direction", d3, 0.02f, -1.0f, 1.0f, "%.2f" ) )
			{
				light->SetDir( Vec3( d3[0], d3[1], d3[2] ) );
				light->UpdateInternalData( spr_lights );
			}
			// intensity
			ImGui::DragFloat( "Intensity", &light->fIntensity, 0.01f, 0.1f, 5.0f, "%.2f" );
			// color
			ImVec4 color;
			DW_COLOR_GETARGB( light->color, color.w, color.x, color.y, color.z );
			ImGui::ColorEdit4( "Color", (float*)&color, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_DisplayHex );
			if ( ImGui::IsItemEdited() )
			{
				light->color = D3DCOLOR_COLORVALUE( color.x, color.y, color.z, 1.0f );
			}

			// show light textures in child window
			{
				ImGui::Separator();
				ImGui::Text( "Light Texture" );
				ImGui::BeginChild( "ChildL", ImVec2( ImGui::GetWindowContentRegionWidth(), 260 ), true, 0 );


				int anmID = light->fidTexture.animIdx;
				if ( anmID < 0 ) anmID = 0;
				if ( anmID > spr_lights->Animations.Count() - 1 ) anmID = 0;
				ImVec2 button_sz( 48, 48 );
				scAnimation* anm = spr_lights->Animations[anmID];
				PTEXTURE imgtex = spr_lights->Textures[0]->pTex;

				ImGuiStyle& style = ImGui::GetStyle();
				float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;

				for ( int n = 0; n < anm->aframesNo; n++ )
				{
					RectLTRB texrect = spr_lights->GetModuleRect_TexCoords( anmID, n, 0 );
					ImVec2 tul( texrect.left, texrect.top );
					ImVec2 tdr( texrect.right, texrect.bottom );

					ImVec4 bgcol( 0.0f, 0.0f, 0.0f, 1.0f );
					if ( n == light->fidTexture.frameIdx )
						bgcol = { 0.5f, 0.0f, 0.0f, 1.0f };

					ImGui::PushID( n );
					if ( ImGui::ImageButton( (void*)(intptr_t)imgtex, button_sz, tul, tdr, 2, bgcol ) )
					{
						light->SetLightTexture( spr_lights, anmID, n );
					}

					float last_button_x2 = ImGui::GetItemRectMax().x;
					float next_button_x2 = last_button_x2 + style.ItemSpacing.x + button_sz.x; // Expected position if next button was on same line
					if ( n + 1 < anm->aframesNo && next_button_x2 < window_visible_x2 )
						ImGui::SameLine();
					ImGui::PopID();
				}


				ImGui::EndChild();
			}

		}
		break;

		case K_LVL_LT_DIRECTIONAL:
		{
			// position
			float f3[3] = { light->pos.xyz.x, light->pos.xyz.y, light->pos.xyz.z };
			if ( ImGui::DragFloat3( "Pos", f3, 1.0f, 0.0f, 100000.0f, "%.2f" ) )
			{
				light->SetPos( Vec3( f3[0], f3[1], 0.0f ) );
			}
			// direction
			float d3[3] = { light->vnDir.x, light->vnDir.y, light->vnDir.z };
			if ( ImGui::DragFloat3( "Direction", d3, 0.02f, -1.0f, 1.0f, "%.2f" ) )
			{
				light->SetDir( Vec3( d3[0], d3[1], d3[2] ) );
				light->UpdateInternalData( spr_lights );
			}
			// intensity
			ImGui::DragFloat( "Intensity", &light->fIntensity, 0.01f, 0.1f, 5.0f, "%.2f" );
			// color
			ImVec4 color;
			DW_COLOR_GETARGB( light->color, color.w, color.x, color.y, color.z );
			ImGui::ColorEdit4( "Color", (float*)&color, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_DisplayHex );
			if ( ImGui::IsItemEdited() )
			{
				light->color = D3DCOLOR_COLORVALUE( color.x, color.y, color.z, 1.0f );
			}
		}
		break;

		case K_LVL_LT_AMBIENTAL:
		{
			// position
			float f3[3] = { light->pos.xyz.x, light->pos.xyz.y, light->pos.xyz.z };
			if ( ImGui::DragFloat3( "Pos", f3, 1.0f, 0.0f, 100000.0f, "%.2f" ) )
			{
				light->SetPos( Vec3( f3[0], f3[1], 0.0f ) );
			}
			// color
			ImVec4 color;
			DW_COLOR_GETARGB( light->color, color.w, color.x, color.y, color.z );
			ImGui::ColorEdit4( "Color", (float*)&color, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_DisplayHex );
			if ( ImGui::IsItemEdited() )
			{
				light->color = D3DCOLOR_COLORVALUE( color.x, color.y, color.z, 1.0f );
			}
		}
		break;

		case K_LVL_LT_IES:
		{
			// position
			float f3[3] = { light->pos.xyz.x, light->pos.xyz.y, light->pos.xyz.z };
			if ( ImGui::DragFloat3( "Pos", f3, 1.0f, 0.0f, 100000.0f, "%.2f" ) )
			{
				light->SetPos( Vec3( f3[0], f3[1], f3[2] ) );
			}
			// direction
			float d3[3] = { light->vnDir.x, light->vnDir.y, light->vnDir.z };
			if ( ImGui::DragFloat3( "Direction", d3, 0.02f, -1.0f, 1.0f, "%.2f" ) )
			{
				light->SetDir( Vec3( d3[0], d3[1], d3[2] ) );
				light->UpdateInternalData( spr_lights );
			}
			// radius
			if ( ImGui::DragFloat( "Radius", &light->fRadius, 1.0f, 16.0f, 1000.0f, "%.2f" ) )
			{
				light->UpdateInternalData();
			}
			// IES profile
			ImGui::InputInt( "IES Profile", &light->nProfileID, 1, 1 );
			// intensity
			ImGui::DragFloat( "Intensity", &light->fIntensity, 0.01f, 0.1f, 5.0f, "%.2f" );
			// color
			ImVec4 color;
			DW_COLOR_GETARGB( light->color, color.w, color.x, color.y, color.z );
			ImGui::ColorEdit4( "Color", (float*)&color, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_DisplayHex );
			if ( ImGui::IsItemEdited() )
			{
				light->color = D3DCOLOR_COLORVALUE( color.x, color.y, color.z, 1.0f );
			}
			// cast shadows
			bool casts_shadows = light->GetCastShadows();
			if ( ImGui::Checkbox( "Shadows", &casts_shadows ) )
			{
				light->SetCastShadows( casts_shadows );
			}
		}
		break;
	}

}

int CLevelEditor::IMGUI_AnimationBrowser( CSpriteLib* sprLib, int nSelectedAnim )
{
	_ASSERT( sprLib != nullptr );
	int ret_sel = -1;
	ImGui::BeginChild( "AnimationBrowser", ImVec2( ImGui::GetWindowContentRegionWidth(), 260 ), true, 0 );

	ImVec2 button_sz( 48, 48 );
	PTEXTURE imgtex = sprLib->Textures[0]->pTex;

	ImGuiStyle& style = ImGui::GetStyle();
	float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;

	for ( int n = 0; n < sprLib->Animations.Count(); n++ )
	{
		//scAnimation* anm = sprLib->Animations[n];

		RectLTRB texrect = sprLib->GetModuleRect_TexCoords( n, 0, 0 );
		ImVec2 tul( texrect.left, texrect.top );
		ImVec2 tdr( texrect.right, texrect.bottom );

		ImVec4 bgcol( 0.0f, 0.0f, 0.0f, 1.0f );
		bool bPushBorder = false;
		if ( n == nSelectedAnim )
		{
			bgcol = { 0.5f, 0.0f, 0.0f, 1.0f };
			ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 1.0f, 0.0f, 0.0f, 1.0f ) );
			bPushBorder = true;
		}

		ImGui::PushID( n );
		if ( ImGui::ImageButton( (void*)(intptr_t)imgtex, button_sz, tul, tdr, 2, bgcol ) )
		{
			ret_sel = n;
		}
		// remove border color for selection
		if ( bPushBorder )
			ImGui::PopStyleColor();

		float last_button_x2 = ImGui::GetItemRectMax().x;
		float next_button_x2 = last_button_x2 + style.ItemSpacing.x + button_sz.x; // Expected position if next button was on same line
		if ( n + 1 < sprLib->Animations.Count() && next_button_x2 < window_visible_x2 )
			ImGui::SameLine();
		ImGui::PopID();
	}

	ImGui::EndChild();
	return ret_sel;
}


void CLevelEditor::IMGUI_AddPropProps( CProp* prop )
{
	if ( prop == nullptr )
	{
		ImGui::Text( "RMB to select" );
		return;
	}

	CSpriteLib* spr_props = m_pLevel->m_sprLib.GetLibByNick( K_LIBNICK_PROPS );

	float f3[3] = { prop->pos.xyz.x, prop->pos.xyz.y, prop->pos.xyz.z };
	if ( ImGui::DragFloat3( "Pos", f3, 1.0f, 0.0f, 100000.0f, "%.2f" ) )
	{
		prop->SetPos( Vec3( f3[0], f3[1], f3[2] ) );
	}
	// cast shadows
	ImGui::Checkbox( "Animated", &prop->bAnimated);

	// show ANIMATIONS browser
	ImGui::Separator();
	ImGui::Text( "Animation" );

	int sel_anim = IMGUI_AnimationBrowser( spr_props, prop->sprite.animIdx );
	// on click
	if ( sel_anim >= 0 ) 
	{
		prop->sprite.SetAnim( sel_anim, 0 );
		prop->fid_ini.Set( prop->sprite.animIdx, prop->sprite.frameIdx );
		prop->PostConstructionInit();
	}

	// show frames in child window
	{
		ImGui::Separator();
		ImGui::Text( "Prop Frame" );
		ImGui::BeginChild( "ChildL", ImVec2( ImGui::GetWindowContentRegionWidth(), 260 ), true, 0 );

		int anmID = prop->sprite.animIdx;
		if ( anmID < 0 ) anmID = 0;
		if ( anmID > spr_props->Animations.Count() - 1 ) anmID = 0;
		ImVec2 button_sz( 48, 48 );
		scAnimation* anm = spr_props->Animations[anmID];
		PTEXTURE imgtex = spr_props->Textures[0]->pTex;

		ImGuiStyle& style = ImGui::GetStyle();
		float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;

		for ( int n = 0; n < anm->aframesNo; n++ )
		{
			RectLTRB texrect = spr_props->GetModuleRect_TexCoords( anmID, n, 0 );
			ImVec2 tul( texrect.left, texrect.top );
			ImVec2 tdr( texrect.right, texrect.bottom );

			ImVec4 bgcol( 0.0f, 0.0f, 0.0f, 1.0f );
			bool bPushBorder = false;
			if ( n == prop->sprite.frameIdx )
			{
				bgcol = { 0.5f, 0.0f, 0.0f, 1.0f };
				ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 1.0f, 0.0f, 0.0f, 1.0f ) );
				bPushBorder = true;
			}

			ImGui::PushID( n );
			if ( ImGui::ImageButton( (void*)(intptr_t)imgtex, button_sz, tul, tdr, 2, bgcol ) )
			{
				prop->sprite.frameIdx = n;
				prop->fid_ini.frameIdx = n;
				prop->PostConstructionInit();
			}
			// remove border color for selection
			if ( bPushBorder )
				ImGui::PopStyleColor();

			float last_button_x2 = ImGui::GetItemRectMax().x;
			float next_button_x2 = last_button_x2 + style.ItemSpacing.x + button_sz.x; // Expected position if next button was on same line
			if ( n + 1 < anm->aframesNo && next_button_x2 < window_visible_x2 )
				ImGui::SameLine();
			ImGui::PopID();
		}


		ImGui::EndChild();
	}
}

void CLevelEditor::DrawVRuler( Vec2 vBase, float fHeight, DWORD col )
{
	if ( fHeight >= 0.0f )
	{
		RectXYWHi cliprct( vBase.x - 10, vBase.y - fHeight, 20, fHeight + 10 );
		RectLTRB clipped( cliprct );
		UTSprite::PaintFModuleClipped( &m_sprCol, vBase, ANM_LVLED_SPR_RULERS, 0, 0, clipped, col );
	}
	else
	{
		RectXYWHi cliprct( vBase.x - 10, vBase.y - 10, 20, -fHeight + 10 );
		RectLTRB clipped( cliprct );
		UTSprite::PaintFModuleClipped( &m_sprCol, vBase, ANM_LVLED_SPR_RULERS, 1, 0, clipped, col );
	}
}

void CLevelEditor::DrawBBox( RectXYWH bbox, DWORD dwCol )
{
	RectXYWH cliprect( bbox.x - 1, bbox.y - 1, bbox.w + 2, 3 );
	UTSprite::PaintFrameClipped( &m_sprCol, Vec2( bbox.x, bbox.y ), ANM_LVLED_SPR_BBOX, 0, cliprect, dwCol );
	cliprect.y += bbox.h;
	UTSprite::PaintFrameClipped( &m_sprCol, Vec2( bbox.x, bbox.y + bbox.h ), ANM_LVLED_SPR_BBOX, 0, cliprect, dwCol );
	cliprect.Set( bbox.x - 1, bbox.y - 1, 3, bbox.h + 2 );
	UTSprite::PaintFrameClipped( &m_sprCol, Vec2( bbox.x, bbox.y ), ANM_LVLED_SPR_BBOX, 1, cliprect, dwCol );
	cliprect.x += bbox.w;
	UTSprite::PaintFrameClipped( &m_sprCol, Vec2( bbox.x + bbox.w, bbox.y ), ANM_LVLED_SPR_BBOX, 1, cliprect, dwCol );
}

void CLevelEditor::DrawHLine( Vec2 vStart, int length, DWORD dwCol )
{
	RectXYWH cliprect( vStart.x, vStart.y - 5, vStart.x + length, vStart.y + 5 );
	UTSprite::PaintFrameClipped( &m_sprCol, vStart, ANM_LVLED_SPR_BBOX, 0, cliprect, dwCol );
}


OPRESULT CLevelEditor::OnCreateDevice( PDEVICE pDevice, const SURFACE_DESC* pBBDesc )
{
	m_pDevice = pDevice;
	m_sprCol.OnCreateDevice( pDevice, pBBDesc );
	return K_OP_OK;
}

OPRESULT CLevelEditor::OnResetDevice( PDEVICE pDevice, const SURFACE_DESC* pBBDesc )
{
	m_pDevice = pDevice;
	m_sprCol.OnResetDevice( pDevice, pBBDesc );
	return K_OP_OK;
}

OPRESULT CLevelEditor::OnLostDevice()
{
	m_pDevice = nullptr;
	m_sprCol.OnLostDevice();
	return K_OP_OK;
}

OPRESULT CLevelEditor::OnDestroyDevice()
{
	m_pDevice = nullptr;
	m_sprCol.OnDestroyDevice();
	return K_OP_OK;
}


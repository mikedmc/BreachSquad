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
	m_pLevel(nullptr), m_pDevice(nullptr),
	eTool(K_LED_LIGHT)
{
}


CLevelEditor::~CLevelEditor()
{
	Close();
	Release();
}


OPRESULT CLevelEditor::Init()
{
	// load necessary sprites from file
	WCHAR wsPath[MAX_PATH];
	wsprintf(wsPath, L"%s/interfaces/lvled.bsx", UTGetAppClass().g_wszAppResDir);
	HRESULT hr = S_OK;
	if (FAILED(m_sprCol.LoadSprites(wsPath)))
	{
		return OPRESULT(K_OP_FAILED, K_SEVERITY_WARNING, L"CLevelEditor::Init: Couldn't find file: %s", wsPath);
	}

	return K_OP_OK;
}


void CLevelEditor::Release()
{
	m_sprCol.Release();
}


void CLevelEditor::Launch(CLevel* level)
{
	//#TODO: load sprites here
	_ASSERT(level != nullptr);
	// only if level already loaded
	if (!level->m_bLoaded)
		return;
	// save pointer to current level
	m_pLevel = level;
	m_vCamPos = Vec3ToVec2XY(level->m_camLevel.GetCamPos());
	m_vCamPos_ini = m_vCamPos;
}


void CLevelEditor::Close()
{
	m_pLevel = nullptr;
}


void CLevelEditor::Update(float dTime)
{
	if (!m_pLevel)
		return;
	// don't do any processing if clicked on imgui
	if (UTimgui().GetWantCaptureMouse())
		return;

	// mouse pos in level world
	Vec2 mousepos = m_pLevel->m_camLevel.ScreenToWorld(g_mouse.pos);

	// left mouse button
	if (g_mouse.Lbut == K_MOUSE_BUTT_JUSTPRESSED)
	{
		switch (eTool)
		{
			case K_LED_LIGHT:
			{
				if ((pSelected != nullptr) && (MUVec2Len(&(V3projV2(pSelected->vPos) - mousepos)) < K_TILE_HSIZE_F))
				{
					// move it
				}
				else
				{
					pSelected = m_pLevel->SpawnLight(Vec3(mousepos.x, mousepos.y, 32.0f), K_LVL_LT_POINT, 0xffffffff, 32.0f);
				}
			}
			break;
		}
	}

	// right mouse button
	if (g_mouse.Rbut == K_MOUSE_BUTT_JUSTPRESSED)
	{
		pSelected = SelectClosest(mousepos);
	}

	// Process realtime keys
	if (DXUTIsKeyDown('A'))
	{
		m_vCamPos.x -= K_LED_CAMSPEED * dTime;
		if (m_vCamPos.x < m_pLevel->m_levelAABB.x)
			m_vCamPos.x = m_pLevel->m_levelAABB.x;
	}
	if (DXUTIsKeyDown('D'))
	{
		m_vCamPos.x += K_LED_CAMSPEED * dTime;
		if (m_vCamPos.x > m_pLevel->m_levelAABB.Right())
			m_vCamPos.x = m_pLevel->m_levelAABB.Right();
	}
	if (DXUTIsKeyDown('W'))
	{
		m_vCamPos.y -= K_LED_CAMSPEED * dTime;
		if (m_vCamPos.y < m_pLevel->m_levelAABB.y)
			m_vCamPos.y = m_pLevel->m_levelAABB.y;
	}
	if (DXUTIsKeyDown('S'))
	{
		m_vCamPos.y += K_LED_CAMSPEED * dTime;
		if (m_vCamPos.y > m_pLevel->m_levelAABB.Bottom())
			m_vCamPos.y = m_pLevel->m_levelAABB.Bottom();
	}
}


void CLevelEditor::ReceiveKeys(UINT key)
{
	if (!IsLaunched())
		return;

	if (UTimgui().GetWantCaptureKeyboard())
		return;

	switch (key)
	{
		case VK_HOME:
		{
			m_vCamPos = m_vCamPos_ini;
		}
		break;
		case VK_DELETE:
		{
			switch (eTool)
			{
				case K_LED_TILE:
					break;
				case K_LED_LIGHT:
					if ((pSelected) && (pSelected->GetClassType() == K_LVL_IAI_TYPE_LIGHT))
					{
						pSelected->Kill();
						pSelected = nullptr;
					}
					break;
				case K_LED_OBJECT:
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
			if (DXUTIsKeyDown(VK_CONTROL))
				dY = 5;
		}
		break;
	}
}


OPRESULT CLevelEditor::SaveLevel(WCHAR* strPath)
{
	return K_OP_FAILED;
}


void CLevelEditor::Paint(ID3DXSprite* pSpr)
{
	if (m_pLevel == nullptr)
		return;
	_ASSERT(m_pDevice != nullptr);

	CCameraTransform::SetActiveCamera(m_pDevice, &UTGetAppClass().g_camScreen);

	switch (eTool)
	{
		case K_LED_LIGHT:
		{
			for (int kk = 0; kk < m_pLevel->m_visibleList.visible_lights.Count(); kk++)
			{
				CLight* lg = m_pLevel->m_visibleList.visible_lights[kk];
				Vec2 lgproj = V3projV2(lg->vPos);
				Vec2 vpos = m_pLevel->m_camLevel.WorldToScreen(lgproj);
				Vec2 vposprj = m_pLevel->m_camLevel.WorldToScreen(lg->pos);
				
				DWORD lcol = (pSelected == lg) ? 0xffff2222 : 0xff22ff22;
				DrawHRuler(vposprj, vposprj.y - vpos.y, lcol);

				int anm = (pSelected == lg) ? ANM_LVLED_SPR_ICONS_BASE_SEL : ANM_LVLED_SPR_ICONS_BASE;
				int iconIdx = (int)lg->type;
				CLAMP(iconIdx, 0, m_sprCol.GetAFramesCnt(anm));
				CSprite::paintFrame(&m_sprCol, vpos.x, vpos.y, anm, iconIdx, 0xffffffff);
			}
		}
		break;
	}

	pSpr->Flush();
}


void CLevelEditor::IMGUI_ShowInterfaces()
{
	// get type of selected element
	eActiveInterfaceType selType = K_LVL_IAI_TYPE_UNKNOWN;
	if (pSelected)
	{
		selType = (eActiveInterfaceType)pSelected->GetClassType();
	}

	{
		ImGuiViewport * vp = ImGui::GetWindowViewport();

		///--- TOOLS WINDOW
		ImGui::Begin("Tools", null, ImGuiWindowFlags_NoNavInputs);
		if (ImGui::Button("V Center", ImVec2(80, 0)))
		{
		}
		if (ImGui::Button("H Center", ImVec2(80, 0)))
		{
		}
		if (ImGui::Button("Pull Up", ImVec2(80, 0)))
		{
		}
		if (ImGui::Button("Push Down", ImVec2(80, 0)))
		{
		}
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.2f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
		if (ImGui::Button("Save All", ImVec2(80, 0)))
		{
			//SaveLevel();
		}
		ImGui::PopStyleColor(2);
		ImGui::End();


		///--- CONTROLS TEMPLATES
		ImGui::Begin("Properties", null, ImGuiWindowFlags_NoNavInputs);

		switch (selType)
		{
			case K_LVL_IAI_TYPE_LIGHT:
				IMGUI_AddLightProps(static_cast<CLight*>(pSelected));
				break;
			case K_LVL_IAI_TYPE_ACTIVE:
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


IActiveInterface* CLevelEditor::SelectClosest(Vec2 vPoint, float fMaxRadius)
{
	if ((eTool < K_LED_TOOL_SELECTABLES_START) || (eTool >= K_LED_TOOLS_CNT))
		return nullptr;

	IActiveInterface* pSel = nullptr;
	float mindist = 100000.0f;
	switch (eTool)
	{
		case K_LED_LIGHT:
		{
			for (int kk = 0; kk < m_pLevel->m_visibleList.visible_lights.Count(); kk++)
			{
				CLight* lg = m_pLevel->m_visibleList.visible_lights[kk];
				float dst = MUVec2Len(&(lg->pos - vPoint));
				if ((dst < fMaxRadius) && (dst < mindist))
				{
					mindist = dst;
					pSel = lg;
				}
			}
		}
		break;
	}

	return pSel;
}

void CLevelEditor::IMGUI_AddLightProps(CLight* light)
{
	// type of light
	int ltype = (int)light->type;
	if (ImGui::Combo("Type", &ltype, K_LIGHT_TYPES_NAMES_ARR, IM_ARRAYSIZE(K_LIGHT_TYPES_NAMES_ARR), IM_ARRAYSIZE(K_LIGHT_TYPES_NAMES_ARR)))
	{
		light->type = (eLightType)ltype;
		light->UpdateInternalData(&m_pLevel->m_sprLights);
	}

	ImGui::Separator();
	// custom data for each light type	
	switch (light->type)
	{
		case K_LVL_LT_POINT:
		{
			// position
			float f3[3] = { light->vPos.x, light->vPos.y, light->vPos.z };
			if (ImGui::DragFloat3("Pos", f3, 1.0f, -128.0f, 100000.0f, "%.2f"))
			{
				light->SetPos(Vec2(f3[0], f3[1]));
				light->vPos.z = f3[2];
			}
			// radius
			if (ImGui::DragFloat("Radius", &light->fRadius, 1.0f, 16.0f, 1000.0f, "%.2f"))
			{
				light->UpdateInternalData();
			}
			// intensity
			ImGui::DragFloat("Intensity", &light->fIntensity, 0.01f, 0.1f, 5.0f, "%.2f");
			// color
			ImVec4 color;
			D3DCOLOR_UNPACKTOFLOAT(light->color, color.w, color.x, color.y, color.z);
			ImGui::ColorEdit4("Color", (float*)&color, ImGuiColorEditFlags_HEX | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_DisplayHex);
			if (ImGui::IsItemEdited())
			{
				light->color = D3DCOLOR_COLORVALUE(color.x, color.y, color.z, 1.0f);
			}
			// cast shadows
			ImGui::Checkbox("Shadows", &light->castShadows);
		}
		break;

		case K_LVL_LT_PROJECTED_DIR:
		{
			// position
			float f3[3] = { light->vPos.x, light->vPos.y, light->vPos.z };
			if (ImGui::DragFloat3("Pos", f3, 1.0f, 0.0f, 100000.0f, "%.2f"))
			{
				light->SetPos(Vec2(f3[0], f3[1]));
				light->vPos.z = 0.0f;
			}
			// direction
			float d3[3] = { light->vnDir.x, light->vnDir.y, light->vnDir.z };
			if (ImGui::DragFloat3("Direction", d3, 0.02f, -1.0f, 1.0f, "%.2f"))
			{
				light->SetDir(Vec3(d3[0], d3[1], d3[2]));
				light->UpdateInternalData(&m_pLevel->m_sprLights);
			}
			// intensity
			ImGui::DragFloat("Intensity", &light->fIntensity, 0.01f, 0.1f, 5.0f, "%.2f");
			// color
			ImVec4 color;
			D3DCOLOR_UNPACKTOFLOAT(light->color, color.w, color.x, color.y, color.z);
			ImGui::ColorEdit4("Color", (float*)&color, ImGuiColorEditFlags_HEX | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_DisplayHex);
			if (ImGui::IsItemEdited())
			{
				light->color = D3DCOLOR_COLORVALUE(color.x, color.y, color.z, 1.0f);
			}

			// show light textures in child window
			{
				ImGui::Separator();
				ImGui::Text("Light Texture");
				ImGui::BeginChild("ChildL", ImVec2(ImGui::GetWindowContentRegionWidth(), 260), true, 0);


				const int anmID = light->animID;
				ImVec2 button_sz(48, 48);
				scAnimation* anm = m_pLevel->m_sprLights.Animations[anmID];
				PTEXTURE imgtex = m_pLevel->m_sprLights.Textures[0]->pTex;

				ImGuiStyle& style = ImGui::GetStyle();
				float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;

				for (int n = 0; n < anm->aframesNo; n++)
				{
					RECTLTRB_F texrect = m_pLevel->m_sprLights.GetModuleRect_TexCoords(anmID, n, 0);
					ImVec2 tul(texrect.left, texrect.top);
					ImVec2 tdr(texrect.right, texrect.bottom);

					ImVec4 bgcol(0.0f, 0.0f, 0.0f, 1.0f);
					if (n == light->frameID)
						bgcol = { 0.5f, 0.0f, 0.0f, 1.0f };

					ImGui::PushID(n);
					if (ImGui::ImageButton((void*)(intptr_t)imgtex, button_sz, tul, tdr, 1, bgcol))
					{
						light->SetLightTexture(&m_pLevel->m_sprLights, anmID, n);
					}

					float last_button_x2 = ImGui::GetItemRectMax().x;
					float next_button_x2 = last_button_x2 + style.ItemSpacing.x + button_sz.x; // Expected position if next button was on same line
					if (n + 1 < anm->aframesNo && next_button_x2 < window_visible_x2)
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
			float f3[3] = { light->vPos.x, light->vPos.y, light->vPos.z };
			if (ImGui::DragFloat3("Pos", f3, 1.0f, 0.0f, 100000.0f, "%.2f"))
			{
				light->SetPos(Vec2(f3[0], f3[1]));
				light->vPos.z = 0.0f;
			}
			// direction
			float d3[3] = { light->vnDir.x, light->vnDir.y, light->vnDir.z };
			if (ImGui::DragFloat3("Direction", d3, 0.02f, -1.0f, 1.0f, "%.2f"))
			{
				light->SetDir(Vec3(d3[0], d3[1], d3[2]));
				light->UpdateInternalData(&m_pLevel->m_sprLights);
			}
			// intensity
			ImGui::DragFloat("Intensity", &light->fIntensity, 0.01f, 0.1f, 5.0f, "%.2f");
			// color
			ImVec4 color;
			D3DCOLOR_UNPACKTOFLOAT(light->color, color.w, color.x, color.y, color.z);
			ImGui::ColorEdit4("Color", (float*)&color, ImGuiColorEditFlags_HEX | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_DisplayHex);
			if (ImGui::IsItemEdited())
			{
				light->color = D3DCOLOR_COLORVALUE(color.x, color.y, color.z, 1.0f);
			}
		}
		break;

		case K_LVL_LT_AMBIENTAL:
		{
			// position
			float f3[3] = { light->vPos.x, light->vPos.y, light->vPos.z };
			if (ImGui::DragFloat3("Pos", f3, 1.0f, 0.0f, 100000.0f, "%.2f"))
			{
				light->SetPos(Vec2(f3[0], f3[1]));
				light->vPos.z = 0.0f;
			}
			// color
			ImVec4 color;
			D3DCOLOR_UNPACKTOFLOAT(light->color, color.w, color.x, color.y, color.z);
			ImGui::ColorEdit4("Color", (float*)&color, ImGuiColorEditFlags_HEX | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_DisplayHex);
			if (ImGui::IsItemEdited())
			{
				light->color = D3DCOLOR_COLORVALUE(color.x, color.y, color.z, 1.0f);
			}
		}
		break;

		case K_LVL_LT_IES:
		{
			// position
			float f3[3] = { light->vPos.x, light->vPos.y, light->vPos.z };
			if (ImGui::DragFloat3("Pos", f3, 1.0f, 0.0f, 100000.0f, "%.2f"))
			{
				light->SetPos(Vec2(f3[0], f3[1]));
				light->vPos.z = f3[2];
			}
			// direction
			float d3[3] = { light->vnDir.x, light->vnDir.y, light->vnDir.z };
			if (ImGui::DragFloat3("Direction", d3, 0.02f, -1.0f, 1.0f, "%.2f"))
			{
				light->SetDir(Vec3(d3[0], d3[1], d3[2]));
				light->UpdateInternalData(&m_pLevel->m_sprLights);
			}
			// radius
			if (ImGui::DragFloat("Radius", &light->fRadius, 1.0f, 16.0f, 1000.0f, "%.2f"))
			{
				light->UpdateInternalData();
			}
			// IES profile
			ImGui::InputInt("IES Profile", &light->nProfileID, 1, 1);
			// intensity
			ImGui::DragFloat("Intensity", &light->fIntensity, 0.01f, 0.1f, 5.0f, "%.2f");
			// color
			ImVec4 color;
			D3DCOLOR_UNPACKTOFLOAT(light->color, color.w, color.x, color.y, color.z);
			ImGui::ColorEdit4("Color", (float*)&color, ImGuiColorEditFlags_HEX | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_DisplayHex);
			if (ImGui::IsItemEdited())
			{
				light->color = D3DCOLOR_COLORVALUE(color.x, color.y, color.z, 1.0f);
			}
			// cast shadows
			ImGui::Checkbox("Shadows", &light->castShadows);
		}
		break;
	}

}

void CLevelEditor::DrawHRuler(Vec2 vBase, float fHeight, DWORD col)
{
	if (fHeight >= 0.0f)
	{
		RECTXYWH cliprct(vBase.x - 10, vBase.y - fHeight, 20, fHeight + 10);
		CSprite::paintFrameClipped(&m_sprCol, vBase.x, vBase.y, ANM_LVLED_SPR_RULERS, 0, cliprct, col);
	}
	else
	{
		RECTXYWH cliprct(vBase.x - 10, vBase.y - 10, 20, -fHeight + 10);
		CSprite::paintFrameClipped(&m_sprCol, vBase.x, vBase.y, ANM_LVLED_SPR_RULERS, 1, cliprct, col);
	}
}

OPRESULT CLevelEditor::OnCreateDevice(PDEVICE pDevice, const SURFACE_DESC* pBBDesc)
{
	m_pDevice = pDevice;
	m_sprCol.OnCreateDevice(pDevice, pBBDesc);
	return K_OP_OK;
}

OPRESULT CLevelEditor::OnResetDevice(PDEVICE pDevice, const SURFACE_DESC* pBBDesc)
{
	m_pDevice = pDevice;
	m_sprCol.OnResetDevice(pDevice, pBBDesc);
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


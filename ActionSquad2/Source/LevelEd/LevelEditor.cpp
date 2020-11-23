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
	"Ambiental",
	"Area",
	"Point",
	"Directional",
	"IES"
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



/*
void CLevelEditor::IMGUI_AddCurControlProps()
{
	// SINGLE SELECTION:
	if ((currCtrlIdx < 0) || (selectedCtrls.GetSize() != 1))
		return;

	int ctrlIdx = -1;
	WCHAR type[MAX_PATH];
	StringCchPrintf(type, MAX_PATH, currLayer->controls[currCtrlIdx]->paramsDict.GetVariantByName(L"Type")->m_strArg.text);
	UINT id = FastHash(type);
	for (int ii = 0; ii < ctrlTemplates.Count(); ii++)
	{
		CVariantCollection* ctrl = ctrlTemplates.GetAt(ii);
		CVariantComplex* ctrlType = ctrl->m_variants.GetAt(0);
		if (id == ctrlType->m_strArg.getHash())
		{
			ctrlIdx = ii;
			break;
		}
	}
	if (ctrlIdx < 0)
		return;

	CControl* ctrl = currLayer->controls[currCtrlIdx];
	CVariantCollection* ctrlTemplate = ctrlTemplates.GetAt(ctrlIdx);

	if ((selectedCtrls.GetSize() == 1) && (currCtrlIdx >= 0))
	{
		ImGui::Separator();
		ImGui::TextDisabled("Control options");
		if (ImGui::Button("Clone Control", ImVec2(120, 0)))
		{
			CloneControl(10, 10);
		}
	}

	ImGui::Separator();
	ImGui::TextDisabled("Control properties");

	bool bNeedsUpdate = false;
	for (int ii = 1; ii < ctrlTemplate->m_variants.Count(); ii++)
	{
		// variable name from template
		CVariantComplex* pVarName = ctrlTemplate->m_variants[ii];
		// actual value from control
		CVariantComplex* pValue = ctrl->paramsDict.GetVariantByNameHash(pVarName->m_name.getHash());
		char sVarName[MAX_PATH];
		wcstombs(sVarName, pVarName->m_name.text, MAX_PATH);
		// hardcoded controls properties
		if (strcmp(sVarName, "ID") == 0)
		{
			char str0[MAX_PATH] = { 0 };
			// ID set? show it!
			if (pValue->m_type != CVariantComplex::K_ARGTYPE_NONE)
			{
				pValue->asString(str0, MAX_PATH);
			}

			ImGui::InputText(sVarName, str0, IM_ARRAYSIZE(str0));
			if (ImGui::IsItemEdited())
			{
				if (pValue->m_type == CVariantComplex::K_ARGTYPE_NONE)
				{
					int varidx = ctrl->paramsDict.SetNamedVarString(L"ID", L"");
					// set pointer on new var
					pValue = ctrl->paramsDict[varidx];
				}
				
				pValue->m_strArg.Init(str0);
				bNeedsUpdate = true;
			}
		}
		else if (strcmp(sVarName, "animID") == 0)
		{
			vector<string> arrAnims;
			// first animation will be the empty animation or not set. Index is -1
			// not set will be the first
			arrAnims.push_back("NOT SET");
			for (int ii = 0; ii < UTGetControlsManager().m_sprCol.Animations.Count(); ii++)
			{
				scAnimation *anm = UTGetControlsManager().m_sprCol.Animations.GetAt(ii);
				char strName[MAX_PATH];
				wcstombs(strName, anm->animName.text, MAX_PATH);
				arrAnims.push_back(strName);
			}

			int nRealIndex = (pValue->m_asUINT32 < 0) ? -1 : pValue->m_asUINT32;
			// we add 1 to bring it in [0..] domain
			int item_current_idx = nRealIndex + 1;

			const char* combo_label = arrAnims[item_current_idx].c_str();
			if (ImGui::BeginCombo(sVarName, combo_label, ImGuiComboFlags_PopupAlignLeft))
			{
				for (int n = 0; n < arrAnims.size(); n++)
				{
					const bool is_selected = (item_current_idx == n);
					if (ImGui::Selectable(arrAnims[n].c_str(), is_selected))
					{
						item_current_idx = n;
						// save back value
						pValue->m_asINT32 = item_current_idx - 1;
					}

					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		}
		else if (strcmp(sVarName, "align") == 0)
		{
			const char* anchor_names[3] = { "min", "center", "max" };
			const char* elem_name = (pValue->m_asINT32 >= -1 && pValue->m_asINT32 <= 1) ? anchor_names[pValue->m_asINT32 + 1] : "unknown";
			ImGui::SliderInt(sVarName, (int*)&pValue->m_asINT32, -1, 1, elem_name);

			if (ImGui::IsItemEdited())
			{
				const int anchor_values[] = { FONTFLAG_ANCHOR_LEFT, FONTFLAG_ANCHOR_CENTER, FONTFLAG_ANCHOR_RIGHT };
				// erase old flags
				int textAlignFlags = ctrl->paramsDict.GetVariantByName(L"nTextAlignFlags")->m_asINT32;
				textAlignFlags &= (~(FONTFLAG_ANCHOR_RIGHT | FONTFLAG_ANCHOR_CENTER | FONTFLAG_ANCHOR_LEFT ));
				// save new ones
				textAlignFlags |= anchor_values[pValue->m_asINT32 + 1];
				ctrl->paramsDict.SetNamedVarINT32(L"nTextAlignFlags", textAlignFlags);
				LOG("changed align %d", pValue->m_asINT32);
			}
		}
		else if (strcmp(sVarName, "valign") == 0)
		{
			const char* anchor_names[3] = { "min", "center", "max" };
			const char* elem_name = (pValue->m_asINT32 >= -1 && pValue->m_asINT32 <= 1) ? anchor_names[pValue->m_asINT32 + 1] : "unknown";
			ImGui::SliderInt(sVarName, (int*)&pValue->m_asINT32, -1, 1, elem_name);

			if (ImGui::IsItemEdited())
			{
				const int anchor_values[] = { FONTFLAG_ANCHOR_TOP, FONTFLAG_ANCHOR_VCENTER, FONTFLAG_ANCHOR_BOTTOM};
				// erase old flags
				int textAlignFlags = ctrl->paramsDict.GetVariantByName(L"nTextAlignFlags")->m_asINT32;
				textAlignFlags &= (~(FONTFLAG_ANCHOR_TOP| FONTFLAG_ANCHOR_VCENTER | FONTFLAG_ANCHOR_BOTTOM));
				// save new ones
				textAlignFlags |= anchor_values[pValue->m_asINT32 + 1];
				ctrl->paramsDict.SetNamedVarINT32(L"nTextAlignFlags", textAlignFlags);
				LOG("changed valign %d", pValue->m_asINT32);
			}
		}
		else if (strcmp(sVarName, "fontID") == 0)
		{
			vector<string> arrFonts;
			// first animation will be the empty animation or not set. Index is -1
			// not set will be the first
			arrFonts.push_back("NOT SET");
			for (int ii = 0; ii < UTGetFontsManager().fonts.Count(); ii++)
			{
				CTexturedFont* font = UTGetFontsManager().fonts.GetAt(ii);
				char strName[MAX_PATH];
				wcstombs(strName, font->shFontName.text, MAX_PATH);
				arrFonts.push_back(strName);
			}

			int nRealIndex = (pValue->m_asUINT32 < 0) ? -1 : pValue->m_asUINT32;
			// we add 1 to bring it in [0..] domain
			int item_current_idx = nRealIndex + 1;

			const char* combo_label = arrFonts[item_current_idx].c_str();
			if (ImGui::BeginCombo(sVarName, combo_label, ImGuiComboFlags_PopupAlignLeft))
			{
				for (int n = 0; n < arrFonts.size(); n++)
				{
					const bool is_selected = (item_current_idx == n);
					if (ImGui::Selectable(arrFonts[n].c_str(), is_selected))
					{
						item_current_idx = n;
						pValue->m_asINT32 = item_current_idx - 1;
					}

					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		}
		else if (strcmp(sVarName, "stringID") == 0)
		{
			// main string is kept as an integer (for speed) so we convert it to string to use it
			char str0[128] = " ";
			if (pValue->m_type == CVariantComplex::K_ARGTYPE_INT32)
			{
				wcstombs(str0, g_stringsMgr.strings[pValue->m_asINT32]->shStringName.text, 128);
			}

			ImGui::InputText(sVarName, str0, IM_ARRAYSIZE(str0));
			if (ImGui::IsItemEdited())
			{
				pValue->m_asINT32 = g_stringsMgr.getStrIdx(str0);
			}
		}
		else // non custom properties get treated by type
		{
			// generic control properties
			switch (pValue->m_type)
			{
				case CVariantComplex::K_ARGTYPE_HEXCOLOR:
				{
					ImVec4 color;
					D3DCOLOR_UNPACKTOFLOAT(pValue->m_asUINT32, color.w, color.x, color.y, color.z);
					
					// small color button
					ImGui::ColorEdit4("sVarName", (float*)&color, ImGuiColorEditFlags_HEX | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_DisplayHex);

					// full fledged color picker
					//ImGui::ColorPicker4(sVarName, (float*)&color, ImGuiColorEditFlags_HEX | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_DisplayHex);
					if (ImGui::IsItemEdited())
					{
						// re-pack color if changed
						pValue->m_asUINT32 = D3DCOLOR_COLORVALUE(color.x, color.y, color.z, color.w);
						bNeedsUpdate = true;
					}
				}
				break;
				case CVariantComplex::K_ARGTYPE_BOOL:
				{
					ImGui::Checkbox(sVarName, &pValue->m_asBool);
					if (ImGui::IsItemEdited())
					{
						bNeedsUpdate = true;
					}
				}
				break;
				case CVariantComplex::K_ARGTYPE_INT32:
				{
					ImGui::InputInt(sVarName, &pValue->m_asINT32);
					if (ImGui::IsItemEdited())
					{
						bNeedsUpdate = true;
					}
				}
				break;
				case CVariantComplex::K_ARGTYPE_FLOAT:
				{
					ImGui::InputFloat(sVarName, &pValue->m_asFloat);
					if (ImGui::IsItemEdited())
					{
						bNeedsUpdate = true;
					}
				}
				break;
				// string and all other types get treated as string
				case CVariantComplex::K_ARGTYPE_STRING:
				default:
				{
					char str0[MAX_PATH];
					pValue->asString(str0, MAX_PATH);

					ImGui::InputText(sVarName, str0, IM_ARRAYSIZE(str0));
					if (ImGui::IsItemEdited())
					{
						pValue->m_strArg.Init(str0);
						bNeedsUpdate = true;
					}
				}
				break;
			}
		}
	}

	// update control
	if (bNeedsUpdate)
	{
		UpdateControlDisplayProps(ctrl);
	}
}


void CLevelEditor::IMGUI_AddLayerProps()
{
	// NO SELECTION (layer properties)
	if ((currLayer != nullptr) && (currCtrlIdx < 0) && (selectedCtrls.GetSize() == 0))
	{
		ImGui::Separator();
		ImGui::TextDisabled("Layer properties");
		//ID
		char str0[128];
		wcstombs(str0, currLayer->ID.text, 128);
		ImGui::InputText("ID", str0, IM_ARRAYSIZE(str0));
		if (ImGui::IsItemEdited())
		{
			currLayer->ID.Init(str0);
		}

		ImGui::InputInt("X", &currLayer->X);
		ImGui::InputInt("Y", &currLayer->Y);

		const char* anchor_names[3] = { "min", "center", "max" };
		const char* elem_name = (currLayer->anchorX >= -1 && currLayer->anchorX <= 1) ? anchor_names[currLayer->anchorX + 1] : "unknown";
		ImGui::SliderInt("X anchor", (int*)&currLayer->anchorX, -1, 1, elem_name);

		elem_name = (currLayer->anchorY >= -1 && currLayer->anchorY <= 1) ? anchor_names[currLayer->anchorY + 1] : "unknown";
		ImGui::SliderInt("Y anchor", (int*)&currLayer->anchorY, -1, 1, elem_name);

		ImGui::Checkbox("Blocking", &currLayer->bBlocking);
		ImGui::Checkbox("Gets Input", &currLayer->bGetsInput);
		ImGui::InputFloat("DestroyTimer", &currLayer->fDestroyTimer, 0.5f, 100.0f, "%.1f", ImGuiConfigFlags_None);

		ImGui::Separator();

		char str1[128];
		wcstombs(str1, currLayer->shFocusedControlID.text, 128);
		ImGui::InputText("Focused Ctrl", str1, IM_ARRAYSIZE(str0));
		if (ImGui::IsItemEdited())
		{
			currLayer->shFocusedControlID.Init(str1);
		}
	}
}
*/



void CLevelEditor::Launch(CLevel* level)
{
	_ASSERT(level != nullptr);
	// only if level already loaded
	if (!level->m_bLoaded)
		return;
	// save pointer to current level
	m_pLevel = level;
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

	if (g_mouse.Lbut == K_MOUSE_BUTT_JUSTPRESSED)
	{
		pSelected = SelectClosest(mousepos);
	}
}


void CLevelEditor::ReceiveKeys(UINT key)
{
	if (UTimgui().GetWantCaptureKeyboard())
		return;

	switch (key)
	{
		case VK_HOME:
		{
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
				CSprite::paintFrame(&m_sprCol, vpos.x, vpos.y, ANM_LVLED_SPR_ICONS_BASE, 0, 0xffffffff);
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
		/*
		if (vp)
			ImGui::SetNextWindowPos(vp->Pos, ImGuiCond_Once);
			*/
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
	}

	// custom data for each light type	
	switch (light->type)
	{
		case K_LVL_LT_POINT:
		{
			// position
			float f3[3] = { light->vPos.x, light->vPos.y, light->vPos.z };
			if (ImGui::DragFloat3("Pos", f3, 1.0f, 0.0f, 100000.0f, "%.2f"))
			{
				light->SetPos(Vec2(f3[0], f3[1]));
				light->vPos.z = f3[2];
			}
			// radius
			ImGui::DragFloat("Radius", &light->fRadius, 1.0f, 16.0f, 1000.0f, "%.2f");
			// intensity
			ImGui::DragFloat("Intensity", &light->fIntensity, 0.01f, 0.1f, 5.0f, "%.2f");
			// color
			ImVec4 color;
			D3DCOLOR_UNPACKTOFLOAT(light->color, color.w, color.x, color.y, color.z);
			// small color button
			ImGui::ColorEdit4("Color", (float*)&color, ImGuiColorEditFlags_HEX | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_DisplayHex);
			if (ImGui::IsItemEdited())
			{
				// re-pack color if changed
				light->color = D3DCOLOR_COLORVALUE(color.x, color.y, color.z, 1.0f);
			}
		}
		break;
	}

}

void CLevelEditor::DrawHRuler(Vec2 vBase, float fHeight, DWORD col)
{
	RECTXYWH cliprct(vBase.x - 10, vBase.y - fHeight, 20, fHeight + 10);
	CSprite::paintFrameClipped(&m_sprCol, vBase.x, vBase.y, ANM_LVLED_SPR_RULERS, 0, cliprct, col);
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
	m_pDevice = NULL;
	m_sprCol.OnLostDevice();
	return K_OP_OK;
}

OPRESULT CLevelEditor::OnDestroyDevice()
{
	m_pDevice = NULL;
	m_sprCol.OnDestroyDevice();
	return K_OP_OK;
}


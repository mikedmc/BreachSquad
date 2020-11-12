#include "dxstdafx.h"
#include "LevelEditor.h"


#include <vector>
#include <string>
using namespace std;

#define K_BBOX_SCALE_BOX_SIZE 10


CLevelEditor::CLevelEditor() :
	m_pLevel(nullptr), m_pSprite(nullptr), m_pDevice(nullptr)
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
	if (FAILED(m_sprMgr.LoadSprites(wsPath)))
	{
		return OPRESULT(K_OP_FAILED, K_SEVERITY_WARNING, L"CLevelEditor::Init: Couldn't find file: %s", wsPath);
	}

	return K_OP_OK;
}


void CLevelEditor::Release()
{
	m_sprMgr.Release();
}


void CLevelEditor::DrawLine(int x1, int y1, int x2, int y2, D3DCOLOR col)
{
	VERT_TL1TC vertices[2];
	vertices[0].pos.x = x1; vertices[0].pos.y = y1; vertices[0].pos.z = 0.0f; vertices[0].pos.w = 1.0f;
	vertices[1].pos.x = x2; vertices[1].pos.y = y2;	vertices[1].pos.z = 0.0f; vertices[1].pos.w = 1.0f;
	vertices[0].color = vertices[1].color = col;
	m_pDevice->SetFVF(VERT_TL1TC::FVF);
	m_pDevice->DrawPrimitiveUP(D3DPT_LINELIST, 2, &vertices, sizeof(VERT_TL1TC));
}


void CLevelEditor::CloneCamTransform(CCameraTransform* pSrcCamera)
{
	camMain = *pSrcCamera;
}


void CLevelEditor::DrawBBox(RECTXYWH rect, bool selected)
{
	VERT_TL1TC vertices[5];
	//init verts
	for (int kk = 0; kk < 5; kk++)
	{
		vertices[kk].pos = D3DXVECTOR4(0.0f, 0.0f, 0.0f, 1.0f);
	}

	DWORD col = 0xffff0000;
	if (selected)
	{
		col = 0xff00ff00;
	}

	// BBox
	vertices[0].pos.x = rect.x; vertices[0].pos.y = rect.y;
	vertices[1].pos.x = rect.x + rect.w; vertices[1].pos.y = rect.y;
	vertices[2].pos.x = rect.x + rect.w; vertices[2].pos.y = rect.y + rect.h;
	vertices[3].pos.x = rect.x; vertices[3].pos.y = rect.y + rect.h;
	vertices[4].pos.x = rect.x; vertices[4].pos.y = rect.y;
 
	vertices[0].color = vertices[1].color = vertices[2].color = vertices[3].color = vertices[4].color = col;

	m_pDevice->SetFVF(VERT_TL1TC::FVF);
	m_pDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, &vertices, sizeof(VERT_TL1TC));
	
	if (selected)
	{
		// top-left scalespot
		vertices[0].pos.x = rect.x; vertices[0].pos.y = rect.y;
		vertices[1].pos.x = rect.x + K_BBOX_SCALE_BOX_SIZE; vertices[1].pos.y = rect.y;
		vertices[2].pos.x = rect.x + K_BBOX_SCALE_BOX_SIZE; vertices[2].pos.y = rect.y + K_BBOX_SCALE_BOX_SIZE;
		vertices[3].pos.x = rect.x; vertices[3].pos.y = rect.y + K_BBOX_SCALE_BOX_SIZE;
		vertices[4].pos.x = rect.x; vertices[4].pos.y = rect.y;

		vertices[0].color = vertices[1].color = vertices[2].color = vertices[3].color = vertices[4].color = col;

		m_pDevice->SetFVF(VERT_TL1TC::FVF);
		m_pDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, &vertices, sizeof(VERT_TL1TC));

	
		// top-right scalespot
		vertices[0].pos.x = rect.x + rect.w; vertices[0].pos.y = rect.y;
		vertices[1].pos.x = rect.x + rect.w; vertices[1].pos.y = rect.y + K_BBOX_SCALE_BOX_SIZE;
		vertices[2].pos.x = rect.x + rect.w - K_BBOX_SCALE_BOX_SIZE; vertices[2].pos.y = rect.y + K_BBOX_SCALE_BOX_SIZE;
		vertices[3].pos.x = rect.x + rect.w - K_BBOX_SCALE_BOX_SIZE; vertices[3].pos.y = rect.y;
		vertices[4].pos.x = rect.x + rect.w; vertices[4].pos.y = rect.y;

		vertices[0].color = vertices[1].color = vertices[2].color = vertices[3].color = vertices[4].color = col;

		m_pDevice->SetFVF(VERT_TL1TC::FVF);
		m_pDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, &vertices, sizeof(VERT_TL1TC));

		// bottom-right scalespot
		vertices[0].pos.x = rect.x + rect.w; vertices[0].pos.y = rect.y + rect.h;
		vertices[1].pos.x = rect.x + rect.w - K_BBOX_SCALE_BOX_SIZE; vertices[1].pos.y = rect.y + rect.h;
		vertices[2].pos.x = rect.x + rect.w - K_BBOX_SCALE_BOX_SIZE; vertices[2].pos.y = rect.y + rect.h - K_BBOX_SCALE_BOX_SIZE;
		vertices[3].pos.x = rect.x + rect.w; vertices[3].pos.y = rect.y + rect.h - K_BBOX_SCALE_BOX_SIZE;
		vertices[4].pos.x = rect.x + rect.w; vertices[4].pos.y = rect.y + rect.h;

		vertices[0].color = vertices[1].color = vertices[2].color = vertices[3].color = vertices[4].color = col;

		m_pDevice->SetFVF(VERT_TL1TC::FVF);
		m_pDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, &vertices, sizeof(VERT_TL1TC));

		// bottom-left scalespot
		vertices[0].pos.x = rect.x; vertices[0].pos.y = rect.y + rect.h;
		vertices[1].pos.x = rect.x; vertices[1].pos.y = rect.y + rect.h - K_BBOX_SCALE_BOX_SIZE;
		vertices[2].pos.x = rect.x + K_BBOX_SCALE_BOX_SIZE; vertices[2].pos.y = rect.y + rect.h - K_BBOX_SCALE_BOX_SIZE;
		vertices[3].pos.x = rect.x + K_BBOX_SCALE_BOX_SIZE; vertices[3].pos.y = rect.y + rect.h;
		vertices[4].pos.x = rect.x; vertices[4].pos.y = rect.y + rect.h;

		vertices[0].color = vertices[1].color = vertices[2].color = vertices[3].color = vertices[4].color = col;

		m_pDevice->SetFVF(VERT_TL1TC::FVF);
		m_pDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, &vertices, sizeof(VERT_TL1TC));

		// top-mid scalespot
		vertices[0].pos.x = rect.x + rect.w / 2 - K_BBOX_SCALE_BOX_SIZE / 2; vertices[0].pos.y = rect.y;
		vertices[1].pos.x = rect.x + rect.w / 2 + K_BBOX_SCALE_BOX_SIZE / 2; vertices[1].pos.y = rect.y;
		vertices[2].pos.x = rect.x + rect.w / 2 + K_BBOX_SCALE_BOX_SIZE / 2; vertices[2].pos.y = rect.y + K_BBOX_SCALE_BOX_SIZE;
		vertices[3].pos.x = rect.x + rect.w / 2 - K_BBOX_SCALE_BOX_SIZE / 2; vertices[3].pos.y = rect.y + K_BBOX_SCALE_BOX_SIZE;
		vertices[4].pos.x = rect.x + rect.w / 2 - K_BBOX_SCALE_BOX_SIZE / 2; vertices[4].pos.y = rect.y;

		vertices[0].color = vertices[1].color = vertices[2].color = vertices[3].color = vertices[4].color = col;

		m_pDevice->SetFVF(VERT_TL1TC::FVF);
		m_pDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, &vertices, sizeof(VERT_TL1TC));

		// bot-mid scalespot
		vertices[0].pos.x = rect.x + rect.w / 2 - K_BBOX_SCALE_BOX_SIZE / 2; vertices[0].pos.y = rect.y + rect.h;
		vertices[1].pos.x = rect.x + rect.w / 2 + K_BBOX_SCALE_BOX_SIZE / 2; vertices[1].pos.y = rect.y + rect.h;
		vertices[2].pos.x = rect.x + rect.w / 2 + K_BBOX_SCALE_BOX_SIZE / 2; vertices[2].pos.y = rect.y + rect.h - K_BBOX_SCALE_BOX_SIZE;
		vertices[3].pos.x = rect.x + rect.w / 2 - K_BBOX_SCALE_BOX_SIZE / 2; vertices[3].pos.y = rect.y + rect.h - K_BBOX_SCALE_BOX_SIZE;
		vertices[4].pos.x = rect.x + rect.w / 2 - K_BBOX_SCALE_BOX_SIZE / 2; vertices[4].pos.y = rect.y + rect.h;

		vertices[0].color = vertices[1].color = vertices[2].color = vertices[3].color = vertices[4].color = col;

		m_pDevice->SetFVF(VERT_TL1TC::FVF);
		m_pDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, &vertices, sizeof(VERT_TL1TC));

		// left-mid scalespot
		vertices[0].pos.x = rect.x; vertices[0].pos.y = rect.y + rect.h / 2 - K_BBOX_SCALE_BOX_SIZE / 2;
		vertices[1].pos.x = rect.x + K_BBOX_SCALE_BOX_SIZE; vertices[1].pos.y = rect.y + rect.h / 2 - K_BBOX_SCALE_BOX_SIZE / 2;
		vertices[2].pos.x = rect.x + K_BBOX_SCALE_BOX_SIZE; vertices[2].pos.y = rect.y + rect.h / 2 + K_BBOX_SCALE_BOX_SIZE / 2;
		vertices[3].pos.x = rect.x; vertices[3].pos.y = rect.y + rect.h / 2 + K_BBOX_SCALE_BOX_SIZE / 2;
		vertices[4].pos.x = rect.x; vertices[4].pos.y = rect.y + rect.h / 2 - K_BBOX_SCALE_BOX_SIZE / 2;

		vertices[0].color = vertices[1].color = vertices[2].color = vertices[3].color = vertices[4].color = col;

		m_pDevice->SetFVF(VERT_TL1TC::FVF);
		m_pDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, &vertices, sizeof(VERT_TL1TC));

		// right-mid scalespot
		vertices[0].pos.x = rect.x + rect.w; vertices[0].pos.y = rect.y + rect.h / 2 - K_BBOX_SCALE_BOX_SIZE / 2;
		vertices[1].pos.x = rect.x + rect.w - K_BBOX_SCALE_BOX_SIZE; vertices[1].pos.y = rect.y + rect.h / 2 - K_BBOX_SCALE_BOX_SIZE / 2;
		vertices[2].pos.x = rect.x + rect.w - K_BBOX_SCALE_BOX_SIZE; vertices[2].pos.y = rect.y + rect.h / 2 + K_BBOX_SCALE_BOX_SIZE / 2;
		vertices[3].pos.x = rect.x + rect.w; vertices[3].pos.y = rect.y + rect.h / 2 + K_BBOX_SCALE_BOX_SIZE / 2;
		vertices[4].pos.x = rect.x + rect.w; vertices[4].pos.y = rect.y + rect.h / 2 - K_BBOX_SCALE_BOX_SIZE / 2;

		vertices[0].color = vertices[1].color = vertices[2].color = vertices[3].color = vertices[4].color = col;

		m_pDevice->SetFVF(VERT_TL1TC::FVF);
		m_pDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, &vertices, sizeof(VERT_TL1TC));
	}
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


void CLevelEditor::SetSpritePtr(ID3DXSprite* pSprite)
{
	m_pSprite = pSprite;
}



void CLevelEditor::Launch(CLevel* level)
{
	m_pLevel = level;
}


void CLevelEditor::Close()
{
	m_pLevel = nullptr;
}


void CLevelEditor::Update(float dTime)
{
	// don't do any processing if clicked on imgui
	if (UTimgui().GetWantCaptureMouse())
		return;

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


void CLevelEditor::Paint()
{
	if (m_pLevel == nullptr)
		return;
	_ASSERT((m_pDevice != nullptr) && (m_pSprite != nullptr));

	CCameraTransform::SetActiveCamera(m_pDevice, &camMain);
}


void CLevelEditor::IMGUI_ShowInterfaces()
{
	/*
	{
		ImGuiViewport * vp = ImGui::GetWindowViewport();
		
		///--- TOOLS WINDOW
		ImGui::Begin("Tools", null, ImGuiWindowFlags_NoNavInputs);
		if (ImGui::Button("Hide BBox", ImVec2(80, 0)))
		{
			hideBBoxes = !hideBBoxes;
		}
		if (ImGui::Button("V Center", ImVec2(80, 0)))
		{
			CenterElements(false, true);
		}
		if (ImGui::Button("H Center", ImVec2(80, 0)))
		{
			CenterElements(true, false);
		}
		if (ImGui::Button("Pull Up", ImVec2(80, 0)))
		{
			ChangeControlPaintOrder(-1);
		}
		if (ImGui::Button("Push Down", ImVec2(80, 0)))
		{
			ChangeControlPaintOrder(1);
		}
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.2f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
		if (ImGui::Button("Save All", ImVec2(80, 0)))
		{
			SaveXML(UTGetControlsManager().loadedFile);
		}
		ImGui::PopStyleColor(2);

		ImGui::End();


		///--- CONTROLS TEMPLATES
		if (vp)
			ImGui::SetNextWindowPos(vp->Pos, ImGuiCond_Once);
		ImGui::Begin("Controls Templates", null, ImGuiWindowFlags_NoNavInputs);

		vector<string> arrItems;
		for (int ii = 0; ii < ctrlTemplates.Count(); ii++)
		{
			CVariantCollection *col = ctrlTemplates.GetAt(ii);
			CVariantComplex* var = col->m_variants.GetAt(0);
			char strName[MAX_PATH];
			wcstombs(strName, var->m_strArg.text, MAX_PATH);
			arrItems.push_back(strName);
		}

		if (ImGui::ListBoxHeader("##", ImVec2(250, 120)))
		{
			for (int kk = 0; kk < arrItems.size(); kk++)
			{
				auto item = arrItems[kk];
				if (ImGui::Selectable(item.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick))
				{
					if (ImGui::IsMouseDoubleClicked(0))
					{
						CVariantCollection* vcol = ctrlTemplates.GetAt(kk);
						AddControl(vcol);
					}
				}
			}
			ImGui::ListBoxFooter();
		}
		ImGui::End();

		///--- LAYERS LIST 
		ImGui::Begin("Interfaces", null, ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoNavFocus );

		vector<string> arrLayerNames;
		int nLayersCnt = UTGetControlsManager().layersDefinitions.Count();
		for (int ii = 0; ii < nLayersCnt; ii++)
		{
			CStringHash * lID = &UTGetControlsManager().layersDefinitions[ii]->ID;
			char strName[MAX_PATH];
			wcstombs(strName, lID->text, MAX_PATH);
			arrLayerNames.push_back(strName);
		}

		ImGui::SetNextItemWidth(-1.0f);
		if (ImGui::ListBoxHeader("##", ImVec2(250, 220)))
		{
			for (int kk = 0; kk < arrLayerNames.size(); kk++)
			{
				auto layer = arrLayerNames[kk];
				if (ImGui::Selectable(layer.c_str(), (kk == currLayerIdx) ? true : false, ImGuiSelectableFlags_None))
				{
					currCtrlIdx = -1;
					selectedCtrls.RemoveAll();
					clickedCtrls.RemoveAll();

					currLayerIdx = kk;
					currLayer = UTGetControlsManager().layersDefinitions.GetAt(kk);
					currLayer->pControlsManager = &UTGetControlsManager();
				}
			}
			ImGui::ListBoxFooter();
		}

		if (ImGui::Button("New Layer", ImVec2(120, 0)))
		{
			CCtrlLayer* nlayer = new CCtrlLayer();
			nlayer->bBlocking = _wtoi(layerTemplate.GetVariantByName(L"isBlocking")->m_strArg.text);
			nlayer->bGetsInput = _wtoi(layerTemplate.GetVariantByName(L"getsInput")->m_strArg.text);
			int lx = _wtoi(layerTemplate.GetVariantByName(L"X")->m_strArg.text);
			int ly = _wtoi(layerTemplate.GetVariantByName(L"Y")->m_strArg.text);
			nlayer->SetPos(lx, ly);
			nlayer->ID.Init(layerTemplate.GetVariantByName(L"ID")->m_strArg.text);
			nlayer->fDestroyTimer = layerTemplate.GetVariantByName(L"fTimer")->asFloat();
			nlayer->shFocusedControlID.Reset();

			nlayer->anchorX = K_CCTRL_LAYER_ANCHOR_CENTER;
			if (layerTemplate.GetVariantByName(L"anchorX")->m_strArg.getHash() == FastHash(L"min"))
				nlayer->anchorX = K_CCTRL_LAYER_ANCHOR_MIN;
			else if (layerTemplate.GetVariantByName(L"anchorX")->m_strArg.getHash() == FastHash(L"max"))
				nlayer->anchorX = K_CCTRL_LAYER_ANCHOR_MAX;

			nlayer->anchorY = K_CCTRL_LAYER_ANCHOR_CENTER;
			if (layerTemplate.GetVariantByName(L"anchorY")->m_strArg.getHash() == FastHash(L"min"))
				nlayer->anchorY = K_CCTRL_LAYER_ANCHOR_MIN;
			else if (layerTemplate.GetVariantByName(L"anchorY")->m_strArg.getHash() == FastHash(L"max"))
				nlayer->anchorY = K_CCTRL_LAYER_ANCHOR_MAX;

			UTGetControlsManager().layersDefinitions.Add(nlayer);

			int idx = UTGetControlsManager().layersDefinitions.Count() - 1;
			currCtrlIdx = -1;
		}
		if (ImGui::Button("Clone Layer", ImVec2(120, 0)))
		{
			if (currLayer == NULL)
				return;

			CCtrlLayer* nlayer = currLayer->Clone();
			testLayer = currLayer;
			WCHAR newName[MAX_PATH];
			StringCchPrintfW(newName, MAX_PATH, L"%s_%d", currLayer->ID.text, randint(100));
			nlayer->ID.Init(newName);
			UTGetControlsManager().layersDefinitions.Add(nlayer);

			int idx = UTGetControlsManager().layersDefinitions.Count() - 1;
			currCtrlIdx = -1;
		}
		ImGui::End();

		///--- CONTROLS/LAYERS PROPERTIES
		ImGui::Begin("Properties");
		vector<string> arrControlsNames;
		if ((currLayerIdx >= 0) && (currLayerIdx < UTGetControlsManager().layersDefinitions.GetSize()))
		{
			CCtrlLayer *layer = UTGetControlsManager().layersDefinitions.GetAt(currLayerIdx);
			for (int ii = 0; ii < layer->controls.Count(); ii++)
			{
				CControl* ctrl = layer->controls.GetAt(ii);
				wstring itemName = ctrl->paramsDict.GetVariantByName(L"Type")->m_strArg.text;
				if (ctrl->paramsDict.GetVariantByName(L"ID") && wcslen(ctrl->paramsDict.GetVariantByName(L"ID")->m_strArg.text) > 0)
				{
					itemName.append(L":");
					itemName.append(ctrl->paramsDict.GetVariantByName(L"ID")->m_strArg.text);
				}
				char strName[MAX_PATH];
				wcstombs(strName, itemName.c_str(), MAX_PATH);

				arrControlsNames.push_back(strName);
			}
		}

		ImGui::SetNextItemWidth(-1.0f);
		if (ImGui::ListBoxHeader("##", ImVec2(250, 200)))
		{
			for (int kk = 0; kk < arrControlsNames.size(); kk++)
			{
				string ctrl(arrControlsNames[kk]);
				char sID[MAX_PATH];
				sprintf(sID, "%s##ID%d", arrControlsNames[kk].c_str(), kk);
				if (ImGui::Selectable(sID, (kk == currCtrlIdx) ? true : false, ImGuiSelectableFlags_None))
				{
					if (!DXUTIsKeyDown(VK_CONTROL))
						selectedCtrls.RemoveAll();

					currCtrlIdx = kk;
					if (!selectedCtrls.Contains(currCtrlIdx))
					{
						selectedCtrls.Add(currCtrlIdx);
					}
					else
					{
						selectedCtrls.Remove(selectedCtrls.IndexOf(currCtrlIdx));
					}
				}
			}
			ImGui::ListBoxFooter();

			// selected control
			if (selectedCtrls.GetSize() == 1)
			{
				IMGUI_AddCurControlProps();
			}

			// current layer
			if (selectedCtrls.GetSize() == 0)
			{
				IMGUI_AddLayerProps();
			}
		}

		ImGui::End();

		//ImGui::Text("Hello from %s!", strName);
		//if (ImGui::Button("Close it"))
		//	bIsOpen = false;

		//  // List box
		//const char* items[] = { "Apple", "Banana", "Cherry", "Kiwi", "Mango", "Orange", "Pineapple", "Strawberry", "Watermelon" };
		//static int item_current = 1;
		//ImGui::ListBox("listbox\n(single select)", &item_current, items, IM_ARRAYSIZE(items), 8);
		//const bool controlsHovered = ImGui::IsItemActive();
		//if (controlsHovered && ImGui::IsMouseDoubleClicked(0))
		//{
		//	LOG("dblclk: %d", item_current);
		//}

		//static int listbox_item_current2 = 2;
		//ImGui::SetNextItemWidth(-1);
		//ImGui::ListBox("##listbox2", &listbox_item_current2, listbox_items, IM_ARRAYSIZE(listbox_items), 4);
	}
*/
}


OPRESULT CLevelEditor::OnCreateDevice(PDEVICE pDevice, const SURFACE_DESC* pBBDesc)
{
	m_pDevice = pDevice;
	m_sprMgr.OnCreateDevice(pDevice, pBBDesc);
	return K_OP_OK;
}

OPRESULT CLevelEditor::OnResetDevice(PDEVICE pDevice, const SURFACE_DESC* pBBDesc)
{
	m_pDevice = pDevice;
	m_sprMgr.OnResetDevice(pDevice, pBBDesc);
	return K_OP_OK;
}

OPRESULT CLevelEditor::OnLostDevice()
{
	m_pDevice = NULL;
	m_sprMgr.OnLostDevice();
	return K_OP_OK;
}

OPRESULT CLevelEditor::OnDestroyDevice()
{
	m_pDevice = NULL;
	m_sprMgr.OnDestroyDevice();
	return K_OP_OK;
}


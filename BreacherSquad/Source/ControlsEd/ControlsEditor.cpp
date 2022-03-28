#include "dxstdafx.h"

#include <vector>
#include <string>
using namespace std;

#define K_BBOX_SCALE_BOX_SIZE 10


CControlsEditor::CControlsEditor()
{
	currLayer = nullptr;
	currLayerIdx = -1;
	currCtrlIdx = -1;

	m_pDevice = nullptr;
}


CControlsEditor::~CControlsEditor()
{
	Close();
}

void CControlsEditor::DrawLine(int x1, int y1, int x2, int y2, D3DCOLOR col)
{
	VERT_TL1TC vertices[2];
	vertices[0].pos.x = x1; vertices[0].pos.y = y1; vertices[0].pos.z = 0.0f; vertices[0].pos.w = 1.0f;
	vertices[1].pos.x = x2; vertices[1].pos.y = y2;	vertices[1].pos.z = 0.0f; vertices[1].pos.w = 1.0f;
	vertices[0].color = vertices[1].color = col;
	m_pDevice->SetFVF(VERT_TL1TC::FVF);
	m_pDevice->DrawPrimitiveUP(D3DPT_LINELIST, 2, &vertices, sizeof(VERT_TL1TC));
}

void CControlsEditor::DrawBBox(RectXYWHi rect, bool selected)
{
	VERT_TL1TC vertices[5];
	//init verts
	for (int kk = 0; kk < 5; kk++)
	{
		vertices[kk].pos = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
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

	if (selected && selectedCtrls.Count() == 1)
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


OPRESULT CControlsEditor::LoadCtrlTemplatesXML(WCHAR* XMLpath)
{
	pugi::xml_document doc;
	if (!doc.load_file(XMLpath))
	{
		ErrorBox(K_ERR_WARNING, L"Unable to load Controls XML:%s\n", XMLpath);
		return K_OP_FAILED;
	}
	pugi::xml_node layerNode = doc.root().first_child();
	for (pugi::xml_attribute atr = layerNode.first_attribute(); atr; atr = atr.next_attribute())
	{
		WCHAR atrval[MAX_PATH];
		swprintf_s(atrval, MAX_PATH, atr.value());
		layerTemplate.SetNamedVarString(atr.name(), atrval);
	}
	pugi::xml_node controlsNodes = doc.root().child(L"Layer");
	for (pugi::xml_node ctrlNode = controlsNodes.child(L"Control"); ctrlNode; ctrlNode = ctrlNode.next_sibling(L"Control"))
	{
		CVariantCollection *nCol = new CVariantCollection();
		for (pugi::xml_attribute atr = ctrlNode.first_attribute(); atr; atr = atr.next_attribute())
		{
			WCHAR atrval[MAX_PATH];
			swprintf_s(atrval, MAX_PATH, atr.value());
			nCol->SetNamedVarString(atr.name(), atrval);
		}
		ctrlTemplates.Add(nCol);
	}

	return K_OP_OK;
}


void CControlsEditor::IMGUI_AddCurControlProps()
{
	// SINGLE SELECTION:
	if ((currCtrlIdx < 0) || (selectedCtrls.GetSize() != 1))
		return;

	int ctrlIdx = -1;
	WCHAR type[MAX_PATH];
	swprintf_s(type, MAX_PATH, currLayer->controls[currCtrlIdx]->paramsDict.GetVariantByName(L"Type")->m_strArg.text);
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
			for ( int jj = 0; jj < UTGetGUI().m_sprCol.Animations.Count(); jj++ )
			{
				scAnimation *anm = UTGetGUI().m_sprCol.Animations.GetAt( jj );
				char strName[ MAX_PATH ];
				wcstombs( strName, anm->animName.text, MAX_PATH );
				arrAnims.push_back( strName );
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
			for ( int jj = 0; jj < __TexFonts().fonts.Count(); jj++ )
			{
				CTexFont* font = __TexFonts().fonts.GetAt( jj );
				char strName[ MAX_PATH ];
				wcstombs( strName, font->shFontName.text, MAX_PATH );
				arrFonts.push_back( strName );
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
				wcstombs(str0, __Texts().strings[pValue->m_asINT32]->shStringName.text, 128);
			}

			ImGui::InputText(sVarName, str0, IM_ARRAYSIZE(str0));
			if (ImGui::IsItemEdited())
			{
				pValue->m_asINT32 = __Texts().GetStrIdx(str0);
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
					DW_COLOR_GETARGB(pValue->m_asUINT32, color.w, color.x, color.y, color.z);
					
					// small color button
					ImGui::ColorEdit4(sVarName, (float*)&color, ImGuiColorEditFlags_HEX | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_DisplayHex);

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


void CControlsEditor::IMGUI_AddLayerProps()
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

void CControlsEditor::AddControl(CVariantCollection* vcol)
{
	if (currLayer == NULL)
		return;

	WCHAR cType[MAX_PATH];
	swprintf_s(cType, MAX_PATH, vcol->GetVariantByName(L"Type")->m_strArg.text);

	CControl* nctrl = new CControl(cType);
	nctrl->Initialize();
	for (int ii = 0; ii < vcol->m_variants.Count(); ii++)
	{
		CVariantComplex *var = vcol->m_variants.GetAt(ii);
		WCHAR propertyName[MAX_PATH];
		WCHAR propertyValue[MAX_PATH];

		swprintf_s(propertyName, MAX_PATH, L"%s", var->m_name.text);
		swprintf_s(propertyValue, MAX_PATH, L"%s", var->m_strArg.text);
		//If template has "empty" as ID then don't add the ID key
		if ((wcscmp(propertyName, L"ID") == 0) && (wcscmp(propertyValue, L"empty") == 0))
			continue;
		
		UTGetGUI().SetParamValue(nctrl, propertyName, propertyValue, true);
	}
	nctrl->layer = currLayer;
	currLayer->controls.Add(nctrl);

	currCtrlIdx = currLayer->controls.Count() - 1;
	selectedCtrls.RemoveAll();
	selectedCtrls.Add(currCtrlIdx);
}

void CControlsEditor::CloneControl(int offx, int offy)
{
	if (currCtrlIdx < 0 || selectedCtrls.Count() > 1)
		return;

	CControl* ctrl = currLayer->controls.GetAt(currCtrlIdx);

	CControl *nctrl = new CControl(ctrl);

	// move it a little
	WCHAR val[MAX_PATH];
	swprintf_s(val, MAX_PATH, L"%d", ctrl->bbox.x + offx);
	UTGetGUI().SetParamValue(nctrl, L"X", val);
	swprintf_s(val, MAX_PATH, L"%d", ctrl->bbox.y + offy);
	UTGetGUI().SetParamValue(nctrl, L"Y", val);

	currLayer->controls.Add(nctrl);
}

void CControlsEditor::Launch()
{
	WCHAR xmlpath[MAX_PATH];
	swprintf_s(xmlpath, MAX_PATH, L"%sControlsEd/ctrlTemplates.xml", UTApp().g_wszExePath);
	LoadCtrlTemplatesXML(xmlpath);

	RectXYWH worldrect = UTApp().g_rect360hWorld;
	camera.SetWorldBounds( worldrect, true, K_CAMTRANS_AXIS_V, worldrect.h, worldrect.h );
	camera.InitCamera( UTApp().g_rectRender, worldrect.h, K_CAMTRANS_AXIS_V, worldrect.Center() );
	camera.SetCamAnimationNone();
	camera.Update( 0.0f );

	tool = TOOL_TYPE_NO_TOOL;
	hideBBoxes = false;
	offset = Vec2(0, 0);
}

void CControlsEditor::SaveXML(WCHAR* XMLpath)
{
	pugi::xml_document doc;
	
	doc.append_child(L"Interfaces");
	doc.child(L"Interfaces").append_attribute(L"Version");
	doc.child(L"Interfaces").append_attribute(L"SpriteCollection");
	doc.child(L"Interfaces").attribute(L"Version").set_value(L"1.0");
	doc.child(L"Interfaces").attribute(L"SpriteCollection").set_value(L"controls.bsx");
	for (int ii = 0; ii < UTGetGUI().layersDefinitions.Count(); ii++)
	{
		CCtrlLayer* layer = UTGetGUI().layersDefinitions.GetAt(ii);

		pugi::xml_node layerNode; 
		pugi::xml_attribute layerAttribute;

		layerNode  = doc.child(L"Interfaces").append_child(L"Layer");

		layerAttribute = layerNode.append_attribute(L"ID");
		layerAttribute.set_value(layer->ID.text);

		layerAttribute = layerNode.append_attribute(L"X");
		layerAttribute.set_value(layer->GetPos().x);

		layerAttribute = layerNode.append_attribute(L"Y");
		layerAttribute.set_value(layer->GetPos().y);

		layerAttribute = layerNode.append_attribute(L"isBlocking");
		layerAttribute.set_value(layer->bBlocking);

		layerAttribute = layerNode.append_attribute(L"getsInput");
		layerAttribute.set_value(layer->bGetsInput);

		//anchors
		layerAttribute = layerNode.append_attribute(L"anchorX");
		layerAttribute.set_value(layer->anchorX);

		layerAttribute = layerNode.append_attribute(L"anchorY");
		layerAttribute.set_value(layer->anchorY);

		if (layer->fDestroyTimer > 0.0f)
		{
			layerAttribute = layerNode.append_attribute(L"fTimer");
			layerAttribute.set_value(layer->fDestroyTimer);
		}

		if (!layer->shFocusedControlID.IsEmpty())
		{
			layerAttribute = layerNode.append_attribute(L"focusedControlID");
			layerAttribute.set_value(layer->shFocusedControlID.text);
		}

		for (int jj = 0; jj < layer->controls.Count(); jj++)
		{
			// dictionarul de date al controlului
			CVariantCollection* ctrlCol = &layer->controls[jj]->paramsDict;


			pugi::xml_node ctrlNode;
			ctrlNode = layerNode.append_child(L"Control");

			// tipul de control (Frame, Button, etc)
			WCHAR ctrlType[MAX_PATH];
			swprintf_s(ctrlType, MAX_PATH, ctrlCol->GetVariantByName(L"Type")->m_strArg.text);

			// parcurg ctrlTemplates ca sa scriu atributele exact in ordinea din templates
			for (int ll = 0; ll < ctrlTemplates.Count(); ll++)
			{
				CVariantCollection* lvcol = ctrlTemplates.GetAt(ll);
				if (wcscmp(ctrlType, lvcol->GetVariantByName(L"Type")->m_strArg.text) == 0)
				{
					for (int kk = 0; kk < lvcol->m_variants.Count(); kk++)
					{
						pugi::xml_attribute ctrlAttribute;
						CVariantComplex* var = lvcol->m_variants.GetAt(kk);
						if (ctrlCol->GetVariantByName(var->m_name.text))
						{
							WCHAR propertyName[MAX_PATH];
							WCHAR propertyValue[MAX_PATH];
							swprintf_s(propertyName, MAX_PATH, var->m_name.text);
							ctrlCol->GetVariantByName(propertyName)->asString(propertyValue, MAX_PATH);

							// ID is empty string or equals the one in templates then we skip it
							if (wcscmp(propertyName, L"ID") == 0)
								if (wcslen(ctrlCol->GetVariantByName(L"ID")->m_strArg.text) == 0
									|| wcscmp(ctrlCol->GetVariantByName(L"ID")->m_strArg.text, var->m_strArg.text) == 0)
									continue;
							//cazuri speciale ce trebuiesc traduse
							if (wcscmp(propertyName, L"animID") == 0)
							{
								int intVal = _wtoi(propertyValue);
								if (intVal >= 0 && wcscmp(propertyValue, L"_EMPTY_") != 0)
									swprintf_s(propertyValue, MAX_PATH, L"%s", UTGetGUI().m_sprCol.Animations[intVal]->animName.text);
								else
									swprintf_s(propertyValue, MAX_PATH, L"%s", var->m_strArg.text);
							}
							else if (wcscmp(propertyName, L"fontID") == 0)
							{
								int intVal = _wtoi(propertyValue);
								if (intVal >= 0 && wcscmp(propertyValue, L"_EMPTY_") != 0)
									swprintf_s(propertyValue, MAX_PATH, L"%s", __TexFonts().fonts[_wtoi(propertyValue)]->shFontName.text);
								else
									swprintf_s(propertyValue, MAX_PATH, L"%s", var->m_strArg.text);
							}
							else if (wcscmp(propertyName, L"stringID") == 0)
							{
								int strIdx = _wtoi(propertyValue);
								if((strIdx < 0) || (strIdx == __Texts().defaultStringIdx))
									swprintf_s(propertyValue, MAX_PATH, L"%s", var->m_strArg.text); //daca nu am pus id text pun ce era in template
								else
									swprintf_s(propertyValue, MAX_PATH, L"%s", __Texts().strings[strIdx]->shStringName.text);
							}
							else if ((wcscmp(propertyName, L"color") == 0) || (wcscmp(propertyName, L"fontColor") == 0))
							{
								// too short? save solid white. Otherwise the color has been converted to hexa already
								if ((wcscmp(propertyValue, L"0") == 0) || (wcslen(propertyValue) < 2))
								{
									swprintf_s(propertyValue, MAX_PATH, L"#ffffffff");
								}
							}

							ctrlAttribute = ctrlNode.append_attribute(propertyName);
							ctrlAttribute.set_value(propertyValue);
						}
					}
					break;
				}
			}
		}
	}

	FILE* file = OS_wfopen(XMLpath, L"w");
	if (file)
	{
		pugi::xml_writer_file writer(file);
		doc.save(writer);
	}
	OS_fclose(file);

	MessageBox(DXUTGetHWND(), XMLpath, L"Saved In", MB_OK);
}

void CControlsEditor::Close()
{
	// control templates
	for (int ii = 0; ii < ctrlTemplates.Count(); ii++)
	{
		CVariantCollection *col = ctrlTemplates.GetAt(ii);
		col->DeleteAll();
		SAFE_DELETE(col);
	}
	ctrlTemplates.RemoveAll();

	// release reference
	currLayer = nullptr;

	currCtrlIdx = -1;
	tool = TOOL_TYPE_NO_TOOL;
	selectedCtrls.RemoveAll();
}

static Vec2 vLastMouse;
void CControlsEditor::Update(float dTime)
{
	// update camera first
	camera.SetViewport( UTApp().g_rectRender );
	camera.Update( dTime );
	const RectXYWH camScreenRect = camera.GetCamWorldAABB();

	Vec2 vecRenderCenter(UTApp().g_rectRender.CenterX(), UTApp().g_rectRender.CenterY());

	clickedInterface = false;

	if (UTimgui().GetWantCaptureMouse())
		clickedInterface = true;

	if ((currLayer != nullptr) && (!clickedInterface))
	{
		if (g_mouse.Lbut == K_MOUSE_BUTT_JUSTPRESSED)
		{			
			vLastMouse = g_mouse.pos; //save last mouse
			if (currCtrlIdx > -1 && selectedCtrls.Count() == 1)
			{
				RectXYWHi bbox = currLayer->controls[currCtrlIdx]->GetBBox();
				
				bbox.x += currLayer->GetPos().x;
				bbox.y += currLayer->GetPos().y;

				RectXYWH rbbox(bbox);
				rbbox = camera.WorldToScreen(rbbox);

				rbbox.x += offset.x + vecRenderCenter.x;
				rbbox.y += offset.y + vecRenderCenter.y;
				bbox.Set(rbbox.x, rbbox.y, rbbox.w, rbbox.h);

				//verific daca dau click pe scale spot-uri
				RectXYWHi scaleSpotTL(bbox.x, bbox.y, K_BBOX_SCALE_BOX_SIZE, K_BBOX_SCALE_BOX_SIZE);
				RectXYWHi scaleSpotTM(bbox.x + bbox.w / 2 - K_BBOX_SCALE_BOX_SIZE / 2, bbox.y, K_BBOX_SCALE_BOX_SIZE, K_BBOX_SCALE_BOX_SIZE);
				RectXYWHi scaleSpotTR(bbox.x + bbox.w - K_BBOX_SCALE_BOX_SIZE, bbox.y, K_BBOX_SCALE_BOX_SIZE, K_BBOX_SCALE_BOX_SIZE);
				RectXYWHi scaleSpotBR(bbox.x + bbox.w - K_BBOX_SCALE_BOX_SIZE, bbox.y + bbox.h - K_BBOX_SCALE_BOX_SIZE, K_BBOX_SCALE_BOX_SIZE, K_BBOX_SCALE_BOX_SIZE);
				RectXYWHi scaleSpotBM(bbox.x + bbox.w / 2 - K_BBOX_SCALE_BOX_SIZE / 2, bbox.y + bbox.h - K_BBOX_SCALE_BOX_SIZE, K_BBOX_SCALE_BOX_SIZE, K_BBOX_SCALE_BOX_SIZE);
				RectXYWHi scaleSpotBL(bbox.x, bbox.y + bbox.h - K_BBOX_SCALE_BOX_SIZE, K_BBOX_SCALE_BOX_SIZE, K_BBOX_SCALE_BOX_SIZE);
				RectXYWHi scaleSpotLM(bbox.x, bbox.y + bbox.h / 2 - K_BBOX_SCALE_BOX_SIZE / 2, K_BBOX_SCALE_BOX_SIZE, K_BBOX_SCALE_BOX_SIZE);
				RectXYWHi scaleSpotRM(bbox.x + bbox.w - K_BBOX_SCALE_BOX_SIZE / 2, bbox.y + bbox.h / 2 - K_BBOX_SCALE_BOX_SIZE / 2, K_BBOX_SCALE_BOX_SIZE, K_BBOX_SCALE_BOX_SIZE);
				if (Rects::PointInRect(g_mouse.pos.x, g_mouse.pos.y, &scaleSpotTL))
				{
					tool = TOOL_TYPE_RESIZE_TOP_LEFT;
				}
				else if (Rects::PointInRect(g_mouse.pos.x, g_mouse.pos.y, &scaleSpotTM))
				{
					tool = TOOL_TYPE_RESIZE_TOP_MID;
				}
				else if (Rects::PointInRect(g_mouse.pos.x, g_mouse.pos.y, &scaleSpotTR))
				{
					tool = TOOL_TYPE_RESIZE_TOP_RIGHT;
				}
				else if (Rects::PointInRect(g_mouse.pos.x, g_mouse.pos.y, &scaleSpotBR))
				{
					tool = TOOL_TYPE_RESIZE_BOTT_RIGHT;
				}
				else if (Rects::PointInRect(g_mouse.pos.x, g_mouse.pos.y, &scaleSpotBM))
				{
					tool = TOOL_TYPE_RESIZE_BOTT_MID;
				}
				else if (Rects::PointInRect(g_mouse.pos.x, g_mouse.pos.y, &scaleSpotBL))
				{
					tool = TOOL_TYPE_RESIZE_BOTT_LEFT;
				}
				else if (Rects::PointInRect(g_mouse.pos.x, g_mouse.pos.y, &scaleSpotLM))
				{
					tool = TOOL_TYPE_RESIZE_LEFT_MID;
				}
				else if (Rects::PointInRect(g_mouse.pos.x, g_mouse.pos.y, &scaleSpotRM))
				{
					tool = TOOL_TYPE_RESIZE_RIGHT_MID;
				}
			}
		}
		if (g_mouse.Lbut == K_MOUSE_BUTT_DRAG)
		{
			SizeWH mousedelta(g_mouse.pos.x - vLastMouse.x, g_mouse.pos.y - vLastMouse.y);

			mousedelta = camera.ScreenToWorld(mousedelta);
			if (fabs(mousedelta.w) >= 1.0f)
			{
				vLastMouse.x = g_mouse.pos.x;
				mousedelta.w = (int)mousedelta.w;
			}
			else
			{
				mousedelta.w = 0.0f;
			}
			if (fabs(mousedelta.h) >= 1.0f)
			{
				vLastMouse.y = g_mouse.pos.y;
				mousedelta.h = (int)mousedelta.h;
			}
			else
			{
				mousedelta.h = 0.0f;
			}

			if (DXUTIsKeyDown(VK_SPACE))
			{
				offset.x += g_mouse.delta.x;
				offset.y += g_mouse.delta.y;
			}
			else
			{
				if (currCtrlIdx > -1)
				{
					if (tool == TOOL_TYPE_MOVE)
					{
						for (int ii = 0; ii < selectedCtrls.Count(); ii++)
						{
							int selCtrl = selectedCtrls.GetAt(ii);
							RectXYWHi bbox = currLayer->controls[selCtrl]->GetBBox();
							bbox.x += mousedelta.w;
							bbox.y += mousedelta.h;

							WCHAR val[MAX_PATH];
							CControl* ctrl = currLayer->controls[selCtrl];
							swprintf_s(val, MAX_PATH, L"%d", bbox.x);
							UTGetGUI().SetParamValue(ctrl, L"X", val);
							swprintf_s(val, MAX_PATH, L"%d", bbox.y);
							UTGetGUI().SetParamValue(ctrl, L"Y", val);
						}
					}
					else
					{
						RectXYWHi BBox = currLayer->controls[currCtrlIdx]->GetBBox();
						switch (tool)
						{
						case TOOL_TYPE_RESIZE_TOP_LEFT:
						{
							// modificari 
							BBox.x += mousedelta.w;
							BBox.y += mousedelta.h;
							BBox.w -= mousedelta.w;
							BBox.h -= mousedelta.h;
							// un fel de clamp pt coordonate (atunci cand atinge w/h minim incepea sa le mute)
							if (BBox.w < 1)
							{
								BBox.x -= (1 - BBox.w);
							}
							if (BBox.h < 1)
							{
								BBox.y -= (1 - BBox.h);
							}
						}
						break;
						case TOOL_TYPE_RESIZE_TOP_MID:
						{
							BBox.y += mousedelta.h;
							BBox.h -= mousedelta.h;
							if (BBox.h < 1)
							{
								BBox.y -= (1 - BBox.h);
							}
						}
						break;
						case TOOL_TYPE_RESIZE_TOP_RIGHT:
						{
							BBox.y += mousedelta.h;
							BBox.w += mousedelta.w;
							BBox.h -= mousedelta.h;
							if (BBox.h < 1)
							{
								BBox.y -= (1 - BBox.h);
							}
						}
						break;
						case TOOL_TYPE_RESIZE_BOTT_RIGHT:
						{
							BBox.w += mousedelta.w;
							BBox.h += mousedelta.h;
						}
						break;
						case TOOL_TYPE_RESIZE_BOTT_MID:
						{
							BBox.h += mousedelta.h;
						}
						break;
						case TOOL_TYPE_RESIZE_BOTT_LEFT:
						{
							BBox.x += mousedelta.w;
							BBox.w -= mousedelta.w;
							BBox.h += mousedelta.h;
							if (BBox.w < 1)
							{
								BBox.x -= (1 - BBox.w);
							}
						}
						break;
						case TOOL_TYPE_RESIZE_LEFT_MID:
						{
							BBox.x += mousedelta.w;
							BBox.w -= mousedelta.w;
							if (BBox.w < 1)
							{
								BBox.x -= (1 - BBox.w);
							}
						}
						break;
						case TOOL_TYPE_RESIZE_RIGHT_MID:
						{
							BBox.w += mousedelta.w;
						}
						break;
						}
						CLAMP(BBox.w, 1, (int)UTApp().g_rectRender.w);
						CLAMP(BBox.h, 1, (int)UTApp().g_rectRender.h);

						// le setez si in dictionar
						for (int ii = 0; ii < selectedCtrls.Count(); ii++)
						{
							int selCtrl = selectedCtrls.GetAt(ii);
							CControl* ctrl = currLayer->controls[selCtrl];
							WCHAR val[MAX_PATH];
							swprintf_s(val, MAX_PATH, L"%d", BBox.x);
							UTGetGUI().SetParamValue(ctrl, L"X", val);
							swprintf_s(val, MAX_PATH, L"%d", BBox.y);
							UTGetGUI().SetParamValue(ctrl, L"Y", val);
							swprintf_s(val, MAX_PATH, L"%d", BBox.w);
							UTGetGUI().SetParamValue(ctrl, L"W", val);
							swprintf_s(val, MAX_PATH, L"%d", BBox.h);
							UTGetGUI().SetParamValue(ctrl, L"H", val);
						}
					}
				}
			}
		}
		if (g_mouse.Lbut == K_MOUSE_BUTT_JUSTRELEASED)
		{
			tool = TOOL_TYPE_MOVE;
		}	
		if (g_mouse.Rbut == K_MOUSE_BUTT_JUSTPRESSED)
		{
			CArray<int> newClickedCtrls;
			for (int kk = currLayer->controls.Count() - 1; kk >= 0; kk--)
			{
				RectXYWHi bbox = currLayer->controls[kk]->GetBBox();
				
					bbox.x += currLayer->GetPos().x;
					bbox.y += currLayer->GetPos().y;

					RectXYWH rbbox(bbox);
					rbbox = camera.WorldToScreen(rbbox);

					rbbox.x += offset.x + vecRenderCenter.x;
					rbbox.y += offset.y + vecRenderCenter.y;
					bbox.Set(rbbox.x, rbbox.y, rbbox.w, rbbox.h);

				if (Rects::PointInRect(g_mouse.pos.x, g_mouse.pos.y, &bbox))
				{
					newClickedCtrls.Add(kk);
				}
			}

			// selectie layer
			if (newClickedCtrls.Count() == 0)
			{
				currCtrlIdx = -1;
				selectedCtrls.RemoveAll();
			}
			else
			{
				if (!DXUTIsKeyDown(VK_CONTROL))
				{
					// daca nu e selectie multipla golesc lista de controale selectate
					selectedCtrls.RemoveAll();

					bool sameCtrls = true;
					// verific daca dau click pe aceeasi multime de controale suprapuse
					if (newClickedCtrls.Count() != clickedCtrls.Count())
						sameCtrls = false;
					else
					{
						for (int ii = 0; ii < clickedCtrls.Count(); ii++)
						{
							if (clickedCtrls.GetAt(ii) != newClickedCtrls.GetAt(ii))
							{
								sameCtrls = false;
								break;
							}
						}
					}
					if (!sameCtrls) // daca nu apas pe aceeasi multime de controale trebuie facuta selectie noua
					{
						int newSelection = newClickedCtrls[0];
						currCtrlIdx = newSelection;
						selectedCtrls.RemoveAll();
						selectedCtrls.Add(currCtrlIdx);
					}
					else // daca apas pe aceeasi lista de controale trebuie sa schimb selectia prin rotatie
					{
						int ctrlIdx = clickedCtrls.IndexOf(currCtrlIdx);
						ctrlIdx++;
						if (ctrlIdx >= clickedCtrls.Count())
							ctrlIdx = 0;
						int newCtrl = clickedCtrls.GetAt(ctrlIdx);

						currCtrlIdx = newCtrl;
						selectedCtrls.RemoveAll();
						selectedCtrls.Add(currCtrlIdx);
					}
					clickedCtrls = newClickedCtrls;
				}
				else // selectie multipla
				{
					int newSelection = newClickedCtrls.GetAt(0);
					if (selectedCtrls.Contains(newSelection))
					{
						selectedCtrls.Remove(selectedCtrls.IndexOf(newSelection));
					}

					currCtrlIdx = newSelection;
					selectedCtrls.Add(currCtrlIdx);
				}
			}

			// remove focus when clicking outside the imgui windows
			ImGui::SetWindowFocus(NULL);
		}
	}	
}

void CControlsEditor::ReceiveKeys(UINT key)
{
	if (UTimgui().GetWantCaptureKeyboard())
		return;

	switch (key)
	{
		case VK_HOME:
		{
			offset.x = 0;
			offset.y = 0;
		}
		break;
		case VK_UP:
		{
			if (selectedCtrls.Count() == 0)
				return;

			int dY = 1;
			if (DXUTIsKeyDown(VK_CONTROL))
				dY = 5;
			for (int ii = 0; ii < selectedCtrls.Count(); ii++)
			{
				int ctrlIdx = selectedCtrls.GetAt(ii);
				CControl* ctrl = currLayer->controls[ctrlIdx];
				WCHAR val[MAX_PATH];
				if (DXUTIsKeyDown(VK_MENU))
				{
					swprintf_s(val, MAX_PATH, L"%d", ctrl->GetBBox().h - dY);
					UTGetGUI().SetParamValue(ctrl, L"H", val);
				}
				else
				{
					swprintf_s(val, MAX_PATH, L"%d", ctrl->GetBBox().y - dY);
					UTGetGUI().SetParamValue(ctrl, L"Y", val);
				}
			}
		}
		break;
		case VK_DOWN:
		{
			if (selectedCtrls.Count() == 0)
				return;

			int dY = 1;
			if (DXUTIsKeyDown(VK_CONTROL))
				dY = 5;
			for (int ii = 0; ii < selectedCtrls.Count(); ii++)
			{
				int ctrlIdx = selectedCtrls.GetAt(ii);
				CControl* ctrl = currLayer->controls[ctrlIdx];
				WCHAR val[MAX_PATH];
				if (DXUTIsKeyDown(VK_MENU))
				{
					swprintf_s(val, MAX_PATH, L"%d", ctrl->GetBBox().h + dY);
					UTGetGUI().SetParamValue(ctrl, L"H", val);
				}
				else
				{
					swprintf_s(val, MAX_PATH, L"%d", ctrl->GetBBox().y + dY);
					UTGetGUI().SetParamValue(ctrl, L"Y", val);
				}
			}
		}
		break;
		case VK_LEFT:
		{
			if (selectedCtrls.Count() == 0)
				return;

			int dX = 1;
			if (DXUTIsKeyDown(VK_CONTROL))
				dX = 5;
			for (int ii = 0; ii < selectedCtrls.Count(); ii++)
			{
				int ctrlIdx = selectedCtrls.GetAt(ii);
				CControl* ctrl = currLayer->controls[ctrlIdx];
				WCHAR val[MAX_PATH];
				if (DXUTIsKeyDown(VK_MENU))
				{
					swprintf_s(val, MAX_PATH, L"%d", ctrl->GetBBox().w - dX);
					UTGetGUI().SetParamValue(ctrl, L"W", val);
				}
				else
				{
					swprintf_s(val, MAX_PATH, L"%d", ctrl->GetBBox().x - dX);
					UTGetGUI().SetParamValue(ctrl, L"X", val);
				}
			}
		}
		break;
		case VK_RIGHT:
		{
			if (selectedCtrls.Count() == 0)
				return;

			int dX = 1;
			if (DXUTIsKeyDown(VK_CONTROL))
				dX = 5;
			for (int ii = 0; ii < selectedCtrls.Count(); ii++)
			{
				int ctrlIdx = selectedCtrls.GetAt(ii);
				CControl* ctrl = currLayer->controls[ctrlIdx];
				WCHAR val[MAX_PATH];
				if (DXUTIsKeyDown(VK_MENU))
				{
					swprintf_s(val, MAX_PATH, L"%d", ctrl->GetBBox().w + dX);
					UTGetGUI().SetParamValue(ctrl, L"W", val);
				}
				else
				{
					swprintf_s(val, MAX_PATH, L"%d", ctrl->GetBBox().x + dX);
					UTGetGUI().SetParamValue(ctrl, L"X", val);
				}
			}
		}
		break;
		case VK_DELETE:
		{
			if (currCtrlIdx >= 0 && selectedCtrls.Count() == 1)
				DeleteControl();
			else if (currLayer)
				DeleteLayer();
		}
		break;
	}
}

void CControlsEditor::DeleteLayer()
{
	int layidx = UTGetGUI().layersDefinitions.IndexOf(currLayer);
	UTGetGUI().layersDefinitions.Remove(layidx);
	SAFE_DELETE(currLayer);
}

void CControlsEditor::UpdateControlDisplayProps(CControl* ctrl)
{
	ctrl->bbox.x = ctrl->paramsDict.GetVariantByName(L"X")->m_asINT32;
	ctrl->bbox.y = ctrl->paramsDict.GetVariantByName(L"Y")->m_asINT32;
	ctrl->bbox.w = ctrl->paramsDict.GetVariantByName(L"W")->m_asINT32;
	ctrl->bbox.h = ctrl->paramsDict.GetVariantByName(L"H")->m_asINT32;
}

void CControlsEditor::ChangeControlPaintOrder(int dir)
{
	if (dir > 0)
	{
		if (currCtrlIdx > -1 && currCtrlIdx > 0)
		{
			SWAP(currLayer->controls[currCtrlIdx], currLayer->controls[currCtrlIdx - 1]);
			selectedCtrls.Remove(selectedCtrls.IndexOf(currCtrlIdx));
			currCtrlIdx--;
			selectedCtrls.Add(currCtrlIdx);
		}
	}
	else if (dir < 0)
	{
		if (currCtrlIdx > -1 && currCtrlIdx < (currLayer->controls.Count() - 1))
		{
			SWAP(currLayer->controls[currCtrlIdx], currLayer->controls[currCtrlIdx + 1]);
			selectedCtrls.Remove(selectedCtrls.IndexOf(currCtrlIdx));
			currCtrlIdx++;
			selectedCtrls.Add(currCtrlIdx);
		}
	}
}

void CControlsEditor::CenterElements(bool H, bool V)
{
	if (H)
	{
		if (currLayer == nullptr)
			return;

		if (selectedCtrls.Count() > 1)
		{
			int xmin = UTApp().g_rectRender.w;
			int xmax = -UTApp().g_rectRender.w;

			for (int ii = 0; ii < selectedCtrls.Count(); ii++)
			{
				CControl* ctrl = currLayer->controls[selectedCtrls[ii]];
				int ctrlX = ctrl->bbox.x;
				int ctrlW = ctrl->bbox.w;
				if (ctrlX < xmin)
					xmin = ctrlX;
				if (ctrlX + ctrlW > xmax)
					xmax = ctrlX + ctrlW;
			}

			int dx = (xmax - xmin) / 2;
			for (int ii = 0; ii < selectedCtrls.Count(); ii++)
			{
				CControl* ctrl = currLayer->controls[selectedCtrls[ii]];
				int ctrlX = ctrl->bbox.x;
				WCHAR val[MAX_PATH];
				int intVal = -(xmin - ctrlX) - dx;
				swprintf_s(val, MAX_PATH, L"%d", intVal);
				UTGetGUI().SetParamValue(ctrl, L"X", val);
			}
		}
		else if (selectedCtrls.Count() == 1)
		{
			if (currCtrlIdx > -1)
			{
				WCHAR val[MAX_PATH];
				CControl* lCtrl = currLayer->controls[currCtrlIdx];
				int ctrlW = lCtrl->bbox.w;
				int intVal = -(ctrlW / 2);
				swprintf_s(val, MAX_PATH, L"%d", intVal);
				UTGetGUI().SetParamValue(lCtrl, L"X", val);
			}
		}
		else if (selectedCtrls.Count() == 0)
		{
			currLayer->X = 0;
		}
	}

	if (V)
	{
		if (currLayer == nullptr)
			return;

		if (selectedCtrls.Count() > 1)
		{
			int ymin = UTApp().g_rectRender.h;
			int ymax = -UTApp().g_rectRender.h;

			for (int ii = 0; ii < selectedCtrls.Count(); ii++)
			{
				CControl* ctrl = currLayer->controls[selectedCtrls[ii]];
				int ctrlY = ctrl->bbox.y;
				int ctrlH = ctrl->bbox.h;
				if (ctrlY < ymin)
					ymin = ctrlY;
				if (ctrlY + ctrlH > ymax)
					ymax = ctrlY + ctrlH;
			}

			int dy = (ymax - ymin) / 2;
			for (int ii = 0; ii < selectedCtrls.Count(); ii++)
			{
				CControl* ctrl = currLayer->controls[selectedCtrls[ii]];
				WCHAR val[MAX_PATH];
				int ctrlY = ctrl->bbox.y;
				int intVal = -(ymin - ctrlY) - dy;
				swprintf_s(val, MAX_PATH, L"%d", intVal);
				UTGetGUI().SetParamValue(ctrl, L"Y", val);
			}
		}
		else  if (selectedCtrls.Count() == 1)
		{
			if (currCtrlIdx > -1)
			{
				WCHAR val[MAX_PATH];
				CControl* lCtrl = currLayer->controls[currCtrlIdx];
				int ctrlH = lCtrl->bbox.h;
				int intVal = -(ctrlH / 2);
				swprintf_s(val, MAX_PATH, L"%d", intVal);
				UTGetGUI().SetParamValue(currLayer->controls[currCtrlIdx], L"Y", val);
			}
		}
		else if (selectedCtrls.Count() == 0)
		{
			currLayer->Y = 0;
		}
	}
}

void CControlsEditor::DeleteControl()
{
	SAFE_DELETE(currLayer->controls[currCtrlIdx]);
	currLayer->controls.Remove(currCtrlIdx);

	selectedCtrls.RemoveAll();
	currCtrlIdx = -1;
}

void CControlsEditor::Paint()
{
	// #TODO: the editor should have own camera to allow us panning and zooming
	if (currLayer)
	{
		Vec2i lpos = currLayer->GetPos();
		Vec2 vecRenderCenter( UTApp().g_rectRender.CenterX(), UTApp().g_rectRender.CenterY() );

		Mat mcam = camera.GetViewTransform();
		Mat matscroll;
		Vec2 scrCenter( vecRenderCenter.x + offset.x + lpos.x, vecRenderCenter.y + offset.y + lpos.y );
		scrCenter = camera.ScreenToWorld( scrCenter );
		MUMatAffine2D( &matscroll, 1.0f, NULL, 0.0f, &scrCenter );
		mcam = matscroll * mcam;

		__Painter().SetViewTransform( mcam );
		__Painter().SetTransform( g_matIdentity );
		// paint controls
		for (int kk = 0; kk < currLayer->controls.Count(); kk++)
		{
			currLayer->controls[kk]->Paint(&UTGetGUI().camera, &g_matIdentity);
		}
	}
}


void CControlsEditor::IMGUI_ShowInterfaces()
{
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
			WCHAR xmlpath[ MAX_PATH ];
			FileManager::GetMediaPath( L"media/interfaces/interfaces.xml", xmlpath, true);
			SaveXML(xmlpath);
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
		int nLayersCnt = UTGetGUI().layersDefinitions.Count();
		for (int ii = 0; ii < nLayersCnt; ii++)
		{
			CStringHash * lID = &UTGetGUI().layersDefinitions[ii]->ID;
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
					currLayer = UTGetGUI().layersDefinitions.GetAt(kk);
					currLayer->pControlsManager = &UTGetGUI();
				}
			}
			ImGui::ListBoxFooter();
		}

		if (ImGui::Button("New Layer", ImVec2(120, 0)))
		{
			CCtrlLayer* nlayer = new CCtrlLayer();
			nlayer->bBlocking = (bool)(_wtoi(layerTemplate.GetVariantByName(L"isBlocking")->m_strArg.text) != 0);
			nlayer->bGetsInput = (bool)(_wtoi(layerTemplate.GetVariantByName(L"getsInput")->m_strArg.text) != 0);
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

			UTGetGUI().layersDefinitions.Add(nlayer);

			//int idx = UTGetGUI().layersDefinitions.Count() - 1;
			currCtrlIdx = -1;
		}
		if (ImGui::Button("Clone Layer", ImVec2(120, 0)))
		{
			if (currLayer == nullptr)
				return;

			CCtrlLayer* nlayer = currLayer->Clone();
			testLayer = currLayer;
			WCHAR newName[MAX_PATH];
			swprintf_s(newName, MAX_PATH, L"%s_%d", currLayer->ID.text, randint(100));
			nlayer->ID.Init(newName);
			UTGetGUI().layersDefinitions.Add(nlayer);

			//int idx = UTGetGUI().layersDefinitions.Count() - 1;
			currCtrlIdx = -1;
		}
		ImGui::End();

		///--- CONTROLS/LAYERS PROPERTIES
		ImGui::Begin("Properties");
		vector<string> arrControlsNames;
		if ((currLayerIdx >= 0) && (currLayerIdx < UTGetGUI().layersDefinitions.GetSize()))
		{
			CCtrlLayer *layer = UTGetGUI().layersDefinitions.GetAt(currLayerIdx);
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

		/*
		ImGui::Text("Hello from %s!", strName);
		if (ImGui::Button("Close it"))
			bIsOpen = false;
		  */


		  // List box
		/*
		const char* items[] = { "Apple", "Banana", "Cherry", "Kiwi", "Mango", "Orange", "Pineapple", "Strawberry", "Watermelon" };
		static int item_current = 1;
		ImGui::ListBox("listbox\n(single select)", &item_current, items, IM_ARRAYSIZE(items), 8);
		const bool controlsHovered = ImGui::IsItemActive();
		if (controlsHovered && ImGui::IsMouseDoubleClicked(0))
		{
			LOG("dblclk: %d", item_current);
		}
		*/

		//static int listbox_item_current2 = 2;
		//ImGui::SetNextItemWidth(-1);
		//ImGui::ListBox("##listbox2", &listbox_item_current2, listbox_items, IM_ARRAYSIZE(listbox_items), 4);

	}

}

void CControlsEditor::PaintBBoxes()
{
	m_pDevice->SetTexture(0, null);

	if (hideBBoxes)
		return;
	Vec2 vecRenderCenter(UTApp().g_rectRender.CenterX(), UTApp().g_rectRender.CenterY());
	//axis
	int w = UTApp().g_rectRender.w;
	int h = UTApp().g_rectRender.h;

	DrawLine(w / 2 + offset.x, 0, w / 2 + offset.x, h);
	DrawLine(0, h / 2 + offset.y, w, h / 2 + offset.y);

	if (currLayer)
	{
		// desenare bounding box-uri
		for (int kk = 0; kk < currLayer->controls.Count(); kk++)
		{
			RectXYWHi rect;
			rect = currLayer->controls[kk]->GetBBox();
			rect.x += currLayer->GetPos().x;
			rect.y += currLayer->GetPos().y;

			Vec2 vul(rect.x, rect.y);
			SizeWH rsz(rect.w, rect.h);
			vul = camera.WorldToScreen(vul);
			//add screen space coords
			vul = vul + vecRenderCenter + offset;

			rsz = camera.WorldToScreen(rsz);
			rect.x = vul.x; rect.y = vul.y;
			rect.w = rsz.w; rect.h = rsz.h;

			if (selectedCtrls.Contains(kk))
				DrawBBox(rect, true);
			else
				DrawBBox(rect, false);
		}
	}

}

OPRESULT CControlsEditor::OnCreateDevice( PDEVICE pDevice, const SURFACE_DESC * pBBDesc )
{
	m_pDevice = pDevice;
	return K_OP_OK;
}

OPRESULT CControlsEditor::OnResetDevice( PDEVICE pDevice, const SURFACE_DESC * pBBDesc )
{
	m_pDevice = pDevice;
	return K_OP_OK;
}

OPRESULT CControlsEditor::OnLostDevice()
{
	m_pDevice = nullptr;
	return K_OP_OK;
}

OPRESULT CControlsEditor::OnDestroyDevice()
{
	m_pDevice = nullptr;
	return K_OP_OK;
}

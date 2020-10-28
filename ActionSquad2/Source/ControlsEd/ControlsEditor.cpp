#include "dxstdafx.h"

#include <vector>
#include <string>
using namespace std;

#define K_BBOX_SCALE_BOX_SIZE 10

RECTXYWH propertiesPanelRect;

CControlsEditor::CControlsEditor()
{
	currLayer = NULL;
	currLayerIdx = -1;
	currCtrlIdx = -1;

	m_pSprite = null;
	m_pd3dDevice = null;
	m_pCamera = null;
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
	m_pd3dDevice->SetFVF(VERT_TL1TC::FVF);
	m_pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, 2, &vertices, sizeof(VERT_TL1TC));
}

void CControlsEditor::SetCameraTransform(CCameraTransform* pCamera)
{
	m_pCamera = pCamera;
}

void CControlsEditor::DrawBBox(RECTXYWH rect, bool selected)
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

	m_pd3dDevice->SetFVF(VERT_TL1TC::FVF);
	m_pd3dDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, &vertices, sizeof(VERT_TL1TC));

	if (selected && selectedCtrls.Count() == 1)
	{
		// top-left scalespot
		vertices[0].pos.x = rect.x; vertices[0].pos.y = rect.y;
		vertices[1].pos.x = rect.x + K_BBOX_SCALE_BOX_SIZE; vertices[1].pos.y = rect.y;
		vertices[2].pos.x = rect.x + K_BBOX_SCALE_BOX_SIZE; vertices[2].pos.y = rect.y + K_BBOX_SCALE_BOX_SIZE;
		vertices[3].pos.x = rect.x; vertices[3].pos.y = rect.y + K_BBOX_SCALE_BOX_SIZE;
		vertices[4].pos.x = rect.x; vertices[4].pos.y = rect.y;

		vertices[0].color = vertices[1].color = vertices[2].color = vertices[3].color = vertices[4].color = col;

		m_pd3dDevice->SetFVF(VERT_TL1TC::FVF);
		m_pd3dDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, &vertices, sizeof(VERT_TL1TC));

	
		// top-right scalespot
		vertices[0].pos.x = rect.x + rect.w; vertices[0].pos.y = rect.y;
		vertices[1].pos.x = rect.x + rect.w; vertices[1].pos.y = rect.y + K_BBOX_SCALE_BOX_SIZE;
		vertices[2].pos.x = rect.x + rect.w - K_BBOX_SCALE_BOX_SIZE; vertices[2].pos.y = rect.y + K_BBOX_SCALE_BOX_SIZE;
		vertices[3].pos.x = rect.x + rect.w - K_BBOX_SCALE_BOX_SIZE; vertices[3].pos.y = rect.y;
		vertices[4].pos.x = rect.x + rect.w; vertices[4].pos.y = rect.y;

		vertices[0].color = vertices[1].color = vertices[2].color = vertices[3].color = vertices[4].color = col;

		m_pd3dDevice->SetFVF(VERT_TL1TC::FVF);
		m_pd3dDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, &vertices, sizeof(VERT_TL1TC));

		// bottom-right scalespot
		vertices[0].pos.x = rect.x + rect.w; vertices[0].pos.y = rect.y + rect.h;
		vertices[1].pos.x = rect.x + rect.w - K_BBOX_SCALE_BOX_SIZE; vertices[1].pos.y = rect.y + rect.h;
		vertices[2].pos.x = rect.x + rect.w - K_BBOX_SCALE_BOX_SIZE; vertices[2].pos.y = rect.y + rect.h - K_BBOX_SCALE_BOX_SIZE;
		vertices[3].pos.x = rect.x + rect.w; vertices[3].pos.y = rect.y + rect.h - K_BBOX_SCALE_BOX_SIZE;
		vertices[4].pos.x = rect.x + rect.w; vertices[4].pos.y = rect.y + rect.h;

		vertices[0].color = vertices[1].color = vertices[2].color = vertices[3].color = vertices[4].color = col;

		m_pd3dDevice->SetFVF(VERT_TL1TC::FVF);
		m_pd3dDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, &vertices, sizeof(VERT_TL1TC));

		// bottom-left scalespot
		vertices[0].pos.x = rect.x; vertices[0].pos.y = rect.y + rect.h;
		vertices[1].pos.x = rect.x; vertices[1].pos.y = rect.y + rect.h - K_BBOX_SCALE_BOX_SIZE;
		vertices[2].pos.x = rect.x + K_BBOX_SCALE_BOX_SIZE; vertices[2].pos.y = rect.y + rect.h - K_BBOX_SCALE_BOX_SIZE;
		vertices[3].pos.x = rect.x + K_BBOX_SCALE_BOX_SIZE; vertices[3].pos.y = rect.y + rect.h;
		vertices[4].pos.x = rect.x; vertices[4].pos.y = rect.y + rect.h;

		vertices[0].color = vertices[1].color = vertices[2].color = vertices[3].color = vertices[4].color = col;

		m_pd3dDevice->SetFVF(VERT_TL1TC::FVF);
		m_pd3dDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, &vertices, sizeof(VERT_TL1TC));

		// top-mid scalespot
		vertices[0].pos.x = rect.x + rect.w / 2 - K_BBOX_SCALE_BOX_SIZE / 2; vertices[0].pos.y = rect.y;
		vertices[1].pos.x = rect.x + rect.w / 2 + K_BBOX_SCALE_BOX_SIZE / 2; vertices[1].pos.y = rect.y;
		vertices[2].pos.x = rect.x + rect.w / 2 + K_BBOX_SCALE_BOX_SIZE / 2; vertices[2].pos.y = rect.y + K_BBOX_SCALE_BOX_SIZE;
		vertices[3].pos.x = rect.x + rect.w / 2 - K_BBOX_SCALE_BOX_SIZE / 2; vertices[3].pos.y = rect.y + K_BBOX_SCALE_BOX_SIZE;
		vertices[4].pos.x = rect.x + rect.w / 2 - K_BBOX_SCALE_BOX_SIZE / 2; vertices[4].pos.y = rect.y;

		vertices[0].color = vertices[1].color = vertices[2].color = vertices[3].color = vertices[4].color = col;

		m_pd3dDevice->SetFVF(VERT_TL1TC::FVF);
		m_pd3dDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, &vertices, sizeof(VERT_TL1TC));

		// bot-mid scalespot
		vertices[0].pos.x = rect.x + rect.w / 2 - K_BBOX_SCALE_BOX_SIZE / 2; vertices[0].pos.y = rect.y + rect.h;
		vertices[1].pos.x = rect.x + rect.w / 2 + K_BBOX_SCALE_BOX_SIZE / 2; vertices[1].pos.y = rect.y + rect.h;
		vertices[2].pos.x = rect.x + rect.w / 2 + K_BBOX_SCALE_BOX_SIZE / 2; vertices[2].pos.y = rect.y + rect.h - K_BBOX_SCALE_BOX_SIZE;
		vertices[3].pos.x = rect.x + rect.w / 2 - K_BBOX_SCALE_BOX_SIZE / 2; vertices[3].pos.y = rect.y + rect.h - K_BBOX_SCALE_BOX_SIZE;
		vertices[4].pos.x = rect.x + rect.w / 2 - K_BBOX_SCALE_BOX_SIZE / 2; vertices[4].pos.y = rect.y + rect.h;

		vertices[0].color = vertices[1].color = vertices[2].color = vertices[3].color = vertices[4].color = col;

		m_pd3dDevice->SetFVF(VERT_TL1TC::FVF);
		m_pd3dDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, &vertices, sizeof(VERT_TL1TC));

		// left-mid scalespot
		vertices[0].pos.x = rect.x; vertices[0].pos.y = rect.y + rect.h / 2 - K_BBOX_SCALE_BOX_SIZE / 2;
		vertices[1].pos.x = rect.x + K_BBOX_SCALE_BOX_SIZE; vertices[1].pos.y = rect.y + rect.h / 2 - K_BBOX_SCALE_BOX_SIZE / 2;
		vertices[2].pos.x = rect.x + K_BBOX_SCALE_BOX_SIZE; vertices[2].pos.y = rect.y + rect.h / 2 + K_BBOX_SCALE_BOX_SIZE / 2;
		vertices[3].pos.x = rect.x; vertices[3].pos.y = rect.y + rect.h / 2 + K_BBOX_SCALE_BOX_SIZE / 2;
		vertices[4].pos.x = rect.x; vertices[4].pos.y = rect.y + rect.h / 2 - K_BBOX_SCALE_BOX_SIZE / 2;

		vertices[0].color = vertices[1].color = vertices[2].color = vertices[3].color = vertices[4].color = col;

		m_pd3dDevice->SetFVF(VERT_TL1TC::FVF);
		m_pd3dDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, &vertices, sizeof(VERT_TL1TC));

		// right-mid scalespot
		vertices[0].pos.x = rect.x + rect.w; vertices[0].pos.y = rect.y + rect.h / 2 - K_BBOX_SCALE_BOX_SIZE / 2;
		vertices[1].pos.x = rect.x + rect.w - K_BBOX_SCALE_BOX_SIZE; vertices[1].pos.y = rect.y + rect.h / 2 - K_BBOX_SCALE_BOX_SIZE / 2;
		vertices[2].pos.x = rect.x + rect.w - K_BBOX_SCALE_BOX_SIZE; vertices[2].pos.y = rect.y + rect.h / 2 + K_BBOX_SCALE_BOX_SIZE / 2;
		vertices[3].pos.x = rect.x + rect.w; vertices[3].pos.y = rect.y + rect.h / 2 + K_BBOX_SCALE_BOX_SIZE / 2;
		vertices[4].pos.x = rect.x + rect.w; vertices[4].pos.y = rect.y + rect.h / 2 - K_BBOX_SCALE_BOX_SIZE / 2;

		vertices[0].color = vertices[1].color = vertices[2].color = vertices[3].color = vertices[4].color = col;

		m_pd3dDevice->SetFVF(VERT_TL1TC::FVF);
		m_pd3dDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, &vertices, sizeof(VERT_TL1TC));
	}
}

void CControlsEditor::UpdateCtrlParamsList()
{
	if (currLayer == NULL || currCtrlIdx == -1)
		return;

	CControl* ctrl = currLayer->controls[currCtrlIdx];
	for (int ii = K_PP_CONTROLS_PROPS_START; ii <= K_PP_CONTROLS_PROPS_END; ii++)
	{
		CDXUTControl *dxCtrl = NULL;
		dxCtrl = propertiesPanel.GetControl(ii);
		if (dxCtrl == NULL)
			continue;

		if (dxCtrl->GetType() == DXUT_CONTROL_STATIC)
		{
			CDXUTEditBox* dxEditbox = (CDXUTEditBox *)propertiesPanel.GetControl(ii + 1);
			if (wcscmp(((CDXUTStatic *)dxCtrl)->GetText(), L"X") == 0)
			{
				WCHAR value[MAX_PATH];
				StringCchPrintf(value, MAX_PATH, L"%d", ctrl->bbox.x);
				dxEditbox->SetText(value);
			}
			else if (wcscmp(((CDXUTStatic *)dxCtrl)->GetText(), L"Y") == 0)
			{
				WCHAR value[MAX_PATH];
				StringCchPrintf(value, MAX_PATH, L"%d", ctrl->bbox.y);
				dxEditbox->SetText(value);
			}
			else if (wcscmp(((CDXUTStatic *)dxCtrl)->GetText(), L"W") == 0)
			{
				WCHAR value[MAX_PATH];
				StringCchPrintf(value, MAX_PATH, L"%d", ctrl->bbox.w);
				dxEditbox->SetText(value);
			}
			else if (wcscmp(((CDXUTStatic *)dxCtrl)->GetText(), L"H") == 0)
			{
				WCHAR value[MAX_PATH];
				StringCchPrintf(value, MAX_PATH, L"%d", ctrl->bbox.h);
				dxEditbox->SetText(value);
			}
		}
	}
}


HRESULT CControlsEditor::LoadCtrlTemplatesXML(WCHAR* XMLpath)
{
	pugi::xml_document doc;
	if (!doc.load_file(XMLpath))
	{
		ErrorBox(K_ERR_WARNING, L"Unable to load Controls XML:%s\n", XMLpath);
		return(E_FAIL);
	}
	pugi::xml_node layerNode = doc.root().first_child();
	for (pugi::xml_attribute atr = layerNode.first_attribute(); atr; atr = atr.next_attribute())
	{
		WCHAR atrval[MAX_PATH];
		StringCchPrintf(atrval, MAX_PATH, atr.value());
		layerTemplate.SetNamedVarString(atr.name(), atrval);
	}
	pugi::xml_node controlsNodes = doc.root().child(L"Layer");
	for (pugi::xml_node ctrlNode = controlsNodes.child(L"Control"); ctrlNode; ctrlNode = ctrlNode.next_sibling(L"Control"))
	{
		CVariantCollection *nCol = new CVariantCollection();
		for (pugi::xml_attribute atr = ctrlNode.first_attribute(); atr; atr = atr.next_attribute())
		{
			WCHAR atrval[MAX_PATH];
			StringCchPrintf(atrval, MAX_PATH, atr.value());
			nCol->SetNamedVarString(atr.name(), atrval);
		}
		ctrlTemplates.Add(nCol);
	}

	return S_OK;
}

void CControlsEditor::FillLayerControlsList(int layIdx)
{
	propertiesPanel.RemoveAllControls();

	propertiesPanel.AddButton(K_PP_BUTTON_CLONE_CONTROL, L"Clone Control", 50, 710, 150, 30);
	propertiesPanel.AddStatic(-1, L"Layer controls", 0, 0, 200, 30);

	CCtrlLayer *layer = UTGetControlsManager().layersDefinitions.GetAt(layIdx);
	currLayer = UTGetControlsManager().layersDefinitions.GetAt(layIdx);
	currLayer->pControlsManager = &UTGetControlsManager();
	currLayerIdx = layIdx;
	//currCtrl = 0;

	CDXUTListBox* pList;
	propertiesPanel.AddListBox(K_PP_CONTROLS_LIST, 5, 25, 240, 130, 0/*CDXUTListBox::MULTISELECTION*/, &pList);
	for (int ii = 0; ii < layer->controls.Count(); ii++)
	{
		CControl* ctrl = layer->controls.GetAt(ii);
		WCHAR itemName[MAX_PATH];
		StringCchPrintf(itemName, MAX_PATH, L"");
		if (ctrl->paramsDict.GetVariantByName(L"Type"))
			StringCchPrintf(itemName, MAX_PATH, ctrl->paramsDict.GetVariantByName(L"Type")->m_strArg.text);
		if (ctrl->paramsDict.GetVariantByName(L"ID") && wcslen(ctrl->paramsDict.GetVariantByName(L"ID")->m_strArg.text) > 0)
		{
			WCHAR itemPart[MAX_PATH];
			StringCchPrintf(itemPart, MAX_PATH, L"");
			StringCchPrintf(itemPart, MAX_PATH, L" - (%s)", ctrl->paramsDict.GetVariantByName(L"ID")->m_strArg.text);
			StringCchCat(itemName, MAX_PATH, itemPart);
		}

		pList->AddItem(itemName, (LPVOID)0x11111111);
	}
}

void CControlsEditor::FillLayerProperties()
{
	for (int ii = K_PP_CONTROLS_PROPS_START; ii <= K_PP_CONTROLS_PROPS_END; ii++)
		propertiesPanel.RemoveControl(ii);

	WCHAR val[MAX_PATH];
	//TODO: aici ar trebuie sa le ia dinamic
	StringCchPrintfW(val, MAX_PATH, L"%s", currLayer->ID.text);
	propertiesPanel.AddStatic(K_PP_CONTROLS_PROPS_START, L"ID", -60, 200 + 30, 200, 30, true);
	propertiesPanel.AddEditBox(K_PP_CONTROLS_PROPS_START + 1, val, 75, 200 + 30, 165, 30);

	StringCchPrintfW(val, MAX_PATH, L"%d", currLayer->GetPos().x);
	propertiesPanel.AddStatic(K_PP_CONTROLS_PROPS_START + 2, L"X", -60, 200 + 60, 200, 30, true);
	propertiesPanel.AddEditBox(K_PP_CONTROLS_PROPS_START + 3, val, 75, 200 + 60, 165, 30);

	StringCchPrintfW(val, MAX_PATH, L"%d", currLayer->GetPos().y);
	propertiesPanel.AddStatic(K_PP_CONTROLS_PROPS_START + 4, L"Y", -60, 200 + 90, 200, 30, true);
	propertiesPanel.AddEditBox(K_PP_CONTROLS_PROPS_START + 5, val, 75, 200 + 90, 165, 30);

	StringCchPrintfW(val, MAX_PATH, L"%d", currLayer->bBlocking);
	propertiesPanel.AddStatic(K_PP_CONTROLS_PROPS_START + 6, L"isBlocking", -60, 200 + 120, 200, 30, true);
	propertiesPanel.AddEditBox(K_PP_CONTROLS_PROPS_START + 7, val, 75, 200 + 120, 165, 30);

	StringCchPrintfW(val, MAX_PATH, L"%d", currLayer->bGetsInput);
	propertiesPanel.AddStatic(K_PP_CONTROLS_PROPS_START + 8, L"getsInput", -60, 200 + 150, 200, 30, true);
	propertiesPanel.AddEditBox(K_PP_CONTROLS_PROPS_START + 9, val, 75, 200 + 150, 165, 30);

	StringCchPrintfW(val, MAX_PATH, L"%.2f", currLayer->fDestroyTimer);
	propertiesPanel.AddStatic(K_PP_CONTROLS_PROPS_START + 10, L"fTimer", -60, 200 + 180, 200, 30, true);
	propertiesPanel.AddEditBox(K_PP_CONTROLS_PROPS_START + 11, val, 75, 200 + 180, 165, 30);

	if(currLayer->anchorX == K_CCTRL_LAYER_ANCHOR_MIN)
		StringCchPrintfW(val, MAX_PATH, L"min");
	else if (currLayer->anchorX == K_CCTRL_LAYER_ANCHOR_MAX)
		StringCchPrintfW(val, MAX_PATH, L"max");
	else
		StringCchPrintfW(val, MAX_PATH, L"center");
	propertiesPanel.AddStatic(K_PP_CONTROLS_PROPS_START + 12, L"anchorX", -60, 200 + 210, 200, 30, true);
	propertiesPanel.AddEditBox(K_PP_CONTROLS_PROPS_START + 13, val, 75, 200 + 210, 165, 30);

	if (currLayer->anchorY == K_CCTRL_LAYER_ANCHOR_MIN)
		StringCchPrintfW(val, MAX_PATH, L"min");
	else if (currLayer->anchorY == K_CCTRL_LAYER_ANCHOR_MAX)
		StringCchPrintfW(val, MAX_PATH, L"max");
	else
		StringCchPrintfW(val, MAX_PATH, L"center");

	propertiesPanel.AddStatic(K_PP_CONTROLS_PROPS_START + 14, L"anchorY", -60, 200 + 240, 200, 30, true);
	propertiesPanel.AddEditBox(K_PP_CONTROLS_PROPS_START + 15, val, 75, 200 + 240, 165, 30);

	//default control focus
	propertiesPanel.AddStatic(K_PP_CONTROLS_PROPS_START + 16, L"focusCtrlID", -60, 200 + 270, 200, 30, true);
	propertiesPanel.AddEditBox(K_PP_CONTROLS_PROPS_START + 17, currLayer->shFocusedControlID.text, 75, 200 + 270, 165, 30);
}

void CControlsEditor::FillControlProperties()
{
	if (currCtrlIdx < 0)
		return;

	for (int ii = K_PP_CONTROLS_PROPS_START; ii <= K_PP_CONTROLS_PROPS_END; ii++)
		propertiesPanel.RemoveControl(ii);
	
	// caut controlul in templates ca sa incarc exact proprietatile din templates
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
	
	int starty = 160; //controls start pos

	CVariantCollection* ctrl = ctrlTemplates.GetAt(ctrlIdx);
	for (int ii = 1; ii < ctrl->m_variants.Count(); ii++)
	{
		// pe par pun staticele cu numele proprietatii (X, Y, W, H etc)
		CVariantComplex* var = ctrl->m_variants[ii];
		propertiesPanel.AddStatic(K_PP_CONTROLS_PROPS_START + ii * 2, var->m_name.text, -60, starty + 30 * (ii - 1), 200, 30, true);

		// pe impar pun editbox-urile cu valoarea proprietatii luata din controlul efectiv
		CVariantComplex* currvar = currLayer->controls[currCtrlIdx]->paramsDict.GetVariantByNameHash(var->m_name.getHash());
		if (var->m_name.IsEqual(L"animID"))
		{
			CDXUTComboBox *CB;
			propertiesPanel.AddComboBox(K_PP_CONTROLS_PROPS_START + ii * 2 + 1, 75, starty + 30 * (ii - 1), 175, 30, 0U, false, &CB);
			for (int ii = 0; ii < UTGetControlsManager().m_sprCol.Animations.Count(); ii++)
			{
				scAnimation *anm = UTGetControlsManager().m_sprCol.Animations.GetAt(ii);
				CB->AddItem(anm->animName.text, NULL);
			}
			CB->AddItem(L"_EMPTY_", NULL);

			if (CB->SetSelectedByIndex(currvar->m_asINT32) != S_OK)
				CB->SetSelectedByText(L"_EMPTY_");
		}
		else if (var->m_name.IsEqual(L"FontID"))
		{
			CDXUTComboBox *CB;
			propertiesPanel.AddComboBox(K_PP_CONTROLS_PROPS_START + ii * 2 + 1, 75, starty + 30 * (ii - 1), 175, 30, 0U, false, &CB);
			for (int ii = 0; ii < UTGetFontsManager().fonts.Count(); ii++)
			{
				CTexturedFont* font = UTGetFontsManager().fonts.GetAt(ii);
				CB->AddItem(font->shFontName.text, NULL);
			}
			CB->AddItem(L"_EMPTY_", NULL);
			if (CB->SetSelectedByIndex(currvar->m_asINT32) != S_OK)
				CB->SetSelectedByText(L"_EMPTY_");
		}
		else if (var->m_name.IsEqual(L"stringID"))
		{
			WCHAR value[MAX_PATH];
			if(currvar->m_asINT32 != g_stringsMgr.defaultStringIdx) //daca nu am stringul setat scriu empty
				StringCchPrintf(value, MAX_PATH, L"%s", g_stringsMgr.strings[currvar->m_asINT32]->shStringName.text);
			else
				StringCchPrintf(value, MAX_PATH, L"empty");

			propertiesPanel.AddEditBox(K_PP_CONTROLS_PROPS_START + ii * 2 + 1, value, 75, starty + 30 * (ii - 1), 165, 30);
		}
		else if ((var->m_name.IsEqual(L"FontColor")) || (var->m_name.IsEqual(L"Color")))
		{
			WCHAR value[MAX_PATH];
			if (currvar->m_type == CVariantComplex::K_ARGTYPE_STRING)
				StringCchPrintf(value, MAX_PATH, L"%s", currvar->m_strArg.text);
			else
				StringCchPrintf(value, MAX_PATH, L"0xffffffff");

			propertiesPanel.AddEditBox(K_PP_CONTROLS_PROPS_START + ii * 2 + 1, value, 75, starty + 30 * (ii - 1), 165, 30);
		}
		else
		{
			WCHAR value[MAX_PATH];
			currvar->asString(value, MAX_PATH);

			if (currvar->m_type == CVariantComplex::K_ARGTYPE_NONE)
			{
				StringCchPrintf(value, MAX_PATH, L"empty");
			}

			propertiesPanel.AddEditBox(K_PP_CONTROLS_PROPS_START + ii * 2 + 1, value, 75, starty + 30 * (ii - 1), 165, 30);
		}
	}

	propertiesPanel.GetListBox(K_PP_CONTROLS_LIST)->m_bHasFocus = true;
}

void CControlsEditor::IMGUI_AddCurControlProps()
{
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

	CVariantCollection* ctrl = ctrlTemplates.GetAt(ctrlIdx);

	if ((selectedCtrls.GetSize() == 1) && (currCtrlIdx >= 0))
	{
		ImGui::Separator();
		ImGui::TextDisabled("Control options");
		if (ImGui::Button("Clone Control", ImVec2(120, 0)))
		{
		}
	}

	ImGui::Separator();
	ImGui::TextDisabled("Control properties");

	for (int ii = 1; ii < ctrl->m_variants.Count(); ii++)
	{
		// variable name from template
		CVariantComplex* pVar = ctrl->m_variants[ii];
		// actual value from control
		CVariantComplex* pValue = currLayer->controls[currCtrlIdx]->paramsDict.GetVariantByNameHash(pVar->m_name.getHash());
		char sVarName[MAX_PATH];
		wcstombs(sVarName, pVar->m_name.text, MAX_PATH);
		// hardcoded controls properties
		if ((pVar->m_name.IsEqual(L"FontColor")) || (pVar->m_name.IsEqual(L"Color")))
		{
			//#TODO: de despartit imgui pe update si paint si de testat var fara fereastra, in debug
			//#TODO: culoarea trebuie tinuta DWORD si doar la incarcare si la salvare covnertite in string
			//#TODO: ca tipuri de date in variant complex am putea sa avem si color ca sa nu mai fac switch dupa nume
			ImVec4 color;
			/*
				char* p = buf;
				while (*p == '#' || ImCharIsBlankA(*p))
					p++;
				i[0] = i[1] = i[2] = i[3] = 0;
					sscanf(p, "%02X%02X%02X%02X", (unsigned int*)&i[0], (unsigned int*)&i[1], (unsigned int*)&i[2], (unsigned int*)&i[3]); // Treat at unsigned (%X is unsigned)
					*/
			ImGui::ColorPicker4(sVarName, (float*)&color, ImGuiColorEditFlags_HEX | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_DisplayHex );

			// D3DCOLOR_COLORVALUE - transforma in hexa
			//sprintf("#%08X", valoare) ca sa salveze in format hexa

			/*
			WCHAR value[MAX_PATH];
			if (currvar->m_type == CVariantComplex::K_ARGTYPE_STRING)
				StringCchPrintf(value, MAX_PATH, L"%s", currvar->m_strArg.text);
			else
				StringCchPrintf(value, MAX_PATH, L"0xffffffff");

			propertiesPanel.AddEditBox(K_PP_CONTROLS_PROPS_START + ii * 2 + 1, value, 75, starty + 30 * (ii - 1), 165, 30);
			*/
		}
		// all other elements
		else
		{
			char str0[128];
			if (pValue->m_type == CVariantComplex::K_ARGTYPE_NONE)
			{
				sprintf(str0, "empty");
			}
			else
			{
				pValue->asString(str0, 128);
			}

			ImGui::InputText(sVarName, str0, IM_ARRAYSIZE(str0));
			if (ImGui::IsItemEdited())
			{
				LOG("text changed %d", randint(1000));
				pValue->m_strArg.Init(str0);
			}
		}


		/*
		// pe par pun staticele cu numele proprietatii (X, Y, W, H etc)
		CVariantComplex* var = ctrl->m_variants[ii];
		propertiesPanel.AddStatic(K_PP_CONTROLS_PROPS_START + ii * 2, var->m_name.text, -60, starty + 30 * (ii - 1), 200, 30, true);

		// pe impar pun editbox-urile cu valoarea proprietatii luata din controlul efectiv
		CVariantComplex* currvar = currLayer->controls[currCtrlIdx]->paramsDict.GetVariantByNameHash(var->m_name.getHash());
		if (var->m_name.IsEqual(L"animID"))
		{
			CDXUTComboBox *CB;
			propertiesPanel.AddComboBox(K_PP_CONTROLS_PROPS_START + ii * 2 + 1, 75, starty + 30 * (ii - 1), 175, 30, 0U, false, &CB);
			for (int ii = 0; ii < UTGetControlsManager().m_sprCol.Animations.Count(); ii++)
			{
				scAnimation *anm = UTGetControlsManager().m_sprCol.Animations.GetAt(ii);
				CB->AddItem(anm->animName.text, NULL);
			}
			CB->AddItem(L"_EMPTY_", NULL);

			if (CB->SetSelectedByIndex(currvar->m_asINT32) != S_OK)
				CB->SetSelectedByText(L"_EMPTY_");
		}
		else if (var->m_name.IsEqual(L"FontID"))
		{
			CDXUTComboBox *CB;
			propertiesPanel.AddComboBox(K_PP_CONTROLS_PROPS_START + ii * 2 + 1, 75, starty + 30 * (ii - 1), 175, 30, 0U, false, &CB);
			for (int ii = 0; ii < UTGetFontsManager().fonts.Count(); ii++)
			{
				CTexturedFont* font = UTGetFontsManager().fonts.GetAt(ii);
				CB->AddItem(font->shFontName.text, NULL);
			}
			CB->AddItem(L"_EMPTY_", NULL);
			if (CB->SetSelectedByIndex(currvar->m_asINT32) != S_OK)
				CB->SetSelectedByText(L"_EMPTY_");
		}
		else if (var->m_name.IsEqual(L"stringID"))
		{
			WCHAR value[MAX_PATH];
			if (currvar->m_asINT32 != g_stringsMgr.defaultStringIdx) //daca nu am stringul setat scriu empty
				StringCchPrintf(value, MAX_PATH, L"%s", g_stringsMgr.strings[currvar->m_asINT32]->shStringName.text);
			else
				StringCchPrintf(value, MAX_PATH, L"empty");

			propertiesPanel.AddEditBox(K_PP_CONTROLS_PROPS_START + ii * 2 + 1, value, 75, starty + 30 * (ii - 1), 165, 30);
		}
		else if ((var->m_name.IsEqual(L"FontColor")) || (var->m_name.IsEqual(L"Color")))
		{
			WCHAR value[MAX_PATH];
			if (currvar->m_type == CVariantComplex::K_ARGTYPE_STRING)
				StringCchPrintf(value, MAX_PATH, L"%s", currvar->m_strArg.text);
			else
				StringCchPrintf(value, MAX_PATH, L"0xffffffff");

			propertiesPanel.AddEditBox(K_PP_CONTROLS_PROPS_START + ii * 2 + 1, value, 75, starty + 30 * (ii - 1), 165, 30);
		}
		else
		{
			WCHAR value[MAX_PATH];
			currvar->asString(value, MAX_PATH);

			if (currvar->m_type == CVariantComplex::K_ARGTYPE_NONE)
			{
				StringCchPrintf(value, MAX_PATH, L"empty");
			}

			propertiesPanel.AddEditBox(K_PP_CONTROLS_PROPS_START + ii * 2 + 1, value, 75, starty + 30 * (ii - 1), 165, 30);
		}
		*/
	}
}


void CControlsEditor::AddControl(CVariantCollection* vcol)
{
	if (currLayer == NULL)
		return;

	WCHAR cType[MAX_PATH];
	StringCchPrintfW(cType, MAX_PATH, vcol->GetVariantByName(L"Type")->m_strArg.text);

	CControl* nctrl = new CControl(cType);
	nctrl->Initialize();
	for (int ii = 0; ii < vcol->m_variants.Count(); ii++)
	{
		CVariantComplex *var = vcol->m_variants.GetAt(ii);
		WCHAR propertyName[MAX_PATH];
		WCHAR propertyValue[MAX_PATH];

		StringCchPrintf(propertyName, MAX_PATH, L"%s", var->m_name.text);
		StringCchPrintf(propertyValue, MAX_PATH, L"%s", var->m_strArg.text);
		//sarim unele proprietati
		if (wcscmp(propertyName, L"ID") == 0 && wcscmp(propertyValue, L"empty"))
			continue;
		
		UTGetControlsManager().SetParamValue(nctrl, propertyName, propertyValue, true);
	}
	nctrl->layer = currLayer;
	currLayer->controls.Add(nctrl);

	currCtrlIdx = currLayer->controls.Count() - 1;
	selectedCtrls.RemoveAll();
	selectedCtrls.Add(currCtrlIdx);

	// reincarca listbox-ul layerului cu controlul nou adaugat
	FillLayerControlsList(UTGetControlsManager().layersDefinitions.IndexOf(currLayer));
	// incarca proprietatile noului control adaugat
	FillControlProperties();
}

void CALLBACK OnPropertiesPanelEvent(UINT nEvent, int nControlID, CDXUTControl* pControl)
{
	g_ControlsEditor.PropertiesPanelCallBack(nEvent, nControlID, pControl);
}


void CControlsEditor::SetSpritePtr(ID3DXSprite* pSprite)
{
	m_pSprite = pSprite;
}

void CControlsEditor::CloneControl()
{
	if (currCtrlIdx < 0 || selectedCtrls.Count() > 1)
		return;

	CControl* ctrl = currLayer->controls.GetAt(currCtrlIdx);

	CControl *nctrl = new CControl(ctrl);
	currLayer->controls.Add(nctrl);
}

void CControlsEditor::PropertiesPanelCallBack(UINT nEvent, int nControlID, CDXUTControl* pControl)
{
	switch (nControlID)
	{
		case K_PP_CONTROLS_LIST: // asta e chemat si cand selectezi din interfata si cand selectezi din cod cu SelectItem
		{
			
			if (!DXUTIsKeyDown(VK_CONTROL))
				selectedCtrls.RemoveAll();

			currCtrlIdx = ((CDXUTListBox *)pControl)->GetSelectedIndex();
			if (!selectedCtrls.Contains(currCtrlIdx))
			{
				selectedCtrls.Add(currCtrlIdx);
			}
			else
			{
				selectedCtrls.Remove(selectedCtrls.IndexOf(currCtrlIdx));
			}
			FillControlProperties();
		}
		break;
		case K_PP_BUTTON_CLONE_CONTROL:
		{
			if (currCtrlIdx < 0 || selectedCtrls.Count() > 1)
				return;

			CloneControl();
			FillLayerControlsList(UTGetControlsManager().layersDefinitions.IndexOf(currLayer));
		}
		break;
		default:
		{
			if (nEvent == EVENT_EDITBOX_STRING)
			{
				for (int ii = K_PP_CONTROLS_PROPS_START; ii < K_PP_CONTROLS_PROPS_END; ii++)
				{
					if (nControlID == ii)
					{
						WCHAR propertyName[MAX_PATH], propertyValue[MAX_PATH];
						StringCchPrintf(propertyName, MAX_PATH, L"%s", propertiesPanel.GetStatic(nControlID - 1)->GetText());
						StringCchPrintf(propertyValue, MAX_PATH, L"%s", ((CDXUTEditBox*)pControl)->GetText());

						if (currCtrlIdx > -1)
						{
							UTGetControlsManager().SetParamValue(currLayer->controls[currCtrlIdx], propertyName, propertyValue);
							// daca modific ID-ul sa se reincarce lista de controale a layer-ului (ca sa se actualizeze ID-ul si acolo)
							if (wcscmp(propertyName, L"ID") == 0)
							{
								FillLayerControlsList(UTGetControlsManager().layersDefinitions.IndexOf(currLayer));
								propertiesPanel.GetListBox(K_PP_CONTROLS_LIST)->SelectItem(currCtrlIdx);
							}
						}
						else
						{
							if (wcscmp(propertyName, L"isBlocking") == 0)
							{
								currLayer->bBlocking = _wtoi(propertyValue);
							}
							else if (wcscmp(propertyName, L"getsInput") == 0)
							{
								currLayer->bGetsInput = _wtoi(propertyValue);
							}
							else if (wcscmp(propertyName, L"X") == 0)
							{
								currLayer->X = _wtoi(propertyValue);
							}
							else if (wcscmp(propertyName, L"Y") == 0)
							{
								currLayer->Y = _wtoi(propertyValue);
							}
							else if (wcscmp(propertyName, L"ID") == 0)
							{
								currLayer->ID.Init(propertyValue);
							}
							else if (wcscmp(propertyName, L"focusCtrlID") == 0)
							{
								currLayer->shFocusedControlID.Init(propertyValue);
							}
							else if (wcscmp(propertyName, L"fTimer") == 0)
							{
								currLayer->fDestroyTimer = _wtof(propertyValue);
							}
							else if (wcscmp(propertyName, L"anchorX") == 0)
							{
								currLayer->anchorX = K_CCTRL_LAYER_ANCHOR_CENTER;
								if (_wcsicmp(propertyValue, L"max") == 0)
									currLayer->anchorX = K_CCTRL_LAYER_ANCHOR_MAX;
								else if (_wcsicmp(propertyValue, L"min") == 0)
									currLayer->anchorX = K_CCTRL_LAYER_ANCHOR_MIN;
							}
							else if (wcscmp(propertyName, L"anchorY") == 0)
							{
								currLayer->anchorY = K_CCTRL_LAYER_ANCHOR_CENTER;
								if (_wcsicmp(propertyValue, L"max") == 0)
									currLayer->anchorY = K_CCTRL_LAYER_ANCHOR_MAX;
								else if (_wcsicmp(propertyValue, L"min") == 0)
									currLayer->anchorY = K_CCTRL_LAYER_ANCHOR_MIN;
							}
						}
						break;
					}
				}
			}
			else if (nEvent == EVENT_COMBOBOX_SELECTION_CHANGED)
			{
				for (int ii = K_PP_CONTROLS_PROPS_START; ii < K_PP_CONTROLS_PROPS_END; ii++)
				{
					if (nControlID == ii)
					{
						WCHAR propertyName[MAX_PATH], propertyValue[MAX_PATH];
						StringCchPrintf(propertyName, MAX_PATH, L"%s", propertiesPanel.GetStatic(nControlID - 1)->GetText());
						StringCchPrintf(propertyValue, MAX_PATH, L"%s", ((CDXUTComboBox*)pControl)->GetSelectedItem()->strText);

						if (currCtrlIdx > -1)
						{
							UTGetControlsManager().SetParamValue(currLayer->controls[currCtrlIdx], propertyName, propertyValue);
						}
						break;
					}	
				}
			}
		}
		break;
	}
}

// --- END CALLBACKS ---

void CControlsEditor::Launch()
{
	propertiesPanelRect = RECTXYWH(UTGetAppClass().g_rectRender.w - 250, 0, 250, UTGetAppClass().g_rectRender.h);
	propertiesPanel.SetCallback(OnPropertiesPanelEvent);

	WCHAR xmlpath[MAX_PATH];
	StringCchPrintf(xmlpath, MAX_PATH, L"%sControlsEd\\ctrlTemplates.xml", UTGetAppClass().g_wszExePath);
	LoadCtrlTemplatesXML(xmlpath);

	propertiesPanel.EnableCaption(true);
	propertiesPanel.SetLocation(propertiesPanelRect.x, propertiesPanelRect.y);
	propertiesPanel.SetSize(propertiesPanelRect.w, propertiesPanelRect.h);
	propertiesPanel.SetCaptionText(L"Layer Properties List");
	propertiesPanel.SetBackgroundColors(D3DCOLOR_ARGB(100, 255, 255, 255));
	propertiesPanel.AddButton(K_PP_BUTTON_CLONE_CONTROL, L"Clone Control", 50, 710, 150, 30);
	
	tool = TOOL_TYPE_NO_TOOL;
	hideBBoxes = false;
	offset = D3DXVECTOR2(0, 0);
}

void CControlsEditor::SaveXML(WCHAR* XMLpath)
{
	pugi::xml_document doc;
	
	doc.append_child(L"Interfaces");
	doc.child(L"Interfaces").append_attribute(L"Version");
	doc.child(L"Interfaces").append_attribute(L"SpriteCollection");
	doc.child(L"Interfaces").attribute(L"Version").set_value(L"1.0");
	doc.child(L"Interfaces").attribute(L"SpriteCollection").set_value(L"controls.bsx");
	for (int ii = 0; ii < UTGetControlsManager().layersDefinitions.Count(); ii++)
	{
		CCtrlLayer* layer = UTGetControlsManager().layersDefinitions.GetAt(ii);

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
		WCHAR val[MAX_PATH];
		if (layer->anchorX == K_CCTRL_LAYER_ANCHOR_MIN)
			StringCchPrintfW(val, MAX_PATH, L"min");
		else if (layer->anchorX == K_CCTRL_LAYER_ANCHOR_MAX)
			StringCchPrintfW(val, MAX_PATH, L"max");
		else
			StringCchPrintfW(val, MAX_PATH, L"center");
		layerAttribute = layerNode.append_attribute(L"anchorX");
		layerAttribute.set_value(val);

		if (layer->anchorY == K_CCTRL_LAYER_ANCHOR_MIN)
			StringCchPrintfW(val, MAX_PATH, L"min");
		else if (layer->anchorY == K_CCTRL_LAYER_ANCHOR_MAX)
			StringCchPrintfW(val, MAX_PATH, L"max");
		else
			StringCchPrintfW(val, MAX_PATH, L"center");
		layerAttribute = layerNode.append_attribute(L"anchorY");
		layerAttribute.set_value(val);

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
			StringCchPrintfW(ctrlType, MAX_PATH, ctrlCol->GetVariantByName(L"Type")->m_strArg.text);

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
							StringCchPrintf(propertyName, MAX_PATH, var->m_name.text);
							ctrlCol->GetVariantByName(propertyName)->asString(propertyValue, MAX_PATH);

							// daca ID-ul este empty string sau coincide cu cel din templates nu-l mai scriu
							if (wcscmp(propertyName, L"ID") == 0)
								if (wcslen(ctrlCol->GetVariantByName(L"ID")->m_strArg.text) == 0
									|| wcscmp(ctrlCol->GetVariantByName(L"ID")->m_strArg.text, var->m_strArg.text) == 0)
									continue;
							//cazuri speciale ce trebuiesc traduse
							if (wcscmp(propertyName, L"animID") == 0)
							{
								int intVal = _wtoi(propertyValue);
								if (intVal >= 0 && wcscmp(propertyValue, L"_EMPTY_"))
									StringCchPrintf(propertyValue, MAX_PATH, L"%s", UTGetControlsManager().m_sprCol.Animations[intVal]->animName.text);
								else
									StringCchPrintf(propertyValue, MAX_PATH, L"%s", var->m_strArg.text);
							}
							else if (wcscmp(propertyName, L"FontID") == 0)
							{
								int intVal = _wtoi(propertyValue);
								if (intVal >= 0 && wcscmp(propertyValue, L"_EMPTY_"))
									StringCchPrintf(propertyValue, MAX_PATH, L"%s", UTGetFontsManager().fonts[_wtoi(propertyValue)]->shFontName.text);
								else
									StringCchPrintf(propertyValue, MAX_PATH, L"%s", var->m_strArg.text);
							}
							else if (wcscmp(propertyName, L"stringID") == 0)
							{
								int strIdx = _wtoi(propertyValue);
								if((strIdx < 0) || (strIdx == g_stringsMgr.defaultStringIdx))
									StringCchPrintf(propertyValue, MAX_PATH, L"%s", var->m_strArg.text); //daca nu am pus id text pun ce era in template
								else
									StringCchPrintf(propertyValue, MAX_PATH, L"%s", g_stringsMgr.strings[strIdx]->shStringName.text);
							}
							else if ((wcscmp(propertyName, L"Color") == 0) || (wcscmp(propertyName, L"FontColor") == 0))
							{
								if ((wcscmp(propertyValue, L"0") == 0) || (wcslen(propertyValue) < 2))
									StringCchPrintf(propertyValue, MAX_PATH, L"0xffffffff");
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
	// panels
	propertiesPanel.RemoveAllControls();

	// control templates
	for (int ii = 0; ii < ctrlTemplates.Count(); ii++)
	{
		CVariantCollection *col = ctrlTemplates.GetAt(ii);
		col->DeleteAll();
		SAFE_DELETE(col);
	}
	ctrlTemplates.RemoveAll();

	// release reference
	if (currLayer)
	{
		currLayer->pControlsManager = NULL;
		currLayer = NULL;
	}

	currCtrlIdx = -1;
	tool = TOOL_TYPE_NO_TOOL;
	selectedCtrls.RemoveAll();
}

static D3DXVECTOR2 vLastMouse;
void CControlsEditor::Update(float dTime)
{
	D3DXVECTOR2 vecRenderCenter(UTGetAppClass().g_rectRender.CenterX(), UTGetAppClass().g_rectRender.CenterY());
	propertiesPanelRect = RECTXYWH(UTGetAppClass().g_rectRender.x + UTGetAppClass().g_rectRender.w - 250, 0, 250, UTGetAppClass().g_rectRender.h);

	propertiesPanel.SetLocation(propertiesPanelRect.x, propertiesPanelRect.y);
	propertiesPanel.SetSize(propertiesPanelRect.w, propertiesPanelRect.h);

	// daca dau click pe interfata sa nu faca update-uri
	if (g_mouse.Lbut == K_MOUSE_BUTT_JUSTPRESSED)
		if ((PointInRect(g_mouse.pos.x, g_mouse.pos.y, &propertiesPanelRect) && !propertiesPanel.GetMinimized())
			)
			clickedInterface = true;
		else
			clickedInterface = false;

	if (UTimgui().GetWantCaptureMouse())
		clickedInterface = true;

	if (currLayer != NULL)
	{
		if (g_mouse.Lbut == K_MOUSE_BUTT_JUSTPRESSED && !clickedInterface)
		{			
			vLastMouse = g_mouse.pos; //save last mouse
			if (currCtrlIdx > -1 && selectedCtrls.Count() == 1)
			{
				RECTXYWH bbox = currLayer->controls[currCtrlIdx]->GetBBox();
				
				if (m_pCamera != null)
				{
					bbox.x += currLayer->GetPos().x;
					bbox.y += currLayer->GetPos().y;

					RECTXYWH_F rbbox(bbox);
					rbbox = m_pCamera->WorldToScreen(rbbox);

					rbbox.x += offset.x + vecRenderCenter.x;
					rbbox.y += offset.y + vecRenderCenter.y;
					bbox.Set(rbbox.x, rbbox.y, rbbox.w, rbbox.h);
				}
				else
				{
					bbox.x += currLayer->GetPos().x +offset.x + vecRenderCenter.x;
					bbox.y += currLayer->GetPos().y +offset.y + vecRenderCenter.y;
				}

				//verific daca dau click pe scale spot-uri
				RECTXYWH scaleSpotTL(bbox.x, bbox.y, K_BBOX_SCALE_BOX_SIZE, K_BBOX_SCALE_BOX_SIZE);
				RECTXYWH scaleSpotTM(bbox.x + bbox.w / 2 - K_BBOX_SCALE_BOX_SIZE / 2, bbox.y, K_BBOX_SCALE_BOX_SIZE, K_BBOX_SCALE_BOX_SIZE);
				RECTXYWH scaleSpotTR(bbox.x + bbox.w - K_BBOX_SCALE_BOX_SIZE, bbox.y, K_BBOX_SCALE_BOX_SIZE, K_BBOX_SCALE_BOX_SIZE);
				RECTXYWH scaleSpotBR(bbox.x + bbox.w - K_BBOX_SCALE_BOX_SIZE, bbox.y + bbox.h - K_BBOX_SCALE_BOX_SIZE, K_BBOX_SCALE_BOX_SIZE, K_BBOX_SCALE_BOX_SIZE);
				RECTXYWH scaleSpotBM(bbox.x + bbox.w / 2 - K_BBOX_SCALE_BOX_SIZE / 2, bbox.y + bbox.h - K_BBOX_SCALE_BOX_SIZE, K_BBOX_SCALE_BOX_SIZE, K_BBOX_SCALE_BOX_SIZE);
				RECTXYWH scaleSpotBL(bbox.x, bbox.y + bbox.h - K_BBOX_SCALE_BOX_SIZE, K_BBOX_SCALE_BOX_SIZE, K_BBOX_SCALE_BOX_SIZE);
				RECTXYWH scaleSpotLM(bbox.x, bbox.y + bbox.h / 2 - K_BBOX_SCALE_BOX_SIZE / 2, K_BBOX_SCALE_BOX_SIZE, K_BBOX_SCALE_BOX_SIZE);
				RECTXYWH scaleSpotRM(bbox.x + bbox.w - K_BBOX_SCALE_BOX_SIZE / 2, bbox.y + bbox.h / 2 - K_BBOX_SCALE_BOX_SIZE / 2, K_BBOX_SCALE_BOX_SIZE, K_BBOX_SCALE_BOX_SIZE);
				if (PointInRect(g_mouse.pos.x, g_mouse.pos.y, &scaleSpotTL))
				{
					tool = TOOL_TYPE_RESIZE_TOP_LEFT;
				}
				else if (PointInRect(g_mouse.pos.x, g_mouse.pos.y, &scaleSpotTM))
				{
					tool = TOOL_TYPE_RESIZE_TOP_MID;
				}
				else if (PointInRect(g_mouse.pos.x, g_mouse.pos.y, &scaleSpotTR))
				{
					tool = TOOL_TYPE_RESIZE_TOP_RIGHT;
				}
				else if (PointInRect(g_mouse.pos.x, g_mouse.pos.y, &scaleSpotBR))
				{
					tool = TOOL_TYPE_RESIZE_BOTT_RIGHT;
				}
				else if (PointInRect(g_mouse.pos.x, g_mouse.pos.y, &scaleSpotBM))
				{
					tool = TOOL_TYPE_RESIZE_BOTT_MID;
				}
				else if (PointInRect(g_mouse.pos.x, g_mouse.pos.y, &scaleSpotBL))
				{
					tool = TOOL_TYPE_RESIZE_BOTT_LEFT;
				}
				else if (PointInRect(g_mouse.pos.x, g_mouse.pos.y, &scaleSpotLM))
				{
					tool = TOOL_TYPE_RESIZE_LEFT_MID;
				}
				else if (PointInRect(g_mouse.pos.x, g_mouse.pos.y, &scaleSpotRM))
				{
					tool = TOOL_TYPE_RESIZE_RIGHT_MID;
				}
			}
		}
		if (g_mouse.Lbut == K_MOUSE_BUTT_DRAG && !clickedInterface)
		{
			SIZEWH_F mousedelta(g_mouse.pos.x - vLastMouse.x, g_mouse.pos.y - vLastMouse.y);
			if (m_pCamera != null)
			{
				mousedelta = m_pCamera->ScreenToWorld(mousedelta);
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
			}
			else
			{
				vLastMouse = g_mouse.pos;
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
							RECTXYWH bbox = currLayer->controls[selCtrl]->GetBBox();
							bbox.x += mousedelta.w;
							bbox.y += mousedelta.h;

							WCHAR val[MAX_PATH];
							CControl* ctrl = currLayer->controls[selCtrl];
							StringCchPrintfW(val, MAX_PATH, L"%d", bbox.x);
							UTGetControlsManager().SetParamValue(ctrl, L"X", val);
							StringCchPrintfW(val, MAX_PATH, L"%d", bbox.y);
							UTGetControlsManager().SetParamValue(ctrl, L"Y", val);
						}
					}
					else
					{
						RECTXYWH BBox = currLayer->controls[currCtrlIdx]->GetBBox();
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
						CLAMP(BBox.w, 1, (int)UTGetAppClass().g_rectRender.w);
						CLAMP(BBox.h, 1, (int)UTGetAppClass().g_rectRender.h);

						// le setez si in dictionar
						for (int ii = 0; ii < selectedCtrls.Count(); ii++)
						{
							int selCtrl = selectedCtrls.GetAt(ii);
							CControl* ctrl = currLayer->controls[selCtrl];
							WCHAR val[MAX_PATH];
							StringCchPrintfW(val, MAX_PATH, L"%d", BBox.x);
							UTGetControlsManager().SetParamValue(ctrl, L"X", val);
							StringCchPrintfW(val, MAX_PATH, L"%d", BBox.y);
							UTGetControlsManager().SetParamValue(ctrl, L"Y", val);
							StringCchPrintfW(val, MAX_PATH, L"%d", BBox.w);
							UTGetControlsManager().SetParamValue(ctrl, L"W", val);
							StringCchPrintfW(val, MAX_PATH, L"%d", BBox.h);
							UTGetControlsManager().SetParamValue(ctrl, L"H", val);
						}
					}

					UpdateCtrlParamsList();
				}
			}
		}
		if (g_mouse.Lbut == K_MOUSE_BUTT_JUSTRELEASED)
		{
			tool = TOOL_TYPE_MOVE;
		}	
		if (g_mouse.Rbut == K_MOUSE_BUTT_JUSTPRESSED)
		{
			CGrowableArray<int> newClickedCtrls;
			for (int kk = currLayer->controls.Count() - 1; kk >= 0; kk--)
			{
				RECTXYWH bbox = currLayer->controls[kk]->GetBBox();
				
				if (m_pCamera != null)
				{
					bbox.x += currLayer->GetPos().x;
					bbox.y += currLayer->GetPos().y;

					RECTXYWH_F rbbox(bbox);
					rbbox = m_pCamera->WorldToScreen(rbbox);

					rbbox.x += offset.x + vecRenderCenter.x;
					rbbox.y += offset.y + vecRenderCenter.y;
					bbox.Set(rbbox.x, rbbox.y, rbbox.w, rbbox.h);
				}
				else
				{
					bbox.x += currLayer->GetPos().x + offset.x + vecRenderCenter.x;
					bbox.y += currLayer->GetPos().y + offset.y + vecRenderCenter.y;
				}

				if (PointInRect(g_mouse.pos.x, g_mouse.pos.y, &bbox))
				{
					newClickedCtrls.Add(kk);
				}
			}

			// selectie layer
			if (newClickedCtrls.Count() == 0)
			{
				currCtrlIdx = -1;
				selectedCtrls.RemoveAll();
				FillLayerProperties();
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
						propertiesPanel.GetListBox(K_PP_CONTROLS_LIST)->SelectItem(newSelection);
					}
					else // daca apas pe aceeasi lista de controale trebuie sa schimb selectia prin rotatie
					{
						int ctrlIdx = clickedCtrls.IndexOf(currCtrlIdx);
						ctrlIdx++;
						if (ctrlIdx >= clickedCtrls.Count())
							ctrlIdx = 0;
						int newCtrl = clickedCtrls.GetAt(ctrlIdx);

						//if (newCtrl != currCtrl)
						//{
							propertiesPanel.GetListBox(K_PP_CONTROLS_LIST)->SelectItem(newCtrl);
						//}
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
					else
					{
						propertiesPanel.GetListBox(K_PP_CONTROLS_LIST)->SelectItem(newSelection);
					}
				}
			}

			
		}
	}	
}

void CControlsEditor::ReceiveKeys(UINT key)
{
	for (int ii = K_PP_CONTROLS_PROPS_START; ii < K_PP_CONTROLS_PROPS_END; ii++)
	{
		if (propertiesPanel.GetControl(ii) && propertiesPanel.GetControl(ii)->m_bHasFocus)
			return;
	}
	
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
				StringCchPrintfW(val, MAX_PATH, L"%d", ctrl->GetBBox().h - dY);
				UTGetControlsManager().SetParamValue(ctrl, L"H", val);
			}
			else
			{
				StringCchPrintfW(val, MAX_PATH, L"%d", ctrl->GetBBox().y - dY);
				UTGetControlsManager().SetParamValue(ctrl, L"Y", val);
			}
		}
		UpdateCtrlParamsList();
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
				StringCchPrintfW(val, MAX_PATH, L"%d", ctrl->GetBBox().h + dY);
				UTGetControlsManager().SetParamValue(ctrl, L"H", val);
			}
			else
			{
				StringCchPrintfW(val, MAX_PATH, L"%d", ctrl->GetBBox().y + dY);
				UTGetControlsManager().SetParamValue(ctrl, L"Y", val);
			}
		}
		UpdateCtrlParamsList();
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
				StringCchPrintfW(val, MAX_PATH, L"%d", ctrl->GetBBox().w - dX);
				UTGetControlsManager().SetParamValue(ctrl, L"W", val);
			}
			else
			{
				StringCchPrintfW(val, MAX_PATH, L"%d", ctrl->GetBBox().x - dX);
				UTGetControlsManager().SetParamValue(ctrl, L"X", val);
			}
		}
		UpdateCtrlParamsList();
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
				StringCchPrintfW(val, MAX_PATH, L"%d", ctrl->GetBBox().w + dX);
				UTGetControlsManager().SetParamValue(ctrl, L"W", val);
			}
			else
			{
				StringCchPrintfW(val, MAX_PATH, L"%d", ctrl->GetBBox().x + dX);
				UTGetControlsManager().SetParamValue(ctrl, L"X", val);
			}
		}
		UpdateCtrlParamsList();
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
	int layidx = UTGetControlsManager().layersDefinitions.IndexOf(currLayer);
	UTGetControlsManager().layersDefinitions.Remove(layidx);
	SAFE_DELETE(currLayer);
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
			FillLayerControlsList(UTGetControlsManager().layersDefinitions.IndexOf(currLayer));
			propertiesPanel.GetListBox(K_PP_CONTROLS_LIST)->SelectItem(currCtrlIdx);
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
			FillLayerControlsList(UTGetControlsManager().layersDefinitions.IndexOf(currLayer));
			propertiesPanel.GetListBox(K_PP_CONTROLS_LIST)->SelectItem(currCtrlIdx);
		}
	}
}

void CControlsEditor::CenterElements(bool H, bool V)
{
	if (H)
	{
		if (currLayer == NULL)
			return;

		if (selectedCtrls.Count() > 1)
		{
			int xmin = UTGetAppClass().g_rectRender.w;
			int xmax = -UTGetAppClass().g_rectRender.w;

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
				StringCchPrintfW(val, MAX_PATH, L"%d", intVal);
				UTGetControlsManager().SetParamValue(ctrl, L"X", val);
				UpdateCtrlParamsList();
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
				StringCchPrintfW(val, MAX_PATH, L"%d", intVal);
				UTGetControlsManager().SetParamValue(lCtrl, L"X", val);
				UpdateCtrlParamsList();
			}
		}
		else if (selectedCtrls.Count() == 0)
		{
			currLayer->X = 0;
			FillLayerProperties();
		}
	}

	if (V)
	{
		if (currLayer == NULL)
			return;

		if (selectedCtrls.Count() > 1)
		{
			int ymin = UTGetAppClass().g_rectRender.h;
			int ymax = -UTGetAppClass().g_rectRender.h;

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
				StringCchPrintfW(val, MAX_PATH, L"%d", intVal);
				UTGetControlsManager().SetParamValue(ctrl, L"Y", val);
				UpdateCtrlParamsList();
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
				StringCchPrintfW(val, MAX_PATH, L"%d", intVal);
				UTGetControlsManager().SetParamValue(currLayer->controls[currCtrlIdx], L"Y", val);
				UpdateCtrlParamsList();
			}
		}
		else if (selectedCtrls.Count() == 0)
		{
			currLayer->Y = 0;
			FillLayerProperties();
		}
	}
}

void CControlsEditor::DeleteControl()
{
	SAFE_DELETE(currLayer->controls[currCtrlIdx]);
	currLayer->controls.Remove(currCtrlIdx);

	selectedCtrls.RemoveAll();
	currCtrlIdx = -1;

	FillLayerControlsList(UTGetControlsManager().layersDefinitions.IndexOf(currLayer));
}

void CControlsEditor::Paint()
{
	if (m_pCamera != null)
	{
		CCameraTransform::SetActiveCamera(m_pd3dDevice, m_pCamera);
	}


	D3DXVECTOR2 vecRenderCenter(UTGetAppClass().g_rectRender.CenterX(), UTGetAppClass().g_rectRender.CenterY());
	if (currLayer)
	{
		POINTXY_INT lpos = currLayer->GetPos();
		D3DXMATRIXA16 mat;

		if(m_pSprite)
			m_pSprite->Flush();

		D3DXVECTOR2 scrCenter(vecRenderCenter.x + offset.x + lpos.x, vecRenderCenter.y + offset.y + lpos.y);
		if (m_pCamera != null)
		{
			scrCenter = m_pCamera->ScreenToWorld(scrCenter);
		}
		D3DXMatrixAffineTransformation2D(&mat, 1.0f, NULL, 0.0f, &scrCenter);
		//set world matrix and keep history so we can access it from the fonts when painting the controls
		App_SetWorldTransform(m_pd3dDevice, &mat);
		// desenare controale
		for (int kk = 0; kk < currLayer->controls.Count(); kk++)
		{
			currLayer->controls[kk]->Paint(currLayer->pControlsManager->m_pCamera, &mat);
		}

		if (m_pSprite)
			m_pSprite->Flush();

		App_SetWorldTransform(m_pd3dDevice, &g_matIdentity);
	}
}

void CControlsEditor::PaintInterface(float fElapsedTime)
{
	// desenare panel-uri
	propertiesPanel.OnRender(fElapsedTime);
}

void CControlsEditor::ShowImguiInterfaces()
{
	{
		ImGuiViewport * vp = ImGui::GetWindowViewport();
		
		///--- TOOLS WINDOW
		ImGui::Begin("Tools");
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
		if (ImGui::Button("Layer Up", ImVec2(80, 0)))
		{
			ChangeControlPaintOrder(-1);
		}
		if (ImGui::Button("Layer Down", ImVec2(80, 0)))
		{
			ChangeControlPaintOrder(1);
		}
		if (ImGui::Button("Save", ImVec2(80, 0)))
		{
			SaveXML(UTGetControlsManager().loadedFile);
		}
		ImGui::End();


		///--- CONTROLS TEMPLATES
		if (vp)
			ImGui::SetNextWindowPos(vp->Pos, ImGuiCond_Once);
		ImGui::Begin("Controls Templates");

		vector<string> arrItems;
		for (int ii = 0; ii < ctrlTemplates.Count(); ii++)
		{
			CVariantCollection *col = ctrlTemplates.GetAt(ii);
			CVariantComplex* var = col->m_variants.GetAt(0);
			char strName[MAX_PATH];
			wcstombs(strName, var->m_strArg.text, MAX_PATH);
			arrItems.push_back(strName);
		}

		ImGui::PushItemWidth(240);
		if (ImGui::ListBoxHeader(" "))
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
		ImGui::PopItemWidth();
		ImGui::End();

		///--- LAYERS LIST 
		ImGui::Begin("Interfaces");

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
		if (ImGui::ListBoxHeader("##", ImVec2(200, 200)))
		{
			for (int kk = 0; kk < arrLayerNames.size(); kk++)
			{
				auto layer = arrLayerNames[kk];
				if (ImGui::Selectable(layer.c_str(), (kk == currLayerIdx) ? true : false, ImGuiSelectableFlags_None))
				{
					int layIdx = kk;
					currCtrlIdx = -1;
					selectedCtrls.RemoveAll();
					FillLayerControlsList(layIdx);
					FillLayerProperties();
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
			FillLayerControlsList(idx);
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
			FillLayerControlsList(idx);
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
		if (ImGui::ListBoxHeader("##", ImVec2(200, 200)))
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

			if (selectedCtrls.GetSize() == 1)
			{
				IMGUI_AddCurControlProps();
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
	m_pd3dDevice->SetTexture(0, null);

	if (hideBBoxes)
		return;
	D3DXVECTOR2 vecRenderCenter(UTGetAppClass().g_rectRender.CenterX(), UTGetAppClass().g_rectRender.CenterY());
	//axis
	int w = UTGetAppClass().g_rectRender.w;
	int h = UTGetAppClass().g_rectRender.h;

	DrawLine(w / 2 + offset.x, 0, w / 2 + offset.x, h);
	DrawLine(0, h / 2 + offset.y, w, h / 2 + offset.y);

	if (currLayer)
	{
		// desenare bounding box-uri
		for (int kk = 0; kk < currLayer->controls.Count(); kk++)
		{
			RECTXYWH rect;
			rect = currLayer->controls[kk]->GetBBox();
			rect.x += currLayer->GetPos().x;
			rect.y += currLayer->GetPos().y;

			if (m_pCamera != null)
			{
				D3DXVECTOR2 vul(rect.x, rect.y);
				SIZEWH_F rsz(rect.w, rect.h);
				vul = m_pCamera->WorldToScreen(vul);
				//adaug screen space coords
				vul = vul + vecRenderCenter + offset;

				rsz = m_pCamera->WorldToScreen(rsz);
				rect.x = vul.x; rect.y = vul.y;
				rect.w = rsz.w; rect.h = rsz.h;

			}

			if (selectedCtrls.Contains(kk))
				DrawBBox(rect, true);
			else
				DrawBBox(rect, false);
		}
	}

}

HRESULT CControlsEditor::OnCreateDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc)
{
	m_pd3dDevice = pd3dDevice;
	return S_OK;
}

HRESULT CControlsEditor::OnResetDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc)
{
	m_pd3dDevice = pd3dDevice;

	return S_OK;
}

HRESULT CControlsEditor::OnLostDevice()
{
	m_pd3dDevice = NULL;
	return S_OK;
}

HRESULT CControlsEditor::OnDestroyDevice()
{
	m_pd3dDevice = NULL;
	return S_OK;
}
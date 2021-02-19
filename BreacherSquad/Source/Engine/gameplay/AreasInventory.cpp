#include "dxstdafx.h"
#include "AreasInventory.h"

CAreasInventory::CAreasInventory()
{
	
}

CAreasInventory::~CAreasInventory()
{
	Release();
}

OPRESULT CAreasInventory::LoadAreasSpecs(WCHAR* strXMLPath)
{
	pugi::xml_document doc;
	if (!doc.load_file(strXMLPath))
	{
		return OPRESULT(K_OP_FAILED, K_SEVERITY_CRITICAL, L"Unable to load Weapon Templates XML:%s\n", strXMLPath);
	}

	//load explosion templates
	arrAreas.clear();

	for (pugi::xml_node bnode = doc.root().first_child(); bnode; bnode = bnode.next_sibling())
	{
		CAreaSpecs as;
		as.sizeBL.x = bnode.attribute(L"BlocksW").as_int();
		as.sizeBL.y = bnode.attribute(L"BlocksH").as_int();
		as.strSpecs = bnode.attribute(L"ConnectorsDesc").as_string();
		as.strTags = bnode.attribute(L"Tags").as_string();
		as.strFilename = bnode.attribute(L"File").as_string();

		arrAreas.push_back(as);
	}

	return K_OP_OK;
}

void CAreasInventory::Release()
{
	arrAreas.clear();
}


///----------------------------------------------------------------------------------
/// SINGLETON
///----------------------------------------------------------------------------------
CAreasInventory& UTGetAreasInv()
{
	static CAreasInventory g_AreasInventory;
	return g_AreasInventory;
}


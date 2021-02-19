#include "dxstdafx.h"
#include "MissionStory.h"

CMissionStory::CMissionStory()
{

}

CMissionStory::~CMissionStory()
{
	Release();
}

OPRESULT CMissionStory::LoadStory(WCHAR* strXMLPath)
{
	pugi::xml_document doc;
	if (!doc.load_file(strXMLPath))
	{
		return OPRESULT(K_OP_FAILED, K_SEVERITY_CRITICAL, L"Unable to load Mission Story XML:%s\n", strXMLPath);
	}

	Release();
	// we must have LevelStory node with children
	pugi::xml_node nodegens = doc.root().child(L"LevelStory");
	for (pugi::xml_node bnode = nodegens.first_child(); bnode; bnode = bnode.next_sibling())
	{
		// add  generation
		CStoryGeneration ngen;
		// for each generation parse all children nodes (area nodes)
		for (pugi::xml_node areanode = bnode.first_child(); areanode; areanode = areanode.next_sibling())
		{
			CStoryRoom nroom;
			nroom.nChildren = areanode.attribute(L"Children").as_int();
			nroom.nID = areanode.attribute(L"ID").as_int();
			nroom.nParentID = areanode.attribute(L"ParentID").as_int();
			nroom.tags_any = areanode.attribute(L"TagsAny").as_string();
			nroom.tags_all = areanode.attribute(L"TagsAll").as_string();
			nroom.tags_none = areanode.attribute(L"TagsNone").as_string();
			// add rooms to generation
			ngen.arrRooms.push_back(nroom);
		}
		// finally add generation
		arrGenerations.push_back(ngen);
	}

	return K_OP_OK;
}

void CMissionStory::Release()
{
	arrGenerations.clear();
}

bool CMissionStory::IsLoaded()
{
	return (arrGenerations.size() > 0);
}

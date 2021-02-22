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

CStoryRoom* CMissionStory::GetNextAvailableRoom(int nGeneration, int nParentID, bool bMarkAsUsed)
{
	if ((nGeneration < 0) || (nGeneration >= arrGenerations.size()))
		return nullptr;
	CStoryGeneration* gen = &arrGenerations[nGeneration];
	for (int kk = 0; kk < gen->arrRooms.size(); kk++)
	{
		if ((gen->arrRooms[kk].bUsed == false) && (gen->arrRooms[kk].nParentID == nParentID))
		{
			if (bMarkAsUsed)
				gen->arrRooms[kk].bUsed = true;
			return &gen->arrRooms[kk];
		}
	}

	return nullptr;
}

void CMissionStory::ComputeRelationshipsGraph()
{
	for (int gg = 0; gg < arrGenerations.size(); gg++)
	{
		int nChildIdx = 0;
		CStoryGeneration* gen = &arrGenerations[gg];
		for (int aa = 0; aa < gen->arrRooms.size(); aa++)
		{
			CStoryRoom* room = &gen->arrRooms[aa];
			room->bUsed = false;
			room->nID = gg * 100 + aa;
			// set children parents ids
			if (gg < arrGenerations.size() - 1)
			{
				int nFakeChildren = 0;
				for (int cc = 0; cc < room->nChildren; cc++)
				{
					if (nChildIdx + cc < arrGenerations[gg + 1].arrRooms.size())
					{
						arrGenerations[gg + 1].arrRooms[nChildIdx + cc].nParentID = room->nID;
					}
					else
					{
						nFakeChildren++;
					}
				}
				room->nChildren -= nFakeChildren;
				nChildIdx += room->nChildren;
			}
			// trim children for last generation
			else if (gg == arrGenerations.size() - 1)
			{
				room->nChildren = 0;
			}
		}
	}
}

void CMissionStory::ClearChildEntries(int nGeneration, int nParentID)
{
	if ((nGeneration < 0) || (nGeneration >= arrGenerations.size()))
		return;
	for (auto room : arrGenerations[nGeneration].arrRooms)
	{
		if (room.nParentID == nParentID)
			room.bUsed = false;
	}
}

void CMissionStory::ClearEntriesFromGeneration(int nGeneration)
{
	if ((nGeneration < 0) || (nGeneration >= arrGenerations.size()))
		return;
	for (int gen = nGeneration; gen < arrGenerations.size(); gen++)
	{
		for (int kk = 0; kk < arrGenerations[gen].arrRooms.size(); kk++)
		{
			arrGenerations[gen].arrRooms[kk].bUsed = false;
		}
	}
}


#include "dxstdafx.h"

bool CChaptersList::IsValidLevel(int nChapter, int nLevel)
{
	if ((nChapter < m_arrChapters.GetSize()) && (nChapter >= 0) && (nLevel < m_arrChapters[nChapter]->nLevelsCnt))
		return true;
	return false;
}

bool CChaptersList::IsChapterUnlocked(int nChapter, int nTotalStars)
{
	//no such chapter, leave it unlocked
	if ((nChapter >= m_arrChapters.GetSize()) || (nChapter < 0))
		return true;
	if (m_arrChapters[nChapter]->nChapterMissionsToUnlock <= nTotalStars)
		return true;
	else
		return false;
	//other cases (default on unlocked)
	//return true;
}

///----------------------------- CHAPTER/LEVEL DATA ----------------------------

bool CChaptersList::LoadChapters(WCHAR * strXmlPath)
{
	//delete all if already loaded
	SAFE_DELETE_GROWABLE_ARRAY(m_arrChapters);
	//load descriptors
	pugi::xml_document doc;
	if (!doc.load_file(strXmlPath))
	{
		ErrorBox(K_ERR_WARNING, L"[WARNING] Chapters:: LoadChapters:: Unable to load chapters list XML:%s", strXmlPath);
		return false;
	}

	//load explosion templates
	pugi::xml_node rootnodeexplo = doc.root().child(L"Chapters");
	//all chapters:
	for (pugi::xml_node chapternode = rootnodeexplo.first_child(); chapternode; chapternode = chapternode.next_sibling())
	{
		CChapterDesc* cdesc = new CChapterDesc();
		cdesc->nLevelsCnt = 0;
		//get chapter data
		cdesc->nChapterNameStrIdx = UTLang().getStrIdx(FastHash(chapternode.attribute(L"sStrID_ChapterName").value()));
		cdesc->nChapterMissionsToUnlock = chapternode.attribute(L"nMissionsToUnlock").as_int();
		cdesc->nChapterImgFrame = chapternode.attribute(L"nChapterImgFrame").as_int();
		//add all missions:
		for (pugi::xml_node missionnode = chapternode.first_child(); missionnode; missionnode = missionnode.next_sibling())
		{
			cdesc->arrLevelNameStrIdx[cdesc->nLevelsCnt] = UTLang().getStrIdx(missionnode.attribute(L"sStrID_LevelName").value());
			cdesc->arrLevelFilenames[cdesc->nLevelsCnt].Init(missionnode.attribute(L"sFilename").value());
			cdesc->nLevelsCnt++;
		}

		m_arrChapters.Add(cdesc);
	}

	LOG(L"Chapters:: Loaded missions list.");
	return true;
}

bool CChaptersList::GetMissionFilename(int nChapterNumber, int nLevelNumber, WCHAR * strDest, int strDestMaxLen)
{
	if (m_arrChapters.GetSize() == 0)
	{
		ErrorBox(K_ERR_WARNING, L"Chapters:: App_GetMissionFilename:: Chapters not loaded!");
		return false;
	}
	if ((nChapterNumber < 0) || (nChapterNumber >= m_arrChapters.GetSize()))
	{
		LOG(L"Chapters:: App_GetMissionFilename:: Couldn't find chapter %d!", nChapterNumber);
		return false;
	}

	CChapterDesc* cdesc = m_arrChapters[nChapterNumber];
	if ((nLevelNumber < 0) || (nLevelNumber >= cdesc->nLevelsCnt))
	{
		LOG(L"Chapters:: App_GetMissionFilename:: Couldn't find level %d in chapter %d!", nLevelNumber, nChapterNumber);
		return false;
	}

	//get file from mods
	WCHAR wcsTarget[MAX_PATH], wcsPath[MAX_PATH];
	StringCchPrintf(wcsTarget, MAX_PATH, L"media/levels/missions/%s", cdesc->arrLevelFilenames[nLevelNumber].text);	
	FileManager::GetMediaPath(wcsTarget, wcsPath);
	StringCchCopy(strDest, strDestMaxLen, wcsPath);
	//old way, without modded levels:
	//StringCchPrintf(strDest, strDestMaxLen, L"%s/levels/missions/%s", UTGetAppClass().g_wszAppResDir, cdesc->arrLevelFilenames[nLevelNumber].text);
	return true;
}

CChaptersList::CChaptersList()
{
}

CChaptersList::~CChaptersList()
{
	SAFE_DELETE_GROWABLE_ARRAY(m_arrChapters);
}

///**************************************************************************************
/// Sigleton de acces
///**************************************************************************************
CChaptersList& UTGetChaptersList()
{
	static CChaptersList g_Chapters;
	return g_Chapters;
}


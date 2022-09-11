#include "dxstdafx.h"

#include <sstream>

///--- MOD DESCRIPTOR ---

CModsManager::CModDescriptor::CModDescriptor()
{
	bSubscribed = false;
	bActive = false;
	uID = 0;
	unTime_create = 0;
	unTime_updated = 0;

	shName.Reset();
	shImagePath.Reset();
	memset(strAuthor, 0, sizeof(strAuthor));
	memset(strDescription, 0, sizeof(strDescription));
	memset(strChangeNotes, 0, sizeof(strChangeNotes));
	memset(strTags, 0, sizeof(strTags));
	memset(strVersion, 0, sizeof(strVersion));

	arrAffectedFiles.RemoveAll();
}

CModsManager::CModDescriptor::~CModDescriptor()
{
	SAFE_DELETE_GROWABLE_ARRAY(arrAffectedFiles);
}

bool CModsManager::CModDescriptor::GetFullPathToModImage(WCHAR* strDest, UINT32 nDestSize)
{
	if (shImagePath.IsEmpty())
		return false;

	std::wostringstream stream;
	stream << uID;

	StringCchPrintf(strDest, nDestSize, L"%s%s/mod_root/%s", UTApp().g_wszModsDir, stream.str().c_str(), shImagePath.text);
	return true;
}

bool CModsManager::CModDescriptor::GetFullPathToAffectedFile(int nFileIdx, WCHAR* strDest, UINT32 nDestSize)
{
	if ((nFileIdx < 0) || (nFileIdx >= arrAffectedFiles.GetSize()))
	   return false;

	CStringHash* shFilePath = arrAffectedFiles[nFileIdx];
	std::wostringstream stream;
	stream << uID;

	StringCchPrintf(strDest, nDestSize, L"%s%s/mod_root/%s", UTApp().g_wszModsDir, stream.str().c_str(), shFilePath->text);
	return true;
}

int CModsManager::CModDescriptor::LoadModDescriptor(const WCHAR* strModRootDirectory)
{
	//build mod descriptor path
	WCHAR strPath[MAX_PATH];
	StringCchCopy(strPath, MAX_PATH, strModRootDirectory);
	//add hardcoded mod descriptor name
	StringCchCat(strPath, MAX_PATH, L"/mod_desc.xml");

	pugi::xml_document doc;
	if (!doc.load_file(strPath))
	{
		ErrorBox(K_ERR_WARNING, L"[WARNING] CModsManager:: Unable to load Mod descriptor XML: %s\n", strPath);
		return 0;
	}

	pugi::xml_node rootnode = doc.root().child(L"ModDescriptor_DKAS");
	if (rootnode == null)
	{
		ErrorBox(K_ERR_WARNING, L"[WARNING] CModsManager:: Unable to load Mod descriptor XML (wrong format): %s\n", strPath);
		return 0;
	}

	//type
	CStringHash shType(rootnode.attribute(L"type").value());
	if (shType.IsEqual(L"SINGLE_LEVEL"))
		eType = K_MOD_TYPE_SINGLE_LEVEL;
	else
		eType = K_MOD_TYPE_GAME_CHANGER;
	//get attributes
	shName.Init(rootnode.attribute(L"name").value());
	shImagePath.Init(rootnode.attribute(L"image").value());
	//author
	StringCchCopy(strAuthor, MAX_PATH, rootnode.attribute(L"author").value());
	if(wcslen(strAuthor) == 0)
		StringCchCopy(strAuthor, MAX_PATH, L"Unknown Hero");
	//description
	StringCchCopy(strDescription, K_MOD_MAX_TEXT_LEN, rootnode.attribute(L"description").value());
	//change notes
	StringCchCopy(strChangeNotes, K_MOD_MAX_TEXT_LEN, rootnode.attribute(L"changeNotes").value());
	//tags
	StringCchCopy(strTags, MAX_PATH, rootnode.attribute(L"tags").value());
	//version
	StringCchCopy(strVersion, MAX_PATH, rootnode.attribute(L"gameVersion").value());

	for (pugi::xml_node bnode = rootnode.first_child(); bnode; bnode = bnode.next_sibling())
	{
		//name
		const WCHAR* strRelPath = bnode.attribute(L"path").value();
		if (wcslen(strRelPath) > 0)
		{
			CStringHash* shPath = new CStringHash(strRelPath);
			arrAffectedFiles.Add(shPath);
		}
	}

	return arrAffectedFiles.Count();
}


///--- MODS MANAGER ---

CModsManager::CModsManager()
{
}

CModsManager::~CModsManager()
{
	SAFE_DELETE_GROWABLE_ARRAY(m_arrMods);
}

HRESULT CModsManager::LoadModsFromCacheFile()
{
	//load document
	WCHAR xmlPath[MAX_PATH];
	StringCchPrintf(xmlPath, MAX_PATH, L"%smods.xml", UTApp().g_wszModsDir);

	pugi::xml_document doc;
	if (!doc.load_file(xmlPath))
	{
		LOG(L"[Warning] Unable to load Mods config XML:%s\n", xmlPath);
		return E_FAIL;
	}
	//remove mods from array
	SAFE_DELETE_GROWABLE_ARRAY(m_arrMods);

	pugi::xml_node rootnode = doc.root().child(L"Mods");
	for (pugi::xml_node bnode = rootnode.first_child(); bnode; bnode = bnode.next_sibling())
	{
		CModDescriptor* ndesc = new CModDescriptor();

#ifdef WIN32
		ndesc->uID = _wtoi64(bnode.attribute(L"id").value());
#else
		ndesc->uID = wtoll(bnode.attribute(L"id").value());
#endif

		ndesc->unTime_create = bnode.attribute(L"creationTime").as_uint();
		ndesc->unTime_updated = bnode.attribute(L"lastUpdateTime").as_uint();
		ndesc->bActive = bnode.attribute(L"active").as_bool();

		//now load mod details (always installed in userdata/mods/MOD_ID folder)
		WCHAR strModPath[MAX_PATH];
		StringCchPrintf(strModPath, MAX_PATH, L"%s%s/mod_root", UTApp().g_wszModsDir, bnode.attribute(L"id").value());
		if (ndesc->LoadModDescriptor(strModPath) != 0)
		{
			m_arrMods.Add(ndesc);
		}
		else
		{
			SAFE_DELETE(ndesc);
		}
	}

	return S_OK;
}

HRESULT CModsManager::SaveModsToCacheFile()
{
	pugi::xml_document doc;

	pugi::xml_node modsListNode = doc.append_child(L"Mods");

	for (int kk = 0; kk < m_arrMods.GetSize(); kk++)
	{
		CModDescriptor* dl = m_arrMods[kk];
		pugi::xml_node modNode = modsListNode.append_child(L"File");
		//64 bit str
		std::wostringstream stream;
		stream << dl->uID;
		modNode.append_attribute(L"id");
		modNode.attribute(L"id").set_value(stream.str().c_str());
		//active
		modNode.append_attribute(L"active");
		modNode.attribute(L"active").set_value(dl->bActive);
		//file created
		modNode.append_attribute(L"creationTime");
		modNode.attribute(L"creationTime").set_value(dl->unTime_create);
		//file modified
		modNode.append_attribute(L"lastUpdateTime");
		modNode.attribute(L"lastUpdateTime").set_value(dl->unTime_updated);
	}

	WCHAR xmlPath[MAX_PATH];
	StringCchPrintf(xmlPath, MAX_PATH, L"%smods.xml", UTApp().g_wszModsDir);

	FILE* file = OS_wfopen(xmlPath, L"w");
	if (file)
	{
		pugi::xml_writer_file writer(file);
		doc.save(writer);
	}
	else
	{
		ErrorBox(K_ERR_WARNING, L"[WARNING] Failed to save mods list file: %s\nMake sure the game has access to the file and that your antivirus solution isn't blocking the game's access.", xmlPath);
	}

	OS_fclose(file);


	return S_OK;
}

CModsManager::CModDescriptor* CModsManager::GetModDescByID(UINT64 unModID)
{
	for (int kk = 0; kk < m_arrMods.GetSize(); kk++)
	{
		if (m_arrMods[kk]->uID == unModID)
			return m_arrMods[kk];
	}

	return null;
}

int CModsManager::GetModIndexByID(UINT64 unModID)
{
	for (int kk = 0; kk < m_arrMods.GetSize(); kk++)
	{
		if (m_arrMods[kk]->uID == unModID)
			return kk;
	}

	return -1;
}

CModsManager::CModDescriptor* CModsManager::GetModDescByIndex(int nIndex)
{
	if ((nIndex < 0) || (nIndex >= m_arrMods.GetSize()))
		return null;

	return m_arrMods[nIndex];
}

CModsManager::CModDescriptor* CModsManager::GetModThatUsesFile(WCHAR* strRelPath, CModDescriptor* pAvoidMod)
{
	UINT32 shPathHash = FastHash(strRelPath);
	if (shPathHash == 0)
		return null;

	for (int kk = 0; kk < m_arrMods.GetSize(); kk++)
	{
		//avoid specified mod
		if (m_arrMods[kk] == pAvoidMod)
			continue;
		//don't check inactives
		if (m_arrMods[kk]->bActive == false)
			continue;
		//don't check conflicts on mods that only add levels (they might have the same name but they'll work)
		if (m_arrMods[kk]->eType == K_MOD_TYPE_SINGLE_LEVEL)
			continue;

		for (int ll = 0; ll < m_arrMods[kk]->arrAffectedFiles.GetSize(); ll++)
		{
			if (shPathHash == m_arrMods[kk]->arrAffectedFiles[ll]->getHash())
				return m_arrMods[kk];
		}
	}

	return null;
}

bool CModsManager::GetFullPathForFile(const WCHAR * wsMediaPath, WCHAR * wsRetPath, int nRetPathSize)
{
	UINT32 shPathHash = FastHash(wsMediaPath);
	if (shPathHash == 0)
		return false;

	for (int kk = 0; kk < m_arrMods.GetSize(); kk++)
	{
		//don't check mods that only add levels
		if (m_arrMods[kk]->eType == K_MOD_TYPE_SINGLE_LEVEL)
			continue;
		//don't check inactive mods
		if (m_arrMods[kk]->bActive == false)
			continue;

		for (int ll = 0; ll < m_arrMods[kk]->arrAffectedFiles.GetSize(); ll++)
		{
			if (shPathHash == m_arrMods[kk]->arrAffectedFiles[ll]->getHash())
			{
				m_arrMods[kk]->GetFullPathToAffectedFile(ll, wsRetPath, nRetPathSize);
				return true;
			}
		}
	}

	return false;
}

bool CModsManager::IsModActive(CModDescriptor* mod)
{
	if (mod == null)
		return false;

	return mod->bActive;
}

CModsManager::CModDescriptor* CModsManager::SetModActive(CModDescriptor* mod, bool bActive)
{
	if (mod == null)
		return null;
	//no change
	if (bActive == mod->bActive)
		return null;
	//we're activating a mod
	if (bActive == true)
	{
		//check for filename conflicts with other activated mods and return false if conflicting
		for (int kk = 0; kk < mod->arrAffectedFiles.GetSize(); kk++)
		{
			CModDescriptor* modConflict = GetModThatUsesFile(mod->arrAffectedFiles[kk]->text, mod);
			if (modConflict)
			{
				return modConflict;
			}
		}
	}

	//set activation and save
	mod->bActive = bActive;
	SaveModsToCacheFile();
	return null;
}

bool CModsManager::IsCompatibleWithCurrentVersion(CModDescriptor* mod)
{
	//don't check conflicts on mods that only add levels (they might have the same name but they'll work)
	if (mod->eType == K_MOD_TYPE_SINGLE_LEVEL)
		return true;

	int nMajor = 0, nMinor = 0, nPatch = 0;
	if (false == GetVersionFromString(mod->strVersion, nMajor, nMinor, nPatch))
	{
		ErrorBox(K_ERR_WARNING, L"Incorrect mod version! [%s] ver[%s]", mod->shName.text, mod->strVersion);
		return false;
	}

	//check mod version
	if ((nMajor < _VERSION_MINMOD_MAJOR_) || (nMinor < _VERSION_MINMOD_MINOR_) || (nPatch < _VERSION_MINMOD_PATCH_))
	{
		// old mods that change sounds or strings will be excluded
		ErrorBox(K_ERR_WARNING, L"Old incompatible mod found! [%s] ver[%s] expecting ver[%s]", mod->shName.text, mod->strVersion, _VERSION_MINMOD_WCHARSTR_);
		return false;
	}

	return true;
}

int CModsManager::GetActiveModsCountByType(eModType eSelType)
{
	int nCnt = 0;
	for (int kk = 0; kk < m_arrMods.GetSize(); kk++)
	{
		if (m_arrMods[kk]->eType == eSelType)
			nCnt++;
	}

	return nCnt;
}

///**************************************************************************************
/// Sigleton
///**************************************************************************************

CModsManager& __Mods()
{
	static CModsManager g_ModsMgr;
	return g_ModsMgr;
}

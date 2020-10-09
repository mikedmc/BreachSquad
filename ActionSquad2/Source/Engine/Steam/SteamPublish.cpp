#include "dxstdafx.h"

#ifdef ENABLE_STEAM

#include "SteamPublish.h"

#include "SteamUtils.h"
#include "steam_api.h"
#include "isteamremotestorage.h"

#include <string>
#include <vector>

//****************************************************************************************

bool Workshop_Publish(const WCHAR* strPathToModRoot)
{
	ISteamRemoteStorage* steamRemoteStorage = SteamRemoteStorage();
	SteamAnswerReceiver steamAnswerReceiver;

	// print some general information on cloud storage
	uint64 totalBytes, availableBytes;
	steamRemoteStorage->GetQuota(&totalBytes, &availableBytes);
	g_pLog->Write("Cloud Quota: Total=%.1f MB, Available=%.1f MB\n", totalBytes / 1048576.0f, availableBytes / 1048576.0f);

	int32 numCloudFiles = steamRemoteStorage->GetFileCount();
	g_pLog->Write("Cloud files:\n");
	for (int i = 0; i < numCloudFiles; ++i)
	{
		int32 fileSize = 0;
		const char* szFileName = steamRemoteStorage->GetFileNameAndSize(i, &fileSize);
		g_pLog->Write("%s (%.1f MB)\n", szFileName, fileSize / (1024.0f * 1024.0f));
	}

	std::wstring sProcessedPath = RemoveQuotationMarks(strPathToModRoot);
	LOG(L"PublishToWorkshop: Mod to publish: %s\n", sProcessedPath.c_str());

	// load xml properties
	CModsManager::CModDescriptor modInfo;
	int result = modInfo.LoadModDescriptor(sProcessedPath.c_str());
	if (result == 0)
	{
		g_pLog->Write("[Error] PublishToWorkshop: Could not load %s/mod.xml\n", sProcessedPath.c_str());
		return false;
	}

	std::string sUploadedModName, sUploadedImageName;
	CHAR strchPathToModRoot[MAX_PATH];
	wcstombs(strchPathToModRoot, sProcessedPath.c_str(), MAX_PATH);
	result = UploadToCloud(steamRemoteStorage, strchPathToModRoot, modInfo, sUploadedModName, sUploadedImageName);
	if (!result)
	{
		return false;
	}

	// share the file we just uploaded to steam cloud
	SteamAPICall_t handle = steamRemoteStorage->FileShare(sUploadedModName.c_str());
	CCallResult<SteamAnswerReceiver, RemoteStorageFileShareResult_t> m_callResultShareFile;
	m_callResultShareFile.Set(handle, &steamAnswerReceiver, &SteamAnswerReceiver::OnFileShared );

	// wait a bit, until we finish sharing the file
	g_pLog->Write("PublishToWorkshop: Sharing file ...\n");
	while(!steamAnswerReceiver.m_bSharedFile)
	{
		SteamAPI_RunCallbacks();
	}

	// publish the file
	CHAR strTags[2048];
	wcstombs(strTags, modInfo.strTags, MAX_PATH);

	std::vector<std::string> sTags;
	if (strlen(strTags) > 0)
		split(strTags, ',', sTags);

	AppId_t appId = STEAM_APP_ID;
	SteamParamStringArray_t publishTags;
	publishTags.m_nNumStrings = sTags.size();
	publishTags.m_ppStrings = new const char *[publishTags.m_nNumStrings];
	for (int i = 0; i < publishTags.m_nNumStrings; ++i)
		publishTags.m_ppStrings[i] = sTags[i].c_str();

	CHAR strModName[MAX_PATH], strModDesc[2048];
	wcstombs(strModName, modInfo.shName.text, MAX_PATH);
	wcstombs(strModDesc, modInfo.strDescription, 2048);

	handle = steamRemoteStorage->PublishWorkshopFile(
		sUploadedModName.c_str(),
		sUploadedImageName.c_str(),
		appId, 
		strModName,
		strModDesc,
		k_ERemoteStoragePublishedFileVisibilityPublic,
		&publishTags,
		k_EWorkshopFileTypeCommunity );

	CCallResult<SteamAnswerReceiver, RemoteStoragePublishFileResult_t > m_callResultPublishFile;
	m_callResultPublishFile.Set(handle, &steamAnswerReceiver, &SteamAnswerReceiver::OnFilePublished );

	// wait a bit, until we finish sharing the file
	g_pLog->Write("PublishToWorkshop: Publishing file ...\n");
	while(!steamAnswerReceiver.m_bPublishedFile)
	{
		SteamAPI_RunCallbacks();
	}

	delete[] publishTags.m_ppStrings;

	// steam cloud files - quick check
	int cloudFiles = steamRemoteStorage->GetFileCount();
	g_pLog->Write("PublishToWorkshop: Steam cloud files: %d\n", cloudFiles);

	return true;
}

//****************************************************************************************

#endif // ENABLE_STEAM
#include "dxstdafx.h"

#ifdef ENABLE_STEAM

#include "SteamSubscriptions.h"

#include <vector>
#include <string>
#include <sstream>
#include <iostream>

#include "FileManager.h"

#include "SteamUtils.h"
#include "steam_api.h"
#include "isteamremotestorage.h"


static bool InstallMod(const char* szFullFileName, CModsManager::CModDescriptor* pModInfo, bool bDontActivateIfNotAlready = false)
{
	//was changed to download and unpack all mods in the Win Temp folder in case user had chinese characters in the user folder (zip packer doesn't support that).
	//to go back just use g_wszModsDir instead of g_wszModsDirTemp and stop copying files from win temp folder

	char szDestinationRoot[MAX_PATH_STD];
	char szModsDir[MAX_PATH_STD];
	wcstombs(szModsDir, UTGetAppClass().g_wszModsDirTemp, MAX_PATH_STD);

	std::ostringstream stream;
	stream << pModInfo->uID; // needed to write int64 as string
	sprintf(szDestinationRoot, "%s%s", szModsDir, stream.str().c_str());
	
	// get the zip's root folder
	char szFullPath[MAX_PATH_STD] = {0};
	char szZipRootFolder[MAX_PATH_STD] = {0};
	bool bFolderOK = FileManager::GetRootFolderFromZip(szFullFileName, szZipRootFolder);
	if (bFolderOK)
	{
		// construct folder name
		sprintf(szFullPath, "%s/%s", szDestinationRoot, szZipRootFolder);

		// delete if it already exists
		FileManager::DeleteFolder(szFullPath);
	}
	else
	{
		g_pLog->Write("[Error] InstallMod() Could not extract root path from %s\n", szFullFileName);
		return false;
	}

	// unzip-it
	bool bUnzipOK = FileManager::UnzipFile(szFullFileName, szDestinationRoot);
	if (!bUnzipOK)
	{
		g_pLog->Write("[Error] InstallMod() Could not unzip mod\n");
		return false;
	}

	// try to enable it
	WCHAR wszFullPath[MAX_PATH];
	mbstowcs(wszFullPath, szFullPath, MAX_PATH);
	int result = pModInfo->LoadModDescriptor(wszFullPath);
	if (result != 0)
	{
		if ((!UTGetModsManager().IsModActive(pModInfo)) && (!bDontActivateIfNotAlready))
		{
			if (UTGetModsManager().IsCompatibleWithCurrentVersion(pModInfo))
				UTGetModsManager().SetModActive(pModInfo, true);
			else
				UTGetModsManager().SetModActive(pModInfo, false);
		}
	}
	else
	{
		g_pLog->Write("[Error] InstallMod() Could not load mod\n");
		return false;
	}

	// keep the archive, all mods are uploaded using the same name, so it doesn't hurt us
	//remove(sFileName.c_str());
	return true;
}

static void DeleteMod(const CModsManager::CModDescriptor* pModInfo)
{
	char szPreviousInstallPath[MAX_PATH_STD] = {0};
	char szModsDir[MAX_PATH_STD];
	wcstombs(szModsDir, UTGetAppClass().g_wszModsDirTemp, MAX_PATH_STD);

	// delete new-style folder (including the unique id)
	std::ostringstream stream;
	stream << pModInfo->uID; // needed to write int64 as string
	sprintf(szPreviousInstallPath, "%s%s", szModsDir, stream.str().c_str());
	FileManager::DeleteFolder(szPreviousInstallPath);
}

static bool UpdateMod(const char* szFullFileName, CModsManager::CModDescriptor* pModInfo)
{
	DeleteMod(pModInfo);

	return InstallMod(szFullFileName, pModInfo, true);
}

//****************************************************************************************

class SteamModsRetrievedAnswerReceiver
{
	public:
		struct ModSubscription
		{
			public:
				ModSubscription(PublishedFileId_t _fileId, uint32 _fileTimeStamp)
					: fileId(_fileId)
					, fileTimeStamp(_fileTimeStamp)
				{
				}

			public:
				PublishedFileId_t	fileId;
				uint32				fileTimeStamp;
		};

	public:
		SteamModsRetrievedAnswerReceiver()
			: m_bModsRetrieved(false), m_modsRetrievedStatus(k_EResultFail), m_totalSubscriptions(0)
		{
		}

	public:
		bool							m_bModsRetrieved;
		EResult							m_modsRetrievedStatus;
		std::vector<ModSubscription>	m_vModsSubscribed;
		int								m_totalSubscriptions; // m_vModsSubscribed.size() must be equal to this, otherwise call EnumerateUserSubscribedFiles(m_totalSubscriptions - m_vModsSubscribed.size()) to download the rest of them

	public:
		void OnSubscribedModsRetrieved( RemoteStorageEnumerateUserSubscribedFilesResult_t* pParam, bool /*flag*/ )
		{
			m_bModsRetrieved = true;
			m_modsRetrievedStatus = pParam->m_eResult;
			if (m_modsRetrievedStatus != k_EResultOK)
			{
				g_pLog->Write("[Error] SteamModsRetrievedAnswerReceiver: Enumerate user subscribed files FAILED with error %d!\n", m_modsRetrievedStatus);
				return;
			}

			m_totalSubscriptions = pParam->m_nTotalResultCount;

			g_pLog->Write("SteamModsRetrievedAnswerReceiver: Enumerate user subscribed files OK = %d!\n", pParam->m_nResultsReturned);		
			for( int i(0); i < pParam->m_nResultsReturned; ++i )
			{
				m_vModsSubscribed.push_back(ModSubscription(pParam->m_rgPublishedFileId[i], pParam->m_rgRTimeSubscribed[i]));
			}
		}
};

//****************************************************************************************

class SteamUGCDownloadFileAnswerReceiver
{
	public:
		SteamUGCDownloadFileAnswerReceiver()
			: m_bAnswerReceived(false)
		{
		}

	public:
		bool			m_bAnswerReceived;
		int32			m_iSizeDownloaded;
		std::string		m_sFileName;

	public:
		void OnUGCDownloadFile( RemoteStorageDownloadUGCResult_t* pParam, bool /*flag*/ )
		{
			g_pLog->Write("SteamUGCDownloadFileAnswerReceiver: Got downloaded file details! Name: %s, Size: %d\n", pParam->m_pchFileName, pParam->m_nSizeInBytes);
			m_bAnswerReceived = true;
			m_iSizeDownloaded = pParam->m_nSizeInBytes;
			m_sFileName = pParam->m_pchFileName;
		}
};

//****************************************************************************************

void Workshop_CheckSubscriptions()
{
	//we create the mods folders in the temp folder so we're (almost) sure we don't have utf8 characters
	char szModsDir[MAX_PATH_STD];
	wcstombs(szModsDir, UTGetAppClass().g_wszModsDirTemp, MAX_PATH_STD);

	UTGetModsManager().LoadModsFromCacheFile();

	SteamModsRetrievedAnswerReceiver steamEnumerateAnswerReceiver;
	int startIndex = 0;
	do
	{
		// check subscriptions, and update accordingly
		SteamAPICall_t enumerateHandle = SteamRemoteStorage()->EnumerateUserSubscribedFiles(startIndex);
		CCallResult<SteamModsRetrievedAnswerReceiver, RemoteStorageEnumerateUserSubscribedFilesResult_t > subscribedModsRetrievedCallResult;
		subscribedModsRetrievedCallResult.Set( enumerateHandle, &steamEnumerateAnswerReceiver, &SteamModsRetrievedAnswerReceiver::OnSubscribedModsRetrieved );

		// wait a bit, until we finish sharing the file
		unsigned int lastTime = OS_GetTimeMS();
		g_pLog->Write("CheckSteamWorkshopSubscriptions: Getting subscribed mod list ... \n");

		if (steamEnumerateAnswerReceiver.m_modsRetrievedStatus != k_EResultOK)
		{
			g_pLog->Write("[Error] OnSubscribedModsRetrieved: Could not enumerate mods (no connection?).\n");
			return;
		}

		while(!steamEnumerateAnswerReceiver.m_bModsRetrieved)
		{
			SteamAPI_RunCallbacks();

			if ((OS_GetTimeMS() - lastTime) > 8000)
			{
				g_pLog->Write("[Error] OnSubscribedModsRetrieved: timed out, exiting.\n");
				return; // do not continue, otherwise mods will get deleted
			}
		}

		steamEnumerateAnswerReceiver.m_bModsRetrieved = false;
		startIndex = steamEnumerateAnswerReceiver.m_vModsSubscribed.size();

	} while (startIndex != steamEnumerateAnswerReceiver.m_totalSubscriptions || steamEnumerateAnswerReceiver.m_modsRetrievedStatus != k_EResultOK);

	if (steamEnumerateAnswerReceiver.m_modsRetrievedStatus != k_EResultOK)
	{
		return; // do not continue, otherwise mods will get deleted
	}

	// start downloading subscribed items
	for( size_t i(0); i < steamEnumerateAnswerReceiver.m_vModsSubscribed.size(); ++i )
	{
		SteamModsRetrievedAnswerReceiver::ModSubscription& modSubscription = steamEnumerateAnswerReceiver.m_vModsSubscribed[i];
		
		SteamAPICall_t publishedFileDetailsHandle = SteamRemoteStorage()->GetPublishedFileDetails(modSubscription.fileId, modSubscription.fileTimeStamp/*(uint32)k_WorkshopForceLoadPublishedFileDetailsFromCache*/); // force get
		SteamPublishedFileDetailsAnswerReceiver steamPublishedFileDetailsAnswerReceiver;
		CCallResult<SteamPublishedFileDetailsAnswerReceiver, RemoteStorageGetPublishedFileDetailsResult_t > publishedFileDetailsCallResult;
		publishedFileDetailsCallResult.Set( publishedFileDetailsHandle, &steamPublishedFileDetailsAnswerReceiver, &SteamPublishedFileDetailsAnswerReceiver::OnPublishedFileGetDetails );

		// wait a bit, until we finish sharing the file
		g_pLog->Write("CheckSteamWorkshopSubscriptions: Getting file details ...\n");
		unsigned int lastTime = OS_GetTimeMS();
		while(!steamPublishedFileDetailsAnswerReceiver.m_bAnswerReceived)
		{
			SteamAPI_RunCallbacks();

			if ((OS_GetTimeMS() - lastTime) > 8000)
			{
				g_pLog->Write("[Error] OnPublishedFileGetDetails: timed out, exiting.\n");
				break;
			}
		}

		// find this file in our cached downloads
		CModsManager::CModDescriptor* pDownloadEntry = UTGetModsManager().GetModDescByID(modSubscription.fileId);

		if (!steamPublishedFileDetailsAnswerReceiver.m_bAnswerReceived && pDownloadEntry)
		{
			// we get here by timing out in the loop above
			pDownloadEntry->bSubscribed = true; // mark as still subscribed, so that we don't delete it yet (can't know for sure what happened since we timed out)
			continue;
		}

		if (steamPublishedFileDetailsAnswerReceiver.m_FileDetails.m_eResult == k_EResultFileNotFound)
		{
			std::ostringstream stream;
			stream << modSubscription.fileId;
			g_pLog->Write("CheckSteamWorkshopSubscriptions: File %s not found! Mod was probably deleted from Workshop, so we will also delete it.\n", stream.str().c_str());
			continue;
		}

		if (pDownloadEntry)
		{
			// still subscribed to this one
			pDownloadEntry->bSubscribed = true;
			
			// check if we need to update
			if (pDownloadEntry->unTime_updated >= steamPublishedFileDetailsAnswerReceiver.m_FileDetails.m_rtimeUpdated)
			{
				g_pLog->Write("CheckSteamWorkshopSubscriptions: Not updating '%s', we already have the latest version.\n", steamPublishedFileDetailsAnswerReceiver.m_FileDetails.m_rgchTitle);
				continue;
			}
			else
			{
				g_pLog->Write("CheckSteamWorkshopSubscriptions: Downloading '%s', an update has been issued.\n", steamPublishedFileDetailsAnswerReceiver.m_FileDetails.m_rgchTitle);
			}
		}

		g_pLog->Write("CheckSteamWorkshopSubscriptions: Downloading UGC '%s'\n", steamPublishedFileDetailsAnswerReceiver.m_FileDetails.m_rgchTitle);
		SteamAPICall_t downloadFileHandle = SteamRemoteStorage()->UGCDownload(steamPublishedFileDetailsAnswerReceiver.m_FileDetails.m_hFile, 0); // 0 - force start
		SteamUGCDownloadFileAnswerReceiver steamUGCDownloadFileAnswerReceiver;
		CCallResult<SteamUGCDownloadFileAnswerReceiver, RemoteStorageDownloadUGCResult_t > downloadUGCFileCallResult;
		downloadUGCFileCallResult.Set( downloadFileHandle, &steamUGCDownloadFileAnswerReceiver, &SteamUGCDownloadFileAnswerReceiver::OnUGCDownloadFile );
		
		// do the actual downloading
		int32 nPrevBytesDownloaded = 0;
		int32 nBytesExpected = 0;
		lastTime = OS_GetTimeMS();
		while(!steamUGCDownloadFileAnswerReceiver.m_bAnswerReceived)
		{
			SteamAPI_RunCallbacks();

			if ((OS_GetTimeMS() - lastTime) > 10000 && !nBytesExpected)
			{
				g_pLog->Write("[Error] CheckSteamWorkshopSubscriptions: timed out, skipping...\n");
				break;
			}

			int32 expected = 0;
			int32 nBytesDownloaded = 0;
			if (SteamRemoteStorage()->GetUGCDownloadProgress(steamPublishedFileDetailsAnswerReceiver.m_FileDetails.m_hFile, &nBytesDownloaded, &expected) == true)
			{
				nBytesExpected = expected; // because if GetUGCDownloadProgress() returns false, expected will also be zero, which happens when finishing the download
				if ((nBytesDownloaded - nPrevBytesDownloaded) > 1048576)
				{
					// print status once in a while
					nPrevBytesDownloaded = nBytesDownloaded;
					g_pLog->Write("CheckSteamWorkshopSubscriptions() download progress: %dKB/%dKB\n", nBytesDownloaded / 1024, nBytesExpected / 1024);
				}
			}
		}

		if (!steamUGCDownloadFileAnswerReceiver.m_iSizeDownloaded)
			continue;

		// read contents into a file
		char szFullFileName[MAX_PATH_STD] = "";
		sprintf(szFullFileName, "%s%s", szModsDir, steamUGCDownloadFileAnswerReceiver.m_sFileName.c_str());
		FILE* f = OS_fopen(szFullFileName, "wb");
		if (!f)
		{
			g_pLog->Write("[Error] Could not open %s for writing, not installing mod\n", szFullFileName);
			continue;
		}

		bool bSuccessReading = true;
		if (steamUGCDownloadFileAnswerReceiver.m_iSizeDownloaded > (10 * 1024 * 1024))
		{
			// read in chunks if larger than...
			const int chunkSize = 5 * 1024 * 1024;
			unsigned char* dataBuffer = new unsigned char[ chunkSize ];

			int32 offset = 0;
			int32 readBytes = 0;
			do
			{
				readBytes = SteamRemoteStorage()->UGCRead(steamPublishedFileDetailsAnswerReceiver.m_FileDetails.m_hFile, dataBuffer, chunkSize, offset, k_EUGCRead_ContinueReadingUntilFinished);
				offset += readBytes;
				if (!readBytes)
				{
					g_pLog->Write("CheckSteamWorkshopSubscriptions: Zero bytes read for %s\n", steamPublishedFileDetailsAnswerReceiver.m_FileDetails.m_rgchTitle);
					bSuccessReading = false;
					break;
				}
				else
				{	
					fwrite(dataBuffer, 1, readBytes, f);
				}

			} while (readBytes == chunkSize);
		}
		else
		{
			unsigned char* dataBuffer = new unsigned char[ steamUGCDownloadFileAnswerReceiver.m_iSizeDownloaded ];
			int32 readBytes = SteamRemoteStorage()->UGCRead(steamPublishedFileDetailsAnswerReceiver.m_FileDetails.m_hFile, dataBuffer, steamUGCDownloadFileAnswerReceiver.m_iSizeDownloaded, 0, k_EUGCRead_Close);
			if (!readBytes)
			{
				g_pLog->Write("CheckSteamWorkshopSubscriptions: Zero bytes read for %s\n", steamPublishedFileDetailsAnswerReceiver.m_FileDetails.m_rgchTitle);
				bSuccessReading = false;
			}
			else
			{
				fwrite(dataBuffer, 1, steamUGCDownloadFileAnswerReceiver.m_iSizeDownloaded, f);
			}
			delete [] dataBuffer;
		}

		OS_fclose(f);

		if (!bSuccessReading)
			continue;

		// write entry into the steam downloaded files status, even if InstallMod failed. If it failed the first time, there's a good chance that the mod is broken and we don't want to download it every time.
		bool bUpdate = pDownloadEntry != NULL;
		if (!pDownloadEntry)
		{
			CModsManager::CModDescriptor* dl = new CModsManager::CModDescriptor();
			UTGetModsManager().m_arrMods.Add(dl);
			pDownloadEntry = UTGetModsManager().m_arrMods[UTGetModsManager().m_arrMods.GetSize() - 1];
		}

		//pDownloadEntry->name            = steamPublishedFileDetailsAnswerReceiver.m_FileDetails.m_rgchTitle;  //not needed
		pDownloadEntry->uID				= steamPublishedFileDetailsAnswerReceiver.m_FileDetails.m_nPublishedFileId;
		pDownloadEntry->unTime_create	= steamPublishedFileDetailsAnswerReceiver.m_FileDetails.m_rtimeCreated;
		pDownloadEntry->unTime_updated  = steamPublishedFileDetailsAnswerReceiver.m_FileDetails.m_rtimeUpdated;
		pDownloadEntry->bSubscribed     = true;
		pDownloadEntry->bActive			= false;  //by default new mods are not activated

		if (bUpdate)
		{
			bool bResult = UpdateMod(szFullFileName, pDownloadEntry);
			if (bResult)
				g_pLog->Write("CheckSteamWorkshopSubscriptions: Successfully updated %s\n", steamPublishedFileDetailsAnswerReceiver.m_FileDetails.m_rgchTitle);
			else
				g_pLog->Write("CheckSteamWorkshopSubscriptions: Failed updating %s, will not try again.\n", steamPublishedFileDetailsAnswerReceiver.m_FileDetails.m_rgchTitle);
		}
		else
		{
			bool bResult = InstallMod(szFullFileName, pDownloadEntry, true); //all mods are disabled by default
			if (bResult)
				g_pLog->Write("CheckSteamWorkshopSubscriptions: Successfully installed %s\n", steamPublishedFileDetailsAnswerReceiver.m_FileDetails.m_rgchTitle);
			else
				g_pLog->Write("CheckSteamWorkshopSubscriptions: Failed installation of %s, will not try again.\n", steamPublishedFileDetailsAnswerReceiver.m_FileDetails.m_rgchTitle);
		}

		// save after each successfully downloaded mod
		UTGetModsManager().SaveModsToCacheFile();

		//copy mod folder to final position (user data)
		WCHAR wcsModDestPath[1024];
		std::wostringstream stream;
		stream << pDownloadEntry->uID; // needed to write int64 as string
		StringCchPrintf(wcsModDestPath, 1024, L"%s%s", UTGetAppClass().g_wszModsDir, stream.str().c_str());
		WCHAR wcsModSrcPath[1024];
		StringCchPrintf(wcsModSrcPath, 1024, L"%s%s", UTGetAppClass().g_wszModsDirTemp, stream.str().c_str());
		//erase dest folder	if it exists
		OS_DeleteRecursive(wcsModDestPath);
		//copy downloaded folder to dest mods folder
		if(CreateDirectory(wcsModDestPath, NULL))
			OS_CopyRecursive(wcsModSrcPath, wcsModDestPath);
	}

	// delete the ones we're not subscribed to anymore
	for (int k = UTGetModsManager().m_arrMods.GetSize() - 1; k >= 0; k--)
	{
		CModsManager::CModDescriptor* dl = UTGetModsManager().m_arrMods[k];
		// check if mods are still compatible
		if (dl->bActive)
		{
			if (!UTGetModsManager().IsCompatibleWithCurrentVersion(dl))
				UTGetModsManager().SetModActive(dl, false);
		}
		
		// delete inactive mods
		if (dl->bSubscribed)
			continue;

		LOG(L"CheckSteamWorkshopSubscriptions: You have unsubscribed (or was deleted by owner) from mod %s, deleting.\n", dl->shName.text);

		//delete temp copy
		DeleteMod(dl); 
		//delete final copy
		WCHAR wcsModDestPath[1024];
		std::wostringstream stream;
		stream << dl->uID; // needed to write int64 as string
		StringCchPrintf(wcsModDestPath, 1024, L"%s%s", UTGetAppClass().g_wszModsDir, stream.str().c_str());
		OS_DeleteRecursive(wcsModDestPath);

		//remove from mods list too
		SAFE_DELETE(UTGetModsManager().m_arrMods[k]);
		UTGetModsManager().m_arrMods.Remove(k);
	}

	UTGetModsManager().SaveModsToCacheFile();
}


//****************************************************************************************

#endif // ENABLE_STEAM
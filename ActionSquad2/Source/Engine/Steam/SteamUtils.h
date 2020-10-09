#pragma once

#include <string>
#include <vector>

#ifdef ENABLE_STEAM
#include "steam_api.h"
#include "isteamremotestorage.h"

//****************************************************************************************

class SteamAnswerReceiver
{
	public:
		SteamAnswerReceiver()
			: m_bSharedFile(false)
			, m_bPublishedFile(false)
		{
		}

	public:
		bool m_bSharedFile;
		bool m_bPublishedFile;

	public:
		void OnFileShared( RemoteStorageFileShareResult_t* /*pParam*/, bool /*flag*/ );
		void OnFilePublished( RemoteStoragePublishFileResult_t* /*pParam*/, bool /*flag*/ );
};

//****************************************************************************************

class SteamPublishedFileDetailsAnswerReceiver
{
	public:
		SteamPublishedFileDetailsAnswerReceiver()
			: m_bAnswerReceived(false)
		{
		}

	public:
		bool											m_bAnswerReceived;
		RemoteStorageGetPublishedFileDetailsResult_t	m_FileDetails;

	public:
		void OnPublishedFileGetDetails( RemoteStorageGetPublishedFileDetailsResult_t* pParam, bool /*flag*/ );
};

//****************************************************************************************

class SteamUserPublishedFilesRetrievedAnswerReceiver
{
	public:
		SteamUserPublishedFilesRetrievedAnswerReceiver()
			: m_bPublishedFilesRetrieved(false), m_modsRetrievedStatus(k_EResultFail), m_totalPublished(0)
		{
		}

	public:
		bool							m_bPublishedFilesRetrieved;
		std::vector<PublishedFileId_t>	m_vPublishedFiles;
		int								m_totalPublished; // m_vPublishedFiles.size() must be equal to this, otherwise call EnumerateUserSubscribedFiles(m_totalSubscriptions - m_vModsSubscribed.size()) to download the rest of them
		EResult							m_modsRetrievedStatus;

	public:
		void OnUserPublishedFilesRetrieved( RemoteStorageEnumerateUserPublishedFilesResult_t* pParam, bool /*flag*/ );
};

//****************************************************************************************

bool UploadToCloud( ISteamRemoteStorage* steamRemoteStorage,
					const char* szModFolderPath,
					const CModsManager::CModDescriptor& modInfo,
					std::string& sUploadedModName,
					std::string& sUploadedImageName );

// set hook to process steam errors / warnings
void SetSteamAPIDebugTextHook();

#endif // ENABLE_STEAM
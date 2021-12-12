#include "dxstdafx.h"

#ifdef ENABLE_STEAM

#include "SteamPublish.h"

#include "FileManager.h"
#include "Mods.h"
#include "SteamUtils.h"

#include "steam_api.h"
#include "isteamremotestorage.h"

#include <string>
#include <vector>

//****************************************************************************************

class SteamUpdatePublishedFileAnswerReceiver
{
	public:
		SteamUpdatePublishedFileAnswerReceiver()
			: m_bAnswerReceived(false)
		{
		}

	public:
		bool m_bAnswerReceived;

	public:
		void OnUpdatedPublishedFile( RemoteStorageUpdatePublishedFileResult_t* /*pParam*/, bool /*flag*/ )
		{
			m_bAnswerReceived = true;
			g_pLog->Write( "SteamPublishedFileDetailsAnswerReceiver: Update published successfully !\n" );
		}
};

//****************************************************************************************

bool Workshop_UpdatePublished(const WCHAR* strPathToModRoot)
{
	ISteamRemoteStorage* steamRemoteStorage = SteamRemoteStorage();
	SteamAnswerReceiver steamAnswerReceiver;

	std::wstring sProcessedPath = RemoveQuotationMarks(strPathToModRoot);
	LOG( L"Workshop_UpdatePublished: Mod to update: %s\n", sProcessedPath.c_str() );

	// load xml properties
	CModsManager::CModDescriptor modInfo;
	int result = modInfo.LoadModDescriptor(sProcessedPath.c_str());
	if (result == 0)
	{
		LOG(L"[Error] Workshop_UpdatePublished: Could not load %s/mod.xml\n", sProcessedPath.c_str());
		return false;
	}

	// check published files, and update accordingly
	SteamUserPublishedFilesRetrievedAnswerReceiver steamEnumerateAnswerReceiver;
	int startIndex = 0;
	do
	{
		SteamAPICall_t enumerateHandle = SteamRemoteStorage()->EnumerateUserPublishedFiles(startIndex);
		CCallResult<SteamUserPublishedFilesRetrievedAnswerReceiver, RemoteStorageEnumerateUserPublishedFilesResult_t > userPublishedFilesRetrievedCallResult;
		userPublishedFilesRetrievedCallResult.Set( enumerateHandle, &steamEnumerateAnswerReceiver, &SteamUserPublishedFilesRetrievedAnswerReceiver::OnUserPublishedFilesRetrieved );

		// wait a bit, until we finish sharing the file
		unsigned int lastTime = OS_GetTimeMS();
		g_pLog->Write( "Workshop_UpdatePublished: Getting published file list ... \n" );
		while(!steamEnumerateAnswerReceiver.m_bPublishedFilesRetrieved)
		{
			SteamAPI_RunCallbacks();

			if ((OS_GetTimeMS() - lastTime) > 15000)
			{
				g_pLog->Write( "[Error] Workshop_UpdatePublished: timed out, exiting.\n" );
				break;
			}
		}

		steamEnumerateAnswerReceiver.m_bPublishedFilesRetrieved = false;
		startIndex = steamEnumerateAnswerReceiver.m_vPublishedFiles.size();

	} while (startIndex != steamEnumerateAnswerReceiver.m_totalPublished || steamEnumerateAnswerReceiver.m_modsRetrievedStatus != k_EResultOK);

	// start checking published items
	bool bFound = false;
	for( size_t i(0); i < steamEnumerateAnswerReceiver.m_vPublishedFiles.size(); ++i )
	{
		const PublishedFileId_t& publishedFile = steamEnumerateAnswerReceiver.m_vPublishedFiles[i];

		SteamAPICall_t publishedFileDetailsHandle = SteamRemoteStorage()->GetPublishedFileDetails(publishedFile, 0); // force get
		SteamPublishedFileDetailsAnswerReceiver steamPublishedFileDetailsAnswerReceiver;
		CCallResult<SteamPublishedFileDetailsAnswerReceiver, RemoteStorageGetPublishedFileDetailsResult_t > publishedFileDetailsCallResult;
		publishedFileDetailsCallResult.Set( publishedFileDetailsHandle, &steamPublishedFileDetailsAnswerReceiver, &SteamPublishedFileDetailsAnswerReceiver::OnPublishedFileGetDetails );

		// wait a bit, until we finish sharing the file
		g_pLog->Write( "Workshop_UpdatePublished: Getting file details ...\n" );
		unsigned int lastTime = OS_GetTimeMS();
		while(!steamPublishedFileDetailsAnswerReceiver.m_bAnswerReceived)
		{
			SteamAPI_RunCallbacks();

			if ((OS_GetTimeMS() - lastTime) > 15000)
			{
				g_pLog->Write( "Workshop_UpdatePublished: timed out, exiting.\n" );
				break;
			}
		}

		if (steamPublishedFileDetailsAnswerReceiver.m_FileDetails.m_eResult == k_EResultFileNotFound)
		{
			g_pLog->Write( "[Error] Workshop_UpdatePublished: File not found!\n" );
			continue;
		}

		g_pLog->Write( "Workshop_UpdatePublished: Got mod '%s'!\n", steamPublishedFileDetailsAnswerReceiver.m_FileDetails.m_rgchTitle);

		UINT32 unTitleHash = FastHash(steamPublishedFileDetailsAnswerReceiver.m_FileDetails.m_rgchTitle);
		if( modInfo.shName.getHash() != unTitleHash )
		{
			continue;
		}

		bFound = true;
		g_pLog->Write( "Workshop_UpdatePublished: Found mod, started updating !\n" );

		PublishedFileUpdateHandle_t publishedFileUpdateHandle =	SteamRemoteStorage()->CreatePublishedFileUpdateRequest( steamPublishedFileDetailsAnswerReceiver.m_FileDetails.m_nPublishedFileId );

		// update title (not like this, title is the same)
		//bool bResult = SteamRemoteStorage()->UpdatePublishedFileTitle(publishedFileUpdateHandle, modInfo.name.GetString() );
		//if( bResult )
		//	g_pLog->Write( "Workshop_UpdatePublished: Updated title !\n" );
		//else
		//	g_pLog->Write( "[Error] Workshop_UpdatePublished: Could not update title !\n" );

		// update description (people like to edit detailed descriptions directly in the Steam Workshop GUI, we shouldn't delete them)
		//bResult = SteamRemoteStorage()->UpdatePublishedFileDescription(publishedFileUpdateHandle, modInfo.description.GetString() );
		//if( bResult )
		//	g_pLog->Write( "Workshop_UpdatePublished: Updated description !\n" );
		//else
		//	g_pLog->Write( "[Error] Workshop_UpdatePublished: Could not update description !\n" );

		// update change notes
		CHAR strNotes[2048];
		wcstombs(strNotes, modInfo.strChangeNotes, 2048);
		bool bResult = SteamRemoteStorage()->UpdatePublishedFileSetChangeDescription(publishedFileUpdateHandle, strNotes );
		if( bResult )
			g_pLog->Write( "Workshop_UpdatePublished: Added change notes !\n" );
		else
			g_pLog->Write( "[Error] Workshop_UpdatePublished: Could not update change notes !\n" );

		// update tags
		CHAR strTags[MAX_PATH];
		wcstombs(strTags, modInfo.strTags, MAX_PATH);

		std::vector<std::string> sTags;
		if (strlen(strTags) > 0)
			split(strTags, ',', sTags);

		SteamParamStringArray_t publishTags;
		publishTags.m_nNumStrings = sTags.size();
		publishTags.m_ppStrings = new const char *[publishTags.m_nNumStrings];
		for ( int jj = 0; jj < publishTags.m_nNumStrings; ++jj )
			publishTags.m_ppStrings[ jj ] = sTags[ jj ].c_str();

		bResult = SteamRemoteStorage()->UpdatePublishedFileTags(publishedFileUpdateHandle, &publishTags);
		if( bResult )
			g_pLog->Write( "Workshop_UpdatePublished: Updated tags !\n" );
		else
			g_pLog->Write( "[Error] Workshop_UpdatePublished: Could not update tags !\n" );

		delete[] publishTags.m_ppStrings;

		// update preview file and mod file
		std::string sUploadedModName, sUploadedImageName;
		CHAR strchPathToModRoot[MAX_PATH];
		wcstombs(strchPathToModRoot, sProcessedPath.c_str(), MAX_PATH);
		result = UploadToCloud(steamRemoteStorage, strchPathToModRoot, modInfo, sUploadedModName, sUploadedImageName);
		if (!result)
		{
			return false;
		}

		// share the mod file we just uploaded to steam cloud
		SteamAPICall_t handle = steamRemoteStorage->FileShare(sUploadedModName.c_str() );
		CCallResult<SteamAnswerReceiver, RemoteStorageFileShareResult_t> m_callResultShareFile;
		m_callResultShareFile.Set(handle, &steamAnswerReceiver, &SteamAnswerReceiver::OnFileShared );

		// wait a bit, until we finish sharing the file
		g_pLog->Write( "Workshop_UpdatePublished: Sharing file ...\n" );
		while(!steamAnswerReceiver.m_bSharedFile)
		{
			SteamAPI_RunCallbacks();
		}

		// preview file update
		bResult = SteamRemoteStorage()->UpdatePublishedFilePreviewFile(publishedFileUpdateHandle, sUploadedImageName.c_str() );
		if( bResult )
			g_pLog->Write( "Workshop_UpdatePublished: Updated preview file !\n" );
		else 
			g_pLog->Write( "[Error] Workshop_UpdatePublished: Could not update preview file !\n" );

		// mod file update
		bResult = SteamRemoteStorage()->UpdatePublishedFileFile(publishedFileUpdateHandle, sUploadedModName.c_str() );
		if( bResult )
			g_pLog->Write( "Workshop_UpdatePublished: Updated mod file !\n" );
		else
			g_pLog->Write( "[Error] Workshop_UpdatePublished: Could not update mod file !\n" );

		SteamAPICall_t updatedPublishedFileHandle = SteamRemoteStorage()->CommitPublishedFileUpdate(publishedFileUpdateHandle);
		SteamUpdatePublishedFileAnswerReceiver steamUpdatePublishedFileAnswerReceiver;
		CCallResult<SteamUpdatePublishedFileAnswerReceiver, RemoteStorageUpdatePublishedFileResult_t > updatePublishedFileCallResult;
		updatePublishedFileCallResult.Set( updatedPublishedFileHandle, &steamUpdatePublishedFileAnswerReceiver, &SteamUpdatePublishedFileAnswerReceiver::OnUpdatedPublishedFile );

		g_pLog->Write( "Workshop_UpdatePublished: Publishing update ...\n" );
		//lastTime = OS_GetTimeMS();
		while(!steamPublishedFileDetailsAnswerReceiver.m_bAnswerReceived)
		{
			SteamAPI_RunCallbacks();

			//if ((OS_GetTimeMS() - lastTime) > 2000)
			//{
			//	g_pLog->Write( "Workshop_UpdatePublished: timed out, exiting.\n" );
			//	break;
			//}
		}

		break;
	}

	if (!bFound)
	{
		LOG(L"[Warning] Workshop_UpdatePublished: Could not update mod '%s', not found in the published mods. You should check the name correspondence.\n", modInfo.shName.text);
	}

	// steam cloud files - quick check
	int cloudFiles = steamRemoteStorage->GetFileCount();
	g_pLog->Write( "Workshop_UpdatePublished: Steam cloud files: %d\n", cloudFiles );

	return bFound;
}

//****************************************************************************************

#endif // ENABLE_STEAM
#include "dxstdafx.h"

#ifdef ENABLE_STEAM

#include "SteamUtils.h"

#include <sstream>

//****************************************************************************************

void SteamAnswerReceiver::OnFileShared( RemoteStorageFileShareResult_t* /*pParam*/, bool /*flag*/ )
{
	g_pLog->Write("SteamAnswerReceiver: ShareFile OK !\n");
	m_bSharedFile = true;
}

void SteamAnswerReceiver::OnFilePublished( RemoteStoragePublishFileResult_t* /*pParam*/, bool /*flag*/ )
{
	g_pLog->Write("SteamAnswerReceiver: PublishFile OK !\n");
	m_bPublishedFile = true;
}

//****************************************************************************************

void SteamPublishedFileDetailsAnswerReceiver::OnPublishedFileGetDetails( RemoteStorageGetPublishedFileDetailsResult_t* pParam, bool /*flag*/ )
{
	g_pLog->Write("SteamPublishedFileDetailsAnswerReceiver: Got published file details !\n");
	m_bAnswerReceived = true;
	m_FileDetails = *pParam;
}

//****************************************************************************************

void SteamUserPublishedFilesRetrievedAnswerReceiver::OnUserPublishedFilesRetrieved( RemoteStorageEnumerateUserPublishedFilesResult_t* pParam, bool /*flag*/ )
{
	m_bPublishedFilesRetrieved = true;
	m_modsRetrievedStatus = pParam->m_eResult;
	if (m_modsRetrievedStatus != k_EResultOK)
	{
		g_pLog->Write("[Error] SteamUserPublishedFilesRetrievedAnswerReceiver: Enumerate user published files FAILED with error %d!\n", m_modsRetrievedStatus);
		return;
	}

	m_totalPublished = pParam->m_nTotalResultCount;

	g_pLog->Write("SteamUserPublishedFilesRetrievedAnswerReceiver: Enumerate user published files OK = %d!\n", pParam->m_nResultsReturned);		
	for( int i(0); i < pParam->m_nResultsReturned; ++i )
	{
		m_vPublishedFiles.push_back(pParam->m_rgPublishedFileId[i]);
	}
}

//****************************************************************************************

bool UploadToCloud( ISteamRemoteStorage* steamRemoteStorage,
					const char* szModFolderPath,
					const CModsManager::CModDescriptor& modInfo,
					std::string& sUploadedModName,
					std::string& sUploadedImageName )
{
	const char* szModUploadFileName = "dkas_modupload.zip"; // always use same name to save cloud space
	const char* szModImageUploadFileName = "dkas_modimageupload"; // always use same name to save cloud space. extension is added depending on source file
	char szTempStr[MAX_PATH];

	// zip the entire folder
	wcstombs(szTempStr, UTApp().g_wszTempFolderPath, MAX_PATH);
	std::string sZipName = szTempStr;
	sZipName += szModUploadFileName;
	bool result = FileManager::ZipFolder(szModFolderPath, sZipName.c_str(), true);
	if (!result)
	{
		g_pLog->Write("[Error] UploadToCloud: Could not zip file %s\n", sZipName.c_str());
		return false;
	}

	// write zip to steam cloud: if the file is larger than 40MB, split it into chunks (also we can't upload files larger than 100MB in a single call)
	long fileSize = FileManager::GetFileSize(sZipName.c_str());
	if (fileSize <= (40 * 1024 * 1024))
	{
		// read file from disk and write it to steam cloud
		unsigned char* fileContent = FileManager::FileLoadBinary(sZipName.c_str(), &fileSize);
		if (!fileContent)
		{
			g_pLog->Write("[Error] UploadToCloud: Could not load file %s\n", sZipName.c_str());
			return false;
		}

		result = steamRemoteStorage->FileWrite(szModUploadFileName, fileContent, fileSize);
		delete[] fileContent;
		if( !result )
		{
			g_pLog->Write("[Error] UploadToCloud: Could not write modUpload.zip to cloud ! \n");
			return false;
		}
	}
	else
	{
		// is this necessary?
		if (steamRemoteStorage->FileExists(szModUploadFileName))
			steamRemoteStorage->FileDelete(szModUploadFileName);

		UGCFileWriteStreamHandle_t writeHandle = steamRemoteStorage->FileWriteStreamOpen(szModUploadFileName);
		if (writeHandle == k_UGCFileStreamHandleInvalid)
			g_pLog->Write("[Error] UploadToCloud: FileWriteStreamOpen(%s) failed!\n", szModUploadFileName);

		FILE* f = fopen(sZipName.c_str(), "rb");
		const int chunkSize = 5 * 1024 * 1024;
		unsigned char* pChunk = new unsigned char[chunkSize];

		unsigned int readBytes = 0;
		do
		{
			readBytes = fread(pChunk, 1, chunkSize, f);
			result = steamRemoteStorage->FileWriteStreamWriteChunk(writeHandle, pChunk, readBytes);
			if( !result )
			{
				g_pLog->Write("[Error] UploadToCloud: FileWriteStreamWriteChunk() Could not write %.1f MB!\n", readBytes / (1024.0f * 1024.0f));
				steamRemoteStorage->FileWriteStreamCancel(writeHandle);
				break;
			}
			else
			{
				g_pLog->Write("Uploading %.1f MB to cloud...\n", readBytes / (1024.0f * 1024.0f));
			}
		} while (readBytes == chunkSize);

		g_pLog->Write("Uploading %.1f MB done!\n", fileSize / (1024.0f * 1024.0f));

		result = steamRemoteStorage->FileWriteStreamClose(writeHandle);
		if( !result )
			g_pLog->Write("[Error] UploadToCloud: FileWriteStreamClose() failed!\n");

		delete [] pChunk;
		fclose(f);
	}

	// remove archive, we no longer need it
	remove(sZipName.c_str());

	// read image file and write to cloud
	CHAR strImgName[MAX_PATH];
	wcstombs(strImgName, modInfo.shImagePath.text, MAX_PATH);
	StringCchPrintfA(szTempStr, MAX_PATH, "%s/%s", szModFolderPath, strImgName);
	LOG("Loading Mod image from: %s", szTempStr);

	unsigned char* fileContent = FileManager::FileLoadBinary(szTempStr, &fileSize);
	if( fileContent )
	{
		// write image to steam cloud
		const char* imageExtension = NULL;
		wcstombs(szTempStr, modInfo.shImagePath.text, MAX_PATH);
		FileManager::ExtractFilenameFromFullPath(szTempStr, NULL, &imageExtension);
		std::string sImageName = szModImageUploadFileName;
		sImageName += ".";
		sImageName += imageExtension;
		result = steamRemoteStorage->FileWrite(sImageName.c_str(), fileContent, fileSize);
		delete [] fileContent;
		if( !result )
		{
			LOG("[Error] UploadToCloud: Could not write %s to cloud ! \n", szTempStr);
		}

		sUploadedImageName = sImageName;
	}
	else
	{
		LOG(L"[Error] UploadToCloud: Could not read mod image file %s! \n", modInfo.shImagePath.text);

		sUploadedImageName = "";
	}

	sUploadedModName = szModUploadFileName;	

	return true;
}



//****************************************************************************************

extern "C" void __cdecl SteamAPIDebugTextHook( int nSeverity, const char *pchDebugText )
{
	// if you're running in the debugger, only warnings (nSeverity >= 1) will be sent
	// if you add -debug_steamapi to the command-line, a lot of extra informational messages will also be sent
	g_pLog->Write( pchDebugText );

	if ( nSeverity >= 1 )
	{
		// place to set a breakpoint for catching API errors
		int x = 3;
		(void)x;
	}
}

//****************************************************************************************

void SetSteamAPIDebugTextHook()
{
	// set our debug handler
	SteamClient()->SetWarningMessageHook( &SteamAPIDebugTextHook );
}

//****************************************************************************************

#endif // ENABLE_STEAM

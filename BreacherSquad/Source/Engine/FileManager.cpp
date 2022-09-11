#include "dxstdafx.h"

#include <sys/stat.h>

#define MINIZ_HEADER_FILE_ONLY
#include "miniz.cpp"

#if TARGET_OS_IPHONE
char g_sziOSAppDirectory[MAX_PATH_STD];
#endif

void FileManager::GetMediaPath(const WCHAR *wsMediaName, WCHAR wsRetPath[MAX_PATH_STD], bool bIgnoreMods)
{
#ifdef ENABLE_STEAM_WORKSHOP

	if (bIgnoreMods)
	{
		StringCchPrintf(wsRetPath, MAX_PATH_STD, L"%s%s", UTApp().g_wszExePath, wsMediaName);
	}
	else
	{
		WCHAR wsPath[MAX_PATH_STD];
		bool bModFound = __Mods().GetFullPathForFile(wsMediaName, wsPath, MAX_PATH_STD);
		if (bModFound)
		{
			StringCchCopy(wsRetPath, MAX_PATH_STD, wsPath);
		}
		else
		{
			StringCchPrintf(wsRetPath, MAX_PATH_STD, L"%s%s", UTApp().g_wszExePath, wsMediaName);
		}
	}

#else
	//no modding? just build full path
	StringCchPrintf(wsRetPath, MAX_PATH_STD, L"%s%s", UTApp().g_wszExePath, wsMediaName);

#endif
}

FILE* FileManager::GetFilePointer(char const* path, char const* mode)
{
	return fopen(path, mode);
}

long FileManager::GetFileSize(const char* szFileName)
{
	struct stat filestatus;
	int result;

	result = stat(szFileName, &filestatus);

	if (result == 0 && (filestatus.st_mode & S_IFDIR))
		return 0; // it's a directory

	return (result >= 0) ? filestatus.st_size : 0;
}

bool FileManager::FileExists(const char* szFileName)
{
	struct stat filestatus;
	int result;

	result = stat(szFileName, &filestatus);

	if (result != 0 || (filestatus.st_mode & S_IFDIR))
		return false;
	return true;
}

bool FileManager::IsDirectory(const char* szPath)
{
	// must not end in '/'
	struct stat filestatus;
	int result = 0;

	int len = strlen(szPath);
	if (szPath[len - 1] == '/')
	{
		char szNewPath[MAX_PATH_STD];		
		strncpy(szNewPath, szPath, len - 1);
		szNewPath[len - 1] = '\0';
		result = stat(szNewPath, &filestatus);
	}
	else
	{
		result = stat(szPath, &filestatus);
	}

	if (result == 0 && (filestatus.st_mode & S_IFDIR))
		return true; // it's a directory
	return false;
}

unsigned char* FileManager::FileLoadBinary(const char *szFilePath, long *length, bool bAddNULL /*= false*/)
{
	long filesize = GetFileSize(szFilePath);
	unsigned char *buffer = new unsigned char[bAddNULL ? filesize + 1 : filesize];
	if (!buffer)
		return NULL;

	bool bResult = FileLoadBinary(szFilePath, filesize, buffer);
	if (!bResult)
	{
		delete [] buffer;
		return NULL;
	}

	if (length)
		*length = bAddNULL ? filesize + 1 : filesize;

	if (bAddNULL)
		buffer[filesize] = '\0';

	return buffer;
}

bool FileManager::FileLoadBinary(const char *szFilePath, long dataSize, unsigned char* pData)
{
	FILE* fpBinary = GetFilePointer(szFilePath, "rb");
	if (!fpBinary)
		return false;

	long actualBytesRead = fread(pData, 1, dataSize, fpBinary);
	assert(actualBytesRead == dataSize);
	fclose(fpBinary);
	return true;
}

/*
unsigned int FileManager::FileCRC(const char* szFileName, const unsigned int& uiHashValue)
{
	long bufferLen = 0;
	unsigned char* pszFileBuffer = FileLoadBinary( szFileName, &bufferLen, false );
	if( !pszFileBuffer )
		return 0;

	unsigned int generatedHash = FastHash( pszFileBuffer, (bufferLen );
	delete[] pszFileBuffer;

	return generatedHash;
}
*/

char* FileManager::FileLoadText(const char *szFilePath, long *length)
{
	return (char *)FileLoadBinary(szFilePath, length, true);
}

void FileManager::FileReplaceExtension(const char *szFileName, const char *szExt, char *szNewFileName)
{	
	const char *pExt = NULL;
	ExtractFilenameFromFullPath(szFileName, NULL, &pExt);

	int extLen = pExt ? strlen(pExt) : 0;
	int fileNameNoExtLen = strlen(szFileName) - extLen;
	strncpy(szNewFileName, szFileName, fileNameNoExtLen);

	const char *psz = szExt;
	while ( *psz != '\0' )
		szNewFileName[fileNameNoExtLen++] = *psz++;
	szNewFileName[fileNameNoExtLen] = *psz;
}

const char * FileManager::ExtractFilenameFromFullPath(const char *fullpath, int *filenameLength, const char **extension)
{
	const char *p = fullpath + strlen(fullpath);
	const char *ext = NULL;
	int fileNameLen = 0;
	
	while ( p != fullpath && *p != '/' && *p != '\\' )
	{
		if ( *p == '.' )
			ext = p + 1;
		--p;
		fileNameLen++;
	}

	if (p != fullpath)
	{	
		++p;
		--fileNameLen;
	}
	
	if ( filenameLength )
		*filenameLength = fileNameLen;
	if ( extension )
		*extension = ext;
	return p;	
}

static void GetFilesInFolderTree_Recursive(const char* szFolderPath, bool bIncludeFolders, List<char*>& fileList, bool bFullPath)
{
	// folders
	List<char*> files;
	OS_GetFolderFiles(szFolderPath, "/", files, false);
	for (int i = 0; i < files.GetNumElements(); ++i)
	{
		if (!strcmp(files[i], ".") || !strcmp(files[i], ".."))
			continue;
			
		// recurse
		char szFullFolderPath[MAX_PATH_STD];
		sprintf(szFullFolderPath, "%s/%s", szFolderPath, files[i]);

		// add folders in shallow-to-deep order
		if (bIncludeFolders)
			fileList.Add(strdup(szFullFolderPath));

		GetFilesInFolderTree_Recursive(szFullFolderPath, bIncludeFolders, fileList, bFullPath);
	}

	for (int i = 0; i < files.GetNumElements(); ++i)
		delete [] files[i];
	files.Reset();

	// files
	OS_GetFolderFiles(szFolderPath, NULL, files, bFullPath);
	for (int i = 0; i < files.GetNumElements(); ++i)
	{
		fileList.Add(files[i]);
	}
}

void FileManager::GetFilesInFolderTree(const char* szFolderPath, bool bIncludeFolders, List<char*>& fileList)
{
	GetFilesInFolderTree_Recursive(szFolderPath, bIncludeFolders, fileList, true);
}

bool FileManager::DeleteFolder(const char* szPath)
{
	g_pLog->Write("FileManager::DeleteFolder() %s\n", szPath);

	if (!IsDirectory(szPath))
	{
		g_pLog->Write("[Error] FileManager::DeleteFolder() %s is not a folder.\n", szPath);
		return false;
	}

	List<char*> fileList;
	GetFilesInFolderTree(szPath, true, fileList);

	// delete contained files first
	for (int i = 0; i < fileList.GetNumElements(); ++i)
	{
		if (IsDirectory(fileList[i]))
			continue;

		g_pLog->Write("FileManager::DeleteFolder() deleting file %s\n", fileList[i]);
		remove(fileList[i]);
	}

	// reverse the iteration, to get the deepest ones first
	for (int i = fileList.GetNumElements() - 1; i >= 0; --i)
	{
		if (!IsDirectory(fileList[i]))
			continue;

		g_pLog->Write("FileManager::DeleteFolder() deleting folder %s\n", fileList[i]);
		OS_DeleteFolder(fileList[i]);
	}

	for (int i = 0; i < fileList.GetNumElements(); ++i)
		delete [] fileList[i];

	// delete the root folder
	g_pLog->Write("FileManager::DeleteFolder() deleting root folder %s\n", szPath);
	OS_DeleteFolder(szPath);

	g_pLog->Write("FileManager::DeleteFolder() done!\n");
	return true;
}

void FileManager::AppendFileToCompressedZip(const char* szZipFileName, const char* szFullFileName, const char* szZipStructureFileName)
{
	g_pLog->Write("FileManager::AppendFileToCompressedZip() Appending %s to %s\n", szFullFileName, szZipFileName);
	mz_bool result = mz_zip_add_file_to_archive_file_in_place(szZipFileName, szZipStructureFileName, szFullFileName, "no comment", (mz_uint16)strlen("no comment"), MZ_BEST_COMPRESSION);
	if (!result)
		g_pLog->Write("[Error] FileManager::AppendFileToCompressedZip() Could not add %s (%s) to %s!\n", szFullFileName, szZipStructureFileName, szZipFileName);
}

bool FileManager::ZipFolder(const char* szFolderPath, const char* szZipFileName, bool bCompress)
{
	g_pLog->Write("FileManager::ZipFolder() Zipping folder %s\n", szFolderPath);

	List<char*> fileList;
	GetFilesInFolderTree(szFolderPath, true, fileList);

	mz_zip_archive zipArchive;
	memset(&zipArchive, 0, sizeof(zipArchive));
	mz_bool result = mz_zip_writer_init_file(&zipArchive, szZipFileName, 0);
	if (!result)
	{
		g_pLog->Write("[Error] FileManager::ZipFolder() Could not init archive %s\n", szZipFileName);
		for (int i = 0; i < fileList.GetNumElements(); ++i)
			delete [] fileList[i];
		return false;
	}

	const char* szBaseFolder = ExtractFilenameFromFullPath(szFolderPath, NULL, NULL);

	char szZipFolderName[MAX_PATH_STD];
	strcpy(szZipFolderName, szBaseFolder);
	strcat(szZipFolderName, "/"); // must end in a forward slash
	result = mz_zip_writer_add_mem(&zipArchive, szZipFolderName, NULL, 0, bCompress ? MZ_BEST_COMPRESSION : MZ_NO_COMPRESSION);

	for (int i = 0; i < fileList.GetNumElements(); ++i)
	{
		const char* szNameInZip = strstr(fileList[i], szBaseFolder);
		if (!szNameInZip)
			szNameInZip = fileList[i];

		g_pLog->Write("FileManager::ZipFolder() adding \"%s\"\n", szNameInZip);

		bool bIsFolder = IsDirectory(fileList[i]);
		if (bIsFolder)
		{
			strcpy(szZipFolderName, szNameInZip);
			strcat(szZipFolderName, "/"); // must end in a forward slash
			result = mz_zip_writer_add_mem(&zipArchive, szZipFolderName, NULL, 0, bCompress ? MZ_BEST_COMPRESSION : MZ_NO_COMPRESSION);
		}
		else
		{
			result = mz_zip_writer_add_file(&zipArchive, szNameInZip, fileList[i], "no comment", (mz_uint16)strlen("no comment"), bCompress ? MZ_BEST_COMPRESSION : MZ_NO_COMPRESSION);
		}

		if (!result)
			g_pLog->Write("[Error] FileManager::ZipFolder() Could not add %s to archive!\n", fileList[i]);

		delete [] fileList[i];
	}

	result = mz_zip_writer_finalize_archive(&zipArchive);
	if (!result)
		g_pLog->Write("[Error] FileManager::ZipFolder() mz_zip_writer_finalize_archive() failed!\n");

	result = mz_zip_writer_end(&zipArchive);
	if (!result)
		g_pLog->Write("[Error] FileManager::ZipFolder() mz_zip_writer_end() failed!\n");

	g_pLog->Write("FileManager::ZipFolder() done\n");
	return true;
}

bool FileManager::UnzipFile(const char* szZipFileName, const char* szDestination)
{
	g_pLog->Write("FileManager::UnzipFile() Unzipping %s to %s\n", szZipFileName, szDestination);

	mz_zip_archive zipArchive;
	memset(&zipArchive, 0, sizeof(zipArchive));
	mz_bool result = mz_zip_reader_init_file(&zipArchive, szZipFileName, 0);
	if (!result)
	{
		g_pLog->Write("[Error] FileManager::UnzipFile() Could not open %s\n", szZipFileName);
		return false;
	}

	if (!OS_CreateFolder(szDestination))
	{
		mz_zip_reader_end(&zipArchive);
		return false;
	}

	char szDstFileName[MAX_PATH_STD];
	for (int i = 0; i < (int)mz_zip_reader_get_num_files(&zipArchive); ++i)
	{
		mz_zip_archive_file_stat file_stat;
		if (!mz_zip_reader_file_stat(&zipArchive, i, &file_stat))
		{
			g_pLog->Write("[Error] FileManager::UnzipFile() mz_zip_reader_file_stat() failed!\n");
			continue;
		}

		if (mz_zip_reader_is_file_a_directory(&zipArchive, file_stat.m_file_index))
		{
			sprintf(szDstFileName, "%s/%s", szDestination, file_stat.m_filename);
			OS_CreateFolder(szDstFileName);
			continue;
		}

		sprintf(szDstFileName, "%s/%s", szDestination, file_stat.m_filename);
		result = mz_zip_reader_extract_to_file(&zipArchive, file_stat.m_file_index, szDstFileName, 0);
		if (!result)
		{
			g_pLog->Write("[Error] FileManager::UnzipFile() mz_zip_reader_extract_file_to_heap(%s) failed!\n", file_stat.m_filename);
			continue;
		}

		//size_t uncomp_size;
		//void *p = mz_zip_reader_extract_to_heap(&zipArchive, file_stat.m_file_index, &uncomp_size, 0);
		//if (!p)
		//{
		//	g_pLog->Write("[Error] FileManager::UnzipFile() mz_zip_reader_extract_file_to_heap(%s) failed!\n", file_stat.m_filename);
		//	mz_zip_reader_end(&zipArchive);
		//	return;
		//}
		//mz_free(p);

		g_pLog->Write("FileManager::UnzipFile() extracted file \"%s\", size %u\n", file_stat.m_filename, file_stat.m_uncomp_size);
	}

	mz_zip_reader_end(&zipArchive);
	g_pLog->Write("FileManager::UnzipFile() done\n");
	return true;
}

// this depends on how the ZipFolder() function works
bool FileManager::GetRootFolderFromZip(const char* szZipFileName, char szFolderName[MAX_PATH_STD])
{
	g_pLog->Write("FileManager::GetRootFolderFromZip() getting root folder from zip %s\n", szZipFileName);

	szFolderName[0] = '\0';

	mz_zip_archive zipArchive;
	memset(&zipArchive, 0, sizeof(zipArchive));
	mz_bool result = mz_zip_reader_init_file(&zipArchive, szZipFileName, 0);
	if (!result)
	{
		g_pLog->Write("[Error] FileManager::GetRootFolderFromZip() Could not open %s\n", szZipFileName);
		return false;
	}

	for (int i = 0; i < (int)mz_zip_reader_get_num_files(&zipArchive); ++i)
	{
		mz_zip_archive_file_stat file_stat;
		if (!mz_zip_reader_file_stat(&zipArchive, i, &file_stat))
		{
			g_pLog->Write("[Error] FileManager::UnzipFile() mz_zip_reader_file_stat() failed!\n");
			continue;
		}

		// stop at the first directory entry
		if (!mz_zip_reader_is_file_a_directory(&zipArchive, file_stat.m_file_index))
			continue;

		int len = strlen(file_stat.m_filename);
		if (file_stat.m_filename[len - 1] == '/')
		{
			strncpy(szFolderName, file_stat.m_filename, len - 1);
			szFolderName[len - 1] = '\0';
		}
		else
		{
			strcpy(szFolderName, file_stat.m_filename);
		}

		break;
	}

	mz_zip_reader_end(&zipArchive);
	return (szFolderName[0] != '\0');
}

void FileManager::CopyFileTo(const char* szFilePath, const char* szNewFilePath)
{
	FILE* fsrc = fopen(szFilePath, "rb");
	FILE* fdest = fopen(szNewFilePath, "wb");
	if (fdest && fsrc)
	{
		char buff[16384];
		int bytesRead = 0;
		do
		{
			bytesRead = fread(buff, 1, sizeof(buff), fsrc);
			fwrite(buff, 1, bytesRead, fdest);
		} while (bytesRead == sizeof(buff));
	}
	if (fdest)
		fclose(fdest);
	if (fsrc)
		fclose(fsrc);
}

// replaces any of the characters " \ / : * ? " < > | " with whitespace
void FileManager::MakeFileNameValid(char* szFilename)
{
	char* p = szFilename;
	while (*p != '\0')
	{
		const char c = *p;
		if (c == '\\' || c == '/' || c == ':' || c == '*' || c == '?'|| c == '"' || c == '<' || c == '>' || c == '|')
			*p = ' ';

		++p;
	}
}

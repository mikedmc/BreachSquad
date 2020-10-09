#pragma once

// had to take out unique_ptr because we couldn't compile c11 on Mac without raising the min required os to 10.7

namespace FileManager
{
	// Gets the full path for the first place item is found (by going through current the mods)
	// All media names start from and contain "media" folder (eg. "media/back/bk_woods.png")
	void			GetMediaPath(const WCHAR *wsMediaName, WCHAR wsRetPath[MAX_PATH_STD], bool bIgnoreMods = false); 

	FILE*			GetFilePointer(char const* path, char const* mode);
	unsigned char*	FileLoadBinary(const char *szFilePath, long *length, bool bAddNULL = false);
	bool			FileLoadBinary(const char *szFilePath, long dataSize, unsigned char* pData); // make sure pData is big enough by calling GetFileSize first
//#TODO:	 replace with GetFileHash from enginecommon
//unsigned int	FileCRC(const char* szFileName,const unsigned int& uiHashValue);
	char*			FileLoadText(const char *szFilePath, long *length);

	long			GetFileSize(const char* szFileName);
	bool			FileExists(const char* szFileName);

	// these do not search the path in existing mods
	bool			DeleteFolder(const char* szPath); // deletes entire folder, including all children and files within
	bool			IsDirectory(const char* szPath);
	void			CopyFileTo(const char* szFilePath, const char* szNewFilePath);

	// the directory structure is parsed in a depth-first order
	//		folders are added in the node parsing order, from shallow to deep, so that you can recreate the hierarchy if you need to
	//		files are added starting from the leaves
	void			GetFilesInFolderTree(const char* szFolderPath, bool bIncludeFolders, List<char*>& fileList); // returns all files by recursively parsing the directory tree

	// zip utility functions
	void			AppendFileToCompressedZip(const char* szZipFileName, const char* szFullFileName, const char* szZipStructureFileName); // szZipStructureFileName is the name inside the zip. must not contain drive letters, start with "/" or contain "\\". can contain folder hierarchies.
	bool			ZipFolder(const char* szFolderPath, const char* szZipFileName, bool bCompress); // when bCompress is false, the zip is created in Store mode
	bool			UnzipFile(const char* szZipFileName, const char* szDestination);
	bool			GetRootFolderFromZip(const char* szZipFileName, char szFolderName[MAX_PATH_STD]); // this depends on how the ZipFolder() function works

	// filename utilities
	void			FileReplaceExtension(const char *szFileName, const char *szExt, char *szNewFileName); // szExt should not contain the dot
	const char *	ExtractFilenameFromFullPath(const char *fullpath, int *filenameLength, const char ** extension);

	void			MakeFileNameValid(char* szFullFilename); // receives the filename (without the path) and replaces any of the characters " \ / : * ? " < > | " with whitespace
}

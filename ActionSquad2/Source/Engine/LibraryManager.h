#pragma once

class CLibraryManager{
public:	
	/*
	int numFiles;	
	long* offsets;
	int currID;
	char** names;

	int* lengths;
	FILE* file;
	void openLibrary(LPWSTR libName);
	void openTemp(int id);
	void openFile(int id);
	void closeLibrary();	
	BYTE readUByte();
	char readByte();
	short readShort();
	unsigned short readUShort();
	unsigned int readUInt();
	int readInt();
	int fileByName(const char* name);
	int readLine(char* dest, int maxLen);
	BYTE* openBuffer(int id);
	*/
	CLibraryManager();
	~CLibraryManager();

	FILE* filePtr;

	HRESULT extractFileToTemp(LPWSTR libName, UINT32 fileID);
	//buffer file
	BYTE* extractFileToBuffer(LPWSTR libName, UINT32 fileID, UINT32 &retBufferSize);
	//file pointer 
	INT32 setFilePointer(LPWSTR libName, UINT32 fileID); //Sets internal fileptr on requested file. RETURNS: selected file size or -1 if error
	INT32 setFilePointer(LPWSTR libName, const char* fileName); //Sets internal fileptr on requested file. RETURNS: selected file size or -1 if error
	HRESULT releaseFilePointer();

	INT32	getFileIdByName(LPWSTR libName, const char* fileName);
	void ReleaseAll();
};


//declar singletonul
CLibraryManager& UTGetLibraryManager();

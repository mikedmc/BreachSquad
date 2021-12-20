#include "dxstdafx.h"
#include "LibraryManager.h"

CLibraryManager::CLibraryManager() 
{
	filePtr = NULL;
}

CLibraryManager::~CLibraryManager()
{
	if(filePtr)
	{
		OS_fclose(filePtr);
	}
}

void CLibraryManager::ReleaseAll()
{
	if(filePtr)
	{
		OS_fclose(filePtr);
	}
}

//***********************************
//		FILE PTR
//***********************************
INT32 CLibraryManager::setFilePointer(LPWSTR libName, UINT32 fileID)
{
	WCHAR path[MAX_PATH];
	if(filePtr)
	{
		OS_fclose(filePtr);
		filePtr = NULL;
	}

	StringCchPrintf(path, MAX_PATH, L"%s%s", UTApp().g_wszAppResDir, libName);
	int err = OS_wfopen_s(&filePtr, path, L"rb");
	if(filePtr == NULL || err != 0)
		return -1;

	BYTE nb;
	UINT32 ni;

	OS_fread(&nb, sizeof(BYTE), 1, filePtr);
	bool hasNames = false;
	if(nb == 1)
		hasNames = true;
	
	UINT32 numfiles = 0;
	OS_fread(&ni, sizeof(UINT32), 1, filePtr);
	numfiles = ni;

	if(fileID > numfiles)
	{
		OS_fclose(filePtr);
		filePtr = NULL;
		return -1;
	}
	
	long skipper = 0;
	INT32 selectedFileSize = -1;
	//skip header
	for( UINT32 i = 0; i < numfiles; i++ )
	{
		//citesc numele fisierului; lungimea stirngului e tinuta in fata pe octet fara semn
		char tmpname[MAX_PATH];
		unsigned char strLen = 0;
		OS_fread(&strLen, 1, 1, filePtr);
		OS_fread(tmpname, strLen, 1, filePtr);
		tmpname[strLen] = 0;
		//citesc lungimea fisierului
		UINT32 len = 0;
		OS_fread(&len, sizeof(UINT32), 1, filePtr);
		//aduna marimile fisierelor ce trebuiesc sarite
		if(i < fileID)
		{
			skipper += len;
		}
		else if(i == fileID)
		{
			selectedFileSize = (INT32)len;
		}
	}	
	//sar peste fisierele dinainte
	fseek(filePtr, skipper, SEEK_CUR); 

	return selectedFileSize;
}


INT32 CLibraryManager::setFilePointer(LPWSTR libName, const char* fileName)
{
	WCHAR path[MAX_PATH];
	if(filePtr)
	{
		OS_fclose(filePtr);
		filePtr = NULL;
	}

	StringCchPrintf(path, MAX_PATH, L"%s%s", UTApp().g_wszAppResDir, libName);
	int err = OS_wfopen_s(&filePtr, path, L"rb");
	if(filePtr == NULL || err != 0)
		return -1;

	BYTE nb;
	UINT32 ni;

	OS_fread(&nb, sizeof(BYTE), 1, filePtr);
	bool hasNames = false;
	if(nb == 1)
		hasNames = true;
	
	UINT32 numfiles = 0;
	OS_fread(&ni, sizeof(UINT32), 1, filePtr);
	numfiles = ni;

	long skipper = 0;
	INT32 selectedFileSize = -1;
	//skip header
	bool filefound = false;
	for(UINT32 i=0; i<numfiles; i++)
	{
		//citesc numele fisierului; lungimea stirngului e tinuta in fata pe octet fara semn
		char tmpname[MAX_PATH];
		unsigned char strLen = 0;
		OS_fread(&strLen, 1, 1, filePtr);
		OS_fread(tmpname, strLen, 1, filePtr);
		tmpname[strLen] = 0;
		//citesc lungimea fisierului
		UINT32 len = 0;
		OS_fread(&len, sizeof(UINT32), 1, filePtr);
		
		if(strncmp(tmpname, fileName, strLen) == 0)
		{
			filefound = true;
			selectedFileSize = (INT32)len;
		}
		//aduna marimile fisierelor ce trebuiesc sarite pana sa il gaseasca pe cel bun
		if(!filefound)
		{
			skipper += len;
		}
	}	
	//sar peste fisierele dinainte
	fseek(filePtr, skipper, SEEK_CUR); 

	return selectedFileSize;
}


HRESULT CLibraryManager::releaseFilePointer()
{
	if(filePtr)
	{
		OS_fclose(filePtr);
		filePtr = NULL;
	}

	return S_OK;
}

//***************************
//		buffer file
//***************************
BYTE* CLibraryManager::extractFileToBuffer(LPWSTR libName, UINT32 fileID, UINT32 &retBufferSize)
{
	retBufferSize = 0;
	BYTE* fileBuffer = NULL;
	INT32 filesize = setFilePointer(libName, fileID);
	if(filesize <= 0)
		return NULL;
	//allocate buffer
	fileBuffer = new BYTE[filesize];
	if(fileBuffer == NULL)
		return NULL;
	//fill buffer
	int numread = OS_fread(fileBuffer, sizeof(BYTE), filesize, filePtr);
	//inchide pachetul
	OS_fclose(filePtr);
	if(numread != filesize)
	{
		SAFE_DELETE_ARRAY(fileBuffer);
		return NULL;
	}
	//return pointer to buffer
	retBufferSize = (UINT32)filesize;
	return fileBuffer;
}

//********************************
//		TEMP FILE
//********************************
HRESULT CLibraryManager::extractFileToTemp(LPWSTR libName, UINT32 fileID)
{
	INT32 filesize = setFilePointer(libName, fileID);
	if(filesize <= 0)
		return E_FAIL;
	//allocate buffer
	BYTE* fileBuffer = new BYTE[filesize];
	if(fileBuffer == NULL)
		return E_FAIL;
	//fill buffer
	int numread = OS_fread(fileBuffer, sizeof(BYTE), filesize, filePtr);
	if(numread != filesize)
	{
		SAFE_DELETE_ARRAY(fileBuffer);
		return E_FAIL;
	}
	releaseFilePointer();
	//scrie noul fisier

	FILE* temp = NULL;
	int err = OS_wfopen_s(&temp, UTApp().g_wszTempFilePath, L"wb");
	if(temp != NULL && err == 0)
	{
		OS_fwrite(fileBuffer, sizeof(BYTE), filesize, temp);
		OS_fclose(temp);		
	}	
	else
	{
		SAFE_DELETE_ARRAY(fileBuffer);
		return E_FAIL;
	}

	SAFE_DELETE_ARRAY(fileBuffer);
	return S_OK;
}

//**************************************
//		FILE id BY NAME
//**************************************
INT32	CLibraryManager::getFileIdByName(LPWSTR libName, const char* fileName)
{
	WCHAR path[MAX_PATH];
	if(filePtr)
	{
		OS_fclose(filePtr);
		filePtr = NULL;
	}

	StringCchPrintf(path, MAX_PATH, L"%s%s", UTApp().g_wszAppResDir, libName);
	int err = OS_wfopen_s(&filePtr, path, L"rb");
	if(filePtr == NULL || err != 0)
		return -1;

	BYTE nb;
	UINT32 ni;

	OS_fread(&nb, sizeof(BYTE), 1, filePtr);
	bool hasNames = false;
	if(nb == 1)
	{
		hasNames = true;
	}
	else
	{
		//daca nu sunt numele incluse nu poti cauta dupa nume
		OS_fclose(filePtr);
		return -1;
	}
	
	UINT32 numfiles = 0;
	OS_fread(&ni, sizeof(UINT32), 1, filePtr);
	numfiles = ni;

	for(int i=0; i < (int)numfiles; i++)
	{
		//citesc numele fisierului; lungimea stirngului e tinuta in fata pe octet fara semn
		char tmpname[MAX_PATH];
		unsigned char strLen = 0;
		OS_fread(&strLen, 1, 1, filePtr);
		OS_fread(tmpname, strLen, 1, filePtr);
		tmpname[strLen] = 0;
		//citesc lungimea fisierului
		UINT32 len = 0;
		OS_fread(&len, sizeof(UINT32), 1, filePtr);
		
		if(strncmp(tmpname, fileName, strLen) == 0)
		{
			OS_fclose(filePtr);
			return i;
		}
	}	

	OS_fclose(filePtr);

	return -1;
}

/*
void CLibraryManager::openTemp(int id){
	fseek(file, offsets[id], SEEK_SET);
	int len = lengths[id];
	BYTE* buffer = new BYTE[len];
	OS_fread(buffer, len, 1, file);	
	FILE* temp = NULL;
	int err = OS_wfopen_s(&temp, tempPath, L"wb");
	if(temp != NULL && err == 0){
		OS_fwrite(buffer, len, 1, temp);
		OS_fclose(temp);		
	}	
	SAFE_DELETE_ARRAY(buffer);
}

void CLibraryManager::closeLibrary(){
	OS_fclose(file);
	//if(PathFileExists(tempPath))
		DeleteFile(tempPath);
	SAFE_DELETE_ARRAY(offsets);
	SAFE_DELETE_ARRAY(lengths);
	for(int i=0; i<numFiles; i++)
		SAFE_DELETE_ARRAY(names[i]);
	SAFE_DELETE_ARRAY(names);
}

void CLibraryManager::openFile(int id){	
	fseek(file, offsets[id], SEEK_SET);
	currID = id;
}

/*
BYTE CLibraryManager::readUByte(){
	//DebugPrintA("readUByte()\n");
	BYTE byte;
	OS_fread(&byte, 1, 1, file);
	return byte;
}

unsigned short CLibraryManager::readUShort(){
	//DebugPrintA("readUShort()\n");
	unsigned short word;
	OS_fread(&word, 2, 1, file);
	return word;
}

char CLibraryManager::readByte(){
	//DebugPrintA("readByte()\n");
	char byte;
	OS_fread(&byte, 1, 1, file);
	return byte;
}

short CLibraryManager::readShort(){
	//DebugPrintA("readShort()\n");
	short word;
	OS_fread(&word, 2, 1, file);
	return word;
}

unsigned int CLibraryManager::readUInt(){
	//DebugPrintA("readUInt()\n");
	unsigned int dword;
	OS_fread(&dword, 4, 1, file);
	return dword;
}

int CLibraryManager::readInt(){
	//DebugPrintA("readUInt()\n");
	int dword;
	OS_fread(&dword, 4, 1, file);
	return dword;
}


int CLibraryManager::fileByName(const char* name){
	for(int i=0; i<numFiles; i++)
		if(strncmp(names[i], name, strlen(names[i])) == 0)
			return i;
	return -1;
}


int CLibraryManager::readLine(char* dest, int maxLen){
	char c;
	int i;
	for(i=0; i<maxLen-1 && (c=readByte()) != '\n'; i++)
		dest[i] = c;
	dest[i] = NULL;
	//DebugPrintA("readLine(): %s\n", dest);
	return i+1; //intoarce nr de caractere citite
	//fgets(dest, maxLen, file);
	//return 0;
}


BYTE* CLibraryManager::openBuffer(int id){
	int pos = ftell(file);
	fseek(file, offsets[id], SEEK_SET);
	int len = lengths[id];
	BYTE* buffer = NULL;
	buffer = new BYTE[len];
	OS_fread(buffer, len, 1, file);	
	fseek(file, pos, SEEK_SET);
	return buffer; //memoria o dezaloca utilizatorul
}
*/

/*
void CLibraryManager::openLibrary(LPWSTR libName){
	WCHAR path[MAX_PATH];
	StringCchPrintf(path, MAX_PATH, L"%sData\\%s", g_wszExePath, libName);
	int err = OS_wfopen_s(&file, path, L"rb");
	if(file == NULL || err != 0)
		return;
	BYTE nb;
	OS_fread(&nb, sizeof(BYTE), 1, file);
	numFiles = nb;
	offsets = new long[numFiles];
	lengths = new int[numFiles];
	names = new char*[numFiles];
	long offset = 1;
	for(int i=0; i<numFiles; i++){
		//citesc numele fisierului; lungimea stirngului e tinuta in fata pe octet fara semn
		unsigned char strLen = 0;
		OS_fread(&strLen, 1, 1, file);
		offset += 1;
		names[i] = new char[strLen+1];
		OS_fread(names[i], strLen, 1, file);
		offset += strLen;
		names[i][strLen] = 0;
		//citesc lungimea fisierului
		int len = 0;
		OS_fread(&len, sizeof(int), 1, file);
		offset += 4;
		//DebugPrintA("file: %d name: %s len: %d\n", i, names[i], len);
		offsets[i] = offset;
		lengths[i] = len;
		//sar peste date
		fseek(file, len, SEEK_CUR); //poate ar trebui sa pun toate informatiile astea intr-un header ca sa nu mai fac skip
		offset += len;		
	}	
	//StringCchPrintf(tempPath, MAX_PATH, L"%sResources\\temp", g_wszExePath);
}
*/


///**************************************************************************************
/// Sigleton de acces
///**************************************************************************************

CLibraryManager& UTGetLibraryManager()
{
	static CLibraryManager g_libMgr;
	return g_libMgr;
}

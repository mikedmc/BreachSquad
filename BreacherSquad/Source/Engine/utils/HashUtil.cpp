#include "dxstdafx.h"
#include "HashUtil.h"

///--- STRING HASH FUNCTION ---
unsigned __int32 FastHash(const WCHAR *str)
{
	int len = wcslen(str);
	unsigned __int32 hash = 5381;
	int c;
	int flen = len;

	while (flen > 0)
	{
		flen--;
		c = tolower(*str++);
		hash = ((hash << 5) + hash) + c; // hash * 33 + c 
	}
	return hash;
}

unsigned __int32 FastHash(const WCHAR *str, int len)
{
	unsigned __int32 hash = 5381;
	int c;
	int flen = len;

	while (flen > 0)
	{
		flen--;
		c = tolower(*str++);
		hash = ((hash << 5) + hash) + c; // hash * 33 + c 
	}
	return hash;
}


unsigned __int32 FastHash(const char *str)
{
	int len = strlen(str);
	unsigned __int32 hash = 5381;
	int c;
	int flen = len;

	while (flen > 0)
	{
		flen--;
		c = tolower(*str++);
		hash = ((hash << 5) + hash) + c; // hash * 33 + c 
	}
	return hash;
}

unsigned __int32 FastHash(const char *str, int len)
{
	unsigned __int32 hash = 5381;
	int c;
	int flen = len;

	while (flen > 0)
	{
		flen--;
		c = tolower(*str++);
		hash = ((hash << 5) + hash) + c; // hash * 33 + c 
	}
	return hash;
}

//case sensitive functions (a bit faster)
unsigned __int32 FastHashCS(const WCHAR *str)
{
	int len = wcslen(str);
	unsigned __int32 hash = 5381;
	int c;
	int flen = len;

	while (flen > 0)
	{
		flen--;
		c = *str++;
		hash = ((hash << 5) + hash) + c; // hash * 33 + c 
	}
	return hash;
}
unsigned __int32 FastHashCS(const WCHAR *str, int len)
{
	unsigned __int32 hash = 5381;
	int c;
	int flen = len;

	while (flen > 0)
	{
		flen--;
		c = *str++;
		hash = ((hash << 5) + hash) + c; // hash * 33 + c 
	}
	return hash;
}

unsigned __int32 FastHashCS(const char *str)
{
	int len = strlen(str);
	unsigned __int32 hash = 5381;
	int c;
	int flen = len;

	while (flen > 0)
	{
		flen--;
		c = *str++;
		hash = ((hash << 5) + hash) + c; // hash * 33 + c 
	}
	return hash;
}

unsigned __int32 FastHashCS(const char *str, int len)
{
	unsigned __int32 hash = 5381;
	int c;
	int flen = len;

	while (flen > 0)
	{
		flen--;
		c = *str++;
		hash = ((hash << 5) + hash) + c; // hash * 33 + c 
	}
	return hash;
}


//calculeaza hash pt un fisier
unsigned __int32 GetFileHash(WCHAR *filename)
{
	long filebufSize = OS_GetFileSize(filename);
	if ((filebufSize <= 0) || (filebufSize > 5000000))
	{
		ErrorBox(K_ERR_WARNING, L"GetFileHash::file size 0 or over 5MB!\n%s", filename);
		return 0;
	}

	errno_t err;
	FILE *fl = NULL;
	if ((err = OS_wfopen_s(&fl, filename, L"rb")) != 0)
	{
		ErrorBox(K_ERR_WARNING, L"GetFileHash::File does not exist!\n%s", filename);
		return 0;
	}

	char* filebuf = new char[filebufSize];

	OS_fread(filebuf, sizeof(char), filebufSize, fl);

	OS_fclose(fl);
	//make hash
	unsigned __int32 filehash = FastHash(filebuf, filebufSize);

	delete[] filebuf;

	return filehash;
}
//cauta un fisier cu acelasi nume (filename.ext.hash), citeste uint32 din el si verifica sa corespunda cu cel al fisierului
HRESULT CheckFileSignatureHash(WCHAR *filename)
{
	WCHAR signaturePath[MAX_PATH];
	StringCchPrintf(signaturePath, MAX_PATH, L"%s.hash", filename);
	unsigned __int32 fileHash = GetFileHash(filename);
	if (fileHash == 0)
	{
		return E_FAIL;
	}

	//deschide semnatura si citeste hash de acolo
	unsigned __int32 signatureHash = 0;
	errno_t err;
	FILE *fl = NULL;
	if ((err = OS_wfopen_s(&fl, signaturePath, L"rb")) != 0)
	{
		return E_FAIL;
	}

	OS_fread(&signatureHash, sizeof(unsigned __int32), 1, fl);
	OS_fclose(fl);

	if (signatureHash != fileHash)
	{
		return E_ACCESSDENIED;
	}

	return S_OK;
}

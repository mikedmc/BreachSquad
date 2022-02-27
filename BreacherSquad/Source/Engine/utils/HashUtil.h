#pragma once
unsigned __int32 FastHash(const char *data, int len);	//case insensitive Fast Hash
unsigned __int32 FastHash(const WCHAR *data, int len);	//case insensitive Fast Hash
unsigned __int32 FastHash(const char *data);	//case insensitive Fast Hash
unsigned __int32 FastHash(const WCHAR *data);	//case insensitive Fast Hash
//case sensitive versions
unsigned __int32 FastHashCS(const char *str, int len); //case sensitive Fast Hash
unsigned __int32 FastHashCS(const WCHAR *str, int len); //case sensitive Fast Hash
unsigned __int32 FastHashCS(const char *str); //case sensitive Fast Hash
unsigned __int32 FastHashCS(const WCHAR *str); //case sensitive Fast Hash
#define HASH(x) FastHash(x, strlen(x))
#define HASHW(x) FastHash(x, wcslen(x))
// computes hash for a file
unsigned __int32 GetFileHash(WCHAR *filename);
// Looks for a folder with the same name but hash extension (filename.ext.hash), reads uint32 from it and checks it with the file's realtime computed hash
HRESULT CheckFileSignatureHash(WCHAR *filename);

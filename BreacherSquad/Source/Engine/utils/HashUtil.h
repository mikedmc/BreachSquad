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
#define GET_FAST_HASH(x) FastHash(x, strlen(x))
#define GET_FAST_WHASH(x) FastHash(x, wcslen(x))
//calculeaza hash pt un fisier
unsigned __int32 GetFileHash(WCHAR *filename);
//cauta un fisier cu acelasi nume (filename.ext.hash), citeste uint32 din el si verifica sa corespunda cu cel al fisierului
HRESULT CheckFileSignatureHash(WCHAR *filename);

#include <stdio.h>
#include "dxstdafx.h"

CStringDesc::CStringDesc()
{
	codes = NULL;
	sText = NULL;
	len = 0;
	shStringName.Reset();
}

CStringDesc::~CStringDesc()
{
	SAFE_DELETE_ARRAY(codes);
	SAFE_DELETE_ARRAY(sText);
	len = 0;
	shStringName.Reset();
}

void CStringDesc::Reset()
{
	SAFE_DELETE_ARRAY(codes);
	SAFE_DELETE_ARRAY(sText);
	len = 0;
	shStringName.Reset();
}

CStringDesc::CStringDesc(const CStringDesc& src)
{
	SAFE_DELETE_ARRAY(codes);
	SAFE_DELETE_ARRAY(sText);
	len = 0;
	shStringName.Reset();

	len = src.len;
	shStringName = src.shStringName;
	codes = new UINT16[len];
	sText = new WCHAR[len];
	memcpy(codes, src.codes, len * sizeof(UINT16));
	memcpy(sText, src.sText, len * sizeof(WCHAR));
}
//-----------------------------

CStringsManager::CStringsManager()
{
	defaultStringIdx = 0;

	alphabet = NULL;
	loaded = false;
}

CStringsManager::~CStringsManager()
{
	Release();
}

void CStringsManager::SetStringDesc(CStringDesc *desc, WCHAR* szFormat, ...)
{
	SAFE_DELETE_ARRAY(desc->codes);
	SAFE_DELETE_ARRAY(desc->sText);
	desc->codes = NULL;
	desc->sText = NULL;
	desc->len = 0;
	desc->shStringName.Reset();

	//formeaza stringul
	WCHAR szBuffer[K_STRMGR_CONTENT_MAX_LEN];
	va_list marker;
	va_start(marker, szFormat);
	StringCchVPrintf(szBuffer, ARRAY_SIZE(szBuffer), szFormat, marker);
	va_end(marker);

	StringCchLength(szBuffer, K_STRMGR_CONTENT_MAX_LEN, &desc->len);
	if(desc->len == 0)
		return;
	desc->sText = new WCHAR[desc->len + 1];
	StringCchCopy(desc->sText, desc->len + 1, szBuffer);
	BuildStringCodes(desc);
}

//File format:
//-------------------------------------------
//<?xml version="1.0" encoding="utf-8"?>
//<PSTexts Version="1.0">
//  <Alphabet>ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789~`!@#$%^&amp;*()-_=+[{]};:'",&lt;.&gt;/?°¢£•ß©Æø¿¡¬√ƒ≈∆«»… ÀÃÕŒœ–—“”‘’÷ÿŸ⁄€‹›üﬂ‡·‚„‰ÂÊÁËÈÍÎÏÌÓÔÒÚÛÙıˆ¯˘˙˚¸˝ˇåú</Alphabet>
//  <Texts Lang="EN" Count="346">
//    <Text ID="TITLE">text De Proba</Text>
//    <Text ID="TITLE2">Text important</Text>
//  </Texts>
//</PSTexts>
//-------------------------------------------
HRESULT CStringsManager::LoadFromXML(WCHAR* fileName, bool bIgnoreMissingChars)
{
	//release strings if already loaded
	Release();

    pugi::xml_document doc;
	if (!doc.load_file(fileName))
	{
		ErrorBox(K_ERR_WARNING, L"Unable to load XML:%s\n", fileName);
		return(E_FAIL);
	}

	pugi::xml_attribute ver = doc.root().child(L"PSTexts").attribute(L"Version");
	if(ver.as_float() != 1.0f)
	{
		ErrorBox(K_ERR_WARNING, L"Strings XML wrong version:%s\n", fileName);
	}

	pugi::xml_node septextsnode = doc.root().child(L"PSTexts");
	//save alphabet
	pugi::xml_node alphabetnode = septextsnode.child(L"Alphabet");
	const WCHAR *retalphabet = alphabetnode.child_value();
	
	if(FAILED(StringCchLength(retalphabet, K_STRMGR_CONTENT_MAX_LEN, &alphabetLen)))
	{
		ErrorBox(K_ERR_WARNING, L"Failed getting Alphabet length.\n", fileName);
		return E_FAIL;
	}
	alphabet = new WCHAR[alphabetLen + 1];
	StringCchCopy(alphabet, alphabetLen + 1, retalphabet);

	//ia parintele listei de strings
	pugi::xml_node textsnode = septextsnode.child(L"Texts");
	const WCHAR *texlang = textsnode.attribute(L"Lang").value();
	UINT langLen;
	if(!FAILED(StringCchLength(texlang, K_STRMGR_CONTENT_MAX_LEN, &langLen)))
	{
		StringCchCopy(language, langLen + 1, texlang);
	}

	int count = 0;
    for (pugi::xml_node stringnode = textsnode.first_child(); stringnode; stringnode = stringnode.next_sibling())
    {
		CStringDesc *nstr = new CStringDesc();

		//Citesc ID-ul stringului si il convertesc in char pt FastHash
		UINT stringLen;
		StringCchLength(stringnode.attribute(L"ID").value(), K_STRMGR_CONTENT_MAX_LEN, &stringLen);
		if(stringLen > 0)
		{
			WCHAR strID[MAX_PATH];
			StringCchCopy(strID, MAX_PATH, stringnode.attribute(L"ID").value());
			//calculez hash-ul numelui ca sa il caut repede
			nstr->shStringName.Init(strID);
		}
		else
		{
			ErrorBox(K_ERR_WARNING, L"Strings XML - 0 length string name! %s\n", stringnode.attribute(L"ID").value());
		}
		//citesc stringul in sine
		const WCHAR *content = stringnode.child_value();
		stringLen = 0;
		if(!FAILED(StringCchLength(content, K_STRMGR_CONTENT_MAX_LEN, &stringLen)))
		{
			if(stringLen > 0)
			{
				if (stringLen > K_STRMGR_CONTENT_MAX_LEN)
				{
					ErrorBox(K_ERR_WARNING, L"String XML - Very long string: %s", stringnode.attribute(L"ID").value());
				}

				nstr->sText = new WCHAR[stringLen + 1];
				StringCchCopy(nstr->sText, stringLen + 1, content);
			}
			else
			{
				nstr->sText = new WCHAR[8];
				StringCchCopy(nstr->sText, 8, L"[EMPTY]");
				stringLen = 7;

				ErrorBox(K_ERR_WARNING, L"String XML - Empty string: %s\nReplaced with [EMPTY]", stringnode.attribute(L"ID").value());
			}
		}
		nstr->len = stringLen; //IMPORTANT

		//aici calculeaza si codurile
		WCHAR notFoundChars[MAX_PATH];
		int retbuild = BuildStringCodes(nstr, notFoundChars);

		if ((retbuild != 0) && (!bIgnoreMissingChars))
			ErrorBox(K_ERR_WARNING, L"(%s) chars not found in string %s\n", notFoundChars, stringnode.attribute(L"ID").value());

		strings.Add(nstr);
		//count intern al functiei
		count++;
    }

	///--- adauga string default pt atunci cand nu gaseste strID cautat ---
	CStringDesc* fstr = new CStringDesc();
	StringCchLength(K_STRMGR_DEFAULT_STRING, K_STRMGR_CONTENT_MAX_LEN, &fstr->len);
	fstr->sText = new WCHAR[fstr->len + 1];
	StringCchCopy(fstr->sText, fstr->len + 1, K_STRMGR_DEFAULT_STRING);
	BuildStringCodes(fstr);
	strings.Add(fstr);
	//salveaza idx string default
	defaultStringIdx = strings.GetSize() - 1;

	loaded = true;

	return S_OK;
}

const int CStringsManager::getLetterIdx(const WCHAR c)
{
	if(c == '\n')
		return K_STRMGR_RETURN;
	if(c == ' ')
		return K_STRMGR_SPACE;
	WCHAR *alphapos = alphabet;
	register int code = 0;
	while(*alphapos != 0)
	{
		if(*alphapos == c)
		{
			return code;
		}
		alphapos++;
		code++;
	}
	//daca nu o gaseste
	return -1;
}

int CStringsManager::BuildStringCodes(CStringDesc *desc, WCHAR *notFoundChars)
{
	int notChars = 0;
	WCHAR *chpos = desc->sText;

	SAFE_DELETE_ARRAY(desc->codes);

	assert((desc->len > 0) && (desc->len < K_STRMGR_CONTENT_MAX_LEN)); 
	desc->codes = new UINT16[desc->len];

	WCHAR *alphapos;
	UINT16 *codespos = desc->codes;

	while(*chpos != 0)
	{
		if(*chpos == '\n')   //vede daca e enter
		{
			*codespos = K_STRMGR_RETURN;
		}
		else
		if((*chpos == ' ') || (*chpos == '\t') || (*chpos == 160))   //space, tab or non-breaking space
		{
			*codespos = K_STRMGR_SPACE;
		}
		else //cauta in alfabet
		{
			alphapos = alphabet;
			register int code = 0;
			while(*alphapos != 0)
			{
				if(*alphapos == *chpos)
				{
					break;
				}
				alphapos++;
				code++;
			}

			if(*alphapos == 0) //daca nu a gasit caracterul
			{
				if(notFoundChars != NULL)
				{
					notFoundChars[notChars] = *chpos;
					notFoundChars[notChars + 1] = 0;
				}

				notChars++;
				*codespos = K_STRMGR_SPACE;
			}
			else
			{
				*codespos = code;
			}
		}
		codespos++;
		chpos++;
	}

	return notChars;
}

void CStringsManager::Release()
{
	if(!loaded)
		return;

	SAFE_DELETE_ARRAY(alphabet);
	for (int kk = 0; kk < strings.GetSize(); kk++)
	{
		SAFE_DELETE(strings[kk]);
	}
	strings.RemoveAll();

	loaded = false;
}

int CStringsManager::getStrIdx(const CHAR* strID)
{
	UINT32 strHash = FastHash(strID, strlen(strID));

	for(int kk=0; kk<strings.GetSize(); kk++)
	{
		if(strings[kk]->shStringName.textHash == strHash)
			return kk;
	}
	//daca nu gaseste ID-ul cautat intoarce idx string default "StrNotFound"
	ErrorBox(K_ERR_WARNING, L"getStrIdx->String not found! %s", strID);
	return defaultStringIdx;
}

int CStringsManager::getStrIdx(const WCHAR* strID)
{
	UINT32 strHash = FastHash(strID, wcslen(strID));

	for(int kk=0; kk<strings.GetSize(); kk++)
	{
		if(strings[kk]->shStringName.textHash == strHash)
			return kk;
	}
	//daca nu gaseste ID-ul cautat intoarce idx string default "StrNotFound"
	//ErrorBox(K_ERR_WARNING, L"getStrIdx->String not found! %s", strID);
	return defaultStringIdx;
}

int CStringsManager::getStrIdx(UINT32 strHash)
{
	for (int kk = 0; kk < strings.GetSize(); kk++)
	{
		if (strings[kk]->shStringName.textHash == strHash)
			return kk;
	}
	//daca nu gaseste ID-ul cautat intoarce idx string default "StrNotFound"
	//ErrorBox(K_ERR_WARNING, L"getStrIdx->String hash not found! hash:%d", strHash);
	return defaultStringIdx;
}

int CStringsManager::SetString(int idx, WCHAR* szFormat, ...)
{
	if(idx >= strings.GetSize())
		return -1;
	int retBuild = -1;

	CStringDesc *nstr = strings[idx];
	
	//formeaza stringul
	WCHAR szBuffer[K_STRMGR_CONTENT_MAX_LEN];
	va_list marker;
	va_start(marker, szFormat);
	StringCchVPrintf(szBuffer, ARRAY_SIZE(szBuffer), szFormat, marker);
	va_end(marker);
	//il scrie

	UINT stringLen;
	if((!FAILED(StringCchLength(szBuffer, K_STRMGR_CONTENT_MAX_LEN, &stringLen))) && (stringLen > 0))
	{
		SAFE_DELETE_ARRAY(nstr->sText);
		nstr->sText = new WCHAR[stringLen + 1];
		StringCchCopy(nstr->sText, stringLen + 1, szBuffer);

		nstr->len = stringLen;
		retBuild = BuildStringCodes(nstr);
	}

	return retBuild;
}

int CStringsManager::SetString(CHAR* id, WCHAR* szFormat, ...)
{
	int idx = getStrIdx(id);
	if((idx < 0) || (idx >= strings.GetSize()) )
	{
		ErrorBox(K_ERR_WARNING, L"CStringsManager::setString(WCHAR_id)->String not found! [%s]\n", id);
		return -1;
	}
	int retBuild = -1;

	CStringDesc *nstr = strings[idx];
	//formeaza stringul
	WCHAR szBuffer[K_STRMGR_CONTENT_MAX_LEN];
	va_list marker;
	va_start(marker, szFormat);
	StringCchVPrintf(szBuffer, ARRAY_SIZE(szBuffer), szFormat, marker);
	va_end(marker);
	//il scrie
	UINT stringLen;
	if((!FAILED(StringCchLength(szBuffer, K_STRMGR_CONTENT_MAX_LEN, &stringLen))) && (stringLen > 0))
	{
		SAFE_DELETE_ARRAY(nstr->sText);
		nstr->sText = new WCHAR[stringLen + 1];
		StringCchCopy(nstr->sText, stringLen + 1, szBuffer);

		nstr->len = stringLen;
		retBuild = BuildStringCodes(nstr);
	}

	return retBuild;
}

int CStringsManager::SetString_NoParse(int idx, WCHAR* szString)
{
	if (idx >= strings.GetSize())
		return -1;
	int retBuild = -1;

	CStringDesc *nstr = strings[idx];

	UINT stringLen;
	if ((!FAILED(StringCchLength(szString, K_STRMGR_CONTENT_MAX_LEN, &stringLen))) && (stringLen > 0))
	{
		SAFE_DELETE_ARRAY(nstr->sText);
		nstr->sText = new WCHAR[stringLen + 1];
		StringCchCopy(nstr->sText, stringLen + 1, szString);

		nstr->len = stringLen;
		retBuild = BuildStringCodes(nstr);
	}

	return retBuild;
}

HRESULT CStringsManager::ReplaceTokenString(int destStrIdx, int srcStrIdx, int tokenNumber, WCHAR* tokenText)
{
	if((destStrIdx >= strings.GetSize()) || (srcStrIdx >= strings.GetSize()))
		return S_FALSE;

	WCHAR tokenStr[MAX_PATH];
	StringCchPrintf(tokenStr, MAX_PATH, L"{%%%d}", tokenNumber);
	WCHAR original[4096];
	StringCchCopy(original, 4096, strings[srcStrIdx]->sText);

	wcs_replace(original, tokenStr, tokenText);

	SetString(destStrIdx, L"%s", original);	

	return S_OK;	
}

HRESULT CStringsManager::ReplaceTokenInt(int destStrIdx, int srcStrIdx, int tokenNumber, int tokenVal)
{
	if((destStrIdx >= strings.GetSize()) || (srcStrIdx >= strings.GetSize()))
		return S_FALSE;

	WCHAR tokenStr[MAX_PATH];
	StringCchPrintf(tokenStr, MAX_PATH, L"{%%%d}", tokenNumber);
	WCHAR tokenValStr[MAX_PATH];
	StringCchPrintf(tokenValStr, MAX_PATH, L"%d", tokenVal);

	WCHAR original[4096];
	StringCchCopy(original, 4096, strings[srcStrIdx]->sText);

	wcs_replace(original, tokenStr, tokenValStr);

	SetString(destStrIdx, L"%s", original);	

	return S_OK;	
}

WCHAR* CStringsManager::GetStringText(int strIdx)
{
	if (strIdx >= strings.GetSize())
		return NULL;
	return strings[strIdx]->sText;
}

HRESULT CStringsManager::ReplaceTokenString(CStringDesc* destStrDesc, int srcStrIdx, int tokenNumber, WCHAR* tokenText)
{
	if ((destStrDesc == NULL) || (srcStrIdx >= strings.GetSize()))
		return S_FALSE;

	WCHAR tokenStr[MAX_PATH];
	StringCchPrintf(tokenStr, MAX_PATH, L"{%%%d}", tokenNumber);
	WCHAR original[4096];
	StringCchCopy(original, 4096, strings[srcStrIdx]->sText);

	wcs_replace(original, tokenStr, tokenText);

	SetStringDesc(destStrDesc, L"%s", original);

	return S_OK;
}

HRESULT CStringsManager::ReplaceTokenInt(CStringDesc* destStrDesc, int srcStrIdx, int tokenNumber, int tokenVal)
{
	if ((destStrDesc == NULL) || (srcStrIdx >= strings.GetSize()))
		return S_FALSE;

	WCHAR tokenStr[MAX_PATH];
	StringCchPrintf(tokenStr, MAX_PATH, L"{%%%d}", tokenNumber);
	WCHAR tokenValStr[MAX_PATH];
	StringCchPrintf(tokenValStr, MAX_PATH, L"%d", tokenVal);

	WCHAR original[4096];
	StringCchCopy(original, 4096, strings[srcStrIdx]->sText);

	wcs_replace(original, tokenStr, tokenValStr);

	SetStringDesc(destStrDesc, L"%s", original);

	return S_OK;
}

HRESULT CStringsManager::GetSubstring(CStringDesc* destStrDesc, int srcStrIdx, int nSubstringIdx, WCHAR wcSeparator)
{
	if ((destStrDesc == null) || (srcStrIdx < 0) || (srcStrIdx >= strings.GetSize()))
		return S_FALSE;

	WCHAR wcSubstr[1024] = { 0 };
	int startIdx = 0;
	int endIdx = 0;
	int currentSubstrIdx = 0;
	bool bFound = false;
		
	while (!bFound)
	{
		//walk until separator or end of string
		while ((strings[srcStrIdx]->sText[endIdx] != wcSeparator) && (endIdx < strings[srcStrIdx]->len))
			endIdx++;
		//found substring
		if (currentSubstrIdx == nSubstringIdx)
		{
			bFound = true;
			if (endIdx - startIdx > 0)
			{
				memcpy(wcSubstr, &strings[srcStrIdx]->sText[startIdx], sizeof(WCHAR) * (endIdx - startIdx));
				SetStringDesc(destStrDesc, wcSubstr);
				return S_OK;
			}
		}
		else
		{
			startIdx = endIdx + 1;
			endIdx = startIdx;
			currentSubstrIdx++;

			if (startIdx >= strings[srcStrIdx]->len)
				return S_FALSE;
		}
	}
	return S_FALSE;
}

int CStringsManager::GetSubstringsCount(int srcStrIdx, WCHAR wcSeparator)
{
	if ((srcStrIdx < 0) || (srcStrIdx >= strings.GetSize()))
		return 0;

	int nSeparators = 0;
	for (int kk = 0; kk < strings[srcStrIdx]->len; kk++)
	{
		if (strings[srcStrIdx]->sText[kk] == wcSeparator)
			nSeparators++;
	}
	return nSeparators + 1;
}

const unsigned int CStringsManager::GetCodePointFromUTF8(const char* utf8_4byteChunk, unsigned char& seqLen)
{
	unsigned char code_unit1 = (unsigned char)utf8_4byteChunk[0];

	if (code_unit1 < 0x80)
	{
		seqLen = 1;
		return code_unit1;
	}
	else
		if (code_unit1 < 0xC2)
		{
			/* continuation or overlong 2-byte sequence */
			goto ERROR1;
		}
		else
			if (code_unit1 < 0xE0)
			{
				/* 2-byte sequence */
				seqLen = 2;

				unsigned char code_unit2 = (unsigned char)utf8_4byteChunk[1];
				if ((code_unit2 & 0xC0) != 0x80)
					goto ERROR2;

				return (code_unit1 << 6) + code_unit2 - 0x3080;
			}
			else
				if (code_unit1 < 0xF0)
				{
					/* 3-byte sequence */
					seqLen = 3;

					unsigned char code_unit2 = (unsigned char)utf8_4byteChunk[1];
					if ((code_unit2 & 0xC0) != 0x80)
						goto ERROR2;
					if (code_unit1 == 0xE0 && code_unit2 < 0xA0)
						goto ERROR2; /* overlong */

					unsigned char code_unit3 = (unsigned char)utf8_4byteChunk[2];
					if ((code_unit3 & 0xC0) != 0x80)
						goto ERROR3;

					return (code_unit1 << 12) + (code_unit2 << 6) + code_unit3 - 0xE2080;
				}
				else
					if (code_unit1 < 0xF5)
					{
						/* 4-byte sequence */
						seqLen = 4;

						unsigned char code_unit2 = (unsigned char)utf8_4byteChunk[1];
						if ((code_unit2 & 0xC0) != 0x80)
							goto ERROR2;
						if (code_unit1 == 0xF0 && code_unit2 < 0x90)
							goto ERROR2; /* overlong */
						if (code_unit1 == 0xF4 && code_unit2 >= 0x90)
							goto ERROR2; /* > U+10FFFF */

						unsigned char code_unit3 = (unsigned char)utf8_4byteChunk[2];
						if ((code_unit3 & 0xC0) != 0x80)
							goto ERROR3;

						unsigned char code_unit4 = (unsigned char)utf8_4byteChunk[3];
						if ((code_unit4 & 0xC0) != 0x80)
							goto ERROR4;

						return (code_unit1 << 18) + (code_unit2 << 12) + (code_unit3 << 6) + code_unit4 - 0x3C82080;
					}
					else
					{
						/* > U+10FFFF */
						goto ERROR1;
					}

ERROR4:
	//ungetc(code_unit4, stdin);
ERROR3:
	//ungetc(code_unit3, stdin);
ERROR2:
	//ungetc(code_unit2, stdin);
ERROR1:
	//return code_unit1 + 0xDC00;

	//ErrorBox(K_ERR_CRITICAL, L"[Error] IFont::GetCodePointFromUTF8(): malformed UTF8\n");
	seqLen = -1;
	return (unsigned int)-1;
}

int CStringsManager::SetStringDescUTF8(CStringDesc *desc, const char* szUTF8string)
{														  
	SAFE_DELETE_ARRAY(desc->codes);
	SAFE_DELETE_ARRAY(desc->sText);
	desc->codes = NULL;
	desc->sText = NULL;
	desc->len = 0;
	desc->shStringName.Reset();

	int len = strlen(szUTF8string);
	WCHAR szBuffer[K_STRMGR_CONTENT_MAX_LEN];

	int actualCharCount = 0; // szText may be unicode, so multiple bytes from szText could correspond to a single unicode char
	for (int i = 0; (i < len) && (actualCharCount < K_STRMGR_CONTENT_MAX_LEN); ++actualCharCount)
	{
		unsigned char seqLen = (unsigned char)-1;
		unsigned int uniCharCodepoint = GetCodePointFromUTF8(&szUTF8string[i], seqLen);
		
		//replace bad unicode sequence:
		if (seqLen == (unsigned char)-1)
		{
			szBuffer[actualCharCount] = '?';
			actualCharCount++;
			break;
		}

		szBuffer[actualCharCount] = (DWORD)uniCharCodepoint;

		i += seqLen;
	}

	if (actualCharCount <= 0)
		return 0;
	if (actualCharCount > 1024)
		actualCharCount = 1024;

	//translate it
	desc->len = actualCharCount;
	desc->sText = new WCHAR[desc->len + 1];
	StringCchCopy(desc->sText, desc->len + 1, szBuffer);
	BuildStringCodes(desc);

	return actualCharCount;
}

int CStringsManager::UTF8toWCHAR(const char* szUTF8string, WCHAR* strDest, int nMaxLenDest)
{
	int len = strlen(szUTF8string);
	WCHAR szBuffer[K_STRMGR_CONTENT_MAX_LEN];

	int actualCharCount = 0; // szText may be unicode, so multiple bytes from szText could correspond to a single unicode char
	for (int i = 0; (i < len) && (actualCharCount < K_STRMGR_CONTENT_MAX_LEN); ++actualCharCount)
	{
		unsigned char seqLen = (unsigned char)-1;
		unsigned int uniCharCodepoint = GetCodePointFromUTF8(&szUTF8string[i], seqLen);

		if (seqLen == (unsigned char)-1)
		{
			szBuffer[actualCharCount] = '?';
			actualCharCount++;
			break; // malformated UTF8
		}

		szBuffer[actualCharCount] = (DWORD)uniCharCodepoint;

		i += seqLen;
	}

	if (actualCharCount <= 0)
		return 0;
	if (actualCharCount > nMaxLenDest - 1)
		actualCharCount = nMaxLenDest - 1;

	memset(strDest, 0, sizeof(WCHAR) * nMaxLenDest);
	memcpy(strDest, szBuffer, sizeof(WCHAR) * min(actualCharCount, nMaxLenDest));

	return actualCharCount;
}

#pragma once

//TODO: Sa stie sa schimbe culoarea fontului, daca e necesar
//TODO: Daca se misca incet getIdx sa nu mai folosesc CArray ci sa aloc static

#define K_STRMGR_DEFAULT_STRING	L"![StringNotFound]!"
//marimea maxima a continutului unui string
#define K_STRMGR_CONTENT_MAX_LEN 2048
//spatiu si enter
#define K_STRMGR_RETURN 0xfffe
#define K_STRMGR_SPACE 0xffff

class CStringDesc {
public:
	CStringHash shStringName;	//numele stringului
	UINT16*		codes;	//coduri catre literele din alfabet
	WCHAR*		sText;	//textul in sine
	UINT		len;	//lungimea stringului

	CStringDesc(const CStringDesc& src);
	CStringDesc();  
	~CStringDesc();

	void Reset();
};

class CStringsManager{
public:
	bool						loaded;						// Are strings loaded?
	int							defaultStringIdx;			// Default string index when string wasn't found

	WCHAR*						alphabet;					// complete alphabet
	std::wstring				strLangAlias;				// saves loaded language code/alias/name

	CArray<CStringDesc*> strings;					// Actual loaded strings

	CStringsManager();
	~CStringsManager();

	// Loads strings from xml and returns number of total strings or -1 in case of error
	int							LoadFromXML(WCHAR* fileName, WCHAR* strLangCode, std::wstring* strMinimumAlphabet = nullptr, bool bIgnoreMissingChars = false);
	void						Release();

	// Finds index of one letter in alphabet
	const int					GetLetterIdx(const WCHAR c);

	// Builds string codes (idx to letters) based on actual string
	// \returns: -1 for error or number of characters not found in alphabet
	int							BuildStringCodes(CStringDesc *desc, WCHAR *notFoundChars = NULL);

	// Sets string
	void						SetStringDesc(CStringDesc *desc, WCHAR* szFormat, ...);

	// Returns index of string or defaultString if not found
	int							GetStrIdx(const CHAR* strID);
	int							GetStrIdx(const WCHAR* strID);
	int							GetStrIdx(UINT32 strHash);

	// Returns string hash by index
	CStringDesc*				GetStringDescByIdx( int nIdx );

	// Sets a string based on a printf format
	// \returns: -1 for error or number of chars not found in alphabet
	int							SetString(int idx, WCHAR* szFormat, ...);
	int							SetString(CHAR* id, WCHAR* szFormat, ...);

	// Sets a string without parsing special characters like %d
	int							SetString_NoParse(int idx, WCHAR* szString);

	// Replaces tokens like {%1} si {%2} with params
	bool						ReplaceTokenString(int destStrIdx, int srcStrIdx, int tokenNumber, WCHAR* tokenText);
	bool						ReplaceTokenInt(int destStrIdx, int srcStrIdx, int tokenNumber, int tokenVal);
	bool						ReplaceTokenString(CStringDesc* destStrDesc, int srcStrIdx, int tokenNumber, WCHAR* tokenText);
	bool						ReplaceTokenInt(CStringDesc* destStrDesc, int srcStrIdx, int tokenNumber, int tokenVal);
	
	
	// \brief Gets the WCHAR text of a specified string
	WCHAR*						GetStringText(int strIdx);
	/*
	 * \brief: Fills a CStringDesc structure with the N-th substring from a string. 
	 * \param: wcSeparator - the separator that separates the substrings
	 * CStringDesc will contain a 0 length string if substring not found.
	 */
	bool						GetSubstring(CStringDesc* destStrDesc, int srcStrIdx, int nSubstringIdx, WCHAR wcSeparator);
	
	// Returns the number of substrings separated by wcSeparator
	int							GetSubstringsCount(int srcStrIdx, WCHAR wcSeparator);

	///--- UTF8 encoding ---

	// Utility that converts UTF8 char to WCHAR
	static const unsigned int	GetCodePointFromUTF8(const char* utf8_4byteChunk, unsigned char& seqLen);

	int							SetStringDescUTF8(CStringDesc *desc, const char* szUTF8string);
	// Generic conversion from UTF8 to WCHAR
	static int					UTF8toWCHAR(const char* szUTF8string, WCHAR* strDest, int nMaxLenDest);
};

// Access singleton
CStringsManager& UTLang();


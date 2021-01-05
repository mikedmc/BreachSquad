#pragma once

//TODO: Sa stie sa schimbe culoarea fontului, daca e necesar
//TODO: Daca se misca incet getIdx sa nu mai folosesc CGrowableArray ci sa aloc static

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
	bool loaded;		//daca e incarcat sau nu
	int defaultStringIdx; //index defaul string

	WCHAR* alphabet;	//alfabetul incarcat din XML
	UINT alphabetLen;	//numarul de litere din alfabet
	WCHAR language[10]; //limba in care sunt textele, din 2 litere

	CGrowableArray <CStringDesc*> strings;

	CStringsManager();
	~CStringsManager();
	HRESULT LoadFromXML(WCHAR* fileName, bool bIgnoreMissingChars = false);
	void Release();

	//gaseste indexul unei litere in alfabet
	const int getLetterIdx(const WCHAR c);
	//Construieste codes in functie de string
	//returns: -1 pt eroare sau numarul de caractere negasite in alfabet (scrie in notFoundChars ce nu a gasit)
	int BuildStringCodes(CStringDesc *desc, WCHAR *notFoundChars = NULL);
	void SetStringDesc(CStringDesc *desc, WCHAR* szFormat, ...); //seteaza stringul in string desc
	//intoarce indexul stringului in lista de stringuri, in functie de hash-ul ID-ului text sau -1 daca nu exista
	//va fi folosita pentru resursele externe (interfete) care iau stringurile dupa nume
	int getStrIdx(const CHAR* strID);
	int getStrIdx(const WCHAR* strID);
	int getStrIdx(UINT32 strHash);
	//Seteaza un string. Primeste ca parametri ID si format string ca la printf
	//returns: -1 pt eroare sau numarul de caractere negasite in alfabet
	int SetString(int idx, WCHAR* szFormat, ...);
	int SetString(CHAR* id, WCHAR* szFormat, ...);
	//sets a string without parsing special characters like %d
	int SetString_NoParse(int idx, WCHAR* szString);
	//inlocuieste parametrii de forma {%1} si {%2} cu parametrii pasati in functie
	//RETURNS: S_FALSE pt eroare sau S_OK
	HRESULT ReplaceTokenString(int destStrIdx, int srcStrIdx, int tokenNumber, WCHAR* tokenText);
	HRESULT ReplaceTokenInt(int destStrIdx, int srcStrIdx, int tokenNumber, int tokenVal);
	HRESULT ReplaceTokenString(CStringDesc* destStrDesc, int srcStrIdx, int tokenNumber, WCHAR* tokenText);
	HRESULT ReplaceTokenInt(CStringDesc* destStrDesc, int srcStrIdx, int tokenNumber, int tokenVal);
	
	/*
	 * \brief Gets the WCHAR text of a specified string
	 */
	WCHAR* GetStringText(int strIdx);
	/*
	 * \brief: Fills a CStringDesc structure with the N-th substring from a string. 
	 * \param: wcSeparator - the separator that separates the substrings
	 * CStringDesc will contain a 0 length string if substring not found.
	 */
	HRESULT GetSubstring(CStringDesc* destStrDesc, int srcStrIdx, int nSubstringIdx, WCHAR wcSeparator);
	/*
	 * Returns the number of substrings separated by wcSeparator
	 */
	int GetSubstringsCount(int srcStrIdx, WCHAR wcSeparator);

	///--- UTF8 encoding ---
	/*
	 * Utility that converts UTF8 char to WCHAR
	 */
	static const unsigned int GetCodePointFromUTF8(const char* utf8_4byteChunk, unsigned char& seqLen);
	int SetStringDescUTF8(CStringDesc *desc, const char* szUTF8string);
	/*
	 * Generic conversion from UTF8 to WCHAR
	 */
	static int UTF8toWCHAR(const char* szUTF8string, WCHAR* strDest, int nMaxLenDest);
};

// Access singleton
CStringsManager& UTLang();


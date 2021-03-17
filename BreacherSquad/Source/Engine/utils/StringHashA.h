#pragma once

#define K_MAX_STRINGHASH_CHAR_LEN 256
// Single byte CHAR String and Hash pairn
class CStringHashA //string-hash pair
{
public:
	CHAR			text[K_MAX_STRINGHASH_CHAR_LEN];
	UINT32			textHash;  //hash-ul numelui

	FORCEINLINE const UINT32 getHash() const { return textHash; }

	CStringHashA() { text[0] = 0; textHash = 0; }
	CStringHashA(WCHAR const * const strText);
	CStringHashA(char const * const strText);
	CStringHashA(const CStringHash &o);

	const bool IsEmpty() const {
		return (textHash == 0);
	}

	const bool IsSet() const {
		return (textHash != 0);
	}

	const bool IsEqual(WCHAR* intext) const {
		return (textHash == FastHash(intext, wcslen(intext)));
	}

	const bool IsEqual(CHAR* intext) const {
		return (textHash == FastHash(intext, strlen(intext)));
	}

	void		Init(CHAR const * const strText);
	void		Init(WCHAR const * const strText);

	void		Reset()
	{
		text[0] = 0;
		textHash = 0;
	}

	bool operator== (CStringHashA const & o) const
	{
		return (textHash == o.textHash);
	}
	bool operator!= (CStringHashA const & o) const
	{
		return (textHash != o.textHash);
	}
};

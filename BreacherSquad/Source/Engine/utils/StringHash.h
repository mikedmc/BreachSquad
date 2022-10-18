#pragma once

///--- string and hash pair ---
#define K_MAX_STRINGHASH_LEN 256
class CStringHash //string-hash pair
{
public:
	WCHAR text[K_MAX_STRINGHASH_LEN];
	UINT32 textHash;  //hash of the string

	FORCEINLINE const UINT32 getHash() const { return textHash; }

	CStringHash() { text[0] = 0; textHash = 0; }
	CStringHash(WCHAR const * const strText)
	{
		int len = wcslen(strText);
		if (len == 0)
		{
			text[0] = 0;
			textHash = 0;
			return;
		}

		wcscpy_s(text, K_MAX_STRINGHASH_LEN, strText);
		textHash = FastHash(text, wcslen(text));
	}

	CStringHash(char const * const strText)
	{
		int len = strlen(strText);
		if (len == 0)
		{
			text[0] = 0;
			textHash = 0;
			return;
		}

		mbstowcs(text, strText, K_MAX_STRINGHASH_LEN);
		textHash = FastHash(text, wcslen(text));
	}
	//copy constructor	
	CStringHash(const CStringHash &o)
	{
		Init(o.text);
	}

	const bool IsEmpty() const {
		return (textHash == 0);
	}

	const bool IsSet() const {
		return (textHash != 0);
	}

	const bool IsEqual(WCHAR* stext) const {
		return (textHash == FastHash(stext, wcslen(stext)));
	}

	void Init(WCHAR const * const strText)
	{
		if (strText == nullptr)
		{
			text[0] = 0;
			textHash = 0;
			return;
		}

		int len = wcslen(strText);
		if (len == 0)
		{
			text[0] = 0;
			textHash = 0;
			return;
		}

		StringCchCopy(text, K_MAX_STRINGHASH_LEN, strText);
		textHash = FastHash(text, wcslen(text));
	}

	void Init(CHAR const * const strText)
	{
		if (strText == nullptr)
		{
			text[0] = 0;
			textHash = 0;
			return;
		}

		int len = strlen(strText);
		if (len == 0)
		{
			text[0] = 0;
			textHash = 0;
			return;
		}

		WCHAR tempstr[MAX_PATH];
		size_t cntConv;
		mbstowcs_s(&cntConv, tempstr, strText, K_MAX_STRINGHASH_LEN);
		StringCchCopy(text, K_MAX_STRINGHASH_LEN, tempstr);
		textHash = FastHash(text, wcslen(text));
	}

	void Reset()
	{
		text[0] = 0;
		textHash = 0;
	}

	bool operator== (CStringHash const & o) const
	{
		return (textHash == o.textHash);
	}
	bool operator!= (CStringHash const & o) const
	{
		return (textHash != o.textHash);
	}
};



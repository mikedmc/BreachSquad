#include "dxstdafx.h"
#include "StringHashA.h"

CStringHashA::CStringHashA(WCHAR const * const strText)
{
	CHAR tempstr[MAX_PATH]{0};
	size_t cntConv;
	wcstombs_s(&cntConv, tempstr, strText, K_MAX_STRINGHASH_CHAR_LEN);
	strcpy_s(text, K_MAX_STRINGHASH_CHAR_LEN, tempstr);

	int len = strlen(text);
	if (len == 0)
	{
		text[0] = 0;
		textHash = 0;
		return;
	}

	textHash = FastHash(text, len);
}

CStringHashA::CStringHashA(char const * const strText)
{
	int len = strlen(strText);
	if (len == 0)
	{
		text[0] = 0;
		textHash = 0;
		return;
	}

	strcpy_s(text, K_MAX_STRINGHASH_CHAR_LEN, strText);
	textHash = FastHash(text, len);
}

CStringHashA::CStringHashA(const CStringHash &o)
{
	Init(o.text);
}

void CStringHashA::Init(CHAR const * const strText)
{
	if (strText == null)
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

	strcpy_s(text, K_MAX_STRINGHASH_CHAR_LEN, strText);
	textHash = FastHash(text, strlen(text));
}

void CStringHashA::Init(WCHAR const * const strText)
{
	if (strText == null)
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

	CHAR tempstr[MAX_PATH]{0};
	size_t cntConv;
	wcstombs_s(&cntConv, tempstr, strText, K_MAX_STRINGHASH_CHAR_LEN);
	strcpy_s(text, K_MAX_STRINGHASH_CHAR_LEN, tempstr);
	textHash = FastHash(text, strlen(text));
}

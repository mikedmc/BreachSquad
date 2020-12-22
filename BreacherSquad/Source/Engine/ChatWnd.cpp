#include "dxstdafx.h"
#include "ChatWnd.h"

CChatWnd::CChatWnd()
{
	fTimeSinceLastInput = 0.0f;
	bReceivingText = false;
	nInputCursor = 0;
	memset(strInputLine, 0, K_CW_MAX_LINE_LEN * sizeof(WCHAR));
}

CChatWnd::~CChatWnd()
{
	SAFE_DELETE_GROWABLE_ARRAY(m_arrLines);
}

void CChatWnd::Clear()
{
	fTimeSinceLastInput = 0.0f;
	fTimeSinceInputStarted = 0.0f;
	bReceivingText = false;

	nInputCursor = 0;
	memset(strInputLine, 0, K_CW_MAX_LINE_LEN * sizeof(WCHAR));

	SAFE_DELETE_GROWABLE_ARRAY(m_arrLines);

	LOG(L"ChatWnd:: Cleared chat.");
}

void CChatWnd::ReceiveChar(UINT32 nCode)
{
	if (!bReceivingText)
		return;

	static unsigned int surrogate = 0;
	if (UTF16_IsSurrogate1(nCode))
	{
		// first part of a surrogate pair: store it and wait for the second one
		surrogate = (unsigned short)nCode;
	}
	else
	{
		unsigned int key = nCode;

		// check if it is the second part of a surrogate pair, or a regular character
		if (UTF16_IsSurrogate2(nCode))
		{
			// convert the UTF-16 surrogate pair to a single UTF-32 value
			assert(surrogate);
			key = VK_SPACE; //display surrogates as space 
			//key = Utils::UTF16To32(surrogate, nCode);
		}
		else if (surrogate)
		{
			LOG(L"[Error] WndProc() received key %u while waiting for the second part of the UTF16 surrogate %u\n", nCode, surrogate);
		}
		//reset surrogate
		surrogate = 0;

		//process input
		if (key == VK_RETURN)
		{
			//too often, skip ENTER processing
			if (fTimeSinceLastInput < K_CW_MIN_TIME_BETWEEN_INPUTS)
				return;

			if (nInputCursor > 0)
			{
				strInputLine[nInputCursor] = 0;
				//get local player name
				CStringHash* strLocalName = &g_netlock.m_sNames[g_netlock.Net_GetPlayerIndex()];
				//add line to collection
				AddLine(strInputLine, strLocalName->text, K_CW_LOCAL_COLOR);
				//send network line
				g_netlock.Net_SendChatLine(strInputLine);
				//reset input
				bReceivingText = false;
				fTimeSinceLastInput = 0.0f;
			}
			//pressed enter with empty string (close chat)
			if ((nInputCursor == 0) && (fTimeSinceLastInput >= K_CW_MIN_TIME_BETWEEN_INPUTS))
			{
				fTimeSinceLastInput = 0.0f;
				fTimeSinceInputStarted = 0.0f;
				//stop input
				CancelInput();
			}
		}
		else if (key == VK_ESCAPE)
		{
			fTimeSinceLastInput = 0.0f;
			//stop input
			CancelInput();
		}
		else if (key == VK_BACK)
		{
			if (nInputCursor > 0)
			{
				nInputCursor--;
				strInputLine[nInputCursor] = 0;
			}
			fTimeSinceInputStarted = 0.0f;
		}
		else //regular character
		{
			if (nInputCursor < K_CW_MAX_LINE_LEN - 1)
			{
				strInputLine[nInputCursor] = (WCHAR)nCode;
				nInputCursor++;
			}
			fTimeSinceInputStarted = 0.0f;
		}
	}
}

void CChatWnd::AddLine(WCHAR * strText, WCHAR * strAuthor, DWORD dwColor /*= 0xffffffff*/)
{
	if (strText == null)
		return;

	int nLen = wcsnlen_s(strText, K_CW_MAX_LINE_LEN);
	CLAMP(nLen, 0, K_CW_MAX_LINE_LEN - 1);
	
	if (nLen <= 0)
		return;

	///--- special net commands ---
	//abort mission
	if (wcscmp(strText, K_CW_STR_CMD_NET_ABORT_MISSION) == 0)
	{
		if ((UTGetAppClass().IsGameNetworked()) && (g_level.m_levelState == K_LVL_STATE_PLAYING))
		{
			LOG(L"[NET] CChatWnd::AddLine received command [%s]", strText);
			g_level.SetLevelState(K_LVL_STATE_MISSION_FAILED, STR_MISSION_ABORTED);
		}
		return;
	}
	if (wcscmp(strText, K_CW_STR_CMD_NET_ASK_ABORT_MISSION) == 0)
	{
		if ((UTGetAppClass().IsGameNetworked()) && (g_level.m_levelState == K_LVL_STATE_PLAYING))
		{
			LOG(L"[NET] CChatWnd::AddLine received command [%s]", strText);
			//add message
			CStringDesc sdOut;
			CChatLine* cl = new CChatLine();
			if ((strAuthor == null) || (wcslen(strAuthor) <= 0))
				g_stringsMgr.ReplaceTokenString(&sdOut, STR_ABORT_MISSION_REQ_MSG, 1, L"PEER");
			else
				g_stringsMgr.ReplaceTokenString(&sdOut, STR_ABORT_MISSION_REQ_MSG, 1, strAuthor);

			StringCchPrintf(cl->sText, MAX_PATH, L"%s", sdOut.sText);
			cl->dwColor = K_CW_SYSTEM_COLOR_GREEN;
			m_arrLines.Add(cl);
			fShowTimer = K_CW_SHOW_CHAT_DURATION;
		}
		return;
	}

	CChatLine* cl = new CChatLine();
	if ((strAuthor == null) || (wcslen(strAuthor) <= 0))
	{
		StringCchPrintf(cl->sText, MAX_PATH, L"%s", strText);
	}
	else
	{
		StringCchPrintf(cl->sText, MAX_PATH, L"%s: %s", strAuthor, strText);
	}

	cl->dwColor = dwColor;

	m_arrLines.Add(cl);

	fShowTimer = K_CW_SHOW_CHAT_DURATION;
	//remove oldest lines
	while (m_arrLines.GetSize() > K_CW_MAX_CHAT_LINES)
	{
		SAFE_DELETE(m_arrLines[0]);
		m_arrLines.Remove(0);
	}
}

void CChatWnd::Init()
{
	LOG(L"CChatWnd::Init()");
	Clear();
}

void CChatWnd::StartInput()
{
	fShowTimer = K_CW_SHOW_CHAT_DURATION;
	fTimeSinceLastInput = 0.0f;
	bReceivingText = true;
	nInputCursor = 0;
	memset(strInputLine, 0, K_CW_MAX_LINE_LEN * sizeof(WCHAR));
}

void CChatWnd::CancelInput()
{
	fShowTimer = K_CW_SHOW_CHAT_DURATION;
	bReceivingText = false;
	nInputCursor = 0;

	fTimeSinceInputStarted = 0.0f;
}

void CChatWnd::Update(float dTime)
{
	fTimeSinceLastInput += dTime;
	if (bReceivingText)
	{
		fTimeSinceInputStarted += dTime;
		if (fTimeSinceInputStarted > K_CW_MAX_INPUT_WAIT)
		{
			CancelInput();
		}
	}
	//show chat timer
	if (!bReceivingText)
		fShowTimer -= dTime;
}

void CChatWnd::Paint(D3DXVECTOR2 vBottomLeft)
{
	//get font
	CTTFont* pTTFont = UTGetTTFManager().GetFont(shTTFID_SZ20.textHash);
	if (pTTFont == null)
	{
		ErrorBox(K_ERR_WARNING, L"[WARNING]CCHatWnd:: Font missing!");
		return;
	}

	D3DXVECTOR2 vPos = vBottomLeft;
	//input line
	if (bReceivingText)
	{
		WCHAR strLine[MAX_PATH];
		CStringHash* strLocalName = &g_netlock.m_sNames[g_netlock.Net_GetPlayerIndex()];
		StringCchPrintf(strLine, MAX_PATH, L"%s: %s", strLocalName->text, strInputLine);
		//measure string
		RECT rct;
		SetRect(&rct, 0, 0, 0, 0);
		pTTFont->pFont->DrawTextW(null, strLine, -1, &rct, DT_SINGLELINE | DT_CALCRECT, 0xffffffff);
		//add cursor at the end
		if (g_timers.GetTimerValue(600) > 0.3f)
			StringCchCat(strLine, MAX_PATH, L"_");

		const int nInputW = 400;
		//draw input box
		RECTXYWH rctxywh(vPos.x, vPos.y, max(nInputW, (rct.right - rct.left + 10)), pTTFont->nFontSize);
		CtrlMgrDrawFrame(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_FRAME1, rctxywh, 0xffffffff);
		//draw text
		pTTFont->DrawTextLine(strLine, vPos.x, vPos.y, DT_LEFT | DT_TOP | DT_SINGLELINE, K_CW_LOCAL_COLOR);
	}
	//rest of the lines
	if (m_arrLines.GetSize() == 0)
		return;

	float fFadePerc = LIMIT(fShowTimer * 2.0f, 0.0f, 1.0f);

	D3DXVECTOR2 vStart = vPos;
	vStart.y -= K_CW_LINE_SPACING;
	for (int kk = m_arrLines.GetSize() - 1; kk >= 0; kk--)
	{
		CChatLine* cl = m_arrLines[kk];
		//paint now
		DWORD color = D3DCOLOR_COLORALPHA(cl->dwColor, fFadePerc);
		pTTFont->DrawTextLine(cl->sText, vStart.x, vStart.y, DT_LEFT | DT_TOP | DT_SINGLELINE, color);
		vStart.y -= K_CW_LINE_SPACING;
	}
}


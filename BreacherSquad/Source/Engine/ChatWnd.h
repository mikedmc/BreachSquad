#pragma once

//max nr of characters on a chat line
#define K_CW_MAX_LINE_LEN 128
//display time for chat lines
#define K_CW_SHOW_CHAT_DURATION 4.0f
//time to wait for keypress before closing the chat
#define K_CW_MAX_INPUT_WAIT 5.0f
//necessary delay between chat lines (avoid spamming of chat lines)
#define K_CW_MIN_TIME_BETWEEN_INPUTS 0.5f
#define K_CW_MAX_CHAT_LINES 8
//default color for local chat lines
#define K_CW_LOCAL_COLOR 0xffffffff
#define K_CW_REMOTE_COLOR 0xfff4d942
#define K_CW_SYSTEM_COLOR 0xfff8a09a
#define K_CW_SYSTEM_COLOR_GREEN 0xff00ff00
//default line spacing
#define K_CW_LINE_SPACING 22.0f

///--- CHAT COMMANDS ---
//used for both players to end a net coop level
#define K_CW_STR_CMD_NET_ABORT_MISSION L"/CMD:NET_ABORT_MISSION"
//used by dead players to send a mission abort request
#define K_CW_STR_CMD_NET_ASK_ABORT_MISSION L"/CMD:NET_ASK_ABORT_MISSION"
//***********************************************************
// Chat Window
//***********************************************************
class CChatWnd
{
private:
	struct CChatLine {
		WCHAR	sText[MAX_PATH];
		DWORD	dwColor;

		CChatLine()
		{
			dwColor = 0xffffffff;
			memset(sText, 0, MAX_PATH * sizeof(WCHAR));
		}
	};
	//lines of text
	CArray<CChatLine*>	m_arrLines;
	//are we reading text?
	bool				bReceivingText;
	//current input line
	int					nInputCursor;
	WCHAR				strInputLine[K_CW_MAX_LINE_LEN];
	//detect surrogate characters
	inline bool UTF16_IsSurrogate1(unsigned int c) { return (c >= 0xD800 && c <= 0xDBFF); }
	inline bool UTF16_IsSurrogate2(unsigned int c) { return (c >= 0xDC00 && c <= 0xDFFF); }

public:
	//time since last input
	float fTimeSinceLastInput;
	//time since chat box opened. Too long and it closes by itself.
	float fTimeSinceInputStarted;
	//show timer
	float fShowTimer;

	CChatWnd();
	~CChatWnd();

	/* Is it receiving text? */
	FORCEINLINE bool IsReceivingInput() const {
		return bReceivingText;
	}
	/* Clears the chat */

	void Clear();

	/* receives characters through WM_CHAR commands */
	void ReceiveChar(UINT32 nCode);
	/* Adds a line of text to the chat history */
	void AddLine(WCHAR* strText, WCHAR * strAuthor = NULL, DWORD dwColor = 0xffffffff);

	void Init();
	/* Enables the input box. Call this to start reading chars. */
	void StartInput();
	/* Cancels current line */
	void CancelInput();

	void Update(float dTime);
	void Paint(D3DXVECTOR2 vBottomLeft);
};


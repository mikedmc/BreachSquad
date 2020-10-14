#pragma once

class CNetLock
{
public:
	///--- HEADER DATA ---
	/* message types */
	enum eNetMessageType
	{
		K_NETMSG_TYPE_INVALID = 0,

		K_NETMSG_TYPE_COMMAND,			//used to send reliable commands
		K_NETMSG_TYPE_GAME_FRAMES,		//game controller sync packets

		K_NETMSGS_COUNT
	};

	/*
	* Network "commands" that get sent reliable. Each command has its own data format
	*/
	enum eNetCommand
	{
		K_NETCMD_INVALID = 0,

		K_NETCMD_LOBBY_HANDSHAKE,			//sent when entered lobby
		K_NETCMD_PLAYER_SELECTION,			//selection changed in PlayerSelScreen (contains all necessary selection data)
		K_NETCMD_LEVEL_RESULTS,				//send level finished commands (restart, cancel, ready, etc)
		K_NETCMD_CHAT_LINE,					//send a chat line
		K_NETCMD_GAMEPLAY_CMD,				//send gameplay related commands (from eNetCommandGameplay)

		K_NETCMDS_COUNT
	};

	// Command params for K_NETCMD_GAMEPLAY_CMD
	enum eNetCommandGameplay
	{
		K_GAMEPLAYCMD_UNDEFINED = 0,
		
		K_GAMPLAYCMD_LEVEL_LOADED,			// sent after loading the level so we sync the clients

		K_GAMPLAYCMDS_COUNT,
	};

	/*
	 * Adds header data to a packet stream
	 */
	struct sPacketHeader {
		BYTE				ubAppVersion;			//saved as UINT32
		eNetMessageType		eMessageType;			//saved as BYTE

		sPacketHeader() : 
			ubAppVersion(_VERSION_HALFBYTE_), eMessageType(K_NETMSG_TYPE_INVALID) 
		{}

		sPacketHeader(eNetMessageType netMsgType) :
			ubAppVersion(_VERSION_HALFBYTE_), eMessageType(netMsgType)
		{}

		void Serialize(BitPacker & streamDest)
		{
			BYTE msgtype = (BYTE)eMessageType;
			assert(msgtype <= 7);
			//combine ubAppVersion(MSB 7 bits) and msgType (LSB 3 bits)
			BYTE wb = ((ubAppVersion << 3) | (msgtype & 0x7));
			streamDest.WriteUChar(wb);
		}

		/* returns false if wrong version or deserialization error */
		bool Deserialize(BitPacker & streamSource)
		{
			BYTE rb = streamSource.ReadUChar();
			BYTE msgtype = (rb & 0x7);
			eMessageType = (eNetMessageType)msgtype;
			ubAppVersion = (rb >> 3);

			if (ubAppVersion != _VERSION_HALFBYTE_)
			{
				LOG(L"[Error] PackHead.Deserialize - Wrong game version!");
				return false;
			}
			return true;
		}
	};


///--- FRAMELOCK GAME SYNC ---
public:	
//--- frame flags ---
//the flags are used to transmit other info about said frame 
// if you add more than 2 flags look in void CNetLock::sPacketNetlock::De/Serialize(BitPacker & streamSource) for encoding on bits
#define K_NETLOCK_FRAMEFLAG_INPUT_PAUSED_INGAME		1
#define K_NETLOCK_FRAMEFLAG_OTHER_FLAG1				2	
#define K_NETLOCK_FRAMEFLAG_OTHER_FLAG2				4	//<-- use these if needed, don't add more or change the serialize/deserialize

	/*
	 * Game frame sync commands packet (netlock)
	 */
	class sPacketNetlock
	{
	public:
		INT32	m_nFrame;			//frame assigned to button states
		DWORD	m_dwSyncCheck;		//used to check the sync of the random numbers generators
		WORD	m_wFrameFlags;		//different flags like MENU_SHOWN, etc
		//#TODO: de facut o structura minima pentru serializarea controllerului in clasa de controllers manager
		bool	m_bButStates[K_CM_COMMANDS_COUNT]; //actual button pressed data

		sPacketNetlock();
		void Reset();
		
		/* saves buttons states into packet data m_bButStates */
		void SaveButtonsPressedPercents(int nFrame, float arrSrcPressedPercents[K_CM_COMMANDS_COUNT]);
		/* gets the commands pressed percents */
		void GetButtonsPressedPercents(float arrDest[K_CM_COMMANDS_COUNT]);
	
		void Serialize(BitPacker & streamDest);
		void Deserialize(BitPacker & streamSource);

		// Returns a DW with the serialized commands (they fit in DW)
		DWORD SerializeToDW();
	};


public:
	//max packages sent together (70 default)
	static const int K_NETLOCK_MAX_STATE_PACKAGES = 70;
	//how many frames can we go ahead before having to wait for the other (60 default)
	static const int K_NETLOCK_MAX_INPUT_FRAMES_AHEAD = 60;
	//frames before and after lastSyncedFrame so we always have the necessary frames
	static const int K_NETLOCK_PACKAGE_CURFRAME_SAFEGUARD = 2;

	float	m_fTimeSinceLastRCV;		//time since last received game frame

	//rolling buffer of states that need to be sent out on the network
	sPacketNetlock m_arrToSend[K_NETLOCK_MAX_STATE_PACKAGES];
	int m_nToSend_Head; //buffer head stays on first valid element
	int m_nToSend_Tail;	//buffer tail stays on first empty element (on head when buffer is empty)
	//rolling buffer of states received from the peer
	sPacketNetlock m_arrReceived[K_NETLOCK_MAX_STATE_PACKAGES];
	int m_nReceived_Head;
	int m_nReceived_Tail;
	int m_nReceived_SyncFrame;  //last updated frame received from peer

	CNetLock();
	~CNetLock();

	/*
	 * Initializes everything for the Netlock step
	 */
	void Net_EnterNetlock();
	
	/*
	 * Packs all necessary data and sends it
	 */
	bool Net_SendFrameData(int nLastSyncedFrame, DWORD dwSyncCheck);

	/*
	* Unpacks received data and populates m_arrToSend and m_arrReceived lists
	*/
	bool Net_ReceiveFrameData(int nLastSyncedFrame);

	// Writes in/out buffers to log
	void Net_LogFrameData(int nCount = 10);

	///--- GENERIC NETWORK EVENT LOOP ---
public:
	/* always called (processing Steam events)
	 * called while in various states where sending/receiving messages is not necessary but where we still need to keep lobby integrity (ex: game ending/ended, restarting/continuing game)
	 */
	void Net_UpdateEventLoop();

	///--- LOBBY ---
	//--- player flags ---
#define K_NETLOCK_PLAYERFLAG_STARTED_LEVEL			1
#define K_NETLOCK_PLAYERFLAG_UNUSED					2

protected:
	int					m_nStep;							//used with different purposes 
	bool				m_bIsGamePrivate;					//is private game requested?

public:
	LobbyID				m_ullCurLobbyID;					//Lobby ID received through command line arg "+connect_lobby" or 0 if didn't receive an invite
	UINT32				m_unRandomSeed;						//random seed used to sync games
	BYTE				m_ucSelChapter, m_ucSelLevel;		//selected chapter and level
	BYTE				m_ucSelMode;						//selected game mode (classic, zombie, etc)
	CStringHash			m_sNames[K_MAX_PLAYERS_CNT];		//coded player names WCHAR [0]-host, [1]-peer
	UINT32				m_nPlayerFlags[K_MAX_PLAYERS_CNT];	//flags for players (ready, etc...)
	//modding
	BYTE				m_ucModData;						//type of mod: 0-disabled, 1-downloaded level
	CHAR				m_csModID_DwnLvl[MAX_PATH];			//ID of steam workshop mod for current loaded level when playing downloaded content (modding)

	/* Creates a new lobby */
	void Net_CreateLobby(bool bFriendsOnly);
	/* Requests a list of lobbies filtered by game version and files CRC */
	void Net_RequestLobbyList(int nMaxLobbies /*,bool bMatchCRC*/ );

	void Net_EnterLobby(bool bHostGame, bool bPrivateLobby);
	void Net_QuitLobby();
	void Net_UpdateLobby(float dTime);

	///------ MISSION FINISHED SCREEN ------
public:
	struct sPacketLevelResults {
		//peer states
		enum eLevelResultsState {
			K_LEVRES_STATE_UNDEFINED = 0,
			K_LEVRES_STATE_CLICKED_CONTINUE,
			K_LEVRES_STATE_CLICKED_RESTART,
			K_LEVRES_STATE_CLICKED_CANCEL,

			K_LEVRES_STATE_ACK_GAME_FINISHED,	//sent to peer to let him know we ended so he can stop sending level sync data
		};
		//current packet state
		eLevelResultsState m_eCurrentState;
		UINT32				m_nDataParam;

		/* Constructor */
		sPacketLevelResults(eLevelResultsState neState) : m_eCurrentState(neState), m_nDataParam(0)
		{}
		sPacketLevelResults() : m_eCurrentState(K_LEVRES_STATE_UNDEFINED), m_nDataParam(0)
		{}

		void Serialize(BitPacker & streamDest)
		{
			streamDest.WriteUChar((BYTE)m_eCurrentState);
			streamDest.WriteUInt(m_nDataParam);
		}

		void Deserialize(BitPacker & streamSource)
		{
			m_eCurrentState = (eLevelResultsState)streamSource.ReadUChar();
			m_nDataParam = streamSource.ReadUInt();
		}
	};

	//array that keeps current peer states
	sPacketLevelResults::eLevelResultsState m_arrLvlResPeerStates[K_MAX_PLAYERS_CNT];

	/* Resets votes from the level results screen */
	void Net_ResetLevelResults();
	void Net_EnterLevelResults();
	bool Net_SendLevelResultsCommand(sPacketLevelResults & playerState);
	void Net_UpdateLevelResults(float dTime);
	/* counts the number of states/selections from all players */
	int Net_LevelResultsCountStates(sPacketLevelResults::eLevelResultsState eState);
	/* Clears the voting selections from all players */
	void Net_LevelResultsClearStates();
	///--- PLAYER SELECTION SCREEN ---
public:
	/*
	 * Data structure that holds the player selection data
	 */
	struct sPacketPlayerSelection {
		BYTE			eType;				//player type
		WORD			nPlayerStars;		//current nr of stars
		BYTE			bSelected;			//is selection final?
		BYTE			nWpnSelection[PSS_ITEMCATS_COUNT];			//selectia curenta pentru cele 4 casute selectabile.
		BYTE			nWpnSelectionPrice[PSS_ITEMCATS_COUNT];	//pretul selectiei curente ca sa putem afisa locked items
		BYTE			nCursorPos;			//pozitia efectiva a cursorului
		//XP
		int				nPlayerXPpoints;	//player total XP points
		BYTE			arrUpgradeBarsPts[K_PSS_UPGRADE_BARS_CNT]; //player upgrade points per bar

		sPacketPlayerSelection() : eType(0), bSelected(0), nCursorPos(0), nPlayerStars(0)
		{
			for (int kk = 0; kk < PSS_ITEMCATS_COUNT; kk++)
			{
				nWpnSelection[0] = 0;
				nWpnSelectionPrice[0] = 0;
			}
			//XP data
			nPlayerXPpoints = 0;
			for (int kk = 0; kk < K_PSS_UPGRADE_BARS_CNT; kk++)
			{
				arrUpgradeBarsPts[kk] = 0;
			}
		}

		void Serialize(BitPacker & streamDest)
		{
			streamDest.WriteUChar(eType);
			streamDest.WriteUShort(nPlayerStars);
			streamDest.WriteUChar(bSelected);
			streamDest.WriteUChar(nCursorPos);
			streamDest.WriteBytes(nWpnSelection, PSS_ITEMCATS_COUNT);
			streamDest.WriteBytes(nWpnSelectionPrice, PSS_ITEMCATS_COUNT);
			//XP
			streamDest.WriteInt(nPlayerXPpoints);
			streamDest.WriteBytes(arrUpgradeBarsPts, K_PSS_UPGRADE_BARS_CNT);
		}

		void Deserialize(BitPacker & streamSource)
		{
			eType = streamSource.ReadUChar();
			nPlayerStars = streamSource.ReadUShort();
			bSelected = streamSource.ReadUChar();
			nCursorPos = streamSource.ReadUChar();
			streamSource.ReadBytes(nWpnSelection, PSS_ITEMCATS_COUNT);
			streamSource.ReadBytes(nWpnSelectionPrice, PSS_ITEMCATS_COUNT);
			//XP
			nPlayerXPpoints = streamSource.ReadInt();
			streamSource.ReadBytes(arrUpgradeBarsPts, K_PSS_UPGRADE_BARS_CNT);
		}
	};

	/* initializes data members for player selection screen */
	void Net_EnterPlayerSelScreen();
	/* sends actual peer selection changes */
	bool Net_SendPlayerSelection(sPacketPlayerSelection & playerSel);
	// Initializes data upon exiting the player selection screen
	void Net_ExitPlayerSelScreen();

	///--- CHAT ---
	bool Net_SendChatLine(WCHAR* strChatLine);

	///--- Gameplay Commands ---
	// Sends a reliable gameplay command (used mainly to sync the clients after loading)
	bool Net_SendGameplayCommand(eNetCommandGameplay eCmd);

	///--- GAMEPLAY ---
	unsigned int Net_GetPlayerIndex() const
	{
#ifdef ENABLE_NETWORKING
		return g_pNetwork->GetCurrentLobby().bIAmOwner ? 0 : 1;
#else
		return 0;
#endif
	}

	unsigned int Net_GetOtherPlayerIndex() const
	{
#ifdef ENABLE_NETWORKING
		return g_pNetwork->GetCurrentLobby().bIAmOwner ? 1 : 0;
#else
		return 1;
#endif
	}

	bool Net_GetIAmHosting() const
	{
#ifdef ENABLE_NETWORKING
		return g_pNetwork->GetCurrentLobby().bIAmOwner;
#else
		return true;
#endif
	}
};
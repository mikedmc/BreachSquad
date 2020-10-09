#pragma once

#define	K_MOD_MAX_TEXT_LEN 2048

class CModsManager {
public:

	enum eModType {
		K_MOD_TYPE_SINGLE_LEVEL = 0,
		K_MOD_TYPE_GAME_CHANGER = 1,

		K_MOD_TYPES_CNT
	};

	class CModDescriptor 
	{
	//mod details loaded from mod descriptor files
	public:
		eModType						eType;					//mod type
		CStringHash						shName;					//mod name
		CStringHash						shImagePath;			//image path (relative filename)
		WCHAR							strAuthor[MAX_PATH];
		WCHAR							strDescription[K_MOD_MAX_TEXT_LEN];
		WCHAR							strTags[MAX_PATH];
		WCHAR							strChangeNotes[K_MOD_MAX_TEXT_LEN];
		WCHAR							strVersion[MAX_PATH];

	//Steam downloaded data (GENERIC DATA)
	public:
		bool							bActive;				//is it activated?
		bool							bSubscribed;			//is user still subscribed?
		UINT64							uID;					//file ID from Steam
		UINT32							unTime_create;			//time of creation
		UINT32							unTime_updated;			//last time of update

		CGrowableArray<CStringHash*>	arrAffectedFiles;		//relative paths (including "media") to affected files
		//CTOR/DTOR
		CModDescriptor();
		~CModDescriptor();

		// Loads the descriptor from file. Returns number of affected files.
		int								LoadModDescriptor(const WCHAR* strModRootDirectory);
		// Gets full path to mod image. Returns true if path is set.
		bool							GetFullPathToModImage(WCHAR* strDest, UINT32 nDestSize);
		// Gets full path to mod affected files. Returns true if path is set.
		bool							GetFullPathToAffectedFile(int nFileIdx, WCHAR* strDest, UINT32 nDestSize);
	};

public:
	CGrowableArray<CModDescriptor*>		m_arrMods;			//loaded mods

public:
	CModsManager();
	~CModsManager();
	// Loads downloaded mods from XML cache - file that keeps track of downloaded content
	HRESULT								LoadModsFromCacheFile();
	// Saves downloaded mods back to XML cache
	HRESULT								SaveModsToCacheFile();
	// Returns the mod descriptor uith specified ID or null if not found
	CModDescriptor*						GetModDescByID(UINT64 unModID);
	//returns the local mod index by specified ID or -1 if not found
	int									GetModIndexByID(UINT64 unModID);
	// Returns the mod descriptor uith specified ID or null if not found
	CModDescriptor*						GetModDescByIndex(int nIndex);

	//Checks all active mods and returns the one that touches specified file or null if none does
	//path is relative to media and includes "media" folder: media/back/bg1.png
	CModDescriptor*						GetModThatUsesFile(WCHAR * strRelPath, CModDescriptor * pAvoidMod = null);
	//Checks all active mods and writes the full path for specified file in wsRetPath
	//path is relative to media and includes "media" folder: media/back/bg1.png
	bool								GetFullPathForFile(const WCHAR * wsMediaPath, WCHAR * wsRetPath, int nRetPathSize);
	// Tells if mod is active
	bool								IsModActive(CModDescriptor * mod);
	// Activates/deactivates a mod and saves the state.
	// If mod has conflicts return conflicting mod 
	CModDescriptor*						SetModActive(CModDescriptor * mod, bool bActive);
	// Returns true if game and mod versions coincide
	bool								IsCompatibleWithCurrentVersion(CModDescriptor * mod);
	// gets the total number of active mods by type
	int									GetActiveModsCountByType(eModType eSelType);
};

///--- SINGLETON ---
CModsManager& UTGetModsManager();

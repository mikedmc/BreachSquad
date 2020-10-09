#pragma once

//standard floor height
#define K_IVM_FLOOR_HEIGHT_TL 5

//floor flags
#define K_IVM_FLOORFLAG_NONE				0
#define K_IVM_FLOORFLAG_TYPE_LEFT_ENDING	1
#define K_IVM_FLOORFLAG_TYPE_MIDDLE			2
#define K_IVM_FLOORFLAG_TYPE_RIGHT_ENDING	4

class CInfiniteVerticalMode
{
public:
	//local structure that holds data about available prefabs
	struct CPrefabDesc {
		CStringHash		strPath;
		int				nWidth_TL;
		int				nFlags;
	};

	//local structure that holds data about spawned enemies
	struct CActGenDesc {
		CStringHash		strTemplateName;
		int				nMinFloor;			//min level to spawn it from
		int				nMaxFloor;			//max level until when we can spawn this enemy (or 0 to ignore)
		float			fProbability;		//probability to have that enemy spawned
	};

private:
	CLevel*					pLevel;					//Pointer to main level class
	int						m_nFloorWidth_TL;		//Standard floor width that will be generated (tiles)
	POINTXY_INT				m_ptGeneratePos_TL;		//New floors get added here (tiles from level origin) - Right side of building! Don't change!
	DWORD					m_dwLastDoorUp_UID;		//UID of last door going up
	bool					m_bFirstDoorUpOnRight;		//current last door going up is on the right side? Don't change!

	POINTXY_INT				m_ptGenCursor;			//cursor that starts from GeneratePos
	bool					m_bDoorOnRightCursor;	//corsor that tells the side of the last door

	int						m_nLastGenFloor;		//currently last generated floor
	
	CStringHash				shBaseLevelPath;		//media relative path to base starting level 
	CStringHash				shLoadedDescriptorPath;	//path to actual loaded descriptor
	CGrowableArray<CPrefabDesc*>		m_arrPrefabsList;	//available prefabs list
	CGrowableArray<CActGenDesc*>		m_arrActorsList;	//available actors for spawning
public:
	//CTOR/DTOR
	CInfiniteVerticalMode();
	~CInfiniteVerticalMode();

	// Initializes internal stuff by loading data from the XML and sets a pointer to the main level class
	// \param wcsDescriptorPath is a resource ID, relative to "media" folder
	bool					Init(CLevel * pLevelPtr, WCHAR * wcsDescriptorPath);
	// Call this when starting a level to reset cursors
	void					RestartLevel();
	// Returns the used files CRC  (with/out mods)
	DWORD					GetFilesCRC(bool bIgnoreMods);
	// Gets the base level path relative to media folder (media ID)
	const WCHAR*			GetBaseLevelMediaID();
	// Releases all internal data
	void					Release();
	// generates floors based on visible area
	void					Update(float dTime);
	// Generates a random building floor based on internal data (difficulty, current building height, etc)
	// Rooms get generated right to left to respect drawing order for doors
	void					GenerateNextHorizontalFloor(bool bScrollLevel = true);
private:
	// Scrolls the tileset down with nRows and deallocates what comes outside of the map (objects, actors,etc)
	// It changes the active rendering area in m_LevelAABB_TL (we render relative to that)
	bool					ScrollDownTilesetActiveArea(int nRows);
	// Gives you the prefab idx from arrPrefabsList that should be generated next
	int						GetPrefabIdx(int nPrefabFlagFilter = K_IVM_FLOORFLAG_NONE, int nFixedWidth = 0);

	// Writes the room sizes that added together reach nFloorWidth_TL
	// Returns the number of rooms (or infinite loop if size cannot be reached)
	// Size limits for the start and end rooms
	int						GetRoomSizes(int nFloorWidth_TL, int ret_arrSizes[32]);
	// Gets a random enemy index (from arrEnemiesList) based on the current generated level
	// \returns -1 when we should deallocate actor or the actual actor index
	int						GetRandomEnemy(int nFloor);
};

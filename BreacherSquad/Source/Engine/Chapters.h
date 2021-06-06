#pragma once

//fixed number of levels per chapter
#define K_GAME_LEVELS_PER_CHAPTER 12

class CChaptersList {
public:

	//structure that contains the list of levels and other data loaded from missions.xml
	struct CChapterDesc {
		int			nChapterNameStrIdx;
		int			nChapterMissionsToUnlock;
		int			nChapterImgFrame;		//frame from animation to pain

		int			nLevelsCnt;
		int			arrLevelNameStrIdx[K_GAME_LEVELS_PER_CHAPTER];
		CStringHash	arrLevelFilenames[K_GAME_LEVELS_PER_CHAPTER];
		//CTOR
		CChapterDesc() : nLevelsCnt(0), nChapterNameStrIdx(-1), nChapterMissionsToUnlock(0), nChapterImgFrame(0)
		{
			for (int kk = 0; kk < K_GAME_LEVELS_PER_CHAPTER; kk++)
			{
				arrLevelNameStrIdx[kk] = -1; //not set
			}
		}
	};

public:
	//list of chapters
	CArray<CChapterDesc*> m_arrChapters;

	CChaptersList();
	~CChaptersList();

	/* Returns number of levels, multiple of GAME_LEVELS_PER_CHAPTER */
	FORCEINLINE const int GetTotalLevelsCnt() const { 
		return m_arrChapters.GetSize() * K_GAME_LEVELS_PER_CHAPTER; 
	}

	FORCEINLINE const int GetChaptersCnt() const {
		return m_arrChapters.GetSize();
	}

	// Returns TRUE if specified chapter and level exist
	bool			IsValidLevel(int nChapter, int nLevel);
	// Returns TRUE if chapter is unlocked (or it doesn't exist)
	bool			IsChapterUnlocked(int nChapter, int nMissionsPlayedTotal);
	// Loads chapter descriptors from missions.xml
	bool			LoadChapters(WCHAR * strXmlPath);

	// \brief Gets the corresponding filename for the specified nChapterNumber and nLevelNumber from loaded chapters list
	// \param nChapterNumber, nLevelNumber - 0 based mission indicators
	// \param strDest, strDestMaxLen - string to write the path to
	bool			GetMissionFilename(int nChapterNumber, int nLevelNumber, WCHAR * strDest, int strDestMaxLen);
};

///--- SINGLETON ---
CChaptersList& UTGetChaptersList();

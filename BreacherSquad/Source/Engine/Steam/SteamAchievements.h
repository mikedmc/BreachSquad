#pragma once

#include "Achievement.h"

#ifdef ENABLE_STEAM
#include "steam_api.h"

// depends on calling periodically SteamAPI_RunCallbacks()
//

// not used atm; used in commented code to check if stat version is higher => old client
#define VERSION_STATS 1

//save the stats every N seconds if called too often
#define K_SA_STATS_SAVE_WAIT_PERIOD 2.0f

class CSteamAchievements
{
	public:
		CSteamAchievements(SGameStat* pStatsArr, int nStatsCnt, SGameAchievement* pAchArr, int nAchCnt);
		~CSteamAchievements();

	public: // Call per frame
		void Update(float dTime);

	public: // Steam callbacks
		STEAM_CALLBACK( CSteamAchievements, OnUserStatsReceived, UserStatsReceived_t, m_CallbackUserStatsReceived );
		STEAM_CALLBACK( CSteamAchievements, OnUserStatsStored, UserStatsStored_t, m_CallbackUserStatsStored );
		STEAM_CALLBACK( CSteamAchievements, OnAchievementStored, UserAchievementStored_t, m_CallbackAchievementStored );

	private:
		SGameStat*			pStatsArray;
		int					m_nStatsCount;
		SGameAchievement*	pAchievementsArray;
		int					m_nAchievementsCount;

		// Stores stats only if asked for (see m_bStoreStats)
		void StoreStatsIfNecessary(float dTime);

	public:
		void UnlockAchievement( SGameAchievement& achievement );
		//Se pare ca Stats sunt gandite pe steam sa fie salvate rar, nu de fiecare data. Ar trebui salvate doar la final de nivel...
		bool SaveStat(SGameStat& stat);
		bool RequestStats();
		// Forces Stats saving
		void StoreStatsNow();

	public:
		ISteamUser*			m_pSteamUser;		// Steam User interface
		ISteamUserStats*	m_pSteamUserStats;	// Steam UserStats interface

		// our GameID
		CGameID				m_GameID;

		// Did we get the stats from Steam?
		bool				m_bRequestedStats;
		bool				m_bStatsValid;

		// Should we store stats?
		bool				m_bStoreStats;
		// Only save the stats after a few seconds so we don't call it too often
		float				m_fStoreStatsTimer;
};

#endif ENABLE_STEAM

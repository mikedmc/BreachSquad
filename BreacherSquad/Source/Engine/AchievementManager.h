#pragma once

#include "Achievement.h"

#ifdef ENABLE_STEAM
	class CSteamAchievements;
#endif // ENABLE_STEAM

#ifdef ENABLE_GALAXY
	class CGalaxyStatsAndAchievements;
#endif // ENABLE_GALAXY

// Achievement Manager
class CAchievementManager
{
	public:
		CAchievementManager();
		~CAchievementManager();

	public:
#ifdef ENABLE_STEAM
		CSteamAchievements*				mSteamAchievements;
#endif // ENABLE_STEAM

#ifdef ENABLE_GALAXY
		CGalaxyStatsAndAchievements*	mGalaxyStatsAndAchievements;
#endif // ENABLE_GALAXY


	public:
		// Initializes the system
		void	Init();			
		// Releases all items
		void	Release();
		//loads/requests stats and achievements
		bool RequestStats();

		void UnlockAchievement( EGameAchievements achievementId );
		bool IsAchievementUnlocked( EGameAchievements achievementId );

		// Saves the stats now 
		void StoreStats();
		// call per frame
		void Update(float dTime); 

	private:
		void UnlockAchievement( SGameAchievement &achievement, int index );

	public:
		static SGameAchievement		g_Achievements[]; //achievements array

/// STATS
	public:
		static SGameStat			g_Stats[]; //static constant stats list
		bool						SetStat(EGameStats eStat, float fValue);
		bool						IncStat(EGameStats eStat, float fAddValue = 1.0f);
};

// Achievements manager singleton
CAchievementManager& __Achievements();

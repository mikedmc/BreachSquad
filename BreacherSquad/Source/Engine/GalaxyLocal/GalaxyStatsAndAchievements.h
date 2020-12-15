#pragma once

#ifdef ENABLE_GALAXY
#include "../Achievement.h"

#include "galaxy/GalaxyID.h"
#include "galaxy/IStats.h"

// Galaxy achievements and stats

class CGalaxyStatsAndAchievements : public 
	galaxy::api::IUserStatsAndAchievementsRetrieveListener, 
	galaxy::api::IStatsAndAchievementsStoreListener, 
	galaxy::api::IAchievementChangeListener
{
public:
	CGalaxyStatsAndAchievements();
	~CGalaxyStatsAndAchievements();

public: // Call per frame
	void Update();

public: // callbacks
	virtual void OnUserStatsAndAchievementsRetrieveSuccess(galaxy::api::GalaxyID userID) override;
	virtual void OnUserStatsAndAchievementsRetrieveFailure(galaxy::api::GalaxyID userID, IUserStatsAndAchievementsRetrieveListener::FailureReason failureReason) override;
	virtual void OnUserStatsAndAchievementsStoreSuccess() override;
	virtual void OnUserStatsAndAchievementsStoreFailure(IStatsAndAchievementsStoreListener::FailureReason failureReason) override;
	virtual void OnAchievementUnlocked(const char* name) override;

private:
	void StoreStatsIfNecessary();

public:
	void UnlockAchievement(SGameAchievement& achievement);
	void RequestAchievements();
	// Forces Stats saving
	void StoreStatsNow();
	// saves specific stat
	bool SaveStat(SGameStat& stat);

public:
	// Did we get the stats from Steam?
	bool				m_bDelayedLoad;
	bool				m_bStatsValid;

	// Should we store stats this frame?
	bool				m_bStoreStats;
};

#endif // ENABLE_GALAXY

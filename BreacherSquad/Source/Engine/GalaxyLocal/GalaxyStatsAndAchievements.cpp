#include "dxstdafx.h"

#ifdef ENABLE_GALAXY

#include "GalaxyStatsAndAchievements.h"

#include "../AchievementManager.h"

#include "galaxy/GalaxyApi.h"
#include "GalaxyUtils.h"

using namespace galaxy::api;

//****************************************************************************************

CGalaxyStatsAndAchievements::CGalaxyStatsAndAchievements()
{
	m_bDelayedLoad = false;
	m_bStatsValid = false;
	m_bStoreStats = false;

	RegisterAsGalaxyListener<IUserStatsAndAchievementsRetrieveListener>(this);
	RegisterAsGalaxyListener<IStatsAndAchievementsStoreListener>(this);
	RegisterAsGalaxyListener<IAchievementChangeListener>(this);
}

CGalaxyStatsAndAchievements::~CGalaxyStatsAndAchievements()
{
	UnregisterAsGalaxyListener<IUserStatsAndAchievementsRetrieveListener>(this);
	UnregisterAsGalaxyListener<IStatsAndAchievementsStoreListener>(this);
	UnregisterAsGalaxyListener<IAchievementChangeListener>(this);
}

//****************************************************************************************

void CGalaxyStatsAndAchievements::Update()
{
	if( m_bDelayedLoad )
	{
		RequestAchievements();
	}

	if( !m_bStatsValid )
		return;

	// Store stats
	StoreStatsIfNecessary();
}

void CGalaxyStatsAndAchievements::RequestAchievements()
{
	m_bDelayedLoad = false;

	if (UTApp().m_Settings.galaxyFullyLoaded && galaxy::api::User())
	{
		if (galaxy::api::User()->SignedIn())
		{
			galaxy::api::Stats()->RequestUserStatsAndAchievements();

			const galaxy::api::IError* error = galaxy::api::GetError();
			if (error)
			{
				g_pLog->Write("Failed retrieving Galaxy stats and achievements: %s", error->GetMsg());
			}
		}
		else
		{
			m_bDelayedLoad = true;
		}
	}
}

void CGalaxyStatsAndAchievements::StoreStatsNow()
{
	Stats()->StoreStatsAndAchievements();
	if (!GetError())
	{
		m_bStoreStats = false;
	}
	else
	{
		g_pLog->Write("Failed storing achievements in GOG: %s", GetError()->GetMsg());
	}
}

void CGalaxyStatsAndAchievements::OnUserStatsAndAchievementsRetrieveSuccess(galaxy::api::GalaxyID userID)
{
	(userID);

	g_pLog->Write("CGalaxyStatsAndAchievements: Received stats and achievements from GOG!\n");

	m_bStatsValid = true;

	// load stats
	for (int iStat = 0; iStat < N_STATS_CNT; ++iStat)
	{
		SGameStat &stat = __Achievements().g_Stats[iStat];
		switch (stat.m_eStatType)
		{
			case STAT_INT:
				stat.m_iValue = galaxy::api::Stats()->GetStatInt(stat.m_pchStatName, userID);
				if (galaxy::api::GetError())
				{
					LOG("Error retrieveing INT stat %s: %s", stat.m_pchStatName, galaxy::api::GetError()->GetMsg());
					continue;
				}

#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
				LOG("[Steam Stats]received [%s]=%d", stat.m_pchStatName, stat.m_iValue);
#endif
				break;

			case STAT_FLOAT:
			case STAT_AVGRATE:
				stat.m_flValue = galaxy::api::Stats()->GetStatFloat(stat.m_pchStatName, userID);
				if (galaxy::api::GetError())
				{
					LOG("Error retrieveing FLOAT stat %s: %s", stat.m_pchStatName, galaxy::api::GetError()->GetMsg());
					continue;
				}

#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
				LOG("[Steam Stats]received [%s]=%.2f", stat.m_pchStatName, stat.m_flValue);
#endif
				break;

			default:
				break;
		}
	}
	// load achievements
	for (int iAch = 0; iAch < ACH_ACHIEVEMENTS_CNT; ++iAch)
	{
		SGameAchievement &ach = __Achievements().g_Achievements[iAch];

		uint32_t unlockedTime = 0;
		galaxy::api::Stats()->GetAchievement(ach.m_pchAchievementID, ach.m_bAchieved, unlockedTime);
		if (galaxy::api::GetError())
		{
			g_pLog->Write("Error retrieveing achievement %s: %s", ach.m_pchAchievementID, galaxy::api::GetError()->GetMsg());
			continue;
		}

		sprintf(ach.m_rgchName, "%s", galaxy::api::Stats()->GetAchievementDisplayName(ach.m_pchAchievementID));
		sprintf(ach.m_rgchDescription, "%s", galaxy::api::Stats()->GetAchievementDescription(ach.m_pchAchievementID));

#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
		LOG("[Achievements]received [%s] achieved:%d", ach.m_rgchName, ach.m_bAchieved);
#endif
	}
}

void CGalaxyStatsAndAchievements::OnUserStatsAndAchievementsRetrieveFailure(galaxy::api::GalaxyID userID, IUserStatsAndAchievementsRetrieveListener::FailureReason failureReason)
{
	(userID);

	g_pLog->Write("Failed retrieveing achievements. Reason: %d", failureReason);
	
	m_bStatsValid = false;
}

void CGalaxyStatsAndAchievements::OnUserStatsAndAchievementsStoreSuccess()
{
	g_pLog->Write("CGalaxyStatsAndAchievements: StoreStats - succes !\n");
}

void CGalaxyStatsAndAchievements::OnUserStatsAndAchievementsStoreFailure(IStatsAndAchievementsStoreListener::FailureReason failureReason)
{
	g_pLog->Write("CGalaxyStatsAndAchievements: StoreStats - failed, %d\n", failureReason);
}

void CGalaxyStatsAndAchievements::OnAchievementUnlocked(const char* name)
{
	g_pLog->Write("CGalaxyStatsAndAchievements: Achievement '%s' unlocked!\n", name);
}

//****************************************************************************************

void CGalaxyStatsAndAchievements::UnlockAchievement(SGameAchievement& achievement)
{
	// mark it down
	Stats()->SetAchievement(achievement.m_pchAchievementID);
	
	// Store stats end of frame
	m_bStoreStats = true;
}

//DMC****************************************************************************************

bool CGalaxyStatsAndAchievements::SaveStat(SGameStat& stat)
{
	bool bretVal = true;

	switch (stat.m_eStatType)
	{
		case STAT_INT:
			Stats()->SetStatInt(stat.m_pchStatName, stat.m_iValue);
			break;

		case STAT_FLOAT:
			Stats()->SetStatFloat(stat.m_pchStatName, stat.m_flValue);
			break;

		case STAT_AVGRATE:
			bretVal = false;
			LOG(L"[ERROR] SaveStat - GOG doesn't support AVGRATE stats.");
			break;

		default:
			bretVal = false;
			LOG(L"[ERROR] SaveStat - Unhandled stat type!");
			break;
	}

	// Store stats end of frame
	m_bStoreStats = true;

	return bretVal;
}

//****************************************************************************************

void CGalaxyStatsAndAchievements::StoreStatsIfNecessary()
{
	if (m_bStoreStats)
	{
		Stats()->StoreStatsAndAchievements();
		if (!GetError())
		{
			m_bStoreStats = false;
		}
		else
		{
			g_pLog->Write("Failed storing achievements in Gog: %s", GetError()->GetMsg());
		}
	}
}

//****************************************************************************************

#endif // ENABLE_GALAXY
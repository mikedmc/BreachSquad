#include "dxstdafx.h"

#define _STAT_ID( id,type,name ) { id, type, name, 0, 0, 0, 0 }
//Stats array which will hold data about the stats and their state
SGameStat CAchievementManager::g_Stats[] =
{
	_STAT_ID(3,  STAT_INT, "N_STAT_KILLED_ENEMIES"),
	_STAT_ID(4,  STAT_INT, "N_STAT_SAVED_HOSTAGES"),
	_STAT_ID(5,  STAT_INT, "N_STAT_ARRESTS_MADE"),
	_STAT_ID(6,  STAT_INT, "N_STAT_SECRET_ITEMS"),
	_STAT_ID(7,  STAT_INT, "N_STAT_SAVED_POLICEMEN"),
	_STAT_ID(8,  STAT_INT, "N_STAT_BOMBS_DISARMED"),
	_STAT_ID(9,  STAT_INT, "N_STAT_DOORS_KICKED"),
	_STAT_ID(10, STAT_INT, "N_STAT_DOORS_EXPLODED"),
	_STAT_ID(11, STAT_INT, "N_STAT_STUNNED_ENEMIES"),
	_STAT_ID(12, STAT_INT, "N_STAT_SNIPED_ENEMIES"),
	_STAT_ID(13, STAT_INT, "N_STAT_FLAMED_ENEMIES"),
	_STAT_ID(14, STAT_INT, "N_STAT_ASSAULTER_KILLS"),
	_STAT_ID(15, STAT_INT, "N_STAT_BREACHER_KILLS"),
	_STAT_ID(16, STAT_INT, "N_STAT_SHIELD_KILLS"),
	_STAT_ID(17, STAT_INT, "N_STAT_FBI_KILLS"),
	_STAT_ID(18, STAT_INT, "N_STAT_RECON_KILLS"),
	_STAT_ID(19, STAT_INT, "N_STAT_OFFDUTY_KILLS")
};
#undef _STAT_ID

#define _ACH_ID( id, name ) { id, #id, name, "", 0 }
SGameAchievement CAchievementManager::g_Achievements[] = 
{
	_ACH_ID(ACH_HEAT_UP_THE_NIGHT, "Heat Up the Night"),
	_ACH_ID(ACH_TWO_IS_A_PARTY, "Two is a Party"),
	_ACH_ID(ACH_GANGLAND_PACIFIER, "Gangland Pacifier"),
	_ACH_ID(ACH_RADICALIZED, "Radicalized"),
	_ACH_ID(ACH_KETCHUP, "Ketchup"),
	_ACH_ID(ACH_3STARS_MISSION, "3 Stars Mission"),
	_ACH_ID(ACH_CLEANUP_THE_HOOD, "Cleanup the Hood"),
	_ACH_ID(ACH_NO_QUARTER, "No Quarter"),
	_ACH_ID(ACH_NO_MERCY, "No Mercy"),
	_ACH_ID(ACH_NO_RESPITE, "No Respite"),
	_ACH_ID(ACH_THE_THIN_BLUE_LINE, "The Thin Blue Line"),
	_ACH_ID(ACH_EOD, "EOD"),
	_ACH_ID(ACH_DOOR_KICKER, "Door Kicker"),
	_ACH_ID(ACH_DOOR_HATER, "Door Hater"),
	_ACH_ID(ACH_ATTENTION_GETTER, "Attention Getter"),
	_ACH_ID(ACH_I_LIKE_THEM_NUMB, "I Like Them Numb"),
	_ACH_ID(ACH_GOOD_BREACH, "Good Breach"),
	_ACH_ID(ACH_NINJA, "Ninja"),
	_ACH_ID(ACH_EFFICIENT, "Efficient"),
	_ACH_ID(ACH_CLEANSE_BY_FIRE, "Cleanse by Fire"),
	_ACH_ID(ACH_TROUBLEMAKERS_ARRIVING, "Troublemakers Arriving"),
	_ACH_ID(ACH_THINGS_HEATING_UP, "Things Heating Up"),
	_ACH_ID(ACH_WE_STAND_ON_GUARD, "Stand on Guard"),
	_ACH_ID(ACH_CAVALRY_IS_HERE, "Cavalry is Here"),
	_ACH_ID(ACH_BUILDING_COMMUNITY, "Building Community"),
	_ACH_ID(ACH_SLACKER, "Slacker"),
	_ACH_ID(ACH_ALERT_EXTINGUISHED, "Alert Extinguished"),
	_ACH_ID(ACH_DARWIN_AWARD, "Darwin Award"),
	_ACH_ID(ACH_BAD_EYESIGHT, "Bad Eyesight"),
	_ACH_ID(ACH_TERMINATOR, "Terminator"),
	_ACH_ID(ACH_STAY_WITH_ME, "Stay With Me"),
	_ACH_ID(ACH_COMPLETIONIST, "Completionist"),
	_ACH_ID(ACH_LOOKS_INTERESTING, "Looks Interesting"),
	_ACH_ID(ACH_PEST_CONTROL, "Pest Control"),
	_ACH_ID(ACH_NICK_OF_TIME, "In the Nick of Time"),
	_ACH_ID(ACH_BITE_THE_APPLE, "Bite the Apple"),
	_ACH_ID(ACH_NOT_IN_MY_CITY, "Not in my City"),
	_ACH_ID(ACH_METRO_CALLING, "Metro Calling"),
	_ACH_ID(ACH_WARRANT_SERVER, "Warrant Server"),
	_ACH_ID(ACH_NOT_ON_MY_WATCH, "Not on my Watch"),
	_ACH_ID(ACH_CARTEL_ANNIHILATOR, "Cartel Annihilator"),
	_ACH_ID(ACH_HELL_IS_COMING, "Hell is Coming"),
	_ACH_ID(ACH_DEATH_DEALER, "Death Dealer"),
	_ACH_ID(ACH_BUCKSHOT_SEASON, "Buckshot Season"),
	_ACH_ID(ACH_LADY_JUSTICE, "Lady Justice"),
	_ACH_ID(ACH_I_SEE_THINGS, "I See Things and I Kill Them"),
	_ACH_ID(ACH_FEDERAL_AUTHORITY, "Federal Authority"),
	_ACH_ID(ACH_GRUMPY_MCANGRY, "Grumpy McAngry"),
	_ACH_ID(ACH_GOING_HOME, "Going Home"),
	_ACH_ID(ACH_COURT_IS_CLEAR, "The Court is Clear"),
	_ACH_ID(ACH_KING_OF_CASTLE, "King of my Castle")
};
#undef _ACH_ID

// CAchievementManager
//

CAchievementManager::CAchievementManager()
{
}

CAchievementManager::~CAchievementManager()
{
}

bool CAchievementManager::RequestStats()
{
#ifdef ENABLE_STEAM
	return mSteamAchievements->RequestStats();
#endif
#ifdef ENABLE_GALAXY
	//requests achievements AND stats
	mGalaxyStatsAndAchievements->RequestAchievements();
	return true;
#endif

}

bool CAchievementManager::SetStat(EGameStats eStat, float fValue)
{
	bool bRetVal = true;
	SGameStat &stat = g_Stats[eStat];
	switch (stat.m_eStatType)
	{
		case STAT_INT:
			stat.m_iValue = (int)(fValue);
#ifdef ENABLE_STEAM
			bRetVal = mSteamAchievements->SaveStat(stat);
#endif
#ifdef ENABLE_GALAXY
			bRetVal = mGalaxyStatsAndAchievements->SaveStat(stat);
#endif
			break;

		case STAT_FLOAT:
			stat.m_flValue = fValue;
#ifdef ENABLE_STEAM
			bRetVal = mSteamAchievements->SaveStat(stat);
#endif
#ifdef ENABLE_GALAXY
			bRetVal = mGalaxyStatsAndAchievements->SaveStat(stat);
#endif
			break;

		case STAT_AVGRATE:
			break;
		default:
			break;
	}

	return bRetVal;
}

bool CAchievementManager::IncStat(EGameStats eStat, float fAddValue)
{
	bool bRetVal = true;
	SGameStat &stat = g_Stats[eStat];
	switch (stat.m_eStatType)
	{
		case STAT_INT:
			stat.m_iValue += (int)(fAddValue);
#ifdef ENABLE_STEAM
			bRetVal = mSteamAchievements->SaveStat(stat);
#endif
#ifdef ENABLE_GALAXY
			bRetVal = mGalaxyStatsAndAchievements->SaveStat(stat);
#endif
			break;

		case STAT_FLOAT:
			stat.m_flValue += fAddValue;
#ifdef ENABLE_STEAM
			bRetVal = mSteamAchievements->SaveStat(stat);
#endif
#ifdef ENABLE_GALAXY
			bRetVal = mGalaxyStatsAndAchievements->SaveStat(stat);
#endif
			break;

		case STAT_AVGRATE:
			break;
		default:
			break;
	}

	return bRetVal;
}

void CAchievementManager::Init()
{
#ifdef ENABLE_STEAM
	mSteamAchievements = new CSteamAchievements(g_Stats, N_STATS_CNT, g_Achievements, ACH_ACHIEVEMENTS_CNT);
#endif // ENABLE_STEAM

#ifdef ENABLE_GALAXY
	mGalaxyStatsAndAchievements = new CGalaxyStatsAndAchievements();
#endif // ENABLE_GALAXY

#ifdef ENABLE_GALAXY
	mGalaxyStatsAndAchievements->RequestAchievements();
#endif
}

void CAchievementManager::Release()
{
#ifdef ENABLE_STEAM
	delete mSteamAchievements;
#endif // ENABLE_STEAM

#ifdef ENABLE_GALAXY
	delete mGalaxyStatsAndAchievements;
#endif // ENABLE_GALAXY
}

void CAchievementManager::UnlockAchievement( EGameAchievements achievementId )
{
	for( int i = 0; i < ACH_ACHIEVEMENTS_CNT; i++ )
	{
		if( g_Achievements[ i ].m_eAchievementID == achievementId )
		{
			UnlockAchievement( g_Achievements[ i ], i );
			break;
		}
	}
}

//#TODO: unlock achievements should work by index nor by id
void CAchievementManager::UnlockAchievement( SGameAchievement& achievement, int index )
{
	//already unlocked
	if (achievement.m_bAchieved)
		return;

	achievement.m_bAchieved = true;
	(void)index; // to supress warning related to unused var index

#ifdef ENABLE_STEAM
	mSteamAchievements->UnlockAchievement( achievement );
#endif
#ifdef ENABLE_GALAXY
	mGalaxyStatsAndAchievements->UnlockAchievement(achievement);
#endif
}

bool CAchievementManager::IsAchievementUnlocked( EGameAchievements achievementId )
{
	for (int i = 0; i < ACH_ACHIEVEMENTS_CNT; i++)
	{
		if( g_Achievements[ i ].m_eAchievementID == achievementId )
			return g_Achievements[ i ].m_bAchieved;
	}
	return false;
}

void CAchievementManager::StoreStats()
{
#ifdef ENABLE_STEAM
	mSteamAchievements->StoreStatsNow();
#endif
#ifdef ENABLE_GALAXY
	mGalaxyStatsAndAchievements->StoreStatsNow();
#endif
}

void CAchievementManager::Update(float dTime)
{
#ifdef ENABLE_STEAM
	mSteamAchievements->Update(dTime);
#endif // ENABLE_STEAM

#ifdef ENABLE_GALAXY
	mGalaxyStatsAndAchievements->Update();
#endif
}

///**************************************************************************************
/// Sigleton de acces
///**************************************************************************************

CAchievementManager& UTGetAchievementManager()
{
	static CAchievementManager g_AchievementManager;
	return g_AchievementManager;
}
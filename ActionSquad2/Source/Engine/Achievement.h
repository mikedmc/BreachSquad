#pragma once

///----------------------------------------
///--- ACHIEVEMENTS 
///----------------------------------------

//vezi https://docs.google.com/spreadsheets/d/1iCXD9fDxX-D17nYVSOdnWzYZ3GanZ8YGNzKZDOCYyeo/edit#gid=1303033964
// these need to match the ones from steam partner website 1-1 !!! same name !
enum EGameAchievements
{
	ACH_HEAT_UP_THE_NIGHT = 0,
	ACH_TWO_IS_A_PARTY = 1,
	ACH_GANGLAND_PACIFIER,
	ACH_RADICALIZED,
	ACH_KETCHUP,
	ACH_3STARS_MISSION,
	ACH_CLEANUP_THE_HOOD,
	ACH_NO_QUARTER,
	ACH_NO_MERCY,
	ACH_NO_RESPITE,
	ACH_THE_THIN_BLUE_LINE,
	ACH_EOD,
	ACH_DOOR_KICKER,
	ACH_DOOR_HATER,
	ACH_ATTENTION_GETTER,
	ACH_I_LIKE_THEM_NUMB,
	ACH_GOOD_BREACH,
	ACH_NINJA,
	ACH_EFFICIENT,
	ACH_CLEANSE_BY_FIRE,
	ACH_TROUBLEMAKERS_ARRIVING,
	ACH_THINGS_HEATING_UP,
	ACH_WE_STAND_ON_GUARD,
	ACH_CAVALRY_IS_HERE,
	ACH_BUILDING_COMMUNITY,
	ACH_SLACKER,
	ACH_ALERT_EXTINGUISHED,
	ACH_DARWIN_AWARD,
	ACH_BAD_EYESIGHT,
	ACH_TERMINATOR,
	ACH_STAY_WITH_ME,
	ACH_COMPLETIONIST,
	ACH_LOOKS_INTERESTING,
	ACH_PEST_CONTROL,
	ACH_NICK_OF_TIME,
	ACH_BITE_THE_APPLE,
	ACH_NOT_IN_MY_CITY,
	ACH_METRO_CALLING,
	ACH_WARRANT_SERVER,
	ACH_NOT_ON_MY_WATCH,
	ACH_CARTEL_ANNIHILATOR,
	ACH_HELL_IS_COMING,
	ACH_DEATH_DEALER,
	ACH_BUCKSHOT_SEASON,
	ACH_LADY_JUSTICE,
	ACH_I_SEE_THINGS,
	ACH_FEDERAL_AUTHORITY,
	ACH_GRUMPY_MCANGRY,
	ACH_GOING_HOME,
	ACH_COURT_IS_CLEAR,
	ACH_KING_OF_CASTLE,
	//last one counts them all
	ACH_ACHIEVEMENTS_CNT
};

struct SGameAchievement
{
	EGameAchievements	m_eAchievementID;
	const char*			m_pchAchievementID;
	char				m_rgchName[128];
	char				m_rgchDescription[256];
	bool				m_bAchieved;
};

///----------------------------------------
///--- STATS 
///----------------------------------------

enum EStatTypes
{
	STAT_INT = 0,
	STAT_FLOAT = 1,
	STAT_AVGRATE = 2,
};

struct SGameStat
{
	int m_ID;
	EStatTypes m_eStatType;
	const char *m_pchStatName;
	int m_iValue;
	float m_flValue;
	float m_flAvgNumerator;
	float m_flAvgDenominator;
};


//indexes into g_Stats[]
enum EGameStats
{
	N_STAT_KILLED_ENEMIES = 0,
	N_STAT_SAVED_HOSTAGES = 1,
	N_STAT_ARRESTS_MADE,
	N_STAT_SECRET_ITEMS,
	N_STAT_SAVED_POLICEMEN,
	N_STAT_BOMBS_DISARMED,
	N_STAT_DOORS_KICKED,
	N_STAT_DOORS_EXPLODED,
	N_STAT_STUNNED_ENEMIES,
	N_STAT_SNIPED_ENEMIES,
	N_STAT_FLAMED_ENEMIES,
	N_STAT_ASSAULTER_KILLS,
	N_STAT_BREACHER_KILLS,
	N_STAT_SHIELD_KILLS,
	N_STAT_FBI_KILLS,
	N_STAT_RECON_KILLS,
	N_STAT_OFFDUTY_KILLS,
	//number of stats
	N_STATS_CNT
};

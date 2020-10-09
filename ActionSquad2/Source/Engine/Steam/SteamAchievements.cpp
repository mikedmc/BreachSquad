#include "dxstdafx.h"

#ifdef ENABLE_STEAM

CSteamAchievements::CSteamAchievements(SGameStat* pStatsArr, int nStatsCnt, SGameAchievement* pAchArr, int nAchCnt)
	: m_pSteamUser( SteamUser() )
	, m_pSteamUserStats( SteamUserStats() )
	, m_GameID( SteamUtils()->GetAppID() )
	, m_CallbackUserStatsReceived( this, &CSteamAchievements::OnUserStatsReceived )
	, m_CallbackUserStatsStored( this, &CSteamAchievements::OnUserStatsStored )
	, m_CallbackAchievementStored( this, &CSteamAchievements::OnAchievementStored )
{
	m_bRequestedStats = false;
	m_bStatsValid = false;
	m_bStoreStats = false;
	m_fStoreStatsTimer = 0.0f;
	//save stats array ptr
	pStatsArray = pStatsArr;
	m_nStatsCount = nStatsCnt;
	//save achievements array ptr
	pAchievementsArray = pAchArr;
	m_nAchievementsCount = nAchCnt;
}

CSteamAchievements::~CSteamAchievements()
{																																   
}

void CSteamAchievements::Update(float dTime)
{
	if (!m_bRequestedStats)
	{
		bool bReqOk = RequestStats();
		//we requested the stats
		m_bRequestedStats = true;
	}
	// Store stats
	StoreStatsIfNecessary(dTime);
}

void CSteamAchievements::UnlockAchievement( SGameAchievement& achievement )
{
	// mark it down
	m_pSteamUserStats->SetAchievement(achievement.m_pchAchievementID);

	// Store stats end of frame
	m_bStoreStats = true;
	//store stats now:
	m_fStoreStatsTimer = K_SA_STATS_SAVE_WAIT_PERIOD - EPS;
}

bool CSteamAchievements::SaveStat(SGameStat& stat)
{
	bool bretVal = true;

	switch (stat.m_eStatType)
	{
		case STAT_INT:
			bretVal = SteamUserStats()->SetStat(stat.m_pchStatName, stat.m_iValue);
			break;

		case STAT_FLOAT:
			bretVal = SteamUserStats()->SetStat(stat.m_pchStatName, stat.m_flValue);
			break;

		case STAT_AVGRATE:
			bretVal = SteamUserStats()->UpdateAvgRateStat(stat.m_pchStatName, stat.m_flAvgNumerator, stat.m_flAvgDenominator);
			// The averaged result is calculated for us
			bretVal = SteamUserStats()->GetStat(stat.m_pchStatName, &stat.m_flValue);
			break;

		default:
			break;
	}	

	// Store stats end of frame
	m_bStoreStats = true;

	return bretVal;
}

bool CSteamAchievements::RequestStats()
{
	// Is Steam loaded? If not we can't get stats.
	if (NULL == SteamUserStats() || NULL == SteamUser())
	{
		LOG(L"[WARNING]RequestStats::Steam not loaded!");
		return false;
	}
	// Is the user logged on?  If not we can't get stats.
	if (!SteamUser()->BLoggedOn())
	{
		LOG(L"[WARNING]RequestStats::Couldn't request stats! User not logged on!");
		return false;
	}
	// Request user stats.
	return SteamUserStats()->RequestCurrentStats();
}

void CSteamAchievements::StoreStatsIfNecessary(float dTime)
{
	if (m_bStoreStats)
	{
		m_fStoreStatsTimer += dTime;
	}
	//save only once every N seconds
	if (( m_fStoreStatsTimer >= K_SA_STATS_SAVE_WAIT_PERIOD ) && (m_fStoreStatsTimer - dTime < K_SA_STATS_SAVE_WAIT_PERIOD))
	{
		//reset timer
		m_fStoreStatsTimer = 0.0f;
		// already set any achievements in UnlockAchievement
		bool bSuccess = m_pSteamUserStats->StoreStats();

		// If this failed, we never sent anything to the server, try again later.
		m_bStoreStats = !bSuccess;
	}
}

void CSteamAchievements::StoreStatsNow()
{
	// already set any achievements in UnlockAchievement
	bool bSuccess = m_pSteamUserStats->StoreStats();

	// If this failed, we never sent anything to the server, try again later.
	m_bStoreStats = !bSuccess;
}

//****************************************************************************************
//
// Steam Callbacks
//
//****************************************************************************************

void CSteamAchievements::OnUserStatsReceived( UserStatsReceived_t* pCallback )
{
	if ( !m_pSteamUserStats )
		return;

	// we may get callbacks for other games' stats arriving, ignore them
	if ( m_GameID.ToUint64() == pCallback->m_nGameID )
	{
		if ( k_EResultOK == pCallback->m_eResult )
		{
			LOG(L"[Achievements] Received stats and achievements from Steam !\n");

			// load stats
			//

			for (int iStat = 0; iStat < m_nStatsCount; ++iStat)
			{
				SGameStat &stat = pStatsArray[iStat];
				switch (stat.m_eStatType)
				{
					case STAT_INT:
						SteamUserStats()->GetStat(stat.m_pchStatName, &stat.m_iValue);
#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
						LOG("[Steam Stats]received [%s]=%d", stat.m_pchStatName, stat.m_iValue);
#endif
						break;

					case STAT_FLOAT:
					case STAT_AVGRATE:
						SteamUserStats()->GetStat(stat.m_pchStatName, &stat.m_flValue);
#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
						LOG("[Steam Stats]received [%s]=%.2f", stat.m_pchStatName, stat.m_flValue);
#endif
						break;

					default:
						break;
				}
			}

			// check if online stats version is different from the local version, and if so - reset local stats / achievements
			//int statsVersion;
			//m_pSteamUserStats->GetStat( "stat_version", &statsVersion );
			//if( statsVersion != VERSION_STATS )
			//	SteamUserStats()->ResetAllStats( true );


			// load achievements
			for ( int iAch = 0; iAch < m_nAchievementsCount; iAch++ )
			{
				SGameAchievement &ach = pAchievementsArray[iAch];
				m_pSteamUserStats->GetAchievement( ach.m_pchAchievementID, &ach.m_bAchieved );
#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
				LOG("[Achievements]received [%s] achieved:%d", ach.m_rgchName, ach.m_bAchieved);
#endif
				//uncomment only if needed:
				//sprintf( ach.m_rgchName, "%s", 
				//	m_pSteamUserStats->GetAchievementDisplayAttribute( ach.m_pchAchievementID, "name" ) );
				//sprintf( ach.m_rgchDescription, "%s", 
				//	m_pSteamUserStats->GetAchievementDisplayAttribute( ach.m_pchAchievementID, "desc" ) );			
			}

			//stats were loaded
			m_bStatsValid = true;
		}
	}

}

//****************************************************************************************

void CSteamAchievements::OnUserStatsStored( UserStatsStored_t *pCallback )
{
	// we may get callbacks for other games' stats arriving, ignore them
	if ( m_GameID.ToUint64() == pCallback->m_nGameID )
	{
		if ( k_EResultOK == pCallback->m_eResult )
		{
			LOG(L"[Achievements] StoreStats - succes !\n");
		}
		else if ( k_EResultInvalidParam == pCallback->m_eResult )
		{
			// One or more stats we set broke a constraint. They've been reverted,
			// and we should re-iterate the values now to keep in sync.
			LOG(L"[Achievements] StoreStats - some failed to validate !\n");

			// Fake up a callback here so that we re-load the values.
			UserStatsReceived_t callback;
			callback.m_eResult = k_EResultOK;
			callback.m_nGameID = m_GameID.ToUint64();
			OnUserStatsReceived( &callback );
		}
		else
		{
			LOG(L"[Achievements] StoreStats - failed, %d\n", pCallback->m_eResult);
		}
	}
}

//****************************************************************************************

void CSteamAchievements::OnAchievementStored( UserAchievementStored_t *pCallback )
{
	// we may get callbacks for other games' stats arriving, ignore them
	if ( m_GameID.ToUint64() == pCallback->m_nGameID )
	{
		if ( 0 == pCallback->m_nMaxProgress )
		{
			LOG("[Achievements] Achievement '%s' unlocked!\n", pCallback->m_rgchAchievementName );
		}
		else
		{
			LOG("[Achievements] Achievement '%s' progress callback, (%d,%d)\n", 
				pCallback->m_rgchAchievementName, pCallback->m_nCurProgress, pCallback->m_nMaxProgress );
		}
	}
}

#endif // ENABLE_STEAM
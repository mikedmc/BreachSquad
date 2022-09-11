#include "dxstdafx.h"

#ifdef ENABLE_STEAM

void CSteamLeaderboards::Init()
{
	m_eStatus = K_JOBSTATUS_EMPTY;
	m_eExecutingJobType = K_JOB_NONE;

	m_currentLeaderboard = NO_LEADERBOARD;
	m_nPlayerScore = 0;
	memset(m_pchCurrentLeaderboardName, 0, sizeof(char) * MAX_PATH);
}

void CSteamLeaderboards::Release()
{

}

/* 
 * \brief Needs to be called every frame 
 */
ELBJobStatus CSteamLeaderboards::Update(float dTime)
{
	//take jobs one by one
	if ((m_eStatus != K_JOBSTATUS_BUSY) && (m_arrJobs.GetSize() > 0))
	{
		CJob* pJob = m_arrJobs[0];
		switch (pJob->m_eJobType)
		{
			case K_JOB_INITIALIZE:
			{
				LOG_DBG(L"Leaderboards::Initialize!");
			}
			break;
			case K_JOB_CHANGE_LEADERBOARD:
			{
				FindOrCreateLeaderboard(pJob->m_pchLeaderboardName);
			}
			break;
			case K_JOB_UPLOAD_SCORE:
			{
				if (!IsLeaderboardActive(pJob->m_pchLeaderboardName))
				{
					InsertJob(0, K_JOB_CHANGE_LEADERBOARD, pJob->m_pchLeaderboardName);
					return K_JOBSTATUS_BUSY;
				}
				//correct leaderboard active, upload
				UploadScore(pJob->m_iPayload);
			}
			break;
			case K_JOB_GET_SCORES_AROUND_USER:
			{
				if (!IsLeaderboardActive(pJob->m_pchLeaderboardName))
				{
					InsertJob(0, K_JOB_CHANGE_LEADERBOARD, pJob->m_pchLeaderboardName);
					return K_JOBSTATUS_BUSY;
				}
				//erase old scores
				ResetScoresList();
				//correct leaderboard active:
				DownloadScoresAroundUser();
			}
			break;
			case K_JOB_GET_SCORES_FROM_FRIENDS:
			{
				if (!IsLeaderboardActive(pJob->m_pchLeaderboardName))
				{
					InsertJob(0, K_JOB_CHANGE_LEADERBOARD, pJob->m_pchLeaderboardName);
					return K_JOBSTATUS_BUSY;
				}
				//erase old scores
				ResetScoresList();
				//correct leaderboard active:
				DownloadScoresForFriends();
			}
			break;
			case K_JOB_GET_SCORES_GLOBAL:
			{
				if (!IsLeaderboardActive(pJob->m_pchLeaderboardName))
				{
					InsertJob(0, K_JOB_CHANGE_LEADERBOARD, pJob->m_pchLeaderboardName);
					return K_JOBSTATUS_BUSY;
				}
				//erase old scores
				ResetScoresList();
				//use payload to specify starting index
				DownloadScoresGlobal(pJob->m_iPayload);
			}
			break;
			case K_JOB_GET_SCORE_FOR_CURRENT_USER:
			{
				if (!IsLeaderboardActive(pJob->m_pchLeaderboardName))
				{
					InsertJob(0, K_JOB_CHANGE_LEADERBOARD, pJob->m_pchLeaderboardName);
					return K_JOBSTATUS_BUSY;
				}
				//correct leaderboard active, upload
				DownloadScoreForCurrentUser();
			}
			break;
			default:
			{
				LOG(L"[Warning] Leaderboards::Update: Unknown Job request!");
			}
			break;
		}
		//delete job
		SAFE_DELETE(m_arrJobs[0]);
		m_arrJobs.Remove(0);
	}
	
	if (m_eStatus == K_JOBSTATUS_JUST_FINISHED)
	{
		m_eStatus = K_JOBSTATUS_IDLE;
		return K_JOBSTATUS_JUST_FINISHED;
	}
		  
	return m_eStatus;
}

/* 
 * \brief Queues a job request
 */
bool CSteamLeaderboards::QueueJob(ELBJobType nJobRequest, const char * pchLeaderboardName, int iPayload /*= 0*/)
{
	if ((nJobRequest == K_JOB_NONE) || (pchLeaderboardName == null) || (strlen(pchLeaderboardName) <= 0))
	{
		LOG( L"[Warning] Leaderboards:: QueueJob - invalid args" );
		return false;
	}

	//make sure MODDERS don't upload scores
	if ((nJobRequest == K_JOB_UPLOAD_SCORE) && (UTApp().IsGameModified()))
	{
		LOG(L"[Warning] Leaderboards:: can't upload scores to leaderboards when game files are modified !");
		return false;
	}

	CJob * pJob = new CJob();
	pJob->m_iPayload = iPayload;
	pJob->m_eJobType = nJobRequest;
	strcpy(pJob->m_pchLeaderboardName, pchLeaderboardName);

	m_arrJobs.Add(pJob);

	return true;
}

/*
 * \brief Inserts a job on a given position
 */
bool CSteamLeaderboards::InsertJob(int nInsertIndex, ELBJobType nJobRequest, const char * pchLeaderboardName, int iPayload /*= 0*/)
{
	if ((nJobRequest == K_JOB_NONE) || (pchLeaderboardName == null) || (strlen(pchLeaderboardName) <= 0))
	{
		LOG(L"[Error] Leaderboards:: InsertJob - invalid args!");
		return false;
	}
	if ((nInsertIndex < 0) || (nInsertIndex >= m_arrJobs.GetSize()))
	{
		LOG(L"[Error] Leaderboards:: InsertJob - invalid index!");
		return false;
	}

	CJob * pJob = new CJob();
	pJob->m_iPayload = iPayload;
	pJob->m_eJobType = nJobRequest;
	strcpy(pJob->m_pchLeaderboardName, pchLeaderboardName);

	m_arrJobs.Insert(nInsertIndex, pJob);

	return true;
}

/*
 * Resets last downloaded scores
 */
void CSteamLeaderboards::ResetScoresList()
{
	m_nPlayerScore = 0;
	//save data in readable structures
	m_scoresList.m_nScoresCnt = 0;
	m_scoresList.m_nPlayerScoreIndex = -1;
	//save job type and last leaderboard name
	m_scoresList.request_eJobType = K_JOB_NONE;
	memset(m_scoresList.request_pchLeaderboardName, 0, sizeof(m_scoresList.request_pchLeaderboardName));

	for (int index = 0; index < NUM_USERS_TO_DOWNLOAD; index++)
	{
		m_scoresList.m_arrScores[index] = -1;
		m_scoresList.m_arrRank[index] = -1;
		sprintf(m_scoresList.m_arrNames[index], "-"); //empty names
	}
}

/* 
 * \brief Fills the provided structures with usernames and scores 
 * \returns -1 when not ready or number of names in list
 * \param eJobRequested - contains the job that requested the current scores (command and leaderboard name)
 */
int CSteamLeaderboards::GetDownloadedScores(CScoresList * pDestList)
{
	if (pDestList == null)
		return -1;
	if (m_eExecutingJobType != K_JOB_NONE)
		return -1;
	//copy data
	memcpy(pDestList, &m_scoresList, sizeof(m_scoresList));

	return m_scoresList.m_nScoresCnt;
}

int CSteamLeaderboards::GetDownloadedScores_PlayerIndex()
{
	return m_scoresList.m_nPlayerScoreIndex;
}

int CSteamLeaderboards::GetUserScore()
{
	return m_nPlayerScore;
}

int CSteamLeaderboards::GetDownloadedScoresCount()
{
	return m_scoresList.m_nScoresCnt;
}

bool CSteamLeaderboards::IsBusy()
{
	return (m_eStatus == K_JOBSTATUS_BUSY);
}

CSteamLeaderboards::CSteamLeaderboards() :
	m_CallbackPersonaStateChange(this, &CSteamLeaderboards::OnPersonaStateChange)
{
	m_eStatus = K_JOBSTATUS_EMPTY;
	m_eExecutingJobType = K_JOB_NONE;

	m_currentLeaderboard = NO_LEADERBOARD;
	memset(m_pchCurrentLeaderboardName, 0, sizeof(char) * MAX_PATH);
}

CSteamLeaderboards::~CSteamLeaderboards()
{
	SAFE_DELETE_GROWABLE_ARRAY(m_arrJobs);
}

void CSteamLeaderboards::UpdateScoresListFromLeaderboardEntries()
{
	//save data in readable structures
	m_scoresList.m_nScoresCnt = m_nLeaderboardEntries;
	m_scoresList.m_nPlayerScoreIndex = -1;
	//save job type and last leaderboard name
	m_scoresList.request_eJobType = m_eExecutingJobType;
	strncpy(m_scoresList.request_pchLeaderboardName, m_pchCurrentLeaderboardName, sizeof(m_scoresList.request_pchLeaderboardName));

	CSteamID m_steamIDLocalUser = SteamUser()->GetSteamID();

	for (int index = 0; index < m_nLeaderboardEntries; index++)
	{
		///rank
		m_scoresList.m_arrRank[index] = m_leaderboardEntries[index].m_nGlobalRank;
		///score:
		m_scoresList.m_arrScores[index] = m_leaderboardEntries[index].m_nScore;
		///name:
		// we get the details of a user from the ISteamFriends interface
		const char *pchName = SteamFriends()->GetFriendPersonaName(m_leaderboardEntries[index].m_steamIDUser);
		// we may not know the name of the other users in the lobby immediately; but we'll receive
		// a PersonaStateUpdate_t callback when they do, and we'll rebuild the list then
		if (pchName && *pchName)
		{
			strncpy(m_scoresList.m_arrNames[index], pchName, sizeof(m_scoresList.m_arrNames[index]));
		}
		else
		{
			strncpy(m_scoresList.m_arrNames[index], "-", sizeof(m_scoresList.m_arrNames[index]));
		}

		//LOG("Leadeboards:: %d.[%s] score:%d", m_scoresList.m_arrRank[index], m_scoresList.m_arrNames[index], m_scoresList.m_arrScores[index]);
		//find local player score index
		if (m_steamIDLocalUser == m_leaderboardEntries[index].m_steamIDUser)
		{
			m_scoresList.m_nPlayerScoreIndex = index;
			//save current player score too
			m_nPlayerScore = m_leaderboardEntries[index].m_nScore;
			//LOG(L"Leadeboards:: Local player index: %d", index);
		}
	}
}

void CSteamLeaderboards::FindOrCreateLeaderboard( const char *pchLeaderboardName )
{
	if (strcmp(pchLeaderboardName, m_pchCurrentLeaderboardName) == 0)
		return;
	
	m_eStatus = K_JOBSTATUS_BUSY;
	m_eExecutingJobType = K_JOB_CHANGE_LEADERBOARD;
	m_currentLeaderboard = NULL;
	//save name too
	memset(m_pchCurrentLeaderboardName, 0, sizeof(char) * MAX_PATH);
	strncpy(m_pchCurrentLeaderboardName, pchLeaderboardName, sizeof(m_pchCurrentLeaderboardName));

	SteamAPICall_t hSteamAPICall = 
		SteamUserStats()->FindOrCreateLeaderboard( pchLeaderboardName, 
			ELeaderboardSortMethod::k_ELeaderboardSortMethodDescending, ELeaderboardDisplayType::k_ELeaderboardDisplayTypeNumeric );

	m_callResultFindLeaderboard.Set( hSteamAPICall, this, &CSteamLeaderboards::OnFindOrCreateLeaderboard );

#if defined(_DEBUG) || defined(DEBUG)
	LOG("Leaderboard switched: %s", pchLeaderboardName);
#endif
}

bool CSteamLeaderboards::UploadScore( int iScore )
{
	if( !m_currentLeaderboard )
		return false;

	m_eStatus = K_JOBSTATUS_BUSY;
	m_eExecutingJobType = K_JOB_UPLOAD_SCORE;

	SteamAPICall_t hSteamAPICall =
		SteamUserStats()->UploadLeaderboardScore( m_currentLeaderboard, k_ELeaderboardUploadScoreMethodKeepBest, iScore, NULL, 0 );

	m_callResultUploadScore.Set( hSteamAPICall, this, &CSteamLeaderboards::OnUploadScore );

	return true;
}

bool CSteamLeaderboards::DownloadScoresAroundUser()
{
	if( !m_currentLeaderboard )
	{
		LOG( L"[Error] Leaderboards:: No leaderboard to work on !" );
		return false;
	}

	if( m_eStatus == K_JOBSTATUS_BUSY )
	{
		LOG( L"[Error] Leaderboards:: Download in progress !" );
		return false;
	}

	m_eStatus = K_JOBSTATUS_BUSY;
	m_eExecutingJobType = K_JOB_GET_SCORES_AROUND_USER;

	// load the specified leaderboard data around the current user
	SteamAPICall_t hSteamAPICall =
		SteamUserStats()->DownloadLeaderboardEntries( m_currentLeaderboard, k_ELeaderboardDataRequestGlobalAroundUser, 
		-( ( NUM_USERS_TO_DOWNLOAD /2 ) -1 ), ( NUM_USERS_TO_DOWNLOAD /2 ) ); // range: [ -( half_max -1 ), me, half_max ] = NUM_USERS_TO_DOWNLOAD

	m_callResultDownloadScore.Set( hSteamAPICall, this, &CSteamLeaderboards::OnDownloadScore );
	return true;
}

bool CSteamLeaderboards::DownloadScoresForFriends()
{
	if( !m_currentLeaderboard )
	{
		LOG( L"[Error] Leaderboards:: No leaderboard to work on !" );
		return false;
	}

	if( m_eStatus == K_JOBSTATUS_BUSY )
	{
		LOG( L"[Error] Leaderboards:: Download already in progress !" );
		return false;
	}

	m_eStatus = K_JOBSTATUS_BUSY;
	m_eExecutingJobType = K_JOB_GET_SCORES_FROM_FRIENDS;

	// load the specified leaderboard data around the current user
	SteamAPICall_t hSteamAPICall =
		SteamUserStats()->DownloadLeaderboardEntries( m_currentLeaderboard, k_ELeaderboardDataRequestFriends, 
			1, NUM_USERS_TO_DOWNLOAD ); // range: [ 0, NUM_USERS_TO_DOWNLOAD ] = NUM_USERS_TO_DOWNLOAD

	m_callResultDownloadScore.Set( hSteamAPICall, this, &CSteamLeaderboards::OnDownloadScore );
	return true;
}

bool CSteamLeaderboards::DownloadScoresGlobal(int nStartIdx)
{
	if (!m_currentLeaderboard)
	{
		LOG(L"[Error] Leaderboards:: No leaderboard to work on !");
		return false;
	}

	if (m_eStatus == K_JOBSTATUS_BUSY)
	{
		LOG(L"[Error] Leaderboards:: Download already in progress !");
		return false;
	}

	m_eStatus = K_JOBSTATUS_BUSY;
	m_eExecutingJobType = K_JOB_GET_SCORES_GLOBAL;

	// load the specified leaderboard data around the current user
	SteamAPICall_t hSteamAPICall =
		SteamUserStats()->DownloadLeaderboardEntries(m_currentLeaderboard, k_ELeaderboardDataRequestGlobal,
			nStartIdx, nStartIdx + NUM_USERS_TO_DOWNLOAD - 1); // range: [ 0, NUM_USERS_TO_DOWNLOAD ] = NUM_USERS_TO_DOWNLOAD

	m_callResultDownloadScore.Set(hSteamAPICall, this, &CSteamLeaderboards::OnDownloadScore);
	return true;
}

bool CSteamLeaderboards::DownloadScoreForCurrentUser()
{
	if (!m_currentLeaderboard)
	{
		LOG(L"[Error] Leaderboards:: No leaderboard to work on !");
		return false;
	}

	if (m_eStatus == K_JOBSTATUS_BUSY)
	{
		LOG(L"[Error] Leaderboards:: Download in progress !");
		return false;
	}

	m_eStatus = K_JOBSTATUS_BUSY;
	m_eExecutingJobType = K_JOB_GET_SCORE_FOR_CURRENT_USER;

	// load the specified leaderboard data around the current user
	CSteamID arrUsers[10];
	arrUsers[0] = SteamUser()->GetSteamID();

	SteamAPICall_t hSteamAPICall =
		SteamUserStats()->DownloadLeaderboardEntriesForUsers(m_currentLeaderboard, arrUsers, 1);

	m_callResultDownloadScore.Set(hSteamAPICall, this, &CSteamLeaderboards::OnDownloadScore);
	return true;
}

/*
 * Tells us if specified leaderboard is active now
 */
bool CSteamLeaderboards::IsLeaderboardActive(const char *pchLeaderboardName)
{
	if (pchLeaderboardName == NULL)
		return false;
	
	if (strcmp(pchLeaderboardName, m_pchCurrentLeaderboardName) == 0)
		return true;

	return false;
}

void CSteamLeaderboards::OnFindOrCreateLeaderboard( LeaderboardFindResult_t *pCallback, bool bIOFailure)
{
	if( bIOFailure )
	{
		m_eStatus = K_JOBSTATUS_ERROR;
		LOG( L"[Leaderboards] OnFindOrCreateLeaderboard - IOFailure !" );
		return;
	}

	// see if we encountered an error during the call
	if ( !pCallback->m_bLeaderboardFound )
	{
		LOG( L"[Error] Leaderboard could not be found. created." );
	}

	m_currentLeaderboard = pCallback->m_hSteamLeaderboard;
	m_eStatus = K_JOBSTATUS_JUST_FINISHED;

	m_eExecutingJobType = K_JOB_NONE;
}

void CSteamLeaderboards::OnUploadScore( LeaderboardScoreUploaded_t *pCallback, bool bIOFailure)
{
	m_eStatus = K_JOBSTATUS_JUST_FINISHED;

	if ( !pCallback->m_bSuccess || bIOFailure )
	{
		LOG( L"[Error] Score could not be uploaded to Steam" );
		m_eStatus = K_JOBSTATUS_ERROR;
	}

	LOG("Leaderboards:: Uploaded score:%d in leaderboard:%s", pCallback->m_nScore, m_pchCurrentLeaderboardName);
	m_eExecutingJobType = K_JOB_NONE;
}

void CSteamLeaderboards::OnDownloadScore( LeaderboardScoresDownloaded_t *pCallback, bool bIOFailure)
{
	m_eStatus = K_JOBSTATUS_JUST_FINISHED;

	if (bIOFailure)
	{
		LOG(L"[Error] Leaderboards:: OnDownloadScore - IOFailure !");
		m_eStatus = K_JOBSTATUS_ERROR;
		m_eExecutingJobType = K_JOB_NONE;
	}


	switch (m_eExecutingJobType)
	{
		case K_JOB_GET_SCORE_FOR_CURRENT_USER:
		{
			LeaderboardEntry_t lEntry;
			if (SteamUserStats()->GetDownloadedLeaderboardEntry(pCallback->m_hSteamLeaderboardEntries, 0, &lEntry, NULL, 0))
				m_nPlayerScore = lEntry.m_nScore;
			else
				m_nPlayerScore = 0;
		}
		break;
		default:
		{
			m_nLeaderboardEntries = min(pCallback->m_cEntryCount, NUM_USERS_TO_DOWNLOAD);

			for (int index = 0; index < m_nLeaderboardEntries; index++)
			{
				SteamUserStats()->GetDownloadedLeaderboardEntry(pCallback->m_hSteamLeaderboardEntries, index, &m_leaderboardEntries[index], NULL, 0);
			}

			//try to write human readable format
			UpdateScoresListFromLeaderboardEntries();
		}
		break;
	}
	
	// Reset job type
	m_eExecutingJobType = K_JOB_NONE;
}

/*
 * Called when requested name changed or wasn't available locally
 */
void CSteamLeaderboards::OnPersonaStateChange(PersonaStateChange_t *pCallback)
{
	for (int index = 0; index < m_nLeaderboardEntries; index++)
	{
		if (m_leaderboardEntries[index].m_steamIDUser == pCallback->m_ulSteamID)
		{
			// get name again ?
			const char *pchName = SteamFriends()->GetFriendPersonaName(m_leaderboardEntries[index].m_steamIDUser);
			if (pchName && *pchName)
			{
				strncpy(m_scoresList.m_arrNames[index], pchName, sizeof(m_scoresList.m_arrNames[index]));
				//LOG("Leaderboards::Persona State Change received name %s", pchName);
				//announce names updating
				if(m_eStatus != K_JOBSTATUS_BUSY)
					m_eStatus = K_JOBSTATUS_JUST_FINISHED;
			}
			break;
		}
	}
}

///**************************************************************************************
/// Sigleton
///**************************************************************************************

CSteamLeaderboards& __Leaderboards()
{
	static CSteamLeaderboards g_SteamLeaderboards;
	return g_SteamLeaderboards;
}


#endif // ENABLE_STEAM
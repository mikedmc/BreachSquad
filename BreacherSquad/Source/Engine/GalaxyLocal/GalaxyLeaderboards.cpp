#include "dxstdafx.h"

#ifdef ENABLE_GALAXY

#include "GalaxyUtils.h"

CGalaxyLeaderboards::CGalaxyLeaderboards()
{
	m_eStatus = K_JOBSTATUS_EMPTY;
	m_eExecutingJobType = K_JOB_NONE;

	m_currentLeaderboard = NO_LEADERBOARD;
	memset(m_pchCurrentLeaderboardName, 0, sizeof(char) * MAX_PATH);
}

CGalaxyLeaderboards::~CGalaxyLeaderboards()
{
	SAFE_DELETE_GROWABLE_ARRAY(m_arrJobs);
}


void CGalaxyLeaderboards::Init()
{
	m_eStatus = K_JOBSTATUS_EMPTY;
	m_eExecutingJobType = K_JOB_NONE;
	m_nPlayerScore = 0;

	RegisterAsGalaxyListener<ILeaderboardEntriesRetrieveListener>(this);
	RegisterAsGalaxyListener<ILeaderboardRetrieveListener>(this);
	RegisterAsGalaxyListener<ILeaderboardScoreUpdateListener>(this);
	RegisterAsGalaxyListener<ILeaderboardsRetrieveListener>(this);
	RegisterAsGalaxyListener<IPersonaDataChangedListener>(this);
}

void CGalaxyLeaderboards::Release()
{
	UnregisterAsGalaxyListener<ILeaderboardEntriesRetrieveListener>(this);
	UnregisterAsGalaxyListener<ILeaderboardRetrieveListener>(this);
	UnregisterAsGalaxyListener<ILeaderboardScoreUpdateListener>(this);
	UnregisterAsGalaxyListener<ILeaderboardsRetrieveListener>(this);
	UnregisterAsGalaxyListener<IPersonaDataChangedListener>(this);
}

/* 
 * \brief Needs to be called every frame 
 */
ELBJobStatus CGalaxyLeaderboards::Update(float dTime)
{
	//take jobs one by one
	if ((m_eStatus != K_JOBSTATUS_BUSY) && (m_arrJobs.GetSize() > 0))
	{
		CJob* pJob = m_arrJobs[0];
		switch (pJob->m_eJobType)
		{
			case K_JOB_INITIALIZE:
			{
				galaxy::api::Stats()->RequestLeaderboards();

				m_eStatus = K_JOBSTATUS_BUSY;
				m_eExecutingJobType = K_JOB_INITIALIZE;
				m_currentLeaderboard = NO_LEADERBOARD;
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
				//correct leaderboard active:
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
bool CGalaxyLeaderboards::QueueJob(ELBJobType nJobRequest, const char * pchLeaderboardName, int iScore /*= 0*/)
{
	if ((nJobRequest == K_JOB_NONE) || (pchLeaderboardName == null) || (strlen(pchLeaderboardName) <= 0))
	{
		LOG( L"[Warning] Leaderboards:: QueueJob - invalid args" );
		return false;
	}

	if (!galaxy::api::User()->SignedIn())
	{
		LOG(L"[Warning] Leaderboards:: QueueJob - not logged in!");
		return false;
	}

	//make sure MODDERS don't upload scores
	if ((nJobRequest == K_JOB_UPLOAD_SCORE) && (UTApp().IsGameModified()))
	{
		LOG(L"[Warning] Leaderboards:: can't upload scores to leaderboards when game files are modified !");
		return false;
	}

	CJob * pJob = new CJob();
	pJob->m_iPayload = iScore;
	pJob->m_eJobType = nJobRequest;
	strcpy(pJob->m_pchLeaderboardName, pchLeaderboardName);

	m_arrJobs.Add(pJob);

	return true;
}

/*
 * \brief Inserts a job on a given position
 */
bool CGalaxyLeaderboards::InsertJob(int nInsertIndex, ELBJobType nJobRequest, const char * pchLeaderboardName, int iScore /*= 0*/)
{
	if ((nJobRequest == K_JOB_NONE) || (pchLeaderboardName == null) || (strlen(pchLeaderboardName) <= 0))
	{
		LOG(L"[Error] Leaderboards:: InsertJob - invalid args!");
		return false;
	}

	if (!galaxy::api::User()->SignedIn())
	{
		LOG(L"[Warning] Leaderboards:: QueueJob - not logged in!");
		return false;
	}

	if ((nInsertIndex < 0) || (nInsertIndex >= m_arrJobs.GetSize()))
	{
		LOG(L"[Error] Leaderboards:: InsertJob - invalid index!");
		return false;
	}

	CJob * pJob = new CJob();
	pJob->m_iPayload = iScore;
	pJob->m_eJobType = nJobRequest;
	strcpy(pJob->m_pchLeaderboardName, pchLeaderboardName);

	m_arrJobs.Insert(nInsertIndex, pJob);

	return true;
}

/*
 * Resets last downloaded scores
 */
void CGalaxyLeaderboards::ResetScoresList()
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
int CGalaxyLeaderboards::GetDownloadedScores(CScoresList * pDestList)
{
	if (pDestList == null)
		return -1;
	if (m_eExecutingJobType != K_JOB_NONE)
		return -1;
	//copy data
	memcpy(pDestList, &m_scoresList, sizeof(m_scoresList));

	return m_scoresList.m_nScoresCnt;
}

int CGalaxyLeaderboards::GetDownloadedScores_PlayerIndex()
{
	return m_scoresList.m_nPlayerScoreIndex;
}

int CGalaxyLeaderboards::GetUserScore()
{
	return m_nPlayerScore;
}

int CGalaxyLeaderboards::GetDownloadedScoresCount()
{
	return m_scoresList.m_nScoresCnt;
}

bool CGalaxyLeaderboards::IsBusy()
{
	return (m_eStatus == K_JOBSTATUS_BUSY);
}

void CGalaxyLeaderboards::UpdateScoresListFromLeaderboardEntries()
{
	//save data in readable structures
	m_scoresList.m_nScoresCnt = m_nLeaderboardEntries;
	m_scoresList.m_nPlayerScoreIndex = -1;
	//save job type and last leaderboard name
	m_scoresList.request_eJobType = m_eExecutingJobType;
	strncpy(m_scoresList.request_pchLeaderboardName, m_pchCurrentLeaderboardName, sizeof(m_scoresList.request_pchLeaderboardName));

	galaxy::api::GalaxyID m_IDLocalUser = galaxy::api::User()->GetGalaxyID();

	for (int index = 0; index < m_nLeaderboardEntries; index++)
	{
		///rank
		m_scoresList.m_arrRank[index] = m_leaderboardEntries[index].nRank;
		///score:
		m_scoresList.m_arrScores[index] = m_leaderboardEntries[index].nScore;
		///name:
		const char *pchName = m_leaderboardEntries[index].strName;
		// we may not know the name of the other users in the lobby immediately; but we'll receive
		// a Persona State Update callback when they do, and we'll rebuild the list then
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
		if (m_IDLocalUser == m_leaderboardEntries[index].idPlayer)
		{
			m_scoresList.m_nPlayerScoreIndex = index;
			//save current player score too
			m_nPlayerScore = m_leaderboardEntries[index].nScore;
			//LOG(L"Leaderboards:: Local player index: %d", index);
		}
	}
}

void CGalaxyLeaderboards::FindOrCreateLeaderboard( const char *pchLeaderboardName )
{
	if (strcmp(pchLeaderboardName, m_pchCurrentLeaderboardName) == 0)
		return;
	
	m_eStatus = K_JOBSTATUS_BUSY;
	m_eExecutingJobType = K_JOB_CHANGE_LEADERBOARD;
	m_currentLeaderboard = NO_LEADERBOARD;
	//save name too
	memset(m_pchCurrentLeaderboardName, 0, sizeof(char) * MAX_PATH);
	strncpy(m_pchCurrentLeaderboardName, pchLeaderboardName, sizeof(m_pchCurrentLeaderboardName));
	
	galaxy::api::Stats()->FindOrCreateLeaderboard(pchLeaderboardName, pchLeaderboardName, galaxy::api::LeaderboardSortMethod::LEADERBOARD_SORT_METHOD_DESCENDING, galaxy::api::LeaderboardDisplayType::LEADERBOARD_DISPLAY_TYPE_NUMBER);
}

bool CGalaxyLeaderboards::UploadScore( int iScore )
{
	if( m_currentLeaderboard == NO_LEADERBOARD )
		return false;

	m_eStatus = K_JOBSTATUS_BUSY;
	m_eExecutingJobType = K_JOB_UPLOAD_SCORE;

	galaxy::api::Stats()->SetLeaderboardScore(m_pchCurrentLeaderboardName, iScore);

	return true;
}

bool CGalaxyLeaderboards::DownloadScoresAroundUser()
{
	if( m_currentLeaderboard == NO_LEADERBOARD )
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
	galaxy::api::Stats()->RequestLeaderboardEntriesAroundUser(m_pchCurrentLeaderboardName, NUM_USERS_TO_DOWNLOAD / 2 - 1, NUM_USERS_TO_DOWNLOAD / 2);
	return true;
}

bool CGalaxyLeaderboards::DownloadScoresForFriends()
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

	ErrorBox(K_ERR_WARNING, L"Not implemented (not needed) !");
	return false;

	//m_eStatus = K_JOBSTATUS_BUSY;
	//m_eExecutingJobType = K_JOB_GET_SCORES_FROM_FRIENDS;

	//galaxy::api::Stats()->RequestLeaderboardEntriesForUsers...
	//return true;
}

bool CGalaxyLeaderboards::DownloadScoresGlobal(int nStartIdx)
{
	if (m_currentLeaderboard == NO_LEADERBOARD)
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
	
	//differences between steam and gog (gog starts from 0, steam from 1)
	galaxy::api::Stats()->RequestLeaderboardEntriesGlobal(m_pchCurrentLeaderboardName, nStartIdx - 1, nStartIdx - 1 + NUM_USERS_TO_DOWNLOAD - 1);
	return true;
}

bool CGalaxyLeaderboards::DownloadScoreForCurrentUser()
{
	if (m_currentLeaderboard == NO_LEADERBOARD)
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
	m_eExecutingJobType = K_JOB_GET_SCORE_FOR_CURRENT_USER;

	//differences between steam and gog (gog starts from 0, steam from 1)
	galaxy::api::GalaxyID arrUsers[10];
	arrUsers[0] = galaxy::api::User()->GetGalaxyID();
	galaxy::api::Stats()->RequestLeaderboardEntriesForUsers(m_pchCurrentLeaderboardName, arrUsers, 1);
	return true;
}

/*
 * Tells us if specified leaderboard is active now
 */
bool CGalaxyLeaderboards::IsLeaderboardActive(const char *pchLeaderboardName)
{
	if (pchLeaderboardName == NULL)
		return false;
	
	if (strcmp(pchLeaderboardName, m_pchCurrentLeaderboardName) == 0)
		return true;

	return false;
}


///--- CALLBACKS ---

void CGalaxyLeaderboards::OnLeaderboardsRetrieveSuccess()
{
	LOG("[GOG:Leaderboards] Leaderboards retrieved.");

	m_eStatus = K_JOBSTATUS_JUST_FINISHED;
	m_eExecutingJobType = K_JOB_NONE;
}

void CGalaxyLeaderboards::OnLeaderboardsRetrieveFailure(galaxy::api::ILeaderboardsRetrieveListener::FailureReason failureReason)
{
	LOG("[WARNING][GOG:Leaderboards] Could not retrieve leaderboards!");
	const galaxy::api::IError* error = galaxy::api::GetError();
	if (error)
	{
		g_pLog->Write("  Error: %s", error->GetMsg());
	}

	m_eStatus = K_JOBSTATUS_ERROR;
	m_eExecutingJobType = K_JOB_NONE;
}

void CGalaxyLeaderboards::OnLeaderboardEntriesRetrieveSuccess(const char* name, uint32_t entryCount)
{
	LOG("[GOG:Leaderboards] Entries (count:%d) retrieve success [%s]", entryCount, name);

	switch (m_eExecutingJobType)
	{
		case K_JOB_GET_SCORE_FOR_CURRENT_USER:
		{
			if (entryCount == 0)
				m_nPlayerScore = 0;
			else
			{
				uint32_t	rank;
				int32_t		score;
				galaxy::api::GalaxyID	idPlayer;
				galaxy::api::Stats()->GetRequestedLeaderboardEntry((uint32_t)0, rank, score, idPlayer);
				m_nPlayerScore = score;
			}
		}
		break;
		default:
		{
			m_nLeaderboardEntries = entryCount;

			// Read actual entries
			for (int kk = 0; kk < entryCount; kk++)
			{
				uint32_t	rank;
				int32_t		score;
				galaxy::api::GalaxyID	idPlayer;
				galaxy::api::Stats()->GetRequestedLeaderboardEntry((uint32_t)kk, rank, score, idPlayer);
				//request info that will be used later (persona info update)
				galaxy::api::Friends()->RequestUserInformation(idPlayer);
				//save leaderboard entry
				m_leaderboardEntries[kk].nRank = rank;
				m_leaderboardEntries[kk].nScore = score;
				m_leaderboardEntries[kk].idPlayer = idPlayer;
				m_leaderboardEntries[kk].strName[0] = 0;
			}

			// Try to write human readable format
			UpdateScoresListFromLeaderboardEntries();
		}
		break;
	}

	m_eStatus = K_JOBSTATUS_JUST_FINISHED;
	m_eExecutingJobType = K_JOB_NONE;
}

void CGalaxyLeaderboards::OnLeaderboardEntriesRetrieveFailure(const char* name, galaxy::api::ILeaderboardEntriesRetrieveListener::FailureReason failureReason)
{
	LOG("[WARNING][GOG:Leaderboards] Could not read entries for [%s]", name);
	const galaxy::api::IError* error = galaxy::api::GetError();
	if (error)
	{
		g_pLog->Write("  Error: %s", error->GetMsg());
	}

	m_nLeaderboardEntries = 0;

	m_eStatus = K_JOBSTATUS_ERROR;
	m_eExecutingJobType = K_JOB_NONE;
}

void CGalaxyLeaderboards::OnLeaderboardRetrieveSuccess(const char* name)
{
	LOG("[GOG:Leaderboards] Retrieve success [%s]", name);
	m_currentLeaderboard = FastHash(name);

	m_eStatus = K_JOBSTATUS_JUST_FINISHED;
	m_eExecutingJobType = K_JOB_NONE;
}

void CGalaxyLeaderboards::OnLeaderboardRetrieveFailure(const char* name, galaxy::api::ILeaderboardRetrieveListener::FailureReason failureReason)
{
	LOG("[WARNING][GOG:Leaderboards] retrieve failed [%s]!", name);
	const galaxy::api::IError* error = galaxy::api::GetError();
	if (error)
	{
		g_pLog->Write("  Error: %s", error->GetMsg());
	}
	
	m_currentLeaderboard = NO_LEADERBOARD;

	m_eStatus = K_JOBSTATUS_ERROR;
	m_eExecutingJobType = K_JOB_NONE;
}


void CGalaxyLeaderboards::OnLeaderboardScoreUpdateSuccess(const char* name, int32_t score, uint32_t oldRank, uint32_t newRank)
{
	LOG("[GOG:Leaderboards] Uploaded score:%d oldRank:%d newRank:%d in leaderboard:%s", score, oldRank, newRank, m_pchCurrentLeaderboardName);

	m_eStatus = K_JOBSTATUS_JUST_FINISHED;
	m_eExecutingJobType = K_JOB_NONE;
}

void CGalaxyLeaderboards::OnLeaderboardScoreUpdateFailure(const char* name, int32_t score, galaxy::api::ILeaderboardScoreUpdateListener::FailureReason failureReason)
{
	if (failureReason != ILeaderboardScoreUpdateListener::FAILURE_REASON_NO_IMPROVEMENT)
	{
		LOG("[WARNING][GOG:Leaderboards] score [%d] update failed for leaderboard [%s]", score, name);
		const galaxy::api::IError* error = galaxy::api::GetError();
		if (error)
		{
			g_pLog->Write("  Error: %s", error->GetMsg());
		}
	}

	m_eStatus = K_JOBSTATUS_ERROR;
	m_eExecutingJobType = K_JOB_NONE;
}

// Called when the name of a userID has been received/changed
void CGalaxyLeaderboards::OnPersonaDataChanged(galaxy::api::GalaxyID userID, uint32_t personaStateChange)
{
	for (int index = 0; index < m_nLeaderboardEntries; index++)
	{
		if (m_leaderboardEntries[index].idPlayer == userID)
		{
			// save name
			char pchName[MAX_PATH] = { 0 };
			galaxy::api::Friends()->GetFriendPersonaNameCopy(userID, pchName, MAX_PATH);

			if (pchName && *pchName)
			{
				strncpy(m_scoresList.m_arrNames[index], pchName, sizeof(m_scoresList.m_arrNames[index]));
				//LOG("Leaderboards::Persona State Change received name %s", pchName);
				//announce names have been updated
				if (m_eStatus != K_JOBSTATUS_BUSY)
					m_eStatus = K_JOBSTATUS_JUST_FINISHED;
			}
			break;
		}
	}
}

///**************************************************************************************
/// Sigleton de acces
///**************************************************************************************

CGalaxyLeaderboards& UTGetLeaderboards()
{
	static CGalaxyLeaderboards g_GalaxyLeaderboards;
	return g_GalaxyLeaderboards;
}


#endif 
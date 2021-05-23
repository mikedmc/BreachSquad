#pragma once

#ifdef ENABLE_GALAXY

#include "galaxy/GalaxyID.h"
#include "galaxy/IStats.h"

//Status for download/upload jobs
enum ELBJobStatus {
	K_JOBSTATUS_EMPTY,				//no request made or reset was called
	K_JOBSTATUS_BUSY,				//busy working on request
	K_JOBSTATUS_JUST_FINISHED,		//request finished ok, visible only for a frame
	K_JOBSTATUS_IDLE,				//no jobs
	K_JOBSTATUS_ERROR,				//request finished with error
};
//type of queued job
enum ELBJobType {
	K_JOB_NONE,

	K_JOB_INITIALIZE,				//GOG needs to call some things before being able to ask for the leaderboards

	K_JOB_CHANGE_LEADERBOARD,
	K_JOB_UPLOAD_SCORE,
	K_JOB_GET_SCORES_AROUND_USER,
	K_JOB_GET_SCORES_FROM_FRIENDS,
	K_JOB_GET_SCORES_GLOBAL,
	K_JOB_GET_SCORE_FOR_CURRENT_USER,	//get the score for local player
};

//scores list structure
#define K_LB_SCORES_LIST_SIZE 10
struct CScoresList {
	int			m_nScoresCnt;											//number of valid scores
	int			m_nPlayerScoreIndex;									//index of local player score or -1 when not in range
	char		m_arrNames[K_LB_SCORES_LIST_SIZE][MAX_PATH];		//names get saved here
	int			m_arrScores[K_LB_SCORES_LIST_SIZE];
	int			m_arrRank[K_LB_SCORES_LIST_SIZE];						//actual rank

	ELBJobType	request_eJobType;										//job request type that generated this structure
	char		request_pchLeaderboardName[MAX_PATH];					//name of leaderboard from which the scores were downloaded

	CScoresList() : m_nScoresCnt(0), m_nPlayerScoreIndex(-1), request_eJobType(K_JOB_NONE)
	{
		for (int index = 0; index < K_LB_SCORES_LIST_SIZE; index++)
		{
			m_arrScores[index] = -1;
			m_arrRank[index] = -1;
			sprintf(m_arrNames[index], "-");
		}
	}
};

class CGalaxyLeaderboards : public
	galaxy::api::ILeaderboardEntriesRetrieveListener,
	galaxy::api::ILeaderboardRetrieveListener,
	galaxy::api::ILeaderboardScoreUpdateListener,
	galaxy::api::ILeaderboardsRetrieveListener,
	galaxy::api::IPersonaDataChangedListener
{
	private:
		static const int		NO_LEADERBOARD = 0;

	public:
		static const int		NUM_USERS_TO_DOWNLOAD = K_LB_SCORES_LIST_SIZE;
		
		//queued job structure
		struct CJob {
			ELBJobType	m_eJobType;
			int			m_iPayload;
			char		m_pchLeaderboardName[ MAX_PATH ];

			CJob() : m_iPayload(0), m_eJobType(K_JOB_NONE)
			{
				memset(m_pchLeaderboardName, 0, sizeof(char) * MAX_PATH);
			}
		};

		//structure to hold leaderboard data
		struct CLeaderboardEntry {
			uint32_t				nRank;
			int32_t					nScore;
			galaxy::api::GalaxyID	idPlayer;
			char					strName[MAX_PATH];
		};

	private:
		CArray<CJob*>			m_arrJobs;
			
	public:	
		void					Init();
		void					Release(); 

		ELBJobStatus			Update(float dTime);
		bool					QueueJob(ELBJobType nJobRequest, const char * pchLeaderboardName, int iPayload = 0);
		bool					InsertJob(int nInsertIndex, ELBJobType nJobRequest, const char * pchLeaderboardName, int iPayload = 0);
		
		void					ResetScoresList();
		int						GetDownloadedScores(CScoresList * pDestList);
		int						GetDownloadedScores_PlayerIndex();
		// Returns current user's score for last requested job
		int						GetUserScore();			
		int						GetDownloadedScoresCount();

		bool					IsBusy();
public:
		CGalaxyLeaderboards();
		~CGalaxyLeaderboards();

	private:
		ELBJobStatus			m_eStatus;	
		ELBJobType				m_eExecutingJobType;	//current job type

	private:
		UINT32					m_currentLeaderboard; // Handle to leaderboard (name hash for gog because it doesn't have a handle for leaderboards)
		char					m_pchCurrentLeaderboardName[MAX_PATH];

	private:
		int						m_nLeaderboardEntries;							// How many scores entries do we have?
		CLeaderboardEntry		m_leaderboardEntries[ NUM_USERS_TO_DOWNLOAD ];	// The actual entries, internal representation
		
		int						m_nPlayerScore;									//The player score that was last downloaded
		CScoresList				m_scoresList;									//The scores that were last downloaded (human readable)
		void					UpdateScoresListFromLeaderboardEntries();		//writes data from leaderboard entries into human readable format

	private:
		void					FindOrCreateLeaderboard( const char *pchLeaderboardName );
		bool					UploadScore( int iScore );
		
		bool					DownloadScoresAroundUser();
		bool					DownloadScoresForFriends();
		bool					DownloadScoresGlobal(int nStartIdx = 0);
		// Downloads the score for the current user from the current leaderboard
		bool					DownloadScoreForCurrentUser();
		/*
		bool					IsValidLeaderboard() const { return m_currentLeaderboard != NO_LEADERBOARD; }
		SteamLeaderboard_t		GetCurrentLeaderboard() const { return m_currentLeaderboard; }
		bool					IsDownloadInProgress() const { return (m_eStatus == K_JOBSTATUS_BUSY); }
		*/
		bool					IsLeaderboardActive( const char *pchLeaderboardName );
		
	public: // callbacks
		virtual void OnLeaderboardsRetrieveSuccess() override;
		virtual void OnLeaderboardsRetrieveFailure(galaxy::api::ILeaderboardsRetrieveListener::FailureReason failureReason) override;
		virtual void OnLeaderboardEntriesRetrieveSuccess(const char* name, uint32_t entryCount) override;
		virtual void OnLeaderboardEntriesRetrieveFailure(const char* name, galaxy::api::ILeaderboardEntriesRetrieveListener::FailureReason failureReason) override;
		virtual void OnLeaderboardRetrieveSuccess(const char* name) override;
		virtual void OnLeaderboardRetrieveFailure(const char* name, galaxy::api::ILeaderboardRetrieveListener::FailureReason failureReason) override;
		virtual void OnLeaderboardScoreUpdateSuccess(const char* name, int32_t score, uint32_t oldRank, uint32_t newRank) override;
		virtual void OnLeaderboardScoreUpdateFailure(const char* name, int32_t score, galaxy::api::ILeaderboardScoreUpdateListener::FailureReason failureReason) override;
		virtual void OnPersonaDataChanged(galaxy::api::GalaxyID userID, uint32_t personaStateChange) override;
};

///**************************************************************************************
/// Sigleton de acces
///**************************************************************************************
CGalaxyLeaderboards& UTGetLeaderboards();

#endif

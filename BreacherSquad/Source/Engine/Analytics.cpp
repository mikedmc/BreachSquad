#include "dxstdafx.h"

#if defined(K_GAME_ENABLE_ANALYTICS)

bool CAnalytics::ExecuteCurlURL(const char* szURL)
{
	if ((K_ANALYTICS_MAX_ERRORS_BEFORE_SHUTDOWN > 0) && (m_nTotalErrors >= K_ANALYTICS_MAX_ERRORS_BEFORE_SHUTDOWN))
		return true;

	CURL* pCurlHandle = curl_easy_init();
	curl_easy_setopt(pCurlHandle, CURLOPT_URL, szURL);
	curl_easy_setopt(pCurlHandle, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(pCurlHandle, CURLOPT_TIMEOUT, 10);
	CURLMcode result = curl_multi_add_handle((CURLM*)m_pMultiHandle, pCurlHandle);
	assert(result == CURLM_OK);

	//add easy handle to actives list
	//m_arrEasyHandles.Add(pCurlHandle);

	return (result == CURLM_OK);
}

CAnalytics::CAnalytics()
{
	m_bInitialized = false;
	m_pMultiHandle = NULL;
	m_nTotalErrors = 0;

	memset(m_strServicePath, 0, sizeof(m_strServicePath));
}

CAnalytics::~CAnalytics()
{
	Shutdown();
}

bool CAnalytics::Init(char * strTrackingID, char * strClientID)
{
	curl_global_init(CURL_GLOBAL_ALL);
	m_pMultiHandle = curl_multi_init();

	if (!m_pMultiHandle)
	{
		ErrorBox(K_ERR_WARNING, L"cURL::Init - MultiHandle failed to init!");
		return false;
	}

	//build service path
	StringCchPrintfA(m_strServicePath, 2048, "%s?v=1&tid=%s&cid=%s", K_ANALYTICS_PATH_TO_SERVICE, strTrackingID, strClientID);
	//USER LANGUAGE: "ul" text	None	20 Bytes	all

	LOG(L"Analytics:: Initialized OK. cURL v%d.%d.%d", LIBCURL_VERSION_MAJOR, LIBCURL_VERSION_MINOR, LIBCURL_VERSION_PATCH);

	m_bInitialized = true;
	m_nTotalErrors = 0;

	return true;
}

void CAnalytics::Shutdown()
{
	if (!m_bInitialized)
		return;

	Update(); //one last update

	LOG(L"Analytics:: Shut Down.");
	m_bInitialized = false;
	//remove all standing handles
	//for (int kk = 0; kk < m_arrEasyHandles.GetSize(); kk++)
	//{
	//	CURL* pHandle = m_arrEasyHandles[kk];
	//	curl_multi_remove_handle(m_pMultiHandle, pHandle);
	//	curl_easy_cleanup(pHandle);
	//}
	//remove multi handle
	curl_multi_cleanup(m_pMultiHandle);
	m_pMultiHandle = NULL;
}

void CAnalytics::TriggerEvent(char* strCategory, char* strAction, char* strLabel, UINT32 nValue)
{
	CHAR strAddr[2048];
	memset(strAddr, 0, 2048);

	StringCchPrintfA(strAddr, 2048, "%s&t=event&ec=%s&ea=%s&el=%s&ev=%u", m_strServicePath, strCategory, strAction, strLabel, nValue);

	if (!ExecuteCurlURL(strAddr))
	{
		LOG(L"Analytics::TriggerEvent failed! [%s]", strAddr);
	}
}

void CAnalytics::TriggerPageView(char* strHostname, char* strPage, char* strTitle)
{
	CHAR strAddr[2048];
	memset(strAddr, 0, 2048);

	StringCchPrintfA(strAddr, 2048, "%s&t=pageview&dh=%s&dp=%s&dt=%s", m_strServicePath, strHostname, strPage, strTitle);
	if(!ExecuteCurlURL(strAddr))
	{
		LOG(L"Analytics::TriggerPageView failed! [%s]", strAddr);
	}
}

void CAnalytics::TriggerScreenView(char* strScreenName, char* strAppName, char* strAppVersion, char* strAppID, char* strAppInstallerID)
{
	CHAR strAddr[2048];
	memset(strAddr, 0, 2048);

	StringCchPrintfA(strAddr, 2048, "%s&t=screenview&cd=%s&an=%s&av=%s&aid=%s&aiid=%s", m_strServicePath, strScreenName, strAppName, strAppVersion, strAppID, strAppInstallerID);
	if (!ExecuteCurlURL(strAddr))
	{
		LOG(L"Analytics::TriggerPageView failed! [%s]", strAddr);
	}
}

bool CAnalytics::Update()
{
	if (!m_bInitialized)
		return false;

	int still_running = 0;
	CURLMcode ret = curl_multi_perform((CURLM*)m_pMultiHandle, &still_running);

	CURLMsg* pMsg = NULL;
	do
	{
		int msgsInQueue = 0;
		pMsg = curl_multi_info_read((CURLM*)m_pMultiHandle, &msgsInQueue);
		if (pMsg && (pMsg->msg == CURLMSG_DONE))
		{
			CURL *pHandle = pMsg->easy_handle;
			//check response code so we disable the Analytics on too many errors
			long response_code;
			if (CURLE_OK == curl_easy_getinfo(pHandle, CURLINFO_RESPONSE_CODE, &response_code))
			{
				if (response_code != 200)
				{
					m_nTotalErrors++;
					LOG(L"Analytics:: Request error code(%ld). Total Errors(%d).", response_code, m_nTotalErrors);
					//too many errors?
					if ((K_ANALYTICS_MAX_ERRORS_BEFORE_SHUTDOWN > 0) && (m_nTotalErrors >= K_ANALYTICS_MAX_ERRORS_BEFORE_SHUTDOWN))
					{
						LOG(L"Analytics:: Too many errors! Webservice is down or firewall is enabled. No more requests will be made!");
					}
				}
			}
			//remove easy handle from array
			//int nIdx = m_arrEasyHandles.IndexOf(pHandle);
			//should always find active handles
			//assert(nIdx >= 0);
			//if (nIdx >= 0)
				//m_arrEasyHandles.Remove(nIdx);

			curl_multi_remove_handle((CURLM*)m_pMultiHandle, pHandle);
			curl_easy_cleanup(pHandle);
		}

	} while (pMsg);

	//not running and no need to call curl_multi_perform => we're not busy
	if ((ret != CURLM_CALL_MULTI_PERFORM) && (still_running == 0))
		return false;

	//still busy
	return true;
}

#endif

///**************************************************************************************
/// Sigleton de acces
///**************************************************************************************
CAnalytics& UTGetAnalytics()
{
	static CAnalytics g_Analytics;
	return g_Analytics;
}


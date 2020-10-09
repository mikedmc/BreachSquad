#pragma once
//#TODO: We should have a small pool of simple handles and use those instead of allocating a new handle every time
//#TODO: de adaugat functie simpla cu categorie si action de formap printf ca sa fie usor de chemat intr-o linie fara sa mai chem printf

#if defined(_DEBUG) || defined(DEBUG)
//#define K_GAME_ENABLE_ANALYTICS
#else
//comment the next line to completely exclude CURL and analytics engine
#define K_GAME_ENABLE_ANALYTICS
#endif

//default path to google analytics
#define K_ANALYTICS_PATH_TO_SERVICE "http://www.google-analytics.com/collect"

//--- triggers and event ---
#define ANALYTICS_EVENT(strCategory, strAction, strLabel, uint32Value)  UTGetAnalytics().TriggerEvent(strCategory, strAction, strLabel, uint32Value)
//--- triggers a page view ---
#define ANALYTICS_PAGEVIEW(strHostname, strPage, strTitle)				UTGetAnalytics().TriggerPageView(strHostname, strPage, strTitle)
//--- triggers a screenview ---
#define ANALYTICS_SCREENVIEW(strScreenName)		UTGetAnalytics().TriggerScreenView(strScreenName, "DKActionSquad", _VERSION_CHARSTR_, "psh.dk.actionsquad", "valve.steam")

#if defined(K_GAME_ENABLE_ANALYTICS)

//if static linking:
//#define CURL_STATICLIB
//#pragma comment(lib, "libcurl_a.lib")
//else if dynamic linking:
#pragma comment(lib, "libcurl.lib")
//endif

#include "curl.h"

//how many errors before shutting down the Analytics module or -1 to remove limit
#define K_ANALYTICS_MAX_ERRORS_BEFORE_SHUTDOWN 10

class CAnalytics {
protected:
	bool	m_bInitialized;			
	//multi handle
	CURLM*  m_pMultiHandle;
	//array of active easy handles 
	//CGrowableArray<CURL*> m_arrEasyHandles;
	//proxy service path - includes clientID and trackingID
	char m_strServicePath[2048]; 
	//count total errors and shut down if too many
	int m_nTotalErrors;

protected:
	/* Starts the execution of a URL in parallel */
	bool ExecuteCurlURL(const char* szURL);

public:
	CAnalytics();
	~CAnalytics();

	/*!	Initialize analytics
	 * /param: strTrackingID: google analytics app tracking ID
	 * /param: strClientID: internatl client ID
	 */
	bool Init(char * strTrackingID, char * strClientID);
	void Shutdown();

	/*!
	* \brief Sends Event Tracking message
	* \param strCategory Specifies the event category. Must not be empty.
	* \param strAction Specifies the event action. Must not be empty.
	* \param strLabel Specifies the event label (optional)
	* \param nValue Specifies the event value. Values must be non-negative. (optional)
	*/
	void TriggerEvent(char* strCategory, char* strAction, char* strLabel, UINT32 nValue);

	/*!
	* \brief Sends Page View event
	* \param strHostname Specifies the hostname from which content was hosted. (optional)
	* \param strPage The path portion of the page URL. Should begin with '/'. (optional)
	* \param strTitle The title of the page / document. (optional)
	*/
	void TriggerPageView(char* strHostname, char* strPage, char* strTitle);

	/*!
	 * \brief Triggers a Screen View
	 * \param strAppName - App name.
	 * \param strScreenName - screen name.
	 * \param strAppVersion, strAppID, strAppInstallerID - custom optionals
	 */
	void TriggerScreenView(char* strScreenName, char* strAppName, char* strAppVersion, char* strAppID, char* strAppInstallerID);

	/*!	Calls all the addresses from the queue, one by one.
	 *	/returns: busy state (true when busy, false when idling)
	 */
	bool Update();
};

#else
//dummy class for when not using analytics	
class CAnalytics {
public:
	HRESULT Init(char * strTrackingID, char * strClientID) 
	{ 
		LOG(L"Analytics:: Analytics Disabled.");
		return S_OK;  
	};
	void Shutdown() {};																	 
	void TriggerEvent(char* strCategory, char* strAction, char* strLabel, UINT32 nValue) {};
	void TriggerPageView(char* strHostname, char* strPage, char* strTitle) {};
	void TriggerScreenView(char* strScreenName, char* strAppName, char* strAppVersion, char* strAppID, char* strAppInstallerID) {};
	bool Update() { return false; }
};

#endif

//declar singletonul de acces
CAnalytics& UTGetAnalytics();
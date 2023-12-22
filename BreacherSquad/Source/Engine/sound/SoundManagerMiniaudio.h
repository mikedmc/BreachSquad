#pragma once

#define MINIAUDIO_IMPLEMENTATION
#define MA_NO_GENERATION
#include "../Libs/release/miniaudio/miniaudio.h"

#define SND_PLAY_FLAGS(sndID, sndFlags)					__Audio().Play(sndID, sndFlags)
#define SND_PLAY(sndID)									__Audio().Play(sndID, 0)
#define SND_PAUSE(sndID, bFadeout)						__Audio().Stop(sndID, bFadeout, false)
#define SND_STOP(sndID, bFadeout)						__Audio().Stop(sndID, bFadeout)
#define SND_STOP_ONE_BUFFER(sndID, bReset)				__Audio().StopOneBuffer(sndID, bReset);
#define SND_SET_FREQUENCY(sndID, fFreq)					__Audio().SetFrequency(sndID, fFreq)
#define SND_PLAY_FADEIN(sndID, maxVol, sndFlags)		__Audio().PlayFadeIn(sndID, maxVol, sndFlags)
#define SND_PLAY_ONCE(sndID, sndFlags)					{if(!__Audio().IsPlaying(sndID)) __Audio().Play(sndID, sndFlags);}
#define SND_SET_GROUP_VOLUME(groupID, fVolume, bFade)	__Audio().SetGroupVolume(groupID, fVolume, bFade)
#define SND_SET_GROUP_FREQUENCY(sGroupName, fFrequency, bFade)	__Audio().SetGroupFrequency(sGroupName, fFrequency, bFade)
#define SND_STOP_GROUP(groupID, bFadeOut, bResetSound)	__Audio().StopGroup(groupID, bFadeOut, bResetSound)
#define SND_IS_PLAYING(nSndIdxOrName)					__Audio().IsPlaying(nSndIdxOrName)
//--- POSITIONAL MACROS ---
#define SND_PLAY_POSITIONAL(sndIDX, vVector)			__Audio().PlayPositional(sndIDX, vVector, 0)
#define SND_PLAY_POSITIONAL_RAND2(sndIDX1, sndIDX2, vVector)			__Audio().PlayPositionalRand2(sndIDX1, sndIDX2, vVector, 0)
#define SND_SET_LISTENER_POS(vVector)					__Audio().SetListenerPos(vVector)


//numar default de buffere
#define SND_SOUND_BUFFERS_DEFAULT_CNT 4
//volum minim
#define SND_MIN_VOL -3500.0f
#define SND_PAN_LEFT -3500.0f
#define SND_PAN_RIGHT 3500.0f

#define SND_FADE_SPEED 0.8f

//updates N times a second
#define SND_UPDATE_PERIOD 0.1f

class CSoundManager;
class CSound;

//---------------------------------
// CSOUND CLASS
//---------------------------------
#define CSOUND_STATUS_IDLE 0
#define CSOUND_STATUS_FADING 1

#define SOUNDHANDLE			ma_sound*

class CSound
{
public:
	ma_sound*				buffers;
	WCHAR					sPath[MAX_PATH]{};		// path to file
	CStringHash				shID;						// ID from xml
	CStringHash				shGroupID;				// goup ID from XML

	float					fVolume;				// wanted volume [0..1]
	float					fVolume_real;			// real volume [0..1]
	float					fVolume_group;			// group volume [0..1] (multiplied with real volume)
	float					fFrequency, fFrequency_real;
	float					fFrequency_group;
	float					fPan;					// doesn't support animation

	bool					bReadyForPlaying;		// everything ready?
	bool					bOnlyLoadWhenPlayed;	// it only loads the sound when playing it

	int						currentBuffer;
	int						buffersCnt;				// number of loaded buffers

	CSound();
	~CSound();
};

//-----------------------------------
// SOUND MANAGER
//-----------------------------------
class CSoundManager : public IEventListener
{
protected:
	ma_engine*				pSE;							// sound engine
	DWORD					sampleRate;						// sample rate set in init
	bool					sndOK;							// everything loaded ok

	bool					m_bPositionalSoundsEnabled;		// disabled by default
	D3DXVECTOR2				m_vListenerPos;
	D3DXVECTOR2				m_vListenerExtents;				// size of hearable area
	float					m_fListenerVolumeFadeStartPercent; // percentage of listener extents the volume starts to linearly fade

	CArray<CSound*>			sounds;

public:	
	CSoundManager();
	~CSoundManager();

	OPRESULT				Init(DWORD dwChannelsCount, DWORD dwSampleRate, DWORD dwPrimaryBitRate);
	void					Release();
	//HRESULT				RestoreBuffer( SOUNDHANDLE pDSB, BOOL* pbWasRestored );

	OPRESULT				LoadSoundsXML(WCHAR* XMLpath);
	OPRESULT				AddSound(WCHAR* sFile, WCHAR* sID, WCHAR* sGroup, int nBuffers, bool bOnlyLoadWhilePlaying, int *retIdx = NULL);

private:
	// Fills actual sound buffers with data
	OPRESULT				LoadSoundBuffers(CSound * pSound);

	// Releases actual sound buffers
	OPRESULT				ReleaseSoundBuffers(CSound * pSound);

public:
	//////////////////////////////////////////////////////////////////////////
	// POSITIONAL SOUNDS
	//////////////////////////////////////////////////////////////////////////
	/*!
	 * \brief Enables software positional sounds. It only sets the position on play (doesn't hadle continuous positional sounds or listener movements)
	 * \param vListenerExtents - half size of listener centered hearing bbox
	 */
	void EnablePositionalSounds(D3DXVECTOR2 vListenerPos, D3DXVECTOR2 vListenerExtents);
	void DisablePositionalSounds();
	FORCEINLINE void SetListenerPos(D3DXVECTOR2 vListenerPos) 
	{
		m_vListenerPos = vListenerPos;
	}
	/*!
	 * \brief Sets the limit from where the sounds start to fade out linearly
	 * \param fPercentOfExtentsWidth - percent until where volume remains at maximum
	 */
	FORCEINLINE void SetListenerVolumeFadeStart(float fPercentOfExtentsWidth)
	{
		m_fListenerVolumeFadeStartPercent = fPercentOfExtentsWidth;
	}
	/*!
	 * \brief Plays a positional sound
	 */
	SOUNDHANDLE PlayPositional(int sndIdx, D3DXVECTOR2 pos, DWORD flags = 0);
	SOUNDHANDLE PlayPositionalRand2(int sndIdx1, int sndIdx2, D3DXVECTOR2 pos, DWORD flags = 0);


	//////////////////////////////////////////////////////////////////////////
	// NORMAL SOUNDS
	//////////////////////////////////////////////////////////////////////////
	SOUNDHANDLE Play(int sndIdx, DWORD flags = 0);
	SOUNDHANDLE PlayID(UINT32 sndNameHash, DWORD flags = 0);
	SOUNDHANDLE Play(CHAR* sndID, DWORD flags = 0);

	SOUNDHANDLE PlayFadeIn(int sndIdx, float vol = 1.0f, DWORD flags = 0);
	SOUNDHANDLE PlayFadeIn(CHAR* sndID, float vol = 1.0f, DWORD flags = 0);
	SOUNDHANDLE PlayFadeInID(UINT32 sndID, float vol = 1.0f, DWORD flags = 0);


	bool IsPlaying(int sndIdx);
	bool IsPlayingID(UINT32 sndID);
	bool IsPlaying(CHAR* sndID);

	void Stop(int sndIdx, bool fadeOut = false, bool resetSound = true);
	void StopID(UINT32 sndID, bool fadeOut = false, bool resetSound = true);
	void Stop(CHAR* sndID, bool fadeOut = false, bool resetSound = true);

	void StopOneBuffer(int sndIdx, bool resetSound = true);
	void StopBuffer(SOUNDHANDLE pDSB, bool resetSound = true);

	void SetVolumeID(UINT32 sndID, float vol, bool fade = false, bool affectPlayingToo = false);
	void SetVolume(CHAR* sndID, float vol, bool fade = false, bool affectPlayingToo = false);
	void SetVolume(int sndIdx, float vol, bool fade = false, bool affectPlayingToo = false);

	void SetGroupVolume(UINT32 nGroupID, float vol, bool fade = false);
	void SetGroupVolume(CHAR* groupName, float vol, bool fade = false);

	void SetGroupFrequency(UINT32 nGroupHash, float fFrequency, bool bFade = false);
	void SetGroupFrequency(CHAR* sGroupName, float fFrequency, bool bFade = false);

	void StopGroup(UINT32 nGroupID, bool fadeOut = false, bool resetSound = true);
	void StopGroup(CHAR* groupName, bool fadeOut = false, bool resetSound = true);

	void SetFrequencyID(UINT32 sndID, float freq);
	void SetFrequency(int sndIdx, float freq);
	void SetFrequency(CHAR* sndID, float freq);
	/*!
	 * \brief Sets sound panning
	 * \param: pan between -1.0 and 1.0
	 */
	void SetPanID(UINT32 sndID, float pan);
	void SetPan(int sndIdx, float pan);
	void SetPan(CHAR* sndID, float pan);

	int getSndIdx(CHAR* sndName);
	int getSndIdxW(const WCHAR* sndName);
	int getSndIdx(UINT32 sndID);
	//aplica volum, pan si freq pe toate bufferele. Se cheama cand modifici parametri manual	
	void ApplySoundSettingsNow(int sndIdx, bool affectPlayingToo = true);
	//aplica imediat toate datele pe un buffer
	void SetBufferSettingsNow(int sndIdx, int nBufferIdx, float fVolume, float fFrequency, float fPanning);
	//update pe fiecare sunet, in fn de status
	float updateTimer;
	void Update(float dTime);

	//--- EVENTS ---
	char const * GetListenerName() override { return "SoundManagerDirectSound"; };
	bool HandleEvent( CEvent &nEvent ) override;
};


// Sounds Manager singleton. Loads all sounds and plays them
CSoundManager& __Audio();
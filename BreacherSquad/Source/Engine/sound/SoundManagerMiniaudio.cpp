#include "dxstdafx.h"

#define STB_VORBIS_HEADER_ONLY
#include "../Libs/release/miniaudio/extras/stb_vorbis.c"

#define MINIAUDIO_IMPLEMENTATION
#if defined(_DEBUG) || defined(DEBUG)
	#define MA_DEBUG_OUTPUT
#endif
#define MA_NO_MP3
#define MA_NO_FLAC
#define MA_NO_GENERATION
#include "../Libs/release/miniaudio/miniaudio.h"

CSoundManager::CSoundManager() :
	sndOK( false ),
	pSE( nullptr ),
	sampleRate( 0 ),
	m_vListenerPos( 0.0f, 0.0f ), m_bPositionalSoundsEnabled( false ), m_vListenerExtents( 100.0f, 100.0f ), m_fListenerVolumeFadeStartPercent( 0.0f ),
	updateTimer( 0.0f )
{
	sounds.RemoveAll();
}

//*****************************************************************************
// CSoundManager
//*****************************************************************************
CSoundManager::~CSoundManager()
{
	Release();
}


//FORMAT
//<?xml version="1.0"?>
//<Sounds Version="1.0">
//	<Sound ID="INGAME1" path="ingame1.ogg" group="music" buffers="1" />
//</Sounds>

OPRESULT CSoundManager::Init( DWORD dwChannelsCount, DWORD dwSampleRate, DWORD dwPrimaryBitRate )
{
	ma_result result;

	ma_engine_config engineConfig;
	engineConfig = ma_engine_config_init();
	engineConfig.channels = dwChannelsCount;
	engineConfig.sampleRate = dwSampleRate;
	// save sample rate
	sampleRate = dwSampleRate;

	pSE = new ma_engine();
	result = ma_engine_init( &engineConfig, pSE );
	if ( result != MA_SUCCESS ) {
		return { K_OP_FAILED, L"Failed to initialize miniaudio sound engine!", K_SEVERITY_WARNING };
	}

	sndOK = true;
	return K_OP_OK;
}

//releases all sounds
void CSoundManager::Release()
{
	if ( sndOK == false )
		return;
	//release all sounds
	for ( int kk = 0; kk < sounds.GetSize(); kk++ )
	{
		SAFE_DELETE( sounds[kk] );
	}
	sounds.RemoveAll();

	ma_engine_uninit( pSE );
	SAFE_DELETE(pSE);

	LOG( L"Sounds:: Sound System Released OK." );

	sndOK = false;
}

OPRESULT CSoundManager::LoadSoundBuffers( CSound* pSound )
{
	// allocate buffers, initialize first sound from file and copy the other buffers
	pSound->buffers = new ma_sound[pSound->buffersCnt];

	ma_result result;
	result = ma_sound_init_from_file_w( pSE, pSound->sPath, NULL, nullptr, nullptr, &pSound->buffers[0] );
	if ( result != MA_SUCCESS )
	{
		SAFE_DELETE_ARRAY( pSound->buffers );
		return { K_OP_FAILED, K_SEVERITY_WARNING, L"Failed to load sound: %s", pSound->shID.text };
	}


	for ( int i = 1; i < pSound->buffersCnt; i++ )
	{
		result = ma_sound_init_copy( pSE, &pSound->buffers[0], NULL, nullptr, &pSound->buffers[i] );
		if ( result != MA_SUCCESS )
		{
			SAFE_DELETE_ARRAY( pSound->buffers );
			return { K_OP_FAILED, K_SEVERITY_WARNING, L"Failed to copy sound buffers: %s", pSound->shID.text };
		}
	}

	// save sound properties
	//pSound->wfx = wfx;	   
	//result = ma_sound_get_data_format(&pSound->buffers[0], NULL, NULL, 


	pSound->bReadyForPlaying = true;
	return K_OP_OK;
}

OPRESULT CSoundManager::ReleaseSoundBuffers( CSound* pSound )
{
	assert( pSound != nullptr );

	for ( int i = 0; i < pSound->buffersCnt; i++ )
	{
		ma_sound_stop( &pSound->buffers[i] );
		ma_sound_uninit( &pSound->buffers[i] );
	}
	SAFE_DELETE_ARRAY( pSound->buffers );

	pSound->bReadyForPlaying = false;

	return K_OP_OK;
}

void CSoundManager::EnablePositionalSounds( D3DXVECTOR2 vListenerPos, D3DXVECTOR2 vListenerExtents )
{
	m_bPositionalSoundsEnabled = true;
	m_vListenerPos = vListenerPos;
	m_vListenerExtents = vListenerExtents;

	LOG( L"Sounds:: Positional Sounds Enabled." );
}

void CSoundManager::DisablePositionalSounds()
{
	m_bPositionalSoundsEnabled = false;
	//reset panning on all sounds
	for ( int kk = 0; kk < sounds.GetSize(); kk++ )
	{
		SetPan( kk, 0.0f );
	}

	LOG( L"Sounds:: Positional Sounds Disabled." );
}

OPRESULT CSoundManager::LoadSoundsXML( WCHAR* XMLpath )
{
	LOG( L"Sounds:: Loading:[%s]...", XMLpath );

	if ( !sndOK )
	{
		ErrorBox( K_ERR_WARNING, L"Sound System failed to initialize! LoadSoundsXML will now exit.\n" );
		return( E_FAIL );
	}

	pugi::xml_document doc;
	if ( !doc.load_file( XMLpath ) )
	{
		ErrorBox( K_ERR_WARNING, L"Unable to load XML:%s\n", XMLpath );
		return( E_FAIL );
	}

	pugi::xml_attribute ver = doc.root().child( L"Sounds" ).attribute( L"Version" );
	if ( ver.as_float() != 1.0f )
	{
		ErrorBox( K_ERR_WARNING, L"Sounds XML wrong version:%s\n", XMLpath );
	}


	pugi::xml_node soundsnode = doc.root().child( L"Sounds" );
	int count = 0;
	for ( pugi::xml_node sndnode = soundsnode.first_child(); sndnode; sndnode = sndnode.next_sibling() )
	{
		WCHAR path[MAX_PATH];
		const WCHAR* cpath = sndnode.attribute( L"path" ).value();
		StringCchCopy( path, MAX_PATH, cpath );

		//Citesc ID-ul
		UINT stringLen;
		StringCchLength( sndnode.attribute( L"ID" ).value(), MAX_PATH, &stringLen );
		WCHAR strID[MAX_PATH]{};
		if ( stringLen > 0 )
		{
			StringCchCopy( strID, stringLen + 1, sndnode.attribute( L"ID" ).value() );
		}
		else
		{
			ErrorBox( K_ERR_WARNING, L"Sounds XML - 0 length ID:%d\n", count );
		}
		//Citesc Group ID-ul
		StringCchLength( sndnode.attribute( L"group" ).value(), MAX_PATH, &stringLen );
		WCHAR strGroupID[MAX_PATH]{};
		if ( stringLen > 0 )
		{
			StringCchCopy( strGroupID, stringLen + 1, sndnode.attribute( L"group" ).value() );
		}
		else
		{
			ErrorBox( K_ERR_WARNING, L"Sounds XML - 0 length ID:%d\n", count );
		}

		int bufCnt;
		if ( !sndnode.attribute( L"buffers" ).empty() )
			bufCnt = sndnode.attribute( L"buffers" ).as_int();
		else bufCnt = SND_SOUND_BUFFERS_DEFAULT_CNT;

		bool bOnlyLoadPlaying = false;
		if ( !sndnode.attribute( L"bOnlyLoadWhilePlaying" ).empty() )
			bOnlyLoadPlaying = sndnode.attribute( L"bOnlyLoadWhilePlaying" ).as_bool();

#if defined(K_SNDMGR_USE_COLLECTION_FILE)

		CHAR cpath[MAX_PATH];
		size_t cntConv;
		wcstombs_s( &cntConv, cpath, path, MAX_PATH );

		int soundFileIDX = __LibraryManager().getFileIdByName( L"\\Sounds\\sounds.sfp", cpath );
		if ( soundFileIDX < 0 )
		{
			ErrorBox( K_ERR_WARNING, L"Could not find file in library: %s\n", path );
		}
		else
		{
			__LibraryManager().extractFileToTemp( L"\\Sounds\\sounds.sfp", soundFileIDX );
			HRESULT hr = AddSound( UTApp().g_wszTempFilePath, sID, sGroupID, bufCnt, bOnlyLoadPlaying );
			if ( FAILED( hr ) )
			{
				ErrorBox( K_ERR_WARNING, L"Could not load sound: %s\n", path );
			}
		}
#else
		WCHAR wcsResAddr[MAX_PATH];
		WCHAR wcsFilePath[MAX_PATH];
		StringCchPrintf( wcsResAddr, MAX_PATH, L"media/sounds/%s", path );
		FileManager::GetMediaPath( wcsResAddr, wcsFilePath );
		//LOG(L"[%d,%d] AddingSound:%s", bufCnt, bOnlyLoadPlaying, wcsResAddr);

		if(OP_FAILED(AddSound( wcsFilePath, strID, strGroupID, bufCnt, bOnlyLoadPlaying )))
		{
			ErrorBox( K_ERR_WARNING, L"Could not load sound: %s\n", path );
		}
#endif

		count++;
	}

	//--- delete temp file ---
	_wremove( UTApp().g_wszTempFilePath );

	LOG( L"Sounds:: %d Sounds Loaded.", count );

	return S_OK;
}



OPRESULT CSoundManager::AddSound( WCHAR* sFile, WCHAR* sID, WCHAR* sGroup, int nBuffers, bool bOnlyLoadWhilePlaying, int* retIdx )
{
	if ( retIdx )
		*retIdx = -1;

	CSound* sound = new CSound();

	wcscpy_s( sound->sPath, sFile );
	sound->shID.Init( sID );
	sound->shGroupID.Init( sGroup );

	sound->buffersCnt = nBuffers;
	if ( sound->buffersCnt <= 0 )
		sound->buffersCnt = 1;
	sound->fVolume = sound->fVolume_real = 1.0f;
	sound->bOnlyLoadWhenPlayed = bOnlyLoadWhilePlaying;
	sound->bReadyForPlaying = false;
	//add sound to list
	sounds.Add( sound );

	//only load sounds that are always loaded
	if ( !sound->bOnlyLoadWhenPlayed )
	{
		V_OP_RET( LoadSoundBuffers( sound ) );
	}
	// set sound properties as soon as they get loaded
	ApplySoundSettingsNow( sounds.GetSize() - 1 );

	if ( retIdx )
	{
		*retIdx = sounds.GetSize() - 1;
	}

	//LOG(L"AddedSound: %s", wszFile);

	return K_OP_OK;
}

int CSoundManager::getSndIdx( CHAR* sndName )
{
	if ( !sndOK )
		return -1;

	UINT32 sndHash = FastHash( sndName, strlen( sndName ) );
	//DebugPrintA("Finding [%s] code %x\n", sndName, sndHash);

	for ( int kk = 0; kk < sounds.GetSize(); kk++ )
	{
		if ( sounds[kk]->shID.textHash == sndHash )
			return kk;
	}

	ErrorBox( K_ERR_WARNING, L"getSndIdx->Could not find sound named: %s", sndName );
	return -1;
}

int CSoundManager::getSndIdxW( const WCHAR* sndName )
{
	if ( !sndOK )
		return -1;
	UINT32 sndHash = FastHash( sndName );

	for ( int kk = 0; kk < sounds.GetSize(); kk++ )
	{
		if ( sounds[kk]->shID.textHash == sndHash )
			return kk;
	}

	ErrorBox( K_ERR_WARNING, L"getSndIdx->Could not find sound named: %s", sndName );
	return -1;
}

int CSoundManager::getSndIdx( UINT32 sndID )
{
	if ( !sndOK )
		return -1;
	for ( int kk = 0; kk < sounds.GetSize(); kk++ )
	{
		if ( sounds[kk]->shID.textHash == sndID )
			return kk;
	}

	ErrorBox( K_ERR_WARNING, L"getSndIdx - Could not find sound id: %d\nReturning -1", sndID );
	return -1;
}

void CSoundManager::ApplySoundSettingsNow( int sndIdx, bool affectPlayingToo )
{
	if ( !sndOK )
		return;

	CSound* snd = sounds[sndIdx];
	//sound not ready, not loaded ot smthg
	if ( !snd->bReadyForPlaying )
		return;

	float localFreq = ( snd->fFrequency_real * snd->fFrequency_group );
	if ( localFreq < SND_PITCH_MIN )
		localFreq = SND_PITCH_MIN;
	if ( localFreq > SND_PITCH_MAX )
		localFreq = SND_PITCH_MAX;
	float panoram = snd->fPan;
	panoram = SND_PAN_LEFT + ( SND_PAN_RIGHT - SND_PAN_LEFT ) * ( ( 1.0f + panoram ) / 2.0f );

	DWORD status;
	for ( int kk = 0; kk < snd->buffersCnt; kk++ )
	{
		SOUNDHANDLE pDSB = &snd->buffers[kk];
		if ( pDSB == nullptr )
			continue;
		if ( !affectPlayingToo )
		{
			if ( ma_sound_is_playing( pDSB ) )
				continue;
		}

		//#TODO: should be optimized to update only if it changes
		ma_sound_set_volume( pDSB, snd->fVolume_real );
		ma_sound_set_pitch( pDSB, localFreq );
		ma_sound_set_pan( pDSB, panoram );
	}
}

void CSoundManager::SetBufferSettingsNow( int sndIdx, int nBufferIdx, float fVolume, float fFrequency, float fPanning )
{
	if ( !sndOK )
		return;
	SOUNDHANDLE pDSB = &sounds[sndIdx]->buffers[nBufferIdx];

	float localFreq = fFrequency;
	if ( localFreq < SND_PITCH_MIN )
		localFreq = SND_PITCH_MIN;
	if ( localFreq > SND_PITCH_MAX )
		localFreq = SND_PITCH_MAX;
	float panoram = sounds[sndIdx]->fPan;
	panoram = SND_PAN_LEFT + ( SND_PAN_RIGHT - SND_PAN_LEFT ) * ( ( 1.0f + panoram ) / 2.0f );

	//#TODO: should be optimized to update only if it changes
	ma_sound_set_volume( pDSB, sounds[sndIdx]->fVolume_real );
	ma_sound_set_pitch( pDSB, localFreq );
	ma_sound_set_pan( pDSB, panoram );
}

//Play
SOUNDHANDLE CSoundManager::Play( int sndIdx, DWORD flags )
{
	if ( !sndOK )
		return nullptr;

	if ( ( sndIdx < 0 ) || ( sndIdx >= sounds.GetSize() ) )
	{
		ErrorBox( K_ERR_WARNING, L"CSoundManager::Play - sound index out of bounds!" );
		return nullptr;
	}

	//not a sound that should load later but not ready also
	if ( ( !sounds[sndIdx]->bOnlyLoadWhenPlayed ) && ( !sounds[sndIdx]->bReadyForPlaying ) )
		return nullptr;
	//dynamic loading precheck
	if ( ( sounds[sndIdx]->bOnlyLoadWhenPlayed ) && ( sounds[sndIdx]->bReadyForPlaying ) )
		ErrorBox( K_ERR_WARNING, L"[SOUND]Play: Dynamic sound already loaded! Make sure it gets deallocated properly!" );
	//must load it now?
	if ( ( sounds[sndIdx]->bOnlyLoadWhenPlayed ) && ( !sounds[sndIdx]->bReadyForPlaying ) )
	{
		if ( OP_FAILED( LoadSoundBuffers( sounds[sndIdx] ) ) )
			return nullptr;

		ApplySoundSettingsNow( sndIdx, true );
		LOG( L"[SOUND]Play: Dynamically loaded sound idx[%d]", sndIdx );
	}

	SOUNDHANDLE pDSB = &sounds[sndIdx]->buffers[sounds[sndIdx]->currentBuffer];
	if ( !pDSB )
	{
		ErrorBox( K_ERR_WARNING, L"pDSB is null!\n" );
		return nullptr;
	}

	// is it playing already, reset it
	if ( ma_sound_is_playing( pDSB ) )
	{
		ma_sound_stop( pDSB );
		ma_sound_seek_to_pcm_frame( pDSB, 0 );
		sounds[sndIdx]->fVolume_real = sounds[sndIdx]->fVolume;
		sounds[sndIdx]->fFrequency_real = sounds[sndIdx]->fFrequency;
		sounds[sndIdx]->fPan = 0.0f;

		SetBufferSettingsNow( sndIdx, sounds[sndIdx]->currentBuffer, sounds[sndIdx]->fVolume_real * sounds[sndIdx]->fVolume_group, sounds[sndIdx]->fFrequency_real * sounds[sndIdx]->fFrequency_group, sounds[sndIdx]->fPan );
	}

	ma_sound_set_looping( pDSB, FLAG_ANY( flags , DSBPLAY_LOOPING ) );
	ma_result result = ma_sound_start( pDSB );
	if ( result != MA_SUCCESS )
	{
		return nullptr;
	}
	// jump to next buffer
	sounds[sndIdx]->currentBuffer++;
	sounds[sndIdx]->currentBuffer %= sounds[sndIdx]->buffersCnt;

	return pDSB;
}

SOUNDHANDLE CSoundManager::PlayID( UINT32 sndNameHash, DWORD flags /*= 0*/ )
{
	if ( !sndOK )
		return nullptr;
	int sndIdx = getSndIdx( sndNameHash );
	return Play( sndIdx, flags );
}

SOUNDHANDLE CSoundManager::Play( CHAR* sndID, DWORD flags )
{
	if ( !sndOK )
		return nullptr;
	int sndIdx = getSndIdx( sndID );
	return Play( sndIdx, flags );
}

SOUNDHANDLE CSoundManager::PlayPositionalRand2( int sndIdx1, int sndIdx2, D3DXVECTOR2 pos, DWORD flags /*= 0*/ )
{
	if ( ( sndIdx1 < 0 ) || ( sndIdx2 < 0 ) )
		return nullptr;

	if ( randint( 1000 ) < 500 )
		return PlayPositional( sndIdx1, pos, flags );

	return PlayPositional( sndIdx2, pos, flags );
}

SOUNDHANDLE CSoundManager::PlayPositional( int sndIdx, D3DXVECTOR2 pos, DWORD flags /*= 0*/ )
{
	if ( !sndOK )
		return nullptr;
	if ( !m_bPositionalSoundsEnabled )
	{
		ErrorBox( K_ERR_WARNING, L"CSoundManager::PlayPositional - Positional sounds not enabled! Use EnablePositionalSounds()!" );
		return nullptr;
	}
	if ( ( sndIdx < 0 ) || ( sndIdx >= sounds.GetSize() ) )
	{
		ErrorBox( K_ERR_WARNING, L"CSoundManager::PlayPositional - sound index out of bounds!" );
		return nullptr;
	}

	HRESULT hr = S_OK;

	//not a sound that should load later but not ready also
	if ( ( !sounds[sndIdx]->bOnlyLoadWhenPlayed ) && ( !sounds[sndIdx]->bReadyForPlaying ) )
		return nullptr;
	//dynamic loading precheck
	if ( ( sounds[sndIdx]->bOnlyLoadWhenPlayed ) && ( sounds[sndIdx]->bReadyForPlaying ) )
		ErrorBox( K_ERR_WARNING, L"[SOUND]Play: Dynamic sound already loaded! Make sure it gets deallocated properly!" );
	//must load it now?
	if ( ( sounds[sndIdx]->bOnlyLoadWhenPlayed ) && ( !sounds[sndIdx]->bReadyForPlaying ) )
	{
		if ( FAILED( hr = LoadSoundBuffers( sounds[sndIdx] ) ) )
			return nullptr;

		ApplySoundSettingsNow( sndIdx, true );
		LOG( L"[SOUND]PlayPositional: Dynamically loaded sound idx[%d]", sndIdx );
	}

	SOUNDHANDLE pDSB = &sounds[sndIdx]->buffers[sounds[sndIdx]->currentBuffer];
	if ( !pDSB )
	{
		ErrorBox( K_ERR_WARNING, L"pDSB is null!\n" );
		return nullptr;
	}

	//find panning
	D3DXVECTOR2 vDist = pos - m_vListenerPos;
	//can't be heared
	if ( ( fabs( vDist.x ) > m_vListenerExtents.x ) || ( fabs( vDist.y ) > m_vListenerExtents.y ) )
		return nullptr;
	//only horizontal panning
	float fPanning = vDist.x / m_vListenerExtents.x;
	//decide volume
	float fVolume = fabs( fPanning );
	if ( fVolume < m_fListenerVolumeFadeStartPercent )
		fVolume = 1.0f;
	else
		fVolume = 1.0f - ( fVolume - m_fListenerVolumeFadeStartPercent ) / ( 1.0f - m_fListenerVolumeFadeStartPercent );

	if ( ma_sound_is_playing( pDSB ) )
	{
		ma_sound_stop( pDSB );
		ma_sound_seek_to_pcm_frame( pDSB, 0 );
		sounds[sndIdx]->fVolume_real = sounds[sndIdx]->fVolume;
		sounds[sndIdx]->fFrequency_real = sounds[sndIdx]->fFrequency;
		sounds[sndIdx]->fPan = 0.0f;

		ApplySoundSettingsNow( sndIdx, false );
	}

	//--- sets sound panning ---
	float panoram = SND_PAN_LEFT + ( SND_PAN_RIGHT - SND_PAN_LEFT ) * ( ( 1.0f + fPanning ) / 2.0f );
	ma_sound_set_looping( pDSB, FLAG_ANY( flags , DSBPLAY_LOOPING ) );
	ma_sound_set_pan( pDSB, panoram );
	ma_sound_set_volume( pDSB, fVolume * ( sounds[sndIdx]->fVolume_real * sounds[sndIdx]->fVolume_group ) );
	ma_result result = ma_sound_start( pDSB );
	if ( result != MA_SUCCESS )
	{
		return nullptr;
	}

	//jump to next buffer
	sounds[sndIdx]->currentBuffer++;
	sounds[sndIdx]->currentBuffer %= sounds[sndIdx]->buffersCnt;

	return pDSB;
}

//PLAY fade in
SOUNDHANDLE CSoundManager::PlayFadeIn( int sndIdx, float vol, DWORD flags )
{
	if ( !sndOK )
		return nullptr;
	if ( ( sndIdx < 0 ) || ( sndIdx >= sounds.GetSize() ) )
	{
		ErrorBox( K_ERR_WARNING, L"CSoundManager::PlayFadeIn - sound index out of bounds!" );
		return nullptr;
	}
	sounds[sndIdx]->fVolume = vol;
	sounds[sndIdx]->fVolume_real = 0.0f;
	return Play( sndIdx, flags );
}

SOUNDHANDLE CSoundManager::PlayFadeIn( CHAR* sndID, float vol, DWORD flags )
{
	if ( !sndOK )
		return nullptr;
	int sndIdx = getSndIdx( sndID );
	if ( ( sndIdx < 0 ) || ( sndIdx >= sounds.GetSize() ) )
	{
		ErrorBox( K_ERR_WARNING, L"CSoundManager::PlayFadeIn(char*) - sound index out of bounds!" );
		return nullptr;
	}
	sounds[sndIdx]->fVolume = vol;
	sounds[sndIdx]->fVolume_real = 0.0f;
	return Play( sndIdx, flags );
}

SOUNDHANDLE CSoundManager::PlayFadeInID( UINT32 sndID, float vol, DWORD flags )
{
	if ( !sndOK )
		return nullptr;
	int sndIdx = getSndIdx( sndID );
	if ( ( sndIdx < 0 ) || ( sndIdx >= sounds.GetSize() ) )
	{
		ErrorBox( K_ERR_WARNING, L"CSoundManager::PlayFadeInID - sound index out of bounds!" );
		return nullptr;
	}
	sounds[sndIdx]->fVolume = vol;
	sounds[sndIdx]->fVolume_real = 0.0f;
	ApplySoundSettingsNow( sndIdx, true );
	return Play( sndIdx, flags );
}

//opreste un singur buffer
void CSoundManager::StopOneBuffer( int sndIdx, bool resetSound )
{
	if ( !sndOK )
		return;
	if ( ( sndIdx < 0 ) || ( sndIdx >= sounds.GetSize() ) )
	{
		ErrorBox( K_ERR_WARNING, L"CSoundManager::StopOneBuffer - sound index out of bounds!" );
		return;
	}

	if ( !IsPlaying( sndIdx ) )
		return;

	for ( int ii = 0; ii < sounds[sndIdx]->buffersCnt; ii++ )
	{
		if ( ma_sound_is_playing( &sounds[sndIdx]->buffers[ii] ) )
		{
			ma_sound_stop( &sounds[sndIdx]->buffers[ii] );
			if ( resetSound )
			{
				ma_sound_seek_to_pcm_frame( &sounds[sndIdx]->buffers[ii], 0 );
			}
			return;
		}
	}
}

void CSoundManager::StopBuffer( SOUNDHANDLE pDSB, bool resetSound )
{
	if ( pDSB == nullptr )
		return;
	ma_sound_stop( pDSB );
	if ( resetSound )
		ma_sound_seek_to_pcm_frame( pDSB, 0 );
}

//STOP
void CSoundManager::Stop( int sndIdx, bool fadeOut, bool resetSound )
{
	if ( !sndOK )
		return;
	if ( ( sndIdx < 0 ) || ( sndIdx >= sounds.GetSize() ) )
	{
		ErrorBox( K_ERR_WARNING, L"CSoundManager::Stop - sound index out of bounds!" );
		return;
	}

	if ( IsPlaying( sndIdx ) )
	{
		if ( ( fadeOut ) && ( sounds[sndIdx]->fVolume_real > 0.0f ) )
		{
			sounds[sndIdx]->fVolume = 0.0f;
		}
		else
		{
			for ( int ii = 0; ii < sounds[sndIdx]->buffersCnt; ii++ )
			{
				sounds[sndIdx]->fVolume = sounds[sndIdx]->fVolume_real = 1.0f;
				ma_sound_stop( &sounds[sndIdx]->buffers[ii] );
				if ( resetSound )
					ma_sound_seek_to_pcm_frame( &sounds[sndIdx]->buffers[ii], 0 );
			}

			//must release it now?
			if ( ( sounds[sndIdx]->bOnlyLoadWhenPlayed ) && ( sounds[sndIdx]->bReadyForPlaying ) )
			{
				ReleaseSoundBuffers( sounds[sndIdx] );
				LOG( L"[SOUND]Dynamically released sound idx[%d]", sndIdx );
			}
		}
	}
}

void CSoundManager::StopID( UINT32 sndID, bool fadeOut, bool resetSound )
{
	if ( !sndOK )
		return;
	int sndIdx = getSndIdx( sndID );
	Stop( sndIdx, fadeOut, resetSound );
}

void CSoundManager::Stop( CHAR* sndID, bool fadeOut, bool resetSound )
{
	if ( !sndOK )
		return;
	int sndIdx = getSndIdx( sndID );
	Stop( sndIdx, fadeOut, resetSound );
}
//IsPlaying
bool CSoundManager::IsPlaying( int sndIdx )
{
	if ( !sndOK )
		return false;
	if ( ( sndIdx < 0 ) || ( sndIdx >= sounds.GetSize() ) )
	{
		ErrorBox( K_ERR_WARNING, L"CSoundManager::IsPlaying - sound index out of bounds!" );
		return false;
	}
	//not loaded?
	if ( !sounds[sndIdx]->bReadyForPlaying )
		return false;

	for ( int ii = 0; ii < sounds[sndIdx]->buffersCnt; ii++ )
	{
		if ( ma_sound_is_playing( &sounds[sndIdx]->buffers[ii] ) )
			return true;
	}
	return false;
}
bool CSoundManager::IsPlaying( CHAR* sndID )
{
	if ( !sndOK )
		return false;
	int sndIdx = getSndIdx( sndID );
	return IsPlaying( sndIdx );
}
bool CSoundManager::IsPlayingID( UINT32 sndID )
{
	if ( !sndOK )
		return false;
	int sndIdx = getSndIdx( sndID );
	return IsPlaying( sndIdx );
}


void CSoundManager::SetVolume( int sndIdx, float vol, bool fade, bool affectPlayingToo )
{
	if ( !sndOK )
		return;
	if ( ( sndIdx < 0 ) || ( sndIdx >= sounds.GetSize() ) )
	{
		ErrorBox( K_ERR_WARNING, L"CSoundManager::SetVolume - sound index out of bounds!" );
		return;
	}


	CSound* snd = sounds[sndIdx];

	if ( ( vol < 0.0f ) || ( vol > 1.0f ) )
		return;

	snd->fVolume = vol;
	if ( !fade )
	{
		snd->fVolume_real = vol;
		ApplySoundSettingsNow( sndIdx, affectPlayingToo );
	}
}

void CSoundManager::SetVolumeID( UINT32 sndID, float vol, bool fade, bool changePlayingToo )
{
	if ( !sndOK )
		return;
	int sndIdx = getSndIdx( sndID );
	SetVolume( sndIdx, vol, fade, changePlayingToo );
}

void CSoundManager::SetVolume( CHAR* sndID, float vol, bool fade, bool changePlayingToo )
{
	if ( !sndOK )
		return;
	int sndIdx = getSndIdx( sndID );
	SetVolume( sndIdx, vol, fade, changePlayingToo );
}


void CSoundManager::SetFrequency( int sndIdx, float freq )
{
	if ( !sndOK )
		return;

	if ( ( sndIdx < 0 ) || ( sndIdx >= sounds.GetSize() ) )
	{
		ErrorBox( K_ERR_WARNING, L"CSoundManager::SetFrequency - sound index out of bounds!" );
		return;
	}


	CSound* snd = sounds[sndIdx];

	if ( ( freq <= 0.0f ) || ( freq > 1.0f ) )
		return;

	snd->fFrequency = freq;
	ApplySoundSettingsNow( sndIdx );
}

void CSoundManager::SetFrequencyID( UINT32 sndID, float freq )
{
	if ( !sndOK )
		return;
	int sndIdx = getSndIdx( sndID );
	SetFrequency( sndIdx, freq );
}

void CSoundManager::SetFrequency( CHAR* sndID, float freq )
{
	if ( !sndOK )
		return;
	int sndIdx = getSndIdx( sndID );
	SetFrequency( sndIdx, freq );
}

void CSoundManager::SetPan( int sndIdx, float pan )
{
	if ( !sndOK )
		return;
	if ( ( sndIdx < 0 ) || ( sndIdx >= sounds.GetSize() ) )
	{
		ErrorBox( K_ERR_WARNING, L"CSoundManager::SetPan - sound index out of bounds!" );
		return;
	}

	CSound* snd = sounds[sndIdx];

	float panoram = pan;
	CLAMP( panoram, -1.0f, 1.0f );

	snd->fPan = panoram;
	ApplySoundSettingsNow( sndIdx );
}

void CSoundManager::SetPanID( UINT32 sndID, float pan )
{
	if ( !sndOK )
		return;
	int sndIdx = getSndIdx( sndID );
	SetPan( sndIdx, pan );
}

void CSoundManager::SetPan( CHAR* sndID, float pan )
{
	if ( !sndOK )
		return;
	int sndIdx = getSndIdx( sndID );
	SetPan( sndIdx, pan );
}
//SetGroupVolume
void CSoundManager::SetGroupVolume( UINT32 nGroupID, float vol, bool fade )
{
	if ( !sndOK )
		return;
	for ( int ii = 0; ii < sounds.GetSize(); ii++ )
	{
		if ( sounds[ii]->shGroupID.textHash == nGroupID )
		{
			sounds[ii]->fVolume_group = vol;
			SetVolume( ii, sounds[ii]->fVolume, fade, true );
		}
	}
}

void CSoundManager::SetGroupVolume( CHAR* groupName, float vol, bool fade )
{
	if ( !sndOK )
		return;
	UINT32 groupHash = FastHash( groupName, strlen( groupName ) );
	for ( int ii = 0; ii < sounds.GetSize(); ii++ )
	{
		if ( sounds[ii]->shGroupID.textHash == groupHash )
		{
			sounds[ii]->fVolume_group = vol;
			SetVolume( ii, sounds[ii]->fVolume, fade, true );
		}
	}
}


void CSoundManager::SetGroupFrequency( UINT32 nGroupHash, float fFrequency, bool bFade )
{
	if ( !sndOK )
		return;
	for ( int ii = 0; ii < sounds.GetSize(); ii++ )
	{
		if ( sounds[ii]->shGroupID.textHash != nGroupHash )
			continue;

		if ( sounds[ii]->fFrequency == fFrequency )
			continue;

		sounds[ii]->fFrequency = fFrequency;
		if ( !bFade )
		{
			sounds[ii]->fFrequency_real = sounds[ii]->fFrequency;
			ApplySoundSettingsNow( ii, true );
		}
	}
}

void CSoundManager::SetGroupFrequency( CHAR* sGroupName, float fFrequency, bool bFade )
{
	if ( !sndOK )
		return;
	UINT32 groupHash = FastHash( sGroupName, strlen( sGroupName ) );

	for ( int ii = 0; ii < sounds.GetSize(); ii++ )
	{
		if ( sounds[ii]->shGroupID.textHash != groupHash )
			continue;

		if ( sounds[ii]->fFrequency == fFrequency )
			continue;

		sounds[ii]->fFrequency = fFrequency;
		if ( !bFade )
		{
			sounds[ii]->fFrequency_real = sounds[ii]->fFrequency;
			ApplySoundSettingsNow( ii, true );
		}
	}
}

void CSoundManager::StopGroup( UINT32 nGroupID, bool fadeOut, bool resetSound )
{
	if ( !sndOK )
		return;
	for ( int ii = 0; ii < sounds.GetSize(); ii++ )
	{
		if ( sounds[ii]->shGroupID.textHash == nGroupID )
			Stop( ii, fadeOut, resetSound );
	}
}

void CSoundManager::StopGroup( CHAR* groupName, bool fadeOut, bool resetSound )
{
	if ( !sndOK )
		return;

	UINT32 groupHash = FastHash( groupName, strlen( groupName ) );
	for ( int ii = 0; ii < sounds.GetSize(); ii++ )
	{
		if ( sounds[ii]->shGroupID.textHash == groupHash )
			Stop( ii, fadeOut, resetSound );
	}
}


void CSoundManager::Update( float dTime )
{
	if ( !sndOK )
		return;

	updateTimer -= dTime;
	if ( updateTimer > 0.0f )
		return;

	updateTimer += SND_UPDATE_PERIOD;

	for ( int kk = 0; kk < sounds.GetSize(); kk++ )
	{
		bool bEasing = false;
		CSound* snd = sounds[kk];

		if ( snd->fVolume_real != snd->fVolume )
		{
			UTMath::EaseTo_linear( &snd->fVolume_real, snd->fVolume, SND_UPDATE_PERIOD * SND_FADE_SPEED );
			bEasing = true;
		}
		if ( snd->fFrequency_real != snd->fFrequency )
		{
			UTMath::EaseTo_linear( &snd->fFrequency_real, snd->fFrequency, SND_UPDATE_PERIOD * SND_FADE_SPEED );
			bEasing = true;
		}

		if ( bEasing )
		{
			//stop by volume only when fading out
			if ( snd->fVolume_real <= 0.0f )
			{
				Stop( kk, false, true );
			}

			ApplySoundSettingsNow( kk, true );
		}
	}
}

//*****************************************************************************
// CSound
//*****************************************************************************
CSound::CSound()
{
	memset( sPath, 0, MAX_PATH * sizeof( WCHAR ) );

	fFrequency = fFrequency_real = fFrequency_group = 1.0f;
	fVolume = fVolume_real = fVolume_group = 1.0f;
	fPan = 0.0f;

	bReadyForPlaying = false;
	bOnlyLoadWhenPlayed = false;

	currentBuffer = 0;
	buffersCnt = 0;
	buffers = nullptr;
}

CSound::~CSound()
{
	if ( buffers == null )
		return;
	for ( int i = 0; i < buffersCnt; i++ )
	{
		ma_sound_uninit( &buffers[i] );
	}
	SAFE_DELETE_ARRAY( buffers );
}


//-----------------------------------------------------------------------------
// EVENTS LISTENER
//-----------------------------------------------------------------------------
bool CSoundManager::HandleEvent( CEvent& nEvent )
{
	//handling EVTT_SOUND events (comenzi de sunet)
	if ( nEvent.m_eventType == CEventTypes::evtT_SOUND )
	{
		if ( nEvent.m_eventCommand == CEventCommands::evtC_SOUND_PLAY_IDX )
		{
			Play( nEvent.GetArgumentByName( L"sndIdx" )->m_asINT32, nEvent.GetArgumentByName( L"sndFlags" )->m_asUINT32 );
			return true;
		}
		if ( nEvent.m_eventCommand == CEventCommands::evtC_SOUND_STOP_IDX )
		{
			Stop( nEvent.GetArgumentByName( L"sndIdx" )->m_asINT32, nEvent.GetArgumentByName( L"bFadeOut" )->m_asBool );
			return true;
		}
		if ( nEvent.m_eventCommand == CEventCommands::evtC_SOUND_PLAY_HASH )
		{
			PlayID( nEvent.GetArgumentByName( L"sndHash" )->m_asUINT32, nEvent.GetArgumentByName( L"sndFlags" )->m_asUINT32 );
			return true;
		}
		if ( nEvent.m_eventCommand == CEventCommands::evtC_SOUND_STOP_HASH )
		{
			StopID( nEvent.GetArgumentByName( L"sndHash" )->m_asUINT32, nEvent.GetArgumentByName( L"fadeOut" )->m_asBool );
			return true;
		}
	}

	return false;
}


///**************************************************************************************
/// Sigleton 
///**************************************************************************************

CSoundManager& __Audio()
{
	static CSoundManager g_SoundMgr;
	return g_SoundMgr;
}


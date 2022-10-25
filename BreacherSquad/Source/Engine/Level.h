#pragma once

#include "gameplay/GameConstants.h"
#include "gameplay/LevelDefines.h"
#include "gameplay/Tile.h"
#include "gameplay/SmartLink.h"

//#include "gameplay/PhysicsPoint.h"
#include "gameplay/ActorTypes.h"
#include "gameplay/LevelTypes.h"
#include "gameplay/components/ActorAICompTypes.h"
#include "gameplay/Level_doofers.h"
#include "gameplay/Level_scriptable.h"

#include "gameplay/ActiveInterface.h"
#include "gameplay/components/ActiveAIComp.h"
#include "gameplay/Light.h"
#include "gameplay/Prop.h"
#include "gameplay/Level_bullets.h"
#include "gameplay/Level_weapons.h"
#include "gameplay/Actor.h"
#include "gameplay/Level_visibility.h"
#include "gameplay/CollisionShape.h"

#include "gameplay/TileBlockMesh.h"
#include "gameplay/LevelArea.h"
#include "gameplay/MissionStory.h"

#include "astar/Pathfinder.h"

#include "CFOVUtil.h"
#include "IngameGUI.h"

using namespace std;
// texture IDs for local texture manager
#define TEXID_TILES_COLOR		HASH("texTilesColor")
#define TEXID_TILES_NORMALS		HASH("texTilesNormals")
#define TEXID_WATER_DETAILS		HASH("texWaterDetail")

enum ELevelState {
	K_LVL_STATE_PLAYING,
	//next items only level finished states (see main.cpp network sync condition)
	K_LVL_STATE_MISSION_ACCOMPLISHED,
	K_LVL_STATE_MISSION_FAILED,
};

class CLevel : public IScriptable
{
public:
	double					fLocalTimeline;					// local ingame timeline
	CTimersArray			m_Timers;						// array of timers used ingame

	CRandom					m_rand;							// network synced random generator
	///--- level state ---
	ELevelState				m_levelState;
	int						m_levelSubState;
	int						m_levelStateParam;				// param sent to level state change action
	float					m_levelStateTimer;				// timer used for some level changes 
	void					SetLevelState( ELevelState eNewState, int nLevelStateParam = 0 );

	bool					m_bLoaded;						// is level loaded?
	int						m_nLoadedLevel, m_nLoadedChapter;			// level and episode of loaded level
	int						m_nLoadedLevelType;				// type of loaded level

	int						m_unLoadedLevelFlags;			// K_LVL_LEVEL_FLAG_ set at level loading (usually prevent saving scores)

	bool					m_bOneUpdateDone;				//#HACK: tells you that at least one update was made so we're ok to paint

	float					m_fTimeMultiplier_real;			// current time multiplier - Don't set directly
	float					m_fTimeMultiplier;				// wanted dTime multiplier (TimeMultiplier_real converges to this)
	float					m_fTimeMultiplierDuration;		// how long the time shift lives
	void					SetTimeMultiplier( float fMultiplier, float fDuration );

	CLevel();
	~CLevel();

public:
	PDEVICE					m_pDevice;

	Pathfinder				m_astar;						// A-start Astar search engine
	CTextureManager			m_texManager;					// General texture manager for misc needed textures
	CSpriteLib				m_sprLights;					// light animations/sprites
	CSpriteLib				m_sprProps;						// decorations
	CMultiSpriteLib			m_sprActors;					// animations for the actors (main characters, enemies etc) appended from different bsx files
	CMissionStory			m_story;						// mission story

	CVisibilityLists		m_visibleList;					// list of visible/active entities
	void					BuildVisibilityLists();
	void					ClearVisibilityLists();

	CBufferedPainter		m_bufferedPainter;				// used when drawing dynamic meshes

	int						tileW, tileH;					// size of tiles
	RectXYWH				m_levelAABB;					// level AABB in pixels - grows when adding areas
	RectXYWHi				m_levelAABB_TL;					// level AABB in tiles  - grows when adding areas

	Vec2					m_vLevelOrigin;					// level origin for the editor (usually around start location)

	vector<RectXYWHi>		m_arrDirtyRectsTL;				// tiles that need updating
	CArray<CLevelArea*>		m_arrAreas;						// loaded areas
	// Transforms mouse coordinates from screen space to game world (necessary for network play)
	bool					NormalizeMouseCoords( int ControllerIID, float fAxisValue, bool bIsHorizontalAxis, float & ret_fAxisValue );
	// Builds frame-by-freame geometry for lights, water, etc (called on Update)
	void					BuildDynamicGeometry( CAABB camAABB );

	///--- AREAS ---
	// adds a dirty rect for updating
	void					AddDirtyRect( int x, int y, int w, int h );
	// Updates the tiles in the dirty rects (#TODO: should return if changes were made)
	void					UpdateDirtyRects();
	// Updates areas visibility and returns number of visible areas
	int						Areas_UpdateVisibility( RectXYWH camRect );
	// Paints tile layer for visible areas
	OPRESULT				Areas_PaintLayer( eAreaLayer layerIdx );
	// Tells if area layer is renderable
	bool					Areas_IsLayerVisible( eAreaLayer layerIdx );
	// Returns array of areas that intersect aabb
	vector<CLevelArea*>		Areas_GetAreasInRect( CAABB aabb );
	// Returns area at point or null if no area there
	CLevelArea*				Areas_GetAt( Vec2 vPos );
	// Returns area with specified ID
	CLevelArea*				Areas_GetByID( UINT32 nID );
	// returns tile at position or null if not found
	CTile*					Areas_GetTileAt( Vec2 vPos );
	// Adds all the tiles in srcRectTL (in tile coords) from all overlapped areas to arrTiles[x + y * w]. 
	// srcRectTL will be part of the level bbox in tile coords. Make sure arrTiles is large enough. Array will be cleared inside the function.
	// arrTiles is an array of CTile pointers
	void					Areas_GetTilesSnapshot( RectXYWHi srcRectTL, CTile** arrTiles, int arrCapacity );
	// Returns true if static srcRect collides with anything (walls, solid boxes, etc)
	// Used to find valid spots for spawning and other "one time only" stuff
	// Do not use for realtime collision detection!
	bool					Areas_IsBoxColliding( CAABB srcBox, bool bCheckProps );
	// Sweeping collision, more expensive, finds collision on moving box
	bool					Areas_IsBoxColliding( CAABB srcBox, Vec2 vecMove, bool bCheckProps );
	// Smooths a path given as an array of world coords writing the final checkpoints as world coordinates in arrOutPoints
	// Returns number of waypoints or 0 if error
	int						SmoothPath( Vec2* arrInPoints, int arrInItems, Vec2* arrOutPoints, int arrOutSize );
	int						SmoothPathEx( CActor * act, Vec2* arrInPoints, int arrInItems, Vec2* arrOutPoints, int arrOutSize );

	///--- OBJECT INTERACTION ---
	// list of all possible actions ingame (they get copied on iActives)
	vector<CScriptAction>	m_arrActionTemplates;
	// returns script action by ID
	OPRESULT				GetScriptAction( const WCHAR* strID, CScriptAction& retAction );
	// Loads actions, inventory objects, etc
	OPRESULT				LoadLevelDefines( WCHAR* strPath );

	///--- TEMPLATES ---
	CArray<CWeaponTemplate*>		m_arrTemplatesWeapon;
	CArray<CExplosionTemplate*>		m_arrTemplatesExplosion;
	CWeaponTemplate*		GetTemplateWeapon( WCHAR * templateName );
	CWeaponTemplate*		GetTemplateWeapon( DWORD templateNameHash );
	CExplosionTemplate*		GetTemplateExplosion( UINT32 templateNameHash );
	OPRESULT				LoadWeaponTemplates( WCHAR * xmlPath );
	// Creates a weapon and returns pointer to it (does not deallocate)
	CWeapon*				Weapon_Create( WCHAR* weaponTemplateName, CActor* pParent );

	CArray<CActorTemplate*> m_arrTemplatesActor;	// Actor Templates array
	CArray<CAITemplate*>	m_arrAItemplates;		// AI behavior templates
	CActorTemplate*			Actor_GetTemplate( const WCHAR * templateName );
	CActorTemplate*			Actor_GetTemplate( const DWORD templateNameHash );
	// Randomizes the actor a little so they don't all have the exact same speeds
	void					RandomizeTemplateActor( CActorTemplate * actTemplate );
	CActorTemplate*			Actor_LoadTemplate( WCHAR * strTemplateFileName );

	CArray<CCollisionShape*>	m_arrColShapes;
	// returns intersection with a collision shape. Like AABB_Segment_Intersection_Arr but with collision shapes
	CCollisionShape*		ColShape_Segment_Intersection_Arr( Vec2 & start, Vec2 & end, CCollisionShape * arrBoxes[], int nBoxesCnt, Vec2 * retCollisionPoint, Vec2 * retNormal );
	// returns the first intersection of aabbSRC with a Collision Shape
	CCollisionShape*		ColShape_CAABB_Intersect_Arr( CAABB& aabbSrc, CCollisionShape * arrBoxes[], int nBoxesCnt );
	// returns segment intersection with tiles, starting form vStart
	CTile*					SegmentTilesIntersection( Vec2 vStart, Vec2 vEnd, Vec2 & retPoint, Vec2 & retNormal, Vec2i * hitTilePosTL = nullptr );
	// optimized version that only checks the neighbours of the starting area (and detects it if not provided)
	CTile*					SegmentTilesIntersectionEx( Vec2 vStart, Vec2 vEnd, Vec2 & retPoint, Vec2 & retNormal, Vec2i * hitTilePosTL = nullptr, CLevelArea* pStartArea = nullptr );

	CArray<CActor*>			m_arrActors;				// actors list
	//sets the current actor's weapon and template upgrades and limitations generated by the weapon	
	void					SetActorWeaponPerks( CActor * pActor, CWeapon * pWeapon );
	// Sets damage over time
	void					SetActorDoT( CActor* act, CDamageOverTime::EDoTType eType, float fDuration, float fDamagePerSec, EActorClass eExcludedClass, EActorClass eFilterClass, DWORD dwOwnerUID );
	//Kills the actor
	void					KillActor( CActor * actor, bool bSplatTarget = false );
	// Spawns a player
	void					SpawnPlayer( Vec2 spawnPos, int nPlayerOrdinal, int nAnimset = 0 );
	// Spawns an actor (NPC)
	CActor*					SpawnActor( Vec2 spawnPos, WCHAR* strTemplateFileName, CStringHash* shStateOverride = null );
	// Spawns a new Active with empty properties
	CProp*					SpawnProp( CLevelArea* pArea, Vec2 spawnPos, int nAnimIdx, int nFrameIdx );
	// Spawns a light
	CLight*					SpawnLight( Vec3 spawnPos, eLightType eType, DWORD dwColor, float fRadius = 64.0f, int profileID = 0, bool bCastShadows = false );
	///--- LIGHTS ---
	CArray<CLight*>			m_arrLights;				//array of lights
	DWORD					m_colAmbientGlobal;			//global ambient color
	float					m_fThunderTimer;			//pentru desenarea efectului de thunder/lightning (0.0f - stopped)

	///--- AI ---
	CArray<CAIEvent*>		m_arrAIevents;
	// Releases all dead objects (bReleaseIt flag set) on a separate step so they don't get deallocated when still in visibility lists 
	void					CleanupDeadObjects();
	// Updates all IActiveInterface implementations
	void					UpdateAI( float dTime, bool bInEditor = false );
	//gaseste cel mai apropiat inamic vizibil
	CActor*					GetClosestTarget( CActor * sourceActor, EActorClass eTargetClassFilter1 = K_ACT_CLASS_ANY, EActorClass eTargetClassFilter2 = K_ACT_CLASS_ANY );
	// Finds closest visible actor of specified name (inside visibility radius)
	// @fMaxDistance - if greater than 0 then it overrides seeDistance
	CActor*					GetClosestActorByTemplateName( CActor * sourceActor, WCHAR * sTargetTemplateName, float fMaxDistance = 0.0f );
	// AI events (radius < 0.0f means infinite)
	void					AddAIEvent( EAIEventType eventType, UINT32 ownerUID, EActorClass ownerClass, Vec2 vPos, float radius, float duration = 0.6f );
	///--- decals ---
	//#TODO: move decals arrays on areas
	CArray<CDecal*>			m_arrDecals;
	void					AddDecal( EDecalLayer nLayer, Vec2 pos, int animIdx, int frameIdx = 0, DWORD color = 0xffffffff, bool bIsAnimated = false );
	void					UpdateDecals( float dTime );
	//adds a blood decal (bLarge when enemy was splattered)
	void					AddDecal_BloodSplat( Vec2 pos, bool bLarge, EActorClass eVictimClass = K_ACT_CLASS_ANY );
	///--- physics points ---
	//#TODO: remove?
	//CLinkedPool<CPhysicsPoint>	m_poolPhysPts; //pool de obiecte fizice
	//void					UpdatePhysicsPoints( float dTime );
	///--- bullets linked pool ---
	CLinkedPool<CBullet>	m_poolBullets;			// bullets pool
	int						m_bulletsMeshIdx;		// idx mesh bullets

	///--- room occluders ---
	int						m_fogofwarMeshIdx;		//idx mesh occluders

	///--- BULLETS ---
	// Shoots a bullet and returns a pointer to the actual bullet. Don't deallocate or make any changes on said pointer.
	CBullet*				ShootBullet( CBulletTemplate * bulletTemplate, EActorClass actorClass, UINT32 nOwnerUID, Vec3 vPos, Vec3 vShootDir );
	// Returns the closest bullet (or null) of nBulletType under fMaxDistance
	CBullet*				GetClosestBullet( Vec2 vCheckPos, EBulletType nBulletType, float fMaxDistance = 0.0f, UINT32 dwOwnerUID = 0 );
	// Releases all bullets of said type from specified owner
	void					ReleaseBulletType( int nBulletType, UINT32 nOwnerUID );
	void					UpdateBullets( float dTime );
	void					PaintBullets( eLVLRenderPass pass );
	// Marks bullets as killed and returns how many were marked 
	// \param dwOwnerUID - specifies the owner UID filter or leave 0 to ignore the owner flag
	int						KillBulletsOfType( int nBulletType, UINT32 dwOwnerUID = 0 );

	///--- level doofers pool ---
	int						m_propsLightsMeshIdx;		//mesh id for props lights
	CLinkedPool<CDoofer>	m_poolDoofers;
	// Adds a generic prop (physical particle)
	// \param nSubType - secondary type of the added Prop, handled differently on every prop
	void					AddDoofer( EDooferType type, Vec2 pos, Vec2 * speed, Vec2 * accel, int nSubType = 0 );
	// ads temp light doofer (dies after a while, for gunshots explosions and such)
	void					AddDoofer_Light( Vec2 pos, int nLightAnimIdx, float fDuration, float fFadeTime, DWORD color, float fScale = 1.0f );
	// \brief helper fn: adds an explosion (logic and visual)
	// \param vDir - for directional explosions like breaching charges
	// \param hash_EXPLO_name - predefined constants for explosion params
	void					AddDoofer_Explo( UINT32 hash_EXPLO_name, Vec2 pos, UINT32 dwOwnerUID, int exploOwnerClass = K_ACT_CLASS_PLAYER, Vec2 vExploDir = { 0.0f, 0.0f }, CAABB* exploAABB = null );
	// Updates all doofers
	void					UpdateDoofers( float dTime );
	// Paints all doofers
	void					PaintDoofers( eLVLRenderPass pass );
	///--- efecte speciale ---
	void					GenerateEffect( ELVLEffectType nEffectType, Vec2 pos, float fSize, DWORD color = 0xffffffff );
	void					GenerateEffect( CStringHash sEffectName, Vec2 pos, float fSize, DWORD color = 0xffffffff );
	///--- misc objects (rails, etc) ---
	CArray<CMiscObjectBase*>	m_arrMiscObjects;

	//pointer to player and controls
	int						m_nPlayers;			//cati playeri joaca jocul same screen
	int						m_nPlayersActive;	//cati players sunt activi, not in limbo
	CActor*					pPlayerActor[K_MAX_PLAYERS_CNT];				//direct pointers to player controllers
	int						m_arrPlayerControllersIIDs[K_MAX_PLAYERS_CNT];	//used to save player controllers IIDs for each player
	int						m_arrPlayerSelHotJoin[K_MAX_PLAYERS_CNT];		//hot join selection
	///------ sync check ------
	DWORD					m_dwSyncCheckHash;		//used to sync network players by adding float actor data, hashing it and sending it over the network
	//strategic abilities
	//selectia strategic points (-1 for no selection)
	int						m_arrPlayerSelStrategic[K_MAX_PLAYERS_CNT];
	//strategic ability for each point (eg: body armor 1pt, extra life 8pts) - ultima abilitate se completeaza din playerSelScreen
	int						m_arrStrategicAbilities[K_MAX_PLAYERS_CNT][K_LVL_MAX_STRATEGIC_POINTS];
	//strategic ability names
	int						m_arrStrategicAbilitiesNames[K_MAX_PLAYERS_CNT][K_LVL_MAX_STRATEGIC_POINTS];
	//initializes strategic abilities arrays arrays
	void					InitializeStrategicAbilities( int nPlayerOrdinal );
	// Last valid spawning pos (level start flags or checkpoints)
	Vec2					vLastSpawnPoint;

	///--- STATISTICS ---
	int						m_arrStats[K_LVL_STATS_CNT];		//array that holds the statistics
	void					ResetLevelStatistics();
	void					IncreaseLevelStatistics( int K_LVL_STATS_n, int nValueToAdd = 1 );
	// Gives strategic points for the strategic points bar 
	// \param: vPos - if set it adds a text particle with the value
	void					GiveStrategicPoints( float fPoints, Vec2 * vPos = null );
	// Activates special ability, if possible
	// \param: nAbilityIdx - ability index in g_arrStrategicAbilities and price at the same time
	// \returns: true if success, false if failed
	bool					ActivateSpecialAbility( int nAbilityIdx, int nTargetPlayerOrdinal );
	// Do we have a line of sight between the 2 points
	bool					IsLineOfSight( Vec2 pt_from, Vec2 pt_to, CLevelArea * pStartArea = nullptr );

	UINT32					m_unLastID;				//Last loaded ID - used to assign unique IDs to runtime spawned elements
	//Generates a new editor ID and increments m_unLastID (used when appending areas)
	UINT32					GenerateNextID();
	// Loads a level from an absolute path
	OPRESULT				LoadLevel( WCHAR * strPathAbs );
	// Loads a new area and adds it to the level (absolute path, real drive path)
	// Adds all elements to the level arrays too
	OPRESULT				LoadArea( WCHAR * strPathAbs, UINT32 nAreaID, Vec2i posTL );
	// Releases all level data
	void					Release();
	// Gives you a random level from a shuffled list so you play all of them in random order
	int						GetNextRandomLevel();
	// Returns pointer to active 
	IActiveInterface*		GetIActiveInterfacePtr( int editorID );
	IActiveInterface*		GetIActiveInterfacePtr_byUID( UINT32 UID );
	//Intoarce pointer la CActive cu UID-ul respectiv
	CProp*					GetActiveByUID( UINT32 UID );
	CActor*					GetActorByUID( UINT32 UID );
	CLight*					GetLightByUID( UINT32 UID );
	// Gets the player with specified UID or NULL if not found
	CActor*					GetPlayerByUID( UINT32 UID );
	// Finds the closest player (visible or not)
	CActor*					GetClosestPlayer( CActor* sourceActor, bool bIgnoreDead = false );
	CActor*					GetClosestPlayer( Vec2 vSrcPos, bool bIgnoreDead = false );
	//tells if pPlayer is networked
	bool					IsNetworkPlayer( CActor* pPlayer );
	//intoarce collision shape-ul care contine punctul point si este de tipul collisionType
	CCollisionShape*		GetCollisionShapeAt( Vec2 point, int collisionType = -1 );
	//gets a collision shape by UID
	CCollisionShape*		GetCollisionShapeByUID( UINT32 nUID );
	//spawn a new collision box
	CCollisionShape*		SpawnCollisionShape( ECollType newType, Vec2 vMin, Vec2 vMax );
	///--- pt vizualizare ---
	IActiveInterface		*m_camTargetActive;		//la ce activ se uita camera sau null cand se uita la players
	IActiveInterface		*m_camTargetOld;		//tine minte pe ce a fost locked ca sa se poata intoarce
	CCameraTransform		m_camLevelToRT;			// camera from level to RT
	CCameraTransform		m_camLevelToScr;		// camera from level to Screen
	Vec2					m_vCamPosDefault;		//camera position when not locked on special actors (hidden rooms, etc)
///--- misc ---
	// Returns the number of XP points gained after current mission
	int						Local_ComputeMissionXP( int nStars );

	///-- update/paint --	
		// Fixed timestep update - game, physics, simulation dependant things
		// Called BEFORE the variable step Update
	void					UpdateFixedTimestep( float dTime_original );
	// Variable dTime update - camera, non game specific updates
	void					Update( float dTime );

	//#TODO: to be replaced with generic function that takes a "channel" param
	// fBetweenFramesPercent is the time accumulator remainder after updateing full simulation fixed steps so we can extrapolate positions and eliminate stutter
	OPRESULT				PaintDeferredBuffers( float fBetweenFramesPercent );
	// renders level pass
	OPRESULT				RenderPass( eLVLRenderPass ePass, Mat* matProj, float fBetweenFramesPercent );
	// the lights pass is so very different that it needs a special function
	OPRESULT				RenderPass_Lights( Mat* matProj, float fBetweenFramesPercent );
	// composes color and lights into one RT
	OPRESULT				RenderPass_Composition( Mat* matProj, float fBetweenFramesPercent );

	// paint level buffers onscreen
	void					Paint();
	// returns pointer to synced RNG
	inline CRandom&	RNG() {
		return m_rand;
	}

	///--- interfaces ---
	CSpriteLib				m_sprInterface;
	// Ingame Interface
	CIngameGUI				m_interfaceIGM;

	// garbage collect
	void					GC();

	///--- SCRIPT ---
		//activeaza cel mai apropiat obiect, primul gasit
	void					TouchClosestActive( CActor * pToucherAct, float dTime );
	// RETURNS: true - instr processed, false - not processed
	bool					ProcessScriptInstruction( CScriptInstruction *instr, UINT32 executorUID, UINT32 scriptUID ) override;
	bool					OnScriptFinished( UINT32 executorUID, UINT32 scriptUID, CVariantMap * pArrScriptVars ) override;
	char const *			GetScriptProcessorName() override { return "CLevel"; }
	// functie ajutatoare pentru procesare instructiuni. Gaseste activ in fn de valoare parametru: SELF pt caller, TARGET pentru targetID si numar pt ID efectiv
	IActiveInterface*		ScriptGetActiveInterfaceByTargetParam( CVariant* vcTarget, UINT32 executorUID );

	// Gets all occluders for a specific light
	// includes tile segments, light range bbox segments and objects aabb segments. vEye MUST be inside bbox!
	// \returns Number of segments returned. Writes segments in provided pRetArr.
	int						GetOccluderSegments( Vec2 vEye, CAABB bbox, COccluderSegment* pRetArr, int maxRetArrSize );

private:
	// paints final game RT to screen coords and adds effects like explosions and other effects using the final RT
	OPRESULT				PaintGameFinalRT();
	// paints ingame elements in "Level To Screen Space" that are not influenced by shader effects (numbers, interact icons, health bars, etc)	
	OPRESULT				PaintOverGameLayer();
	// special layer (pixel perfect or different scaling) in "Scaled Screen Space" for ingame interface, menus and screen vignettes
	OPRESULT				PaintGUILayer();
	// Cheaper method of shadow casting but not precise enough. Can be used on low end devices
	int						BuildLightVolume360( CLight * light, _VERTEX_PNCT4T4 *outVerts, int outVertsMaxCnt );

public: //--- framework methods ---
	OPRESULT OnCreateDevice( PDEVICE pDevice, const SURFACE_DESC* pBBDesc = nullptr, void* pUserContext = nullptr );
	OPRESULT OnResetDevice( PDEVICE pDevice, const SURFACE_DESC* pBBDesc = nullptr, void* pUserContext = nullptr );
	OPRESULT OnLostDevice( void* pUserContext = nullptr );
	OPRESULT OnDestroyDevice( void* pUserContext = nullptr );
};

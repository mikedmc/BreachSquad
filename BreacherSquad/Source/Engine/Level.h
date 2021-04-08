#pragma once

#include "gameplay/GameConstants.h"
#include "gameplay/Tile.h"

#include "gameplay/PhysicsPoint.h"
#include "gameplay/LevelTypes.h"
#include "gameplay/Level_doofers.h"
#include "gameplay/Level_scriptable.h"

#include "gameplay/ActiveInterface.h"
#include "gameplay/CollisionShape.h"
#include "gameplay/Light.h"
#include "gameplay/Prop.h"
#include "gameplay/Level_visibility.h"
#include "gameplay/Level_bullets.h"
#include "gameplay/Level_weapons.h"
#include "gameplay/Actor.h"

#include "gameplay/TileBlockMesh.h"
#include "gameplay/LevelArea.h"
#include "gameplay/MissionStory.h"

#include "CFOVUtil.h"

using namespace std;


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
	void					SetLevelState(ELevelState eNewState, int nLevelStateParam = 0);

	bool					m_bLoaded;						// is level loaded?
	int						m_nLoadedLevel, m_nLoadedChapter;			// level and episode of loaded level
	int						m_nLoadedLevelType;				// type of loaded level

	int						m_unLoadedLevelFlags;			// K_LVL_LEVEL_FLAG_ set at level loading (usually prevent saving scores)

	bool					m_bOneUpdateDone;				//#HACK: tells you that at least one update was made so we're ok to paint
	
	float					m_fTimeMultiplier_real;			// current time multiplier - Don't set directly
	float					m_fTimeMultiplier;				// wanted dTime multiplier (TimeMultiplier_real converges to this)
	float					m_fTimeMultiplierDuration;		// how long the time shift lives
	void					SetTimeMultiplier(float fMultiplier, float fDuration);

	CLevel();
	~CLevel();

public:
	PDEVICE					m_pDevice;		
	ID3DXSprite*			m_pSprite; 
	void SetSpritePtr(ID3DXSprite* pSprite) {
		m_pSprite = pSprite;
	}

	CTextureManager			m_texManager;					// General texture manager for misc needed textures
	CSpriteCollection		m_sprLights;					// light animations/sprites
	CSpriteCollection		m_sprProps;						// decorations
	CSpriteCollection		m_sprActors;					// animations for the actors (main characters, enemies etc)
	CMissionStory			m_story;						// mission story

	CVisibilityLists		m_visibleList;					// list of visible/active entities
	void					BuildVisibilityLists();
	void					ClearVisibilityLists();

	CBufferedPainter		m_bufferedPainter;				// used when drawing dynamic meshes

	int						tileW, tileH;					// size of tiles
	RECTXYWH_F				m_levelAABB;					// level AABB in pixels - grows when adding areas
	RECTXYWH				m_levelAABB_TL;					// level AABB in tiles  - grows when adding areas
	int						m_tilesTexBaseIdx;				// tileset base texture index
	int						m_tilesTexNormIdx;				// tileset normals texture index
	Vec2					m_vLevelOrigin;					// level origin for the editor (usually around start location)

	vector<RECTXYWH>		m_arrDirtyRectsTL;				// tiles that need updating
	CGrowableArray<CLevelArea*>		m_arrAreas;				// loaded areas
	// Transforms mouse coordinates from screen space to game world (necessary for network play)
	bool					NormalizeMouseCoords(int ControllerIID, float fAxisValue, bool bIsHorizontalAxis, float & ret_fAxisValue);
	// Builds frame-by-freame geometry for lights, water, etc (called on Update)
	void					BuildDynamicGeometry(CAABB camAABB);

	///--- AREAS ---
	// Updates the tiles in the dirty rects (should return if changes were made)
	void					UpdateDirtyRects();
	// Updates areas visibility and returns number of visible areas
	int						Areas_UpdateVisibility(RECTXYWH_F camRect);
	// Paints tile layer for visible areas
	OPRESULT				Areas_PaintLayer(eAreaLayer layerIdx);
	// Returns array of areas that intersect aabb
	vector<CLevelArea*>		Areas_GetAreasInRect(CAABB aabb);
	// Returns area at point
	CLevelArea*				Areas_GetAt(Vec2 vPos);
	// Returns area with specified ID
	CLevelArea*				Areas_GetByID(UINT32 nID);
	// Adds all the tiles in srcRectTL (in tile coords) from all overlapped areas to arrTiles[x + y * w]. 
	// srcRectTL will be part of the level bbox in tile coords. Make sure arrTiles is large enough. Array will be cleared inside the function.
	// arrTiles is an array of CTile pointers
	void					Areas_GetTilesSnapshot(RECTXYWH srcRectTL, CTile** arrTiles, int arrCapacity);

	///--- TEMPLATES ---
	CGrowableArray<CWeaponTemplate*>		m_arrTemplatesWeapon;
	CGrowableArray<CExplosionTemplate*>		m_arrTemplatesExplosion;
	CWeaponTemplate*		GetTemplateWeapon(WCHAR * templateName);
	CWeaponTemplate*		GetTemplateWeapon(DWORD templateNameHash);
	CExplosionTemplate*		GetTemplateExplosion(UINT32 templateNameHash);
	OPRESULT				LoadWeaponTemplates(WCHAR * xmlPath);
	// Creates a weapon and returns pointer to it (does not deallocate)
	CWeapon*				Weapon_Create(WCHAR* weaponTemplateName, CActor* pParent);

	CGrowableArray<CActorTemplate*> m_arrTemplatesActor;	//actor templates array
	CGrowableArray<CAITemplate*>	m_arrAItemplates;		//array folosit pentru salvarea template-urilor AI
	CActorTemplate*			Actor_GetTemplate(const WCHAR * templateName);
	CActorTemplate*			Actor_GetTemplate(const DWORD templateNameHash);
	// Randomizes the actor a little so they don't all have the exact same speeds
	void					RandomizeTemplateActor(CActorTemplate * actTemplate);
	CActorTemplate*			Actor_LoadTemplate(WCHAR * strTemplateFileName);

	CGrowableArray<CCollisionShape*>	m_arrColShapes;
	// returns intersection with a collision shape. Like AABB_Segment_Intersection_Arr but with collision shapes
	CCollisionShape*		ColShape_Segment_Intersection_Arr(Vec2 & start, Vec2 & end, CCollisionShape * arrBoxes[], int nBoxesCnt, Vec2 * retCollisionPoint, Vec2 * retNormal);
	// returns the first intersection of aabbSRC with a Collision Shape
	CCollisionShape*		ColShape_CAABB_Intersect_Arr(CAABB * aabbSrc, CCollisionShape * arrBoxes[], int nBoxesCnt);
	// returns segment intersection with tiles, starting form vStart
	CTile*					SegmentTilesIntersection(Vec2 vStart, Vec2 vEnd, Vec2 & retPoint, Vec2 & retNormal, Vec2i * hitTilePosTL = nullptr);

	CGrowableArray<CProp*>	m_arrProps;					// objects list
	CFixedArray<CProp*, 256>	m_arrPropsPtrInteract;	// array containing objects that you can interact with (for speed checks)

	CGrowableArray<CActor*>	m_arrActors;				// actors list
	// Seteaza noua stare si are in vedere si incheierea starii precedente
	void					Actor_SetAIState(CActor * actor, CAIState* pNewState);
	//sets the current actor's weapon and template upgrades and limitations generated by the weapon	
	void					SetActorWeaponPerks(CActor * pActor, CWeapon * pWeapon);
	// Seteaza noua stare si are in vedere si incheierea starii precedente
	// \returns true:success false:state not found
	bool					Actor_SetAIState(CActor * actor, WCHAR * strStateName);
	// Sets behavior by idx, from current state behaviors array
	// \param: ret_bFinished - set to true if current behavior doesn't need an update (like set_animation or set_flag, etc)
	// \returns: true if set, false if error
	bool					SetActorAIBehaviorIdx(CActor * actor, int nBehaviorIdx, bool &ret_bFinished);
	// Sets damage over time
	void					SetActorDoT(CActor* act, CDamageOverTime::EDoTType eType, float fDuration, float fDamagePerSec, EActorClass eExcludedClass, EActorClass eFilterClass, DWORD dwOwnerUID);
	// Called when changing behaviors (to exit them gracefully)
	void					OnActorBehaviorFinished(CActor * actor, EAIBehaviorType eOldBehavior);
	//Kills the actor
	void					KillActor(CActor * actor, bool bSplatTarget = false);
	// Use it to damage enemies and player
	// @fHitPointsTaken - negative value - kills it immediately
	// RETURNS: damage made, type of material hit.
	CBulletHitReturnData	HitActor(CActor * actor, CBullet * pBullet, Vec2 * pvProjectileMomentum = NULL);
	
	//hits the actor with other things than bullets
	CBulletHitReturnData	HitActor(CActor * actor, float fDamage, UINT32 dwOwnerUID, EActorClass eOwnerClass, Vec2 *vDir = null, UINT32 dwBulletFlags = 0, int nArmorPiercingRating = 100, float fStunDuration = 0.0f);
	void					SetActorStun(CActor* actor, float fStunDuration);
	// Spawns a player
	void					SpawnPlayer(Vec2 spawnPos, int nPlayerOrdinal, int nAnimset = 0);
	// Spawns an actor (NPC)
	CActor*					SpawnActor(Vec2 spawnPos, WCHAR* strTemplateFileName, CStringHash* shStateOverride = null);
	// Spawns a new Active with empty properties
	CProp*					SpawnProp(Vec2 spawnPos, int nAnimIdx, int nFrameIdx, int nLayer = K_TILE_LAYER_FLOOR);
	// Spawns a light
	CLight*					SpawnLight(Vec3 spawnPos, eLightType eType, DWORD dwColor, float fRadius = 64.0f, int profileID = 0, bool bCastShadows = false);
	// Gives a score for the user powerups placement 
	// \brief: used to move player spawned objects away from intersections with other interactibles and walls
	int						GetPowerupPlacingScore(CProp * active, Vec2 vPlacerPos);
	// Finds the best spawning rect for a proposed position
	// \returns false when can't be spawned safely
	// \param rectProposed_ret - the proposed placing rectangle
	bool					GetBestSpawningPos(Vec2 * vSpawn_ret, CAABB rectStart, CAABB * rectToAvoid = NULL);
	///--- LIGHTS ---
	CGrowableArray<CLight*> m_arrLights;				//array of lights
	DWORD					m_colAmbientGlobal;			//global ambient color
	float					m_fThunderTimer;			//pentru desenarea efectului de thunder/lightning (0.0f - stopped)
	
	///--- AI ---
	CGrowableArray<CAIEvent*> m_arrAIevents;
	/* Releases all dead objects (bReleaseIt flag set) on a separate step so they don't get deallocated when still in visibility lists */
	void					CleanupDeadObjects();
	/*--- updates all IActiveInterface implementations ---*/
	void					UpdateAI(float dTime, bool bInEditor = false);
	//updates AI for base class (common AIs)
	bool					UpdateAI_base(IActiveInterface* active, float dTime, double fTimeline);
	//updates AI for derived classes (particulare)
	void					UpdateAI_light(CLight* light, float dTime);
	void					UpdateAI_prop(CProp* prop, float dTime);
	void					UpdateAI_actor(CActor* actor, float dTime);
	void					UpdateAI_collshape(CCollisionShape * colshape, float dTime);
	//Seteaza AI si face toate setarile initiale din AI
	void					SetAI(IActiveInterface * active, EAIstate AIstate, CVariantCollection * params, INT32 targetID = -1);
	//Suprascrie parametrii din AI sau adauga params noi
	void					SetAIparams(IActiveInterface * active, CVariantCollection * params, bool bClearParams = false);
	//gaseste cel mai apropiat inamic vizibil
	CActor*					GetClosestTarget(CActor * sourceActor, EActorClass eTargetClassFilter1 = K_LVL_ACT_CLASS_ANY, EActorClass eTargetClassFilter2 = K_LVL_ACT_CLASS_ANY);
	// Finds closest visible actor of specified name (inside visibility radius)
	// @fMaxDistance - if greater than 0 then it overrides seeDistance
	CActor*					GetClosestActorByTemplateName(CActor * sourceActor, WCHAR * sTargetTemplateName, float fMaxDistance = 0.0f);
	//AI events (radius < 0.0f means infinite)
	void					AddAIEvent(EAIEventType eventType, UINT32 ownerUID, int ownerClass, Vec2 vPos, float radius, float duration = 0.6f, UINT32 targetUID = 0);
	// Deletes a targeted event
	// @targetUID - if not set it deletes all events of said type
	void					DeleteAITargetedEvent(EAIEventType eEvtType, UINT32 targetUID = 0);
	//Gaseste cel mai apropiat event (de tipul typeFilter daca il specific) cu linie directa de vedere
	CAIEvent*				GetMostImportantAIEvent(CActor * callerActor, EAIEventType eTypeFilter = K_LVL_AI_EVENT_ANY);
	///--- decals ---
	CGrowableArray<CDecal*> m_arrDecals;
	void					AddDecal(EDecalLayer nLayer, Vec2 pos, int animIdx, int frameIdx = 0, DWORD color = 0xffffffff, bool bIsAnimated = false);
	void					UpdateDecals(float dTime);
	//adds a blood decal (bLarge when enemy was splattered)
	void					AddDecal_BloodSplat(Vec2 pos, bool bLarge, EActorClass eVictimClass = K_LVL_ACT_CLASS_ANY);
	///--- physics points ---
	CLinkedPool<CPhysicsPoint>	m_poolPhysPts; //pool de obiecte fizice
	void					UpdatePhysicsPoints(float dTime);
	///--- bullets linked pool ---
	CLinkedPool<CBullet>	m_poolBullets;			//pool-ul de gloante
	int						m_bulletsMeshIdx;		//idx gloante
	CFixedArray<CBullet*, 128> m_arrBulletsTemp;	//Temporary bullets list used for misc checks

	///--- water ---
	int						m_waterMeshIdx;			//idx la meshul apelor vizibile in ecran
	int						m_waterTexIdx;			//idx textura normale apa in texManager
	int						m_waterAnimIdx;			//idx animatie apa din fisierul de fundal
	///--- room occluders ---
	int						m_fogofwarMeshIdx;		//idx mesh occludere

	///--- WEAPONS ---
	// can we shoot the weapon? some weapons have pre-shoot conditions (maybe not working underWATER)
	bool					Weapon_CanShoot(CWeapon * weapon);
	// Trage cu arma specificata
	// \returns: true daca a putut sa traga sau false daca nu
	bool					Weapon_Shoot(CWeapon * weapon, Vec3 vDir);
	// \returns: weapon status
	EnumWeaponStatus		Weapon_Update(CWeapon * weapon, float dTime);
	
	// \returns: true if started reloading, false if already full or no ammo
	bool					Weapon_Jam(CWeapon * weapon);
	// stops reloading the weapon
	void					Weapon_StopReloading(CWeapon * weapon);
	// seteaza comenzi arma
	void					Weapon_ResetBurst(CWeapon * weapon);

	///--- BULLETS ---
	// Shoots a bullet and returns a pointer to the actual bullet. Don't deallocate or make any changes on said pointer.
	CBullet*				ShootBullet(CBulletTemplate * bulletTemplate, int actorClass, UINT32 nOwnerUID, Vec3 vPos, Vec3 vShootDir);
	// Returns the closest bullet (or null) of nBulletType under fMaxDistance
	CBullet*				GetClosestBullet(Vec2 vCheckPos, EBulletType nBulletType, float fMaxDistance = 0.0f, int dwOwnerUID = 0);
	// Releases all bullets of said type from specified owner
	void					ReleaseBullet(int nBulletType, UINT32 nOwnerUID);
	void					UpdateBullets(float dTime);
	void					PaintBullets(eLVLRenderPass pass);
	// Marks bullets as killed and returns how many were marked 
	// \param dwOwnerUID - specifies the owner UID filter or leave 0 to ignore the owner flag
	int						KillBulletsOfType(int nBulletType, UINT32 dwOwnerUID = 0);

	///--- level doofers pool ---
	int						m_propsLightsMeshIdx;		//id-ul meshului pentru desenarea luminii propsurilor
	CLinkedPool<CDoofer>	m_poolDoofers;		
	// Adds a generic prop (physical particle)
	// \param nSubType - secondary type of the added Prop, handled differently on every prop
	void					AddDoofer(EDooferType type, Vec2 pos, Vec2 * speed, Vec2 * accel, int nSubType = 0);
	//adauga prop - o lumina provizorie (gunshots, etc)
	void					AddDoofer_Light(Vec2 pos, int nLightAnimIdx, float fDuration, float fFadeTime, DWORD color, float fScale = 1.0f);
	// \brief helper fn: adds an explosion (logic and visual)
	// \param vDir - for directional explosions like breaching charges
	// \param hash_EXPLO_name - predefined constants for explosion params
	void					AddDoofer_Explo(UINT32 hash_EXPLO_name, Vec2 pos, UINT32 dwOwnerUID, int exploOwnerClass = K_LVL_ACT_CLASS_PLAYER, Vec2 vExploDir = { 0.0f, 0.0f }, CAABB* exploAABB = null);
	// Updates all doofers
	void					UpdateDoofers(float dTime);
	// Paints all doofers
	void					PaintDoofers();	  
	///--- efecte speciale ---
	void					GenerateEffect(ELVLEffectType nEffectType, Vec2 pos, float fSize, DWORD color = 0xffffffff);
	void					GenerateEffect(CStringHash sEffectName, Vec2 pos, float fSize, DWORD color = 0xffffffff);
	///--- misc objects (rails, etc) ---
	CGrowableArray<CMiscObjectBase*>	m_arrMiscObjects;

	//pointer to player and controls
	int						m_nPlayers;			//cati playeri joaca jocul same screen
	int						m_nPlayersActive;	//cati players sunt activi, not in limbo
	CActor*					pPlayerActor[K_MAX_PLAYERS_CNT];				//direct pointers to player controllers
	int						m_arrPlayerControllersIIDs[K_MAX_PLAYERS_CNT];	//used to save player controllers IIDs for each player
	int						m_arrPlayerSelHotJoin[K_MAX_PLAYERS_CNT];		//hot join selection
	Vec2					m_arrPlayerLastSafePos[K_MAX_PLAYERS_CNT];
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
	void					InitializeStrategicAbilities(int nPlayerOrdinal);
	// Last valid spawning pos (level start flags or checkpoints)
	Vec2					vLastSpawnPoint; 
	
	///--- STATISTICS ---
	int						m_arrStats[K_LVL_STATS_CNT];		//array that holds the statistics
	void					ResetLevelStatistics();
	void					IncreaseLevelStatistics(int K_LVL_STATS_n, int nValueToAdd = 1);
	// Gives strategic points for the strategic points bar 
	// \param: vPos - if set it adds a text particle with the value
	void					GiveStrategicPoints(float fPoints, Vec2 * vPos = null); 
	// Activates special ability, if possible
	// \param: nAbilityIdx - ability index in g_arrStrategicAbilities and price at the same time
	// \returns: true if success, false if failed
	bool					ActivateSpecialAbility(int nAbilityIdx, int nTargetPlayerOrdinal);
	// Do we have a line of sight between the 2 points
	bool					IsLineOfSight(Vec2 pt1, Vec2 pt2, Vec2 * retVecCollisionPt = null, Vec2 * retVecCollisionNormal = null);

	UINT32					m_unLastID;				//Last loaded ID - used to assign unique IDs to runtime spawned elements
	//Generates a new editor ID and increments m_unLastID (used when appending areas)
	UINT32					GenerateNextID();		
	// Loads a level from an absolute path
	OPRESULT				LoadLevel(WCHAR * strPathAbs);
	// Loads a new area and adds it to the level (absolute path, real drive path)
	// Adds all elements to the level arrays too
	OPRESULT				LoadArea(WCHAR * strPathAbs, UINT32 nAreaID, Vec2i posTL);
	// Releases all level data
	void					Release();
	// Gives you a random level from a shuffled list so you play all of them in random order
	int						GetNextRandomLevel();
	//intoarce pointer catre activul cu id-ul (din editor) respectiv - derivate din CActiveInterface
	IActiveInterface*		GetIActiveInterfacePtr(int ID);
	IActiveInterface*		GetIActiveInterfacePtr_byUID(UINT32 UID);
	//Intoarce pointer la CActive cu UID-ul respectiv
	CProp*					GetActiveByUID(UINT32 UID);
	CActor*					GetActorByUID(UINT32 UID);
	CLight*					GetLightByUID(UINT32 UID);
	// Gets the player with specified UID or NULL if not found
	CActor*					GetPlayerByUID(UINT32 UID);
	// Finds the closest player (visible or not)
	CActor*					GetClosestPlayer(CActor* sourceActor, bool bIgnoreDead = false);
	CActor*					GetClosestPlayer(Vec2 vSrcPos, bool bIgnoreDead = false);
	//tells if pPlayer is networked
	bool					IsNetworkPlayer(CActor* pPlayer);
	//intoarce collision shape-ul care contine punctul point si este de tipul collisionType
	CCollisionShape*		GetCollisionShapeAt(Vec2 point, int collisionType = -1);
	//gets a collision shape by UID
	CCollisionShape*		GetCollisionShapeByUID(UINT32 nUID);
	//spawn a new collision box
	CCollisionShape*		SpawnCollisionShape(int nType, Vec2 vMin, Vec2 vMax);
	///--- pt vizualizare ---
	IActiveInterface		*m_camTargetActive;		//la ce activ se uita camera sau null cand se uita la players
	IActiveInterface		*m_camTargetOld;		//tine minte pe ce a fost locked ca sa se poata intoarce
	CCameraTransform		m_camLevel;
	Vec2					m_vCamPosDefault;		//camera position when not locked on special actors (hidden rooms, etc)
///--- misc ---
	// Returns the number of XP points gained after current mission
	int						Local_ComputeMissionXP(int nStars);

///-- update/paint --	
	// Main level Update
	void					Update(float dTime_original);

	//#TODO: to be replaced with generic function that takes a "channel" param
	OPRESULT				PaintDeferredBuffers();
	OPRESULT				RenderPass(eLVLRenderPass ePass, Mat* matProj);
	// the lights pass is so very different that it needs a special function
	OPRESULT				RenderPass_Lights(Mat* matProj);
	// composes color and lights into one RT
	OPRESULT				RenderPass_Composition(Mat* matProj);

	// level paint into composition texture 
	void					Paint();
	// paints final res effects (water, distortion, icons, etc)
	HRESULT					PaintUsingFinalRTT();
	
///--- interfaces ---
	CSpriteCollection		m_sprInterface;
	//interfata in sine
	CCustomInterfaceIGM		m_interfaceIGM;
	//controlul de ingame hints
	CCustomInterfaceTextBubble m_interfaceTextBubble;

///--- SCRIPT ---
	//activeaza cel mai apropiat obiect, primul gasit
	void					TouchClosestActive(CActor * pToucherAct, float dTime);
	// Starts a script sending AI params as script local vars
	void					StartScript(WCHAR* scriptName, IActiveInterface * active);
	void					StartScript(UINT32 scriptNameHash, IActiveInterface * active);
	// RETURNS: true - instr processed, false - not processed
	bool					ProcessScriptInstruction(CScriptInstruction *instr, UINT32 executorUID, UINT32 scriptUID);
	bool					OnScriptFinished(UINT32 executorUID, UINT32 scriptUID, CVariantCollection * pArrScriptVars);
	char const *			GetScriptProcessorName(void) { return "CLevel"; }
	// functie ajutatoare pentru procesare instructiuni. Gaseste activ in fn de valoare parametru: SELF pt caller, TARGET pentru targetID si numar pt ID efectiv
	IActiveInterface*		ScriptGetActiveInterfaceByTargetParam(CVariantComplex* vcTarget, UINT32 executorUID);

	// Gets all occluders for a specific light
	// includes tile segments, light range bbox segments and objects aabb segments. vEye MUST be inside bbox!
	// \returns Number of segments returned. Writes segments in provided pRetArr.
	int						GetOccluderSegments(Vec2 vEye, CAABB bbox, COccluderSegment* pRetArr, int maxRetArrSize);

private:
	// Cheaper method of shadow casting but not precise enough. Can be used on low end devices
	int						BuildLightVolume360(CLight * light, _VERTEX_PNCT4T4 *outVerts, int outVertsMaxCnt);

public: //--- framework methods ---
	OPRESULT OnCreateDevice(PDEVICE pDevice, const SURFACE_DESC* pBBDesc = NULL, void* pUserContext = NULL);
	OPRESULT OnResetDevice(PDEVICE pDevice, const SURFACE_DESC* pBBDesc = NULL, void* pUserContext = NULL);
	OPRESULT OnLostDevice(void* pUserContext = NULL);
	OPRESULT OnDestroyDevice(void* pUserContext = NULL);
};

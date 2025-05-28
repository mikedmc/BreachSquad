#pragma once

#include "utils/VecProj.h"
#include "SmartLink.h"

// declare useful classes
class CLevelArea;

#define	K_LVL_AI_DECISION_INTERVAL				0.25f
#define	K_LVL_AI_DECISION_INTERVAL_VARIATION	0.05f

enum EActiveInterfaceType {
	K_LVL_IAI_TYPE_UNKNOWN,
	K_LVL_IAI_TYPE_BASE,		// basic IActiveInterface
	K_LVL_IAI_TYPE_LIGHT,
	K_LVL_IAI_TYPE_PROP,
	K_LVL_IAI_TYPE_ACTOR,
	K_LVL_IAI_TYPE_COLSHAPE,
};

///----------------------------------------------------------------------------------
/// Base class for all active elements (actors, objects, etc) 
///----------------------------------------------------------------------------------
class IActiveInterface
{
protected:
	bool					bPendingKill;			// exited gameplay, waits for garbage collection
	bool					bEnabled;				// sometimes the actives need to be disabled ( eg: after being killed )
#if defined(_DEBUG) || defined(DEBUG)
	float					fPendingKillTimer;		//#TEMP: checks time since killed to make sure it's deallocating them
#endif

public:
	EAIstate				AIstate;				// state AI (AI_STATE ENUM)
	CVariantMap				varAIparams;			// AIstate params (partially coming from the editor)

public:
	UINT32					UID;					// ingame UID
	CLevelArea*				pArea;					// pointer to current area
	int						ID;						// ID exported from editor (not the same as UID).
	int						editor_paintOrderIdx;			// Paint order saved like index from editor when importing object
	VecProj					pos;					// Z coord gets added to Y to simulate 3D when rendering (see Z_TO_H macros)
	VecProj					pos_ini;				// initial position needed for relative calculations. Usually only set when spawned.

	bool					bTouching;				// set to true while being touched so we don't start another
	UINT32 					nTouchingUID;   		// UID of toucher or 0 for none
													   
	DWORD					color;
	int						heightZ;				// height of object, same as in bbox Z axis
	CAABBEx					bbox;					// bbox in projected screen space, with backup copy inside. Derived from editor.
	CAABBEx					bbox_floor;				// bbox that represents the floor rectangle. has backup copy. From editor/setup.
	CAABBEx					bbox_cull;				// bbox used for culling, big enough to contain all gfx

public: 
	DWORD					color_ini;
	INT32					targetID_ini;			// target ID read from the editor
	CSmartLink				pTarget;				// smart link to another object (self resetting)

	bool					bCanInteract;			// can interact with it?  #TODO: replace with interact-type or actions list
	bool					bHideInteractIcon;		// hide the icon //#TODO: remove this flag

	CFixedArray<CScriptAction, 10>	arrActions;		// array of possible actions on this object (does not include actions from inventory and actors)
	CStringHash				shScriptActions;		// string containing script actions names for matching (eg. BREACH,LOCKPICK)
	UINT32					nRunningScriptUID;		// UID of script that is running now on this element

	bool					bSetEnabled;			// sets the bEnabled flag when updating the active. Is it still useful? It was needed when setting the object to enabled after it had been processed.
	bool					bAnimated;				// este animat? daca da face play la animatie
	bool					bSkipRender;			// skips render...

	//CTOR/DTOR
	IActiveInterface();
	virtual ~IActiveInterface();

	virtual const EActiveInterfaceType GetClassType() const { return K_LVL_IAI_TYPE_BASE; }
	
	inline UINT32				GetUID() const { return UID; }
	//Returns: UID of activ that interacted with it
	inline UINT32				GetToucherUID() const { return nTouchingUID; }
	// Tells if object is waiting to be deallocated
	inline bool					IsPendingKill() { return bPendingKill; }
	// Returns true if object can be released
	bool						CanBeReleased();
	// Loads logic from binary file (editor exported logic)
	void						LoadLogic(FILE* fl);
	//functie care se cheama cand interactionezi cu obiectul sau cand este pTarget
	void						Touch(UINT32 touchingIActiveUID, float dTime, UINT32 overrideScriptHash = 0, bool bTouchTarget = true);
	// sets the enabled flag on/off
	void						SetEnabled( bool enabled, bool forced = false );
	// toggles the enabled state
	inline void					ToggleEnabled() { bSetEnabled = !bSetEnabled; }
	// true if not pending kill and not disabled
	virtual bool				IsAlive();
	// Call this to mark it for destruction
	void						Kill();
	// Starts a script sending AI params as script local vars
	void						StartScript( WCHAR* scriptName );
	void						StartScript( UINT32 scriptNameHash );
	// Sets varAIparams. params = nullptr just clears the params
	void						SetAIparams( CVariantMap * params, bool bClearParams );
public: 
	// Sets the AI state (useless for actors)
	virtual void				SetAI( EAIstate newstate ) = 0;
	// completely sets position and all related data(pos, bbox, etc)
	virtual void				SetPos(Vec3 newPos) = 0;
	virtual void				Move(Vec3 delta) = 0;
	// Gets called after active was added to the actives array, after being fully initialized (end of loading or spawn)
	virtual void				PostConstructionInit() = 0;
	// Gets called by the engine as soon as the object gets initialized
	virtual void				BeginPlay() = 0;
	// Gets called when gets killed
	virtual void				EndPlay() = 0;
};

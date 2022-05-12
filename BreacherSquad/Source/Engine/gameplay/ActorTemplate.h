#pragma once

// max no of anim sets
#define K_ACT_ANIM_MAX_SETS 2
// max no of verse sets
#define K_ACT_VERSES_MAX_SETS 2
// max number of skins
#define K_ACT_SKINS_MAX_SETS 20

// These are caps flags!
enum EActorCapabilitiesFlags {
	K_ACT_CAPS_NONE = 0,
	K_ACT_CAPS_CAN_COVER = 1,
	K_ACT_CAPS_CAN_INTERACT = 2,
	K_ACT_CAPS_CAN_ROLL = 4,
	K_ACT_CAPS_CAN_CROUCH = 8,
	//misc flags
	K_ACT_CAPS_NOT_A_TARGET = 128,		// Literally not a target
};

class CActorTemplate
{
public:

	// descriptor for skins array
	struct CSkinDesc {
		CStringHash			name;
		DWORD				layersVisMask;			// layer visibility bit mask that gets applied (less important bit is layer index 0)
		DWORD				hand2Mask;				// secondary hand layer mask
		DWORD				hand1Mask;				// main hand layer mask

		CSkinDesc() : layersVisMask( 0xffffffff ), hand2Mask( 0 ), hand1Mask( 0 )
		{}
	};

	// Animations descriptor (keeps animation names for each of the 6 angles, for every set)
	// We only use N, NE, SE, S and flip them for the left quadrant but still we try and load all directions
	struct CAnimDesc {
		// Holds animation names [animSet][angle]
		CStringHash			animNamesA[ K_ACT_ANIM_MAX_SETS ][ EDIR6S_CNT ];

		CAnimDesc()
		{
			Reset();
		};

		void Reset()
		{
			for ( int sets = 0; sets < K_ACT_ANIM_MAX_SETS; sets++ )
				for ( int ang = 0; ang < EDIR6S_CNT; ang++ )
					animNamesA[ sets ][ ang ].Reset(); //not set
		}
	};

	CStringHash		shID;				// ID: actor template file used as template ID
	CStringHash		shSourceXML;		// skeleton xml file name (not full path)
	CStringHash		shAIState_ini;		// initial AI state

	CSkinDesc		arrSkins[ K_ACT_SKINS_MAX_SETS ];								// Array that contains skin names and descriptions
	int				arrSkinsCnt;
	CAnimDesc		arrAnims[ K_ACT_ANIMS_CNT ];									// Array that keeps animation data from actor.xml
	int				soundIDs[ K_LVL_ACT_VERSES_COUNT ][ K_ACT_VERSES_MAX_SETS ];	// Contains sound ids-s mapped on different actions (called verses, see EActorSoundVerse)

	///--- GENERICS: !!! when adding new generics don't forget to edit OverwriteGenericDataFromTemplate !!!
	EMaterialType	eMaterial;			// type of material
	EActorClass		actorClass;			// class of actor
	CAITemplate*	AItemplate;			// pointer to AI template from the templates library

	CAABB			bbox;				// 2d bbox on floor plane defined around the character origin (not always centered)
	float			heightZ;			// height of character on Z 
	float			heartZ;				// Z coord of heart position (#TODO: could remove, used for bullet shoot height)
	CStringHash		shWeaponDefault;

	UINT32			eCaps;				// see EActorCapabilitiesFlags

	float			fMass;
	float			fLife;
	float			fArmor;
	float			fSpeedMove;			// slow moving speed
	float			fSpeedRun;			// fast moving speed
	float			fDistSee;
	float			fAttackMin, fAttackMax;

	CActorTemplate();

	// Fills variables with default values if not set (some templates are missing values)
	// \brief: We need this because of the upgrade templates that must have a lot of values on K_NOT_SET (0xDEADBEEF)
	void FillDefaultValuesIfNotSet();

	// Overwrites the current animations with the ones that are set in pTemplate
	// \returns: true if animations have been changed
	bool OverwriteAnimsFromTemplate( CActorTemplate* pTemplate, bool bEraseOldAnimations = false );

	//Overwrites "generics" with the ones that are set in pTemplate (only if not K_NOT_SET)
	void OverwriteGenericDataFromTemplate( CActorTemplate* pTemplate );

	//Adds "GENERIC" data from pTemplate to current template (weapon upgrades and such)
	void AddGenericDataFromTemplate( CActorTemplate* pTemplate );
	
	// returns skin layer visibility flag or 0xffffffff if skin not found
	UINT32 GetSkinMaskValue( WCHAR* skinName );
};

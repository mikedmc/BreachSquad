#pragma once

#define K_VL_MAX_SORTED_VISIBLES 100

enum eVisibleSortableType {
	K_VST_UNKNOWN = 0,
	K_VST_PROP,
	K_VST_ACTOR,
};

// struct that holds all sortable items with type and void pointers to them
struct CVisibleSortable {
	eVisibleSortableType	eType;
	void*					pPtr;		// cast this pointer based on eType
	float					fValue;		// sorting key1

	CVisibleSortable(eVisibleSortableType neType, void* pnPtr, float fSortValue)
	{
		eType = neType;
		pPtr = pnPtr;
		fValue = fSortValue;
	}

	CVisibleSortable() :
		eType(K_VST_UNKNOWN), pPtr(nullptr), fValue(0.0f)
	{}
};

///--- list of visible/active entities ---
class CVisibilityLists {
	//visual
public:
	//liste de pointeri activi
	CFixedArray<CLight*, 128> visible_lights;
	//lista de activi vizibili
	CFixedArray<CProp*, 512> visible_props;
	//decals
	CFixedArray<CDecal*, 512> visible_decals[K_LVL_DECAL_LAYERS_CNT];
	//visible actors
	CFixedArray<CActor*, 256> visible_actors;
	//collision shapes folosite la construierea volumelor de umbra
	CFixedArray<CCollisionShape*, 512> visible_colShapesLights;

	// display elements sorted by Y
	CFixedArray<CVisibleSortable, 256> arrSortedItems;

	//logic
public:
	//collision shapes (visible or closeby)
	CFixedArray<CCollisionShape*, 512>	logic_colShapes;
	//collision shapes from a larger area
	CFixedArray<CCollisionShape*, 1024>	logic_colShapesExtended;

	CFixedArray<CProp*, 512> logic_props_closeby;
	CFixedArray<CCollisionShape*, 256> logic_colShapesSpecial; //special collision shapes (water, triggers)
	//CTOR/DTOR
	CVisibilityLists() {}
};

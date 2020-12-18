#pragma once


///--- list of visible/active entities ---
class CVisibilityLists {
	//visual
public:
	//liste de pointeri activi
	CFixedArray<CLight*, 128> visible_lights;
	//lista de activi vizibili
	CFixedArray<CProp*, 512> visible_props;
	//decals
	CFixedArray<CDecal*, 512> visible_decals[K_LVL_DECAL_LAYERS];
	//visible actors
	CFixedArray<CActor*, 256> visible_actors;
	//collision shapes folosite la construierea volumelor de umbra
	CFixedArray<CCollisionShape*, 512> visible_colShapesLights;

	//logic
public:
	//collision shapes (visible or closeby)
	CFixedArray<CCollisionShape*, 512>	logic_colShapes;
	//collision shapes from a larger area
	CFixedArray<CCollisionShape*, 1024>	logic_colShapesExtended;

	CFixedArray<CProp*, 512> logic_props_closeby;
	CFixedArray<CActor*, 256> logic_actors_closeby; //closeby actors - bullets tests
	CFixedArray<CCollisionShape*, 256> logic_colShapesSpecial; //special collision shapes (water, triggers)
	//CTOR/DTOR
	CVisibilityLists() {}
};

#include "dxstdafx.h"
#include "Level_visibility.h"
#include <algorithm> // std::sort

// function used when sorting the visible elements by Y
int VisibleItemsSorter(const void * a, const void * b)
{
	float fa = ((CVisibleSortable*)a)->fValue;
	float fb = ((CVisibleSortable*)b)->fValue;
	return (fa > fb) - (fa < fb);
}


//used to save last player positions
static Vec2 s_vLastPlayerPos[K_MAX_PLAYERS_CNT];

void CLevel::BuildVisibilityLists()
{
	//level aabb
	CAABB lvlAABB;
	lvlAABB.Set(m_levelAABB);
	///--- visual stuff - depends only on camaabb ---
	RectXYWH camrect_old = m_camLevelToRT.GetCamWorldAABB();
	// build a camera view rectangle constant across different resolutions so it doesn't desync when on multiplayer
	//it will need intervention if camera constraint changes axis in order to maintain maximum visible area
	SizeWH camrectsz(K_GAME_WIDTH, K_GAME_HEIGHT);
	RectXYWH camrect(camrect_old.CenterX() - camrectsz.w * 0.5f, camrect_old.CenterY() - camrectsz.h * 0.5f, camrectsz.w, camrectsz.h);
	// maximize camrect vertically
	CAABB camaabb(Vec2(camrect.x, camrect.y), Vec2(camrect.Right(), camrect.Bottom()));
	// union of all visible lights AABBs
	CAABB lightsCommonAABB(Vec2(-1000.0f, -1000.0f), Vec2(-1000.0f, -1000.0f));
	//for detecting visible props (objects)
	CAABB propsPaintAABB = camaabb;
	//for detecting visible actors
	CAABB actorsPaintAABB = camaabb; 
	actorsPaintAABB.Inflate(Vec2(K_TILE_SIZE, K_TILE_SIZE));

	ClearVisibilityLists();
	

	///----- logical stuff - depends on both players -----
	//filter only useful collision boxes here (bullets intersections and such)
	CAABB collisionAreaAABBs[K_MAX_PLAYERS_CNT];
	//larger boxes
	CAABB collisionAreaAABBs_extended[K_MAX_PLAYERS_CNT];
	//filter only closeby actors (events triggering, bullets collisions etc)
	CAABB actorsNearbyAABBs[K_MAX_PLAYERS_CNT];
	//filter closeby props
	CAABB propsNearbyAABBs[K_MAX_PLAYERS_CNT];
	for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
	{
		//saves the last position so it doesn't go crazy when the players get freed
		if (pPlayerActor[kk] != null)
		{
			s_vLastPlayerPos[kk] = pPlayerActor[kk]->pos.xy;
		}
		//get a 2 screen area around each player's last pos (could be smaller, yes)
		collisionAreaAABBs[kk].Set(s_vLastPlayerPos[kk] - camaabb.vSize, s_vLastPlayerPos[kk] + camaabb.vSize);
		AABB::KeepInside(collisionAreaAABBs[kk], lvlAABB);
		//double that area to be sure (needed sometimes)
		collisionAreaAABBs_extended[kk].Set(s_vLastPlayerPos[kk] - camaabb.vSize * 2.0f, s_vLastPlayerPos[kk] + camaabb.vSize * 2.0f);
		AABB::KeepInside(collisionAreaAABBs_extended[kk], lvlAABB);
		//a larger area for events activation and such (slightly smaller area)
		actorsNearbyAABBs[kk].Set(s_vLastPlayerPos[kk] - camaabb.vHalfSize * 1.5f, s_vLastPlayerPos[kk] + camaabb.vHalfSize * 1.5f);
		AABB::KeepInside(actorsNearbyAABBs[kk], lvlAABB);

		//filter props area
		propsNearbyAABBs[kk] = actorsNearbyAABBs[kk];
	}

	//flag used for rendering:
	bool bFirstShadowingLightSet = false;
	//select lights that have AABBs that touch the visible area
	m_visibleList.visible_lights.Clear();
	for (int kk = 0; kk < m_arrLights.GetSize(); kk++)
	{
		CLight* light = m_arrLights[kk];
		
		if (!light->IsAlive() || light->bSkipRender)
			continue;

		switch (light->type)
		{
			case K_LVL_LT_AMBIENTAL:
			case K_LVL_LT_DIRECTIONAL:
			{
				// use initial value because current box is moved with light position
				if (camaabb.Intersects(light->bbox.GetSnapshot()))
				{
					m_visibleList.visible_lights.Add(m_arrLights[kk]);
				}
			}
			break;
			default:
			{
				if (camaabb.IntersectsCircle(light->pos.xy_proj, light->fRadius))
				{
					if (m_visibleList.visible_lights.Add(m_arrLights[kk]) < 0)
						break;
					if (m_arrLights[kk]->castShadows)
					{
						CAABB bbox_max(Vec2(light->pos.xy_proj.x - light->fRadius, light->pos.xy_proj.y - light->fRadius), 
							Vec2(light->pos.xy_proj.x + light->fRadius, light->pos.xy_proj.y + light->fRadius));
						//la prima lumina cu shadow seteaza lightsCommonAABB fix pe bbox-ul luminii
						if (bFirstShadowingLightSet == false)
						{
							bFirstShadowingLightSet = true;
							lightsCommonAABB = bbox_max;
						}
						else
						{
							lightsCommonAABB = AABB::Union(lightsCommonAABB, bbox_max);
						}
					}
				}
			}
			break;
		}
	}
	// select all boxes that might cast shadows from active lights
	m_visibleList.visible_colShapesLights.Clear();
	m_visibleList.logic_colShapes.Clear();
	m_visibleList.logic_colShapesExtended.Clear();
	m_visibleList.logic_colShapesSpecial.Clear();
	for (int kk = 0; kk < m_arrColShapes.GetSize(); kk++)
	{
		if (!m_arrColShapes[kk]->IsAlive())
			continue;

		switch (m_arrColShapes[kk]->eType)
		{
			//case K_LVL_COLL_TYPE_TRIGGER:
			/*
			case K_SHAPE_SOLID:
			case K_LVL_COLL_TYPE_WATER:
			case K_LVL_COLL_TYPE_COVER:
			{
				//le selectez doar pe cele din ecran
				if ((collisionAreaAABBs[0].Intersects(m_arrColShapes[kk]->bbox)) || (collisionAreaAABBs[1].Intersects(m_arrColShapes[kk]->bbox)))
					m_visibleList.logic_colShapesSpecial.Add(m_arrColShapes[kk]);
			}
			break;
			*/
			case K_SHAPE_SOLID:
			{

				//commented intersection with larger area so we add them all
				//if ((collisionAreaAABBs_extended[0].Intersects(&m_arrColShapes[kk]->bbox)) || (collisionAreaAABBs_extended[1].Intersects(&m_arrColShapes[kk]->bbox)))
				{
					//#TODO: if extended colshapes contains ALL collision shapes we could build it only once
					m_visibleList.logic_colShapesExtended.Add(m_arrColShapes[kk]);
					//add all doors/windows to a special list so we can check them rapidly when breaking them with explosions
					if ((m_arrColShapes[kk]->AIstate == K_AI_STATE_COLL_BREAKABLE_WINDOW) ||
						(m_arrColShapes[kk]->AIstate == K_AI_STATE_COLL_BREAKABLE_DOOR))
					{
						m_visibleList.logic_colShapesSpecial.Add(m_arrColShapes[kk]);
					}
					//gather all important collision boxes (nearby) - check only if included in extended area which is larger but still centered
					if ((collisionAreaAABBs[0].Intersects(m_arrColShapes[kk]->bbox)) || (collisionAreaAABBs[1].Intersects(m_arrColShapes[kk]->bbox)))
					{
						m_visibleList.logic_colShapes.Add(m_arrColShapes[kk]);
					}
				}
			}
			break;
		}
		//find bboxes that can cast shadows
		if (bFirstShadowingLightSet)
		{
			//bagam doar pe cele care fac umbra
			if (!m_arrColShapes[kk]->castShadows)
				continue;
			if (lightsCommonAABB.Intersects(m_arrColShapes[kk]->bbox))
			{
				m_visibleList.visible_colShapesLights.Add(m_arrColShapes[kk]);
			}
		}
	}
	//actorii vizibili
	m_visibleList.visible_actors.Clear();
	m_visibleList.logic_actors_closeby.Clear();
	for (int kk = 0; kk < m_arrActors.GetSize(); kk++)
	{
		CActor* actor = m_arrActors[kk];
		if ((!actor->IsAlive()) || (actor->bSkipRender))
			continue;
		//is it nearby?
		if ((actorsNearbyAABBs[0].Intersects(actor->bbox_cull)) || (actorsNearbyAABBs[1].Intersects(actor->bbox_cull)))
		{
			m_visibleList.logic_actors_closeby.Add(actor);
		}
		//must be painted?
		if (actorsPaintAABB.Intersects(actor->bbox_cull))
		{
			m_visibleList.visible_actors.Add(actor);
			// add it to the sorted list
			m_visibleList.arrSortedItems.Add(CVisibleSortable(K_VST_ACTOR, actor, actor->pos.xyz.y));
		}
	}
	//all props onscreen for rendering
	m_visibleList.visible_props.Clear();
	m_visibleList.logic_props_closeby.Clear();

	for (int ar = 0; ar < m_arrAreas.Count(); ar++)
	{
		CLevelArea* area = m_arrAreas[ar];
		if (!area->bActive)
			continue;

		for (int kk = 0; kk < area->m_arrProps.GetSize(); kk++)
		{
			CProp* prop = area->m_arrProps[ kk ];
			if ((!prop->IsAlive()) || prop->bSkipRender)
				continue;
			//visible props
			if (propsPaintAABB.Intersects(prop->bbox_cull))
			{
				m_visibleList.visible_props.Add(prop);
				// add it to the sorted list
				m_visibleList.arrSortedItems.Add(CVisibleSortable(K_VST_PROP, prop, prop->pos.xyz.y));
			}
			//logical closeby actives
			if ((propsNearbyAABBs[0].Intersects(prop->bbox_cull)) || (propsNearbyAABBs[1].Intersects(prop->bbox_cull)))
			{
				m_visibleList.logic_props_closeby.Add(prop);
			}
		}
	}
	//clear all decals layers
	for (int kk = 0; kk < K_LVL_DECAL_LAYERS_CNT; kk++)
	{
		m_visibleList.visible_decals[kk].Clear();
	}

	for (int kk = 0; kk < m_arrDecals.GetSize(); kk++)
	{
		if (propsPaintAABB.Intersects(m_arrDecals[kk]->aabb))
		{
			m_visibleList.visible_decals[m_arrDecals[kk]->layer].Add(m_arrDecals[kk]);
		}
	}

	///--- sort all visible items
	m_visibleList.arrSortedItems.Sort(VisibleItemsSorter);
}

void CLevel::ClearVisibilityLists()
{
	m_visibleList.visible_props.Clear();
	m_visibleList.logic_props_closeby.Clear();

	for (int kk = 0; kk < K_LVL_DECAL_LAYERS_CNT; kk++)
		m_visibleList.visible_decals[kk].Clear();

	m_visibleList.visible_actors.Clear();
	m_visibleList.visible_lights.Clear();
	m_visibleList.visible_colShapesLights.Clear();

	m_visibleList.logic_actors_closeby.Clear();
	m_visibleList.logic_colShapes.Clear();
	m_visibleList.logic_colShapesExtended.Clear();
	m_visibleList.logic_colShapesSpecial.Clear();

	m_visibleList.arrSortedItems.Clear();
}

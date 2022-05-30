#pragma once
#include "stlastar.h" 

#define K_ASTAR_COST_NOTPASS 9
#define K_ASTAR_COST_FLOOR 1
// use for ice or other zero frictions flooring
#define K_ASTAR_COST_ZERO 0

// Map node class
class MapSearchNode
{
public:
	int x;	 // the (x,y) positions of the node
	int y;

	MapSearchNode() { x = y = 0; }
	MapSearchNode( int px, int py ) { x = px; y = py; }

	float				GoalDistanceEstimate( MapSearchNode &nodeGoal );
	bool				IsGoal( MapSearchNode &nodeGoal );
	bool				GetSuccessors( AStarSearch<MapSearchNode> *astarsearch, MapSearchNode *parent_node );
	float				GetCost( MapSearchNode &successor );
	bool				IsSameState( MapSearchNode &rhs );

	void				PrintNodeInfo();
};

// Class that handles the search
class CAStarSearch
{
private:
	AStarSearch<MapSearchNode>	astarsearch;

public:
	// we keep map data static so we can access it from nodes
	static char**		m_map;
	static int			m_mapW;
	static int			m_mapH;

public:
	CAStarSearch();
	~CAStarSearch();

	// Gets value at coords
	static int			GetMap( int x, int y );
	// Sets pointer to current map. Set to null when no map is present.
	static void			SetMapPointer( char** mapPtr, int mapW = 0, int mapH = 0 );
	// Finds path to destination. Returns number of steps or 0 for no solution
	int					FindPath( Vec2i startTL, Vec2i endTL, Vec2i* arrRetPath, int arrRetPathSize );
};
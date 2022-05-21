#include "dxstdafx.h"
#include "astarsearch.h"


//#include <iostream>
//#include <stdio.h>
#include <math.h>

#define DEBUG_LISTS 0
#define DEBUG_LIST_LENGTHS_ONLY 0

using namespace std;

// Reset static data
char**	CAStarSearch::m_map			= nullptr;
int		CAStarSearch::m_mapW		= 0;
int		CAStarSearch::m_mapH		= 0;

CAStarSearch::CAStarSearch()
{

}

CAStarSearch::~CAStarSearch()
{
	astarsearch.FreeSolutionNodes();
	astarsearch.EnsureMemoryFreed();
	m_map = nullptr;
	m_mapW = m_mapH = 0;
}

int CAStarSearch::GetMap( int x, int y )
{
	if ( m_map == nullptr ||
		x < 0 || x >= m_mapW ||
		y < 0 || y >= m_mapH
		)
	{
		return K_ASTAR_COST_NOTPASS;
	}

	return m_map[x][y];
}


void CAStarSearch::SetMapPointer( char** mapPtr, int mapW /*= 0*/, int mapH /*= 0 */ )
{
	m_map = mapPtr;
	m_mapW = mapW;
	m_mapH = mapH;
}

bool MapSearchNode::IsSameState( MapSearchNode &rhs )
{
	// same state in a maze search is simply when (x,y) are the same
	if( (x == rhs.x) && (y == rhs.y) )
	{
		return true;
	}
	else
	{
		return false;
	}
}

void MapSearchNode::PrintNodeInfo()
{
	LOG( "Node position : (%d,%d)\n", x,y );
}

// The heuristic function that estimates the distance from a Node to the Goal
float MapSearchNode::GoalDistanceEstimate( MapSearchNode &nodeGoal )
{
	return abs(x - nodeGoal.x) + abs(y - nodeGoal.y);
}

bool MapSearchNode::IsGoal( MapSearchNode &nodeGoal )
{

	if( (x == nodeGoal.x) && (y == nodeGoal.y) )
	{
		return true;
	}
	return false;
}

// This generates the successors to the given Node. It uses a helper function called
// AddSuccessor to give the successors to the AStar class. The A* specific initialisation
// is done for each node internally, so here you just set the state information that
// is specific to the application
bool MapSearchNode::GetSuccessors( AStarSearch<MapSearchNode> *astarsearch, MapSearchNode *parent_node )
{
	int parent_x = -1;
	int parent_y = -1;

	if ( parent_node )
	{
		parent_x = parent_node->x;
		parent_y = parent_node->y;
	}


	MapSearchNode NewNode;

	// push each possible move except allowing the search to go backwards

	if ( ( CAStarSearch::GetMap( x - 1, y ) < K_ASTAR_COST_NOTPASS )
		&& !( ( parent_x == x - 1 ) && ( parent_y == y ) )
		)
	{
		NewNode = MapSearchNode( x - 1, y );
		astarsearch->AddSuccessor( NewNode );
	}

	if ( ( CAStarSearch::GetMap( x, y - 1 ) < K_ASTAR_COST_NOTPASS )
		&& !( ( parent_x == x ) && ( parent_y == y - 1 ) )
		)
	{
		NewNode = MapSearchNode( x, y - 1 );
		astarsearch->AddSuccessor( NewNode );
	}

	if ( ( CAStarSearch::GetMap( x + 1, y ) < K_ASTAR_COST_NOTPASS )
		&& !( ( parent_x == x + 1 ) && ( parent_y == y ) )
		)
	{
		NewNode = MapSearchNode( x + 1, y );
		astarsearch->AddSuccessor( NewNode );
	}


	if ( ( CAStarSearch::GetMap( x, y + 1 ) < K_ASTAR_COST_NOTPASS )
		&& !( ( parent_x == x ) && ( parent_y == y + 1 ) )
		)
	{
		NewNode = MapSearchNode( x, y + 1 );
		astarsearch->AddSuccessor( NewNode );
	}

	return true;
}

// given this node, what does it cost to move to successor. In the case
// of our map the answer is the map terrain value at this node since that is 
// conceptually where we're moving
float MapSearchNode::GetCost( MapSearchNode &successor )
{
	return (float) CAStarSearch::GetMap( x, y );

}


int CAStarSearch::FindPath( Vec2i startTL, Vec2i endTL, Vec2i* arrRetPath, int arrRetPathSize )
{
	_ASSERT( arrRetPath != nullptr && arrRetPathSize > 0 );
	// Our sample problem defines the world as a 2d array representing a terrain
	// Each element contains an integer from 0 to 5 which indicates the cost 
	// of travel across the terrain. Zero means the least possible difficulty 
	// in travelling (think ice rink if you can skate) whilst 5 represents the 
	// most difficult. 9 indicates that we cannot pass.

	// make sure we don't start or end within walls
	if ( GetMap( startTL.x, startTL.y ) == K_ASTAR_COST_NOTPASS )
		return 0;
	if ( GetMap( endTL.x, endTL.y ) == K_ASTAR_COST_NOTPASS )
		return 0;

	// Create a start state
	MapSearchNode nodeStart;
	nodeStart.x = startTL.x; nodeStart.y = startTL.y;

	// Define the goal state
	MapSearchNode nodeEnd;
	nodeEnd.x = endTL.x; nodeEnd.y = endTL.y;

	// Set Start and goal states
	astarsearch.SetStartAndGoalStates( nodeStart, nodeEnd );

	unsigned int SearchState;
	unsigned int SearchSteps = 0;

	do
	{
		SearchState = astarsearch.SearchStep();

		SearchSteps++;

#if DEBUG_LISTS

		cout << "Steps:" << SearchSteps << "\n";

		int len = 0;

		cout << "Open:\n";
		MapSearchNode *p = astarsearch.GetOpenListStart();
		while ( p )
		{
			len++;
#if !DEBUG_LIST_LENGTHS_ONLY			
			( ( MapSearchNode * ) p )->PrintNodeInfo();
#endif
			p = astarsearch.GetOpenListNext();

		}

		cout << "Open list has " << len << " nodes\n";

		len = 0;

		cout << "Closed:\n";
		p = astarsearch.GetClosedListStart();
		while ( p )
		{
			len++;
#if !DEBUG_LIST_LENGTHS_ONLY			
			p->PrintNodeInfo();
#endif			
			p = astarsearch.GetClosedListNext();
		}

		cout << "Closed list has " << len << " nodes\n";
#endif

	} while ( SearchState == AStarSearch<MapSearchNode>::SEARCH_STATE_SEARCHING );

	if ( SearchState == AStarSearch<MapSearchNode>::SEARCH_STATE_SUCCEEDED )
	{
		int steps = 0;
		MapSearchNode *node = astarsearch.GetSolutionStart();

		//node->PrintNodeInfo();
		arrRetPath[steps] = { node->x, node->y };
		steps++;
		for ( ;; )
		{
			node = astarsearch.GetSolutionNext();
			// exit if no other node
			if ( !node )
				break;

			//node->PrintNodeInfo();
			arrRetPath[steps] = { node->x, node->y };
			steps++;
		};

		LOG( "AStar steps %d", steps );

		// Once you're done with the solution you can free the nodes up
		astarsearch.FreeSolutionNodes();
		return steps;
	}
	else if ( SearchState == AStarSearch<MapSearchNode>::SEARCH_STATE_FAILED )
	{
		cout << "Search terminated. Did not find goal state\n";
		astarsearch.EnsureMemoryFreed();
		return 0;
	}

	return 0;
}



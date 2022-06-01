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
	// Manhattan distance:
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


int CAStarSearch::FindPath( Vec2i startTL, Vec2i endTL, Vec2i* arrRetPath, int arrRetPathSize, bool bSmoothPath )
{
	// smoothed steps are written here before being returned
	static Vec2i tempSmoothSteps[128];

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
			_ASSERT(steps < arrRetPathSize);
		};

		// Once you're done with the solution you can free the nodes up
		astarsearch.FreeSolutionNodes();
		astarsearch.EnsureMemoryFreed();

		LOG( "AStar steps %d", steps );

		// does the smoothing using a line algo on the collision map (checks tile by tile)
		if ( bSmoothPath )
		{
			int smoothsteps = SmoothPath( arrRetPath, steps, tempSmoothSteps, 128 );
			if ( smoothsteps > 0 )
			{
				steps = smoothsteps;
				memcpy( arrRetPath, tempSmoothSteps, sizeof( Vec2i ) * smoothsteps );
			}
			else
			{
				ErrorBox( K_ERR_WARNING, L"AStar::FindPath:SmoothPath failed! Returning full path." );
			}
			LOG( "AStar smoothed steps %d", steps );
		}

		return steps;
	}
	else if ( SearchState == AStarSearch<MapSearchNode>::SEARCH_STATE_FAILED )
	{
		cout << "Search terminated. Did not find goal state\n";
		astarsearch.FreeSolutionNodes();
		astarsearch.EnsureMemoryFreed();
		return 0;
	}

	return 0;
}

int CAStarSearch::SmoothPath( Vec2i* arrInPoints, int arrInItems, Vec2i* arrOutPoints, int arrOutSize )
{
	// no input items
	if ( arrInItems == 0 || arrInPoints == nullptr )
		return 0;
	// 1-2 input points
	_ASSERT( arrOutSize > 2 );
	if ( arrInItems <= 2 )
	{
		for ( int kk = 0; kk < arrInItems; kk++ )
			arrOutPoints[kk] = arrInPoints[kk];
		// returns 1 or 2
		return arrInItems;
	}
	// do the actual smoothing
	int outcur = 0;		// out vector cursor
	int incur = 0;		// in vector current point
	// add first point as checkpoint
	Vec2i vFrom = arrInPoints[incur];
	arrOutPoints[outcur++] = vFrom;
	while ( incur < arrInItems - 1 )
	{
		// walk on next points while they are still visible
		for ( int tocur = incur + 1; tocur < arrInItems; tocur++ )
		{
			int nFoundCur = -1;
			// we reached last element, save it as waypoint
			if ( tocur >= arrInItems - 1 )
			{
				nFoundCur = arrInItems - 1;
			}
			// we can't see this point so we save last point as checkpoint and start again
			// we consider a collision COST_FLOOR_BORDER so it doesn't cut corners near wall corners (as they have higher cost)
			// the downside is that if the only path is through high cost areas it adds more waypoints
			else if ( SegmentMapCollision( vFrom.x, vFrom.y, arrInPoints[tocur].x, arrInPoints[tocur].y, K_ASTAR_COST_FLOOR_BORDER ) )
			{
				// if next point isn't visible (some engine element blocking the way or something)
				// then just add it as a checkpoint instead of returning invalid smoothing
				if ( tocur - 1 == incur )
					nFoundCur = tocur;
				else
					nFoundCur = tocur - 1; // save last visible point otherwise
			}

			if ( nFoundCur >= 0 )
			{
				// save last visible point
				arrOutPoints[outcur++] = arrInPoints[nFoundCur];
				_ASSERT( outcur < arrOutSize );
				incur = nFoundCur;
				vFrom = arrInPoints[incur];
				break;
			}
		}
	}

	return outcur;

}

bool CAStarSearch::SegmentMapCollision( int x1, int y1, int x2, int y2, int minValueToDetect )
{
	// standard bresenham line algo but instead of drawing it just checks for values in map
	int deltaX, deltaY,
		absDeltaX, absDeltaY,
		x, y,
		incX, incY,
		val,
		i;

	deltaX = x2 - x1;
	deltaY = y2 - y1;

	absDeltaX = abs( deltaX );
	absDeltaY = abs( deltaY );

	x = x1;
	y = y1;

	incX = SIGNZ( deltaX );
	incY = SIGNZ( deltaY );

	if ( absDeltaX >= absDeltaY )
	{
		val = absDeltaY >> 1;

		for ( i = 0; i < absDeltaX; i++ )
		{
			val += absDeltaY;
			if ( val >= absDeltaX )
			{
				val -= absDeltaX;
				y += incY;
			}
			x += incX;
			if ( ( x >= 0 && x < m_mapW ) && ( y >= 0 && y < m_mapH ) )
			{
				if ( m_map[x][y] >= minValueToDetect )
					return true;
			}
		}
	}
	else
	{
		val = absDeltaX >> 1;

		for ( i = 0; i < absDeltaY; i++ )
		{
			val += absDeltaX;
			if ( val >= absDeltaY )
			{
				val -= absDeltaY;
				x += incX;
			}
			y += incY;
			if ( x >= 0 && x < m_mapW && y >= 0 && y < m_mapH )
			{
				if ( m_map[x][y] >= minValueToDetect )
					return true;
			}
		}
	}
	// no collision
	return false;
}

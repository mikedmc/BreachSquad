#include "dxstdafx.h"
#include "LevelArea.h"

CLevelArea::CLevelArea( UINT32 nID )
{
	bActive = false;

	ID = nID;
	tiles = nullptr;
	bVisible = false;
}

CLevelArea::~CLevelArea()
{
	Release();
}

OPRESULT CLevelArea::BuildBuffers( PDEVICE pDevice )
{
	_ASSERT( pDevice != nullptr );

	areaMesh.Init( pDevice );
	V_OP_RET( areaMesh.BuildBuffers( tiles, sizeTL, Vec2( AABBbounds.vMin.x, AABBbounds.vMin.y ) ) );

	return K_OP_OK;
}

void CLevelArea::Release()
{
	// release area props
	SAFE_DELETE_GROWABLE_ARRAY( m_arrProps );

	// release tiles
	if ( tiles != nullptr )
	{
		for ( int kk = 0; kk < sizeTL.w; kk++ )
		{
			SAFE_DELETE_ARRAY( tiles[kk] );
		}
		SAFE_DELETE_ARRAY( tiles );
	}

	arrNeighbours.Clear();
	areaMesh.Release();
}

CTile* CLevelArea::GetTile( int xTL, int yTL )
{
	//#TODO: should return a generic empty tile??
	if ( ( xTL < AABBbounds_TL.x ) || ( yTL < AABBbounds_TL.y ) || ( xTL >= AABBbounds_TL.x + AABBbounds_TL.w ) || ( yTL >= AABBbounds_TL.y + AABBbounds_TL.h ) )
		return nullptr;

	return &tiles[xTL - AABBbounds_TL.x][yTL - AABBbounds_TL.y];
}

bool CLevelArea::UpdateVisibility( RectXYWH camRect )
{
	CAABB camAABB( camRect );
	if ( AABBbounds.Intersects( camAABB ) )
	{
		bVisible = true;
		// we activate the area on first encounter and leave it active forever
		bActive = true;
	}
	else
	{
		bVisible = false;
	}
	// check mesh visibility (level blocks)
	areaMesh.UpdateVisibility( camRect );

	return bVisible;
}


bool CLevelArea::IsLayerMeshVisible( eAreaLayer layer )
{
	for ( int kk = 0; kk < areaMesh.arrVisible.Count(); kk++ )
	{
		if ( areaMesh.arrVisible[kk]->m_arrMeshIdx[layer] >= 0 )
			return true;
	}

	return false;
}

CTile* CLevelArea::SegmentTilesIntersection( Vec2 vStart, Vec2 vEnd, Vec2 & retPoint, Vec2 & retNormal, Vec2i *hitTilePosTL, DWORD tileFlagsNonCollide )
{
	// Works by walking from tile to tile on slopes, on X axis and Y axis then finding the closest point
	//#INFO: when going from right to left and bottom to top, if the end point is on the tile border it doesn't detect the intersection. Might happen to slow moving bullets but it should be fine.

	// early check if not intersecting
	CAABB moveBB;
	moveBB.Set_Corrected( vStart, vEnd );
	if ( !moveBB.Intersects( AABBbounds ) )
		return nullptr;

	//#OPTIMIZE: de pus tileflags options la coliziuni
	//#OPTIMIZE: de renuntat la GetTile pentru acces direct. Verificare bounds prin clamping
	// bring it in local space
	Vec2i startTL( (int)floor( vStart.x / K_TILE_SIZE ), (int)floor( vStart.y / K_TILE_SIZE ) );
	Vec2i endTL( (int)floor( vEnd.x / K_TILE_SIZE ), (int)floor( vEnd.y / K_TILE_SIZE ) );

	Vec2i vMinTL( AABBbounds_TL.x, AABBbounds_TL.y );
	Vec2i vMaxTL( AABBbounds_TL.x + AABBbounds_TL.w - 1, AABBbounds_TL.y + AABBbounds_TL.h - 1 );
	// check if current start is colliding. Return colliding.
	CTile* pstarttl = GetTile( startTL.x, startTL.y );
	if ( pstarttl != nullptr )
	{
		if ( FLAG_NONE( pstarttl->flags, tileFlagsNonCollide ) )
		{
			retPoint = vStart;
			MUVec2Norm( &retNormal, &( vStart - vEnd ) );
			return pstarttl;
		}
	}

	Vec2 vDir = vEnd - vStart;
	float fDirLen = MUVec2Len( &vDir );
	Vec2 vDirN = vDir / fDirLen;

	bool bFoundV = false;
	Vec2 vRetPtV( 0.0f, 0.0f );
	Vec2 vRetNrmV( 0.0f, 0.0f );
	Vec2i chktlV( 0, 0 );

	if ( startTL.x != endTL.x )
	{
		// find first vertical grid collisions
		if ( vDir.x < 0.0f )
		{
			float vLimit = max( AABBbounds.vMin.x, vEnd.x );
			// distance to margin
			float dstX = vStart.x - startTL.x * K_TILE_SIZE;
			// find first intersection with vertical axes
			float vecmul = dstX / fabs( vDirN.x );
			Vec2 vFrom( vStart.x + vDirN.x * vecmul, vStart.y + vDirN.y * vecmul );
			Vec2 vStep( SIGN( vDirN.x ) * K_TILE_SIZE, vDirN.y * ( K_TILE_SIZE / fabs( vDirN.x ) ) );
			// walk from tile to tile horizontally until destination
			Vec2 vCur = vFrom;
			// test collisions on left side
			while ( vCur.x >= vLimit )
			{
				chktlV = Vec2i( (int)( ( vCur.x - K_TILE_SIZE / 2.0f ) / K_TILE_SIZE ), (int)( vCur.y / K_TILE_SIZE ) );
				CTile* ptl = GetTile( chktlV.x, chktlV.y );
				if ( ( ptl != nullptr ) && ( FLAG_NONE( ptl->flags, tileFlagsNonCollide ) ) )
				{
					bFoundV = true;
					vRetPtV = vCur;
					vRetNrmV = Vec2( 1.0f, 0.0f );
					break;
				}

				vCur.x += vStep.x;
				vCur.y += vStep.y;
			}
		}
		else if ( vDir.x > 0.0f )
		{
			float vLimit = min( vEnd.x, AABBbounds.vMax.x );
			// distance to margin
			float dstX = ( startTL.x + 1 ) * K_TILE_SIZE - vStart.x;
			// find first intersection with vertical axes
			float vecmul = dstX / fabs( vDirN.x );
			Vec2 vFrom( vStart.x + vDirN.x * vecmul, vStart.y + vDirN.y * vecmul );
			Vec2 vStep( SIGN( vDirN.x ) * K_TILE_SIZE, vDirN.y * ( K_TILE_SIZE / fabs( vDirN.x ) ) );
			// walk from tile to tile horizontally until destination
			Vec2 vCur = vFrom;
			// test collisions on right side
			while ( vCur.x <= vLimit )
			{
				chktlV = Vec2i( (int)( ( vCur.x + K_TILE_SIZE / 2.0f ) / K_TILE_SIZE ), (int)( vCur.y / K_TILE_SIZE ) );
				CTile* ptl = GetTile( chktlV.x, chktlV.y );
				if ( ( ptl != nullptr ) && ( FLAG_NONE( ptl->flags, tileFlagsNonCollide ) ) )
				{
					bFoundV = true;
					vRetPtV = vCur;
					vRetNrmV = Vec2( -1.0f, 0.0f );
					break;
				}
				vCur.x += vStep.x;
				vCur.y += vStep.y;
			}
		}
	}

	// Horizontal axes
	bool	bFoundH = false;
	Vec2	vRetPtH( 0.0f, 0.0f );
	Vec2	vRetNrmH( 0.0f, 0.0f );
	Vec2i	chktlH( 0, 0 );			// return hit tile pos
	if ( startTL.y != endTL.y )
	{
		// find first vertical grid collisions
		if ( vDir.y < 0.0f )
		{
			float vLimit = max( AABBbounds.vMin.y, vEnd.y );
			// distance to margin
			float dstY = vStart.y - startTL.y * K_TILE_SIZE;
			// find first intersection with vertical axes
			float vecmul = dstY / fabs( vDirN.y );
			Vec2 vFrom( vStart.x + vDirN.x * vecmul, vStart.y + vDirN.y * vecmul );
			Vec2 vStep( vDirN.x * ( K_TILE_SIZE / fabs( vDirN.y ) ), SIGN( vDirN.y ) * K_TILE_SIZE );
			// walk from tile to tile horizontally until destination
			Vec2 vCur = vFrom;
			// test collisions on left side
			while ( vCur.y >= vLimit )
			{
				chktlH = Vec2i( (int)( ( vCur.x ) / K_TILE_SIZE ), (int)( ( vCur.y - K_TILE_SIZE / 2.0f ) / K_TILE_SIZE ) );
				CTile* ptl = GetTile( chktlH.x, chktlH.y );
				if ( ( ptl != nullptr ) && ( FLAG_NONE( ptl->flags, tileFlagsNonCollide ) ) )
				{
					bFoundH = true;
					vRetPtH = vCur;
					vRetNrmH = Vec2( 0.0f, 1.0f );
					break;
				}

				vCur.x += vStep.x;
				vCur.y += vStep.y;
			}
		}
		else if ( vDir.y > 0.0f )
		{
			float vLimit = min( vEnd.y, AABBbounds.vMax.y );
			// distance to margin
			float dstY = ( startTL.y + 1 ) * K_TILE_SIZE - vStart.y;
			// find first intersection with vertical axes
			float vecmul = dstY / fabs( vDirN.y );
			Vec2 vFrom( vStart.x + vDirN.x * vecmul, vStart.y + vDirN.y * vecmul );
			Vec2 vStep( vDirN.x * ( K_TILE_SIZE / fabs( vDirN.y ) ), SIGN( vDirN.y ) * K_TILE_SIZE );
			// walk from tile to tile horizontally until destination
			Vec2 vCur = vFrom;
			// test collisions on right side
			while ( vCur.y <= vLimit )
			{
				chktlH = Vec2i( (int)( ( vCur.x ) / K_TILE_SIZE ), (int)( ( vCur.y + K_TILE_SIZE / 2 ) / K_TILE_SIZE ) );
				CTile* ptl = GetTile( chktlH.x, chktlH.y );
				if ( ( ptl != nullptr ) && ( FLAG_NONE( ptl->flags, tileFlagsNonCollide ) ) )
				{
					bFoundH = true;
					vRetPtH = vCur;
					vRetNrmH = Vec2( 0.0f, -1.0f );
					break;
				}

				vCur.x += vStep.x;
				vCur.y += vStep.y;
			}
		}
	}


	// return closest value
	if ( bFoundH && bFoundV )
	{
		float minH = MUVec2LenSq( &( vRetPtH - vStart ) );
		float minV = MUVec2LenSq( &( vRetPtV - vStart ) );
		if ( minV < minH )
		{
			retPoint = vRetPtV;
			retNormal = vRetNrmV;
			if ( hitTilePosTL ) *hitTilePosTL = chktlV;
			return &tiles[chktlV.x - vMinTL.x][chktlV.y - vMinTL.y];
		}
		else
		{
			retPoint = vRetPtH;
			retNormal = vRetNrmH;
			if ( hitTilePosTL ) *hitTilePosTL = chktlH;
			return &tiles[chktlH.x - vMinTL.x][chktlH.y - vMinTL.y];
		}
	}
	else if ( bFoundH )
	{
		retPoint = vRetPtH;
		retNormal = vRetNrmH;
		if ( hitTilePosTL ) *hitTilePosTL = chktlH;
		return &tiles[chktlH.x - vMinTL.x][chktlH.y - vMinTL.y];
	}
	else if ( bFoundV )
	{
		retPoint = vRetPtV;
		retNormal = vRetNrmV;
		if ( hitTilePosTL ) *hitTilePosTL = chktlV;
		return &tiles[chktlV.x - vMinTL.x][chktlV.y - vMinTL.y];
	}

	// no collisions
	return nullptr;
}



int CLevelArea::GetTilesCollisionBoxes( RectXYXYi srcBoxTL, CAABB* ret_arrAABBs, int nArrCapacity )
{
	_ASSERT( ret_arrAABBs != nullptr );
	int nAdded = 0;
	// clamp src box to valid area
	RectXYXYi box = srcBoxTL;
	// bring to local space
	box.Move( -AABBbounds_TL.x, -AABBbounds_TL.y );
	CLAMP( box.x1, 0, AABBbounds_TL.w - 1 );
	CLAMP( box.y1, 0, AABBbounds_TL.h - 1 );
	CLAMP( box.x2, 0, AABBbounds_TL.w - 1 );
	CLAMP( box.y2, 0, AABBbounds_TL.h - 1 );
	//#TODO: should mix consecutive tiles into a single box as optimization, at least on horizontal
	for ( int yy = box.y1; yy <= box.y2; yy++ )
	{
		_ASSERT( ( yy < sizeTL.h ) && ( yy >= 0 ) );
		for ( int xx = box.x1; xx <= box.x2; xx++ )
		{
			_ASSERT( ( xx < sizeTL.w ) && ( xx >= 0 ) );
			if ( ( tiles[xx][yy].flags & K_TILEFLAG_WALKABLE ) == 0 )
			{
				//#MAYBE: on release make it exit early  if over capacity
				_ASSERT( nAdded < nArrCapacity );
				ret_arrAABBs[nAdded++] = tiles[xx][yy].bbox;
			}
		}
	}

	return nAdded;
}

int CLevelArea::GetPropsCollisionBoxes( CAABB srcBox, CAABB* ret_arrAABBs, int nArrCapacity )
{
	int nAdded = 0;

	for ( int kk = 0; kk < m_arrProps.Count(); kk++ )
	{
		CProp* prop = m_arrProps[kk];
		if ( ( !prop->IsAlive() ) || ( ( prop->flags & K_PROPFLAG_COLLIDES_ACTOR ) == 0 ) )
			continue;
		if ( prop->bbox_floor.Intersects( srcBox ) )
		{
			ret_arrAABBs[nAdded++] = m_arrProps[kk]->bbox_floor;
		}
	}

	return nAdded;
}

int CLevelArea::GetPropsTouchingBox( CAABB srcBox, CProp* ret_arrProps[], int nArrCapacity, bool bOnlyInteractibles )
{
	int nAdded = 0;

	for ( int kk = 0; kk < m_arrProps.Count(); kk++ )
	{
		CProp* prop = m_arrProps[kk];
		if ( !prop->IsAlive() )
			continue;
		if ( bOnlyInteractibles && ( prop->bCanInteract == false ) )
			continue;
		if ( prop->bbox_floor.Intersects( srcBox ) )
		{
			ret_arrProps[nAdded++] = m_arrProps[kk];
		}
	}

	return nAdded;
}

int CLevelArea::GetPropsTouchingBox( CAABB srcBox, CArray<CProp*>& ret_arrProps, bool bOnlyInteractibles )
{
	int nAdded = 0;

	for ( int kk = 0; kk < m_arrProps.Count(); kk++ )
	{
		CProp* prop = m_arrProps[kk];
		if ( !prop->IsAlive() )
			continue;
		if ( bOnlyInteractibles && ( prop->bCanInteract == false ) )
			continue;
		if ( prop->bbox_floor.Intersects( srcBox ) )
		{
			ret_arrProps.Add( m_arrProps[kk] );
		}
	}

	return nAdded;
}

int CLevelArea::GetTilesByFlag( RectXYXYi srcBoxTL, UINT32 dwFlagAny, CTile* ret_arrTiles, int nArrCapacity )
{
	return 0;
	/*
	_ASSERT(ret_arrOccluders != nullptr);
	int nAdded = 0;
	// clamp src box to valid area
	RECTXYXY box = srcBoxTL;
	box.Clamp(AABBbounds_TL.x, AABBbounds_TL.y, AABBbounds_TL.x + AABBbounds_TL.w - 1, AABBbounds_TL.y + AABBbounds_TL.h - 1);
	// bring to local space
	box.Move(-AABBbounds_TL.x, -AABBbounds_TL.y);
	//#TODO: should mix consecutive tiles into a single box as optimization, at least on horizontal
	for (int yy = box.y1; yy <= box.y2; yy++)
	{
		_ASSERT((yy < sizeTL.h) && (yy >= 0));
		for (int xx = box.x1; xx <= box.x2; xx++)
		{
			_ASSERT((xx < sizeTL.w) && (xx >= 0));
			if ((tiles[xx][yy].flags & K_TILEFLAG_WALKABLE) == 0)
			{
			}
		}
	}

	return nAdded;
	*/
}

bool CLevelArea::IsBoxColliding( CAABB srcBox, bool bCheckProps /*= true */ )
{
	// convert to tiles min and max and clamp src box to valid area
	RectXYXYi box( floor( srcBox.vMin.x / K_TILE_SIZE_F ), floor( srcBox.vMin.y / K_TILE_SIZE_F ),
		ceil( srcBox.vMax.x / K_TILE_SIZE_F ), ceil( srcBox.vMax.y / K_TILE_SIZE_F ) );
	// bring to local space
	box.Move( -AABBbounds_TL.x, -AABBbounds_TL.y );
	CLAMP( box.x1, 0, AABBbounds_TL.w - 1 );
	CLAMP( box.y1, 0, AABBbounds_TL.h - 1 );
	CLAMP( box.x2, 0, AABBbounds_TL.w - 1 );
	CLAMP( box.y2, 0, AABBbounds_TL.h - 1 );

	for ( int yy = box.y1; yy <= box.y2; yy++ )
	{
		_ASSERT( ( yy < sizeTL.h ) && ( yy >= 0 ) );
		for ( int xx = box.x1; xx <= box.x2; xx++ )
		{
			_ASSERT( ( xx < sizeTL.w ) && ( xx >= 0 ) );
			if ( ( tiles[xx][yy].flags & K_TILEFLAG_WALKABLE ) == 0 )
			{
				if ( tiles[xx][yy].bbox.Intersects( srcBox ) )
					return true;
			}
		}
	}

	// check props if requested
	if ( bCheckProps )
	{
		for ( int kk = 0; kk < m_arrProps.Count(); kk++ )
		{
			CProp* prop = m_arrProps[kk];
			if ( ( !prop->IsAlive() ) || ( ( prop->flags & K_PROPFLAG_COLLIDES_ACTOR ) == 0 ) )
				continue;
			if ( prop->bbox_floor.Intersects( srcBox ) )
			{
				return true;
			}
		}
	}

	return false;
}

OPRESULT CLevelArea::OnCreateDevice( PDEVICE pDevice, const SURFACE_DESC* pBBDesc /*= NULL*/)
{
	V_OP_RET( areaMesh.OnCreateDevice( pDevice, pBBDesc ) );
	return K_OP_OK;
}

OPRESULT CLevelArea::OnResetDevice( PDEVICE pDevice, const SURFACE_DESC* pBBDesc /*= NULL*/ )
{
	V_OP_RET( areaMesh.OnResetDevice( pDevice, pBBDesc ) );
	return K_OP_OK;
}

OPRESULT CLevelArea::OnLostDevice( )
{
	areaMesh.OnLostDevice();
	return K_OP_OK;
}

OPRESULT CLevelArea::OnDestroyDevice( )
{
	areaMesh.OnDestroyDevice();
	return K_OP_OK;
}


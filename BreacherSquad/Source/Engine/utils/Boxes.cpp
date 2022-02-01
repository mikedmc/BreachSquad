#include "dxstdafx.h"
#include "Boxes.h"

Vec2 Rects::FromRectToRect( Vec2 & point, RECTXYWH_F & src, RECTXYWH_F & dest )
{
	return Vec2( ( ( point.x - src.x ) / src.w ) * dest.w + dest.x, ( ( point.y - src.y ) / src.h ) * dest.h + dest.y );
}

Vec2 Rects::FromRectToRect( Vec2 & point, RECTLTRB_F& src, RECTLTRB_F& dest )
{
	return Vec2( ( ( point.x - src.left ) / ( src.right - src.left ) ) * ( dest.right - dest.left ) + dest.left, ( ( point.y - src.top ) / ( src.bottom - src.top ) ) * ( dest.bottom - dest.top ) + dest.top );
}

Vec2 Rects::FromRectToRect( Vec2 & point, RECTXYXY_F & src, RECTXYXY_F & dest )
{
	return Vec2( ( ( point.x - src.x1 ) / ( src.x2 - src.x1 ) ) * ( dest.x2 - dest.x1 ) + dest.x1, ( ( point.y - src.y1 ) / ( src.y2 - src.y1 ) ) * ( dest.y2 - dest.y1 ) + dest.y1 );
}

RECTXYWH::RECTXYWH() :
	x( 0 ), y( 0 ), w( 0 ), h( 0 )
{

}

RECTXYWH::RECTXYWH( int nx, int ny, int nw, int nh ) :
	x( nx ), y( ny ), w( nw ), h( nh )
{

}

RECTXYWH::RECTXYWH( const RECTXYWH& rectsrc ) :
	x( rectsrc.x ), y( rectsrc.y ), w( rectsrc.w ), h( rectsrc.h )
{

}

RECTXYWH::RECTXYWH( const RECT& rectsrc ) :
	x( rectsrc.left ), y( rectsrc.top ), w( rectsrc.right - rectsrc.left ), h( rectsrc.bottom - rectsrc.top )
{

}

void RECTXYWH::Inflate( int dx, int dy )
{
	x -= dx; w += 2 * dx;
	y -= dy; h += 2 * dy;
}

void RECTXYWH::Move( int movex, int movey )
{
	x += movex;
	y += movey;
}

void RECTXYWH::Set( int nx, int ny, int nw, int nh )
{
	x = nx; y = ny; w = nw; h = nh;
}

const int RECTXYWH::Bottom() const
{
	return y + h;
}

const int RECTXYWH::Right() const
{
	return x + w;
}

const int RECTXYWH::CenterX() const
{
	return x + w / 2;
}

const int RECTXYWH::CenterY() const
{
	return y + h / 2;
}

const Vec2i RECTXYWH::Center() const
{
	return Vec2i( x + w / 2, y + h / 2 );
}

bool RECTXYWH::Contains( Vec2i pt )
{
	return ( ( pt.x >= x ) && ( pt.y >= y ) && ( pt.x < x + w - 1 ) && ( pt.y < y + h - 1 ) );
}

bool RECTXYWH::Intersects( const RECTXYWH& rhs )
{
	if ( ( rhs.x >= x + w ) || ( rhs.x + rhs.w <= x ) || ( rhs.y >= y + h ) || ( rhs.y + rhs.h <= y ) )
		return false;
	return true;
}

void RECTXYWH::IntersectWith( const RECTXYWH& clampToThis )
{
	Vec2i vmin( x, y );
	Vec2i vmax( x + w - 1, y + h - 1 );

	if ( vmin.x < clampToThis.x ) vmin.x = clampToThis.x;
	if ( vmin.y < clampToThis.y ) vmin.y = clampToThis.y;
	if ( vmax.x >= clampToThis.x + clampToThis.w - 1 ) vmax.x = clampToThis.x + clampToThis.w - 1;
	if ( vmax.y >= clampToThis.y + clampToThis.h - 1 ) vmax.y = clampToThis.y + clampToThis.h - 1;

	x = vmin.x; y = vmin.y;

	if ( vmax.x < vmin.x )
		w = 0;
	else
		w = vmax.x - vmin.x + 1;
	if ( vmax.y < vmin.y )
		h = 0;
	else
		h = vmax.y - vmin.y + 1;
}

bool RECTXYWH::operator!=( const RECTXYWH& rhs )
{
	return ( ( x != rhs.x ) || ( y != rhs.y ) || ( w != rhs.w ) || ( h != rhs.h ) );
}

bool RECTXYWH::operator==( const RECTXYWH& rhs )
{
	return ( ( x == rhs.x ) && ( y == rhs.y ) && ( w == rhs.w ) && ( h == rhs.h ) );
}

RECTXYWH_F::RECTXYWH_F() :
	x( 0.0f ), y( 0.0f ), w( 0.0f ), h( 0.0f )
{

}

RECTXYWH_F::RECTXYWH_F( float nx, float ny, float nw, float nh ) :
	x( nx ), y( ny ), w( nw ), h( nh )
{

}

RECTXYWH_F::RECTXYWH_F( const RECTXYWH_F& rectsrc ) :
	x( rectsrc.x ), y( rectsrc.y ), w( rectsrc.w ), h( rectsrc.h )
{

}

RECTXYWH_F::RECTXYWH_F( const RECTXYWH rectsrc ) :
	x( ( float ) rectsrc.x ), y( ( float ) rectsrc.y ), w( ( float ) rectsrc.w ), h( ( float ) rectsrc.h )
{

}

void RECTXYWH_F::Union( RECTXYWH_F other )
{
	Vec2 vMin, vMax;
	vMin.x = min( x, other.x );
	vMin.y = min( y, other.y );
	vMax.x = max( x + w, other.x + other.w );
	vMax.y = max( y + h, other.y + other.h );
	x = vMin.x; y = vMin.y;
	w = vMax.x - vMin.x; h = vMax.y - vMin.y;
}

void RECTXYWH_F::Set( float nx, float ny, float nw, float nh )
{
	x = nx; y = ny; w = nw; h = nh;
}

void RECTXYWH_F::Move( float movex, float movey )
{
	x += movex;
	y += movey;
}

void RECTXYWH_F::Inflate( float scalar )
{
	x -= scalar;
	y -= scalar;
	w += 2.0f * scalar;
	h += 2.0f * scalar;
}

const float RECTXYWH_F::Bottom() const
{
	return y + h;
}

const float RECTXYWH_F::Right() const
{
	return x + w;
}

const float RECTXYWH_F::CenterX() const
{
	return x + w / 2.0f;
}

const float RECTXYWH_F::CenterY() const
{
	return y + h / 2.0f;
}

const Vec2 RECTXYWH_F::Center() const
{
	return Vec2( x + w / 2.0f, y + h / 2.0f );
}

RECTLTRB_F::RECTLTRB_F() :
	left( 0.0f ), top( 0.0f ), right( 0.0f ), bottom( 0.0f )
{

}

RECTLTRB_F::RECTLTRB_F( RECT& rectSrc ) :
	left( ( float ) rectSrc.left ), top( ( float ) rectSrc.top ), right( ( float ) rectSrc.right ), bottom( ( float ) rectSrc.bottom )
{

}

RECTLTRB_F::RECTLTRB_F( float nleft, float ntop, float nright, float nbottom ) :
	left( nleft ), top( ntop ), right( nright ), bottom( nbottom )
{

}

RECTLTRB_F::RECTLTRB_F( const RECTLTRB_F& rectsrc ) :
	left( rectsrc.left ), right( rectsrc.right ), top( rectsrc.top ), bottom( rectsrc.bottom )
{

}

RECTLTRB_F::RECTLTRB_F( const RECTXYWH_F rectsrc ) :
	left( rectsrc.x ), top( rectsrc.y ), right( rectsrc.x + rectsrc.w ), bottom( rectsrc.y + rectsrc.h )
{

}

void RECTLTRB_F::Set( float nleft, float ntop, float nright, float nbottom )
{
	left = nleft; top = ntop; right = nright; bottom = nbottom;
}

void RECTLTRB_F::Move( float dx, float dy )
{
	left += dx;
	top += dy;
	right += dx;
	bottom += dy;
}

bool RECTLTRB_F::Intersects( RECTLTRB_F & dest )
{
	if ( ( left >= dest.right ) || ( right <= dest.left ) || ( top >= dest.bottom ) || ( bottom <= dest.top ) )
		return false;
	return true;
}

bool RECTLTRB_F::Contains( RECTLTRB_F & dest )
{
	if ( ( dest.left >= left ) && ( dest.top >= top ) && ( dest.right <= right ) && ( dest.bottom <= bottom ) )
		return true;
	return false;
}

float RECTLTRB_F::Width()
{
	return right - left;
}

float RECTLTRB_F::Height()
{
	return bottom - top;
}

bool RECTLTRB_F::Intersection( RECTLTRB_F & a, RECTLTRB_F & b, RECTLTRB_F & retVal )
{
	Vec2 vMax, vMin;
	vMin.x = max( a.left, b.left );
	vMin.y = max( a.top, b.top );
	vMax.x = min( a.right, b.right );
	vMax.y = min( a.bottom, b.bottom );
	retVal.Set( vMin.x, vMin.y, vMax.x, vMax.y );
	//negative means no intersection
	if ( ( vMax.x < vMin.x ) || ( vMax.y < vMin.y ) )
		return false;
	return true;
}

RECTXYXY_F::RECTXYXY_F() :
	x1( 0.0f ), y1( 0.0f ), x2( 0.0f ), y2( 0.0f )
{

}

RECTXYXY_F::RECTXYXY_F( RECT& rectSrc ) :
	x1( ( float ) rectSrc.left ), y1( ( float ) rectSrc.top ), x2( ( float ) rectSrc.right ), y2( ( float ) rectSrc.bottom )
{

}

RECTXYXY_F::RECTXYXY_F( float nx1, float ny1, float nx2, float ny2 ) :
	x1( nx1 ), y1( ny1 ), x2( nx2 ), y2( ny2 )
{

}

RECTXYXY_F::RECTXYXY_F( const RECTXYXY_F& rectsrc ) :
	x1( rectsrc.x1 ), x2( rectsrc.x2 ), y1( rectsrc.y1 ), y2( rectsrc.y2 )
{

}

RECTXYXY_F::RECTXYXY_F( const RECTXYWH_F rectsrc ) :
	x1( rectsrc.x ), y1( rectsrc.y ), x2( rectsrc.x + rectsrc.w ), y2( rectsrc.y + rectsrc.h )
{

}

RECTXYXY::RECTXYXY() :
	x1( 0 ), y1( 0 ), x2( 0 ), y2( 0 )
{

}

RECTXYXY::RECTXYXY( RECT& rectSrc ) :
	x1( rectSrc.left ), y1( rectSrc.top ), x2( rectSrc.right ), y2( rectSrc.bottom )
{

}

RECTXYXY::RECTXYXY( int nx1, int ny1, int nx2, int ny2 ) :
	x1( nx1 ), y1( ny1 ), x2( nx2 ), y2( ny2 )
{

}

RECTXYXY::RECTXYXY( const RECTXYXY& rectsrc ) :
	x1( rectsrc.x1 ), x2( rectsrc.x2 ), y1( rectsrc.y1 ), y2( rectsrc.y2 )
{

}

RECTXYXY::RECTXYXY( const RECTXYWH rectsrc ) :
	x1( rectsrc.x ), y1( rectsrc.y ), x2( rectsrc.x + rectsrc.w ), y2( rectsrc.y + rectsrc.h )
{

}

void RECTXYXY::Clamp( int xmin, int ymin, int xmax, int ymax )
{
	CLAMP( x1, xmin, xmax );
	CLAMP( x2, xmin, xmax );
	CLAMP( y1, ymin, ymax );
	CLAMP( y2, ymin, ymax );
}

void RECTXYXY::Move( int dx, int dy )
{
	x1 += dx; x2 += dx;
	y1 += dy; y2 += dy;
}


bool Rects::PointInRect( Vec2 pt, RECTXYWH_F rct )
{
	if ( ( pt.x < rct.x ) || ( pt.y < rct.y ) || ( pt.x > rct.x + rct.w ) || ( pt.y > rct.y + rct.h ) )
		return false;
	return true;
}

bool Rects::PointInRect( int x, int y, int rx, int ry, int rw, int rh )
{
	if ( ( x < rx ) || ( y < ry ) || ( x > rx + rw ) || ( y > ry + rh ) )
		return false;
	return true;
}

bool Rects::PointInRect( int x, int y, RECTXYWH *r )
{
	if ( ( x < r->x ) || ( x > r->x + r->w ) || ( y < r->y ) || ( y > r->y + r->h ) )
		return false;
	return true;
}

bool Rects::PointInRect( float x, float y, RECTXYWH_F *r )
{
	if ( ( x < r->x ) || ( x > r->x + r->w ) || ( y < r->y ) || ( y > r->y + r->h ) )
		return false;
	return true;
}

bool Rects::PointInRect( POINT *pt, RECTXYWH *r )
{
	if ( ( pt->x < r->x ) || ( pt->x > r->x + r->w ) || ( pt->y < r->y ) || ( pt->y > r->y + r->h ) )
		return false;
	return true;
}

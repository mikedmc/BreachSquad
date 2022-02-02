#include "dxstdafx.h"
#include "Boxes.h"

Vec2 Rects::FromRectToRect( Vec2 & point, RectXYWH & src, RectXYWH & dest )
{
	return Vec2( ( ( point.x - src.x ) / src.w ) * dest.w + dest.x, ( ( point.y - src.y ) / src.h ) * dest.h + dest.y );
}

Vec2 Rects::FromRectToRect( Vec2 & point, RectLTRB& src, RectLTRB& dest )
{
	return Vec2( ( ( point.x - src.left ) / ( src.right - src.left ) ) * ( dest.right - dest.left ) + dest.left, ( ( point.y - src.top ) / ( src.bottom - src.top ) ) * ( dest.bottom - dest.top ) + dest.top );
}

Vec2 Rects::FromRectToRect( Vec2 & point, RectXYXY & src, RectXYXY & dest )
{
	return Vec2( ( ( point.x - src.x1 ) / ( src.x2 - src.x1 ) ) * ( dest.x2 - dest.x1 ) + dest.x1, ( ( point.y - src.y1 ) / ( src.y2 - src.y1 ) ) * ( dest.y2 - dest.y1 ) + dest.y1 );
}

RectXYWHi::RectXYWHi() :
	x( 0 ), y( 0 ), w( 0 ), h( 0 )
{

}

RectXYWHi::RectXYWHi( int nx, int ny, int nw, int nh ) :
	x( nx ), y( ny ), w( nw ), h( nh )
{

}

RectXYWHi::RectXYWHi( const RectXYWHi& rectsrc ) :
	x( rectsrc.x ), y( rectsrc.y ), w( rectsrc.w ), h( rectsrc.h )
{

}

RectXYWHi::RectXYWHi( const RECT& rectsrc ) :
	x( rectsrc.left ), y( rectsrc.top ), w( rectsrc.right - rectsrc.left ), h( rectsrc.bottom - rectsrc.top )
{

}

void RectXYWHi::Inflate( int dx, int dy )
{
	x -= dx; w += 2 * dx;
	y -= dy; h += 2 * dy;
}

void RectXYWHi::Move( int movex, int movey )
{
	x += movex;
	y += movey;
}

void RectXYWHi::Set( int nx, int ny, int nw, int nh )
{
	x = nx; y = ny; w = nw; h = nh;
}

const int RectXYWHi::Bottom() const
{
	return y + h;
}

const int RectXYWHi::Right() const
{
	return x + w;
}

const int RectXYWHi::CenterX() const
{
	return x + w / 2;
}

const int RectXYWHi::CenterY() const
{
	return y + h / 2;
}

const Vec2i RectXYWHi::Center() const
{
	return Vec2i( x + w / 2, y + h / 2 );
}

bool RectXYWHi::Contains( Vec2i pt )
{
	return ( ( pt.x >= x ) && ( pt.y >= y ) && ( pt.x < x + w - 1 ) && ( pt.y < y + h - 1 ) );
}

bool RectXYWHi::Intersects( const RectXYWHi& rhs )
{
	if ( ( rhs.x >= x + w ) || ( rhs.x + rhs.w <= x ) || ( rhs.y >= y + h ) || ( rhs.y + rhs.h <= y ) )
		return false;
	return true;
}

void RectXYWHi::IntersectWith( const RectXYWHi& clampToThis )
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

bool RectXYWHi::operator!=( const RectXYWHi& rhs )
{
	return ( ( x != rhs.x ) || ( y != rhs.y ) || ( w != rhs.w ) || ( h != rhs.h ) );
}

bool RectXYWHi::operator==( const RectXYWHi& rhs )
{
	return ( ( x == rhs.x ) && ( y == rhs.y ) && ( w == rhs.w ) && ( h == rhs.h ) );
}

RectXYWH::RectXYWH() :
	x( 0.0f ), y( 0.0f ), w( 0.0f ), h( 0.0f )
{

}

RectXYWH::RectXYWH( float nx, float ny, float nw, float nh ) :
	x( nx ), y( ny ), w( nw ), h( nh )
{

}

RectXYWH::RectXYWH( const RectXYWH& rectsrc ) :
	x( rectsrc.x ), y( rectsrc.y ), w( rectsrc.w ), h( rectsrc.h )
{

}

RectXYWH::RectXYWH( const RectXYWHi rectsrc ) :
	x( ( float ) rectsrc.x ), y( ( float ) rectsrc.y ), w( ( float ) rectsrc.w ), h( ( float ) rectsrc.h )
{

}

void RectXYWH::Union( RectXYWH other )
{
	Vec2 vMin, vMax;
	vMin.x = min( x, other.x );
	vMin.y = min( y, other.y );
	vMax.x = max( x + w, other.x + other.w );
	vMax.y = max( y + h, other.y + other.h );
	x = vMin.x; y = vMin.y;
	w = vMax.x - vMin.x; h = vMax.y - vMin.y;
}

void RectXYWH::Set( float nx, float ny, float nw, float nh )
{
	x = nx; y = ny; w = nw; h = nh;
}

void RectXYWH::Move( float movex, float movey )
{
	x += movex;
	y += movey;
}

void RectXYWH::Inflate( float scalar )
{
	x -= scalar;
	y -= scalar;
	w += 2.0f * scalar;
	h += 2.0f * scalar;
}

const float RectXYWH::Bottom() const
{
	return y + h;
}

const float RectXYWH::Right() const
{
	return x + w;
}

const float RectXYWH::CenterX() const
{
	return x + w / 2.0f;
}

const float RectXYWH::CenterY() const
{
	return y + h / 2.0f;
}

const Vec2 RectXYWH::Center() const
{
	return Vec2( x + w / 2.0f, y + h / 2.0f );
}

RectLTRB::RectLTRB() :
	left( 0.0f ), top( 0.0f ), right( 0.0f ), bottom( 0.0f )
{

}

RectLTRB::RectLTRB( RECT& rectSrc ) :
	left( ( float ) rectSrc.left ), top( ( float ) rectSrc.top ), right( ( float ) rectSrc.right ), bottom( ( float ) rectSrc.bottom )
{

}

RectLTRB::RectLTRB( float nleft, float ntop, float nright, float nbottom ) :
	left( nleft ), top( ntop ), right( nright ), bottom( nbottom )
{

}

RectLTRB::RectLTRB( const RectLTRB& rectsrc ) :
	left( rectsrc.left ), right( rectsrc.right ), top( rectsrc.top ), bottom( rectsrc.bottom )
{

}

RectLTRB::RectLTRB( const RectXYWH rectsrc ) :
	left( rectsrc.x ), top( rectsrc.y ), right( rectsrc.x + rectsrc.w ), bottom( rectsrc.y + rectsrc.h )
{

}

void RectLTRB::Set( float nleft, float ntop, float nright, float nbottom )
{
	left = nleft; top = ntop; right = nright; bottom = nbottom;
}

void RectLTRB::Move( float dx, float dy )
{
	left += dx;
	top += dy;
	right += dx;
	bottom += dy;
}

bool RectLTRB::Intersects( RectLTRB & dest )
{
	if ( ( left >= dest.right ) || ( right <= dest.left ) || ( top >= dest.bottom ) || ( bottom <= dest.top ) )
		return false;
	return true;
}

bool RectLTRB::Contains( RectLTRB & dest )
{
	if ( ( dest.left >= left ) && ( dest.top >= top ) && ( dest.right <= right ) && ( dest.bottom <= bottom ) )
		return true;
	return false;
}

float RectLTRB::Width()
{
	return right - left;
}

float RectLTRB::Height()
{
	return bottom - top;
}

SizeWH RectLTRB::Size()
{
	return SizeWH( right - left, bottom - top );
}

bool RectLTRB::Intersection( RectLTRB & a, RectLTRB & b, RectLTRB & retVal )
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

RectXYXY::RectXYXY() :
	x1( 0.0f ), y1( 0.0f ), x2( 0.0f ), y2( 0.0f )
{

}

RectXYXY::RectXYXY( RECT& rectSrc ) :
	x1( ( float ) rectSrc.left ), y1( ( float ) rectSrc.top ), x2( ( float ) rectSrc.right ), y2( ( float ) rectSrc.bottom )
{

}

RectXYXY::RectXYXY( float nx1, float ny1, float nx2, float ny2 ) :
	x1( nx1 ), y1( ny1 ), x2( nx2 ), y2( ny2 )
{

}

RectXYXY::RectXYXY( const RectXYXY& rectsrc ) :
	x1( rectsrc.x1 ), x2( rectsrc.x2 ), y1( rectsrc.y1 ), y2( rectsrc.y2 )
{

}

RectXYXY::RectXYXY( const RectXYWH rectsrc ) :
	x1( rectsrc.x ), y1( rectsrc.y ), x2( rectsrc.x + rectsrc.w ), y2( rectsrc.y + rectsrc.h )
{

}

RectXYXYi::RectXYXYi() :
	x1( 0 ), y1( 0 ), x2( 0 ), y2( 0 )
{

}

RectXYXYi::RectXYXYi( RECT& rectSrc ) :
	x1( rectSrc.left ), y1( rectSrc.top ), x2( rectSrc.right ), y2( rectSrc.bottom )
{

}

RectXYXYi::RectXYXYi( int nx1, int ny1, int nx2, int ny2 ) :
	x1( nx1 ), y1( ny1 ), x2( nx2 ), y2( ny2 )
{

}

RectXYXYi::RectXYXYi( const RectXYXYi& rectsrc ) :
	x1( rectsrc.x1 ), x2( rectsrc.x2 ), y1( rectsrc.y1 ), y2( rectsrc.y2 )
{

}

RectXYXYi::RectXYXYi( const RectXYWHi rectsrc ) :
	x1( rectsrc.x ), y1( rectsrc.y ), x2( rectsrc.x + rectsrc.w ), y2( rectsrc.y + rectsrc.h )
{

}

void RectXYXYi::Clamp( int xmin, int ymin, int xmax, int ymax )
{
	CLAMP( x1, xmin, xmax );
	CLAMP( x2, xmin, xmax );
	CLAMP( y1, ymin, ymax );
	CLAMP( y2, ymin, ymax );
}

void RectXYXYi::Move( int dx, int dy )
{
	x1 += dx; x2 += dx;
	y1 += dy; y2 += dy;
}


bool Rects::PointInRect( Vec2 pt, RectXYWH rct )
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

bool Rects::PointInRect( int x, int y, RectXYWHi *r )
{
	if ( ( x < r->x ) || ( x > r->x + r->w ) || ( y < r->y ) || ( y > r->y + r->h ) )
		return false;
	return true;
}

bool Rects::PointInRect( float x, float y, RectXYWH *r )
{
	if ( ( x < r->x ) || ( x > r->x + r->w ) || ( y < r->y ) || ( y > r->y + r->h ) )
		return false;
	return true;
}

bool Rects::PointInRect( POINT *pt, RectXYWHi *r )
{
	if ( ( pt->x < r->x ) || ( pt->x > r->x + r->w ) || ( pt->y < r->y ) || ( pt->y > r->y + r->h ) )
		return false;
	return true;
}

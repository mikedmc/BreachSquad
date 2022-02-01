#pragma once

class RectXYWHi {
public:
	int x, y, w, h;
	RectXYWHi();
	RectXYWHi( int nx, int ny, int nw, int nh );
	RectXYWHi( const RectXYWHi& rectsrc );
	RectXYWHi( const RECT& rectsrc );

	bool operator==( const RectXYWHi& rhs );
	bool operator!=( const RectXYWHi& rhs );

	void				Inflate( int dx, int dy );
	void				Move( int movex, int movey );
	void				Set( int nx, int ny, int nw, int nh );
	const int			Bottom() const;
	const int			Right() const;
	const int			CenterX() const;
	const int			CenterY() const;
	const Vec2i		Center() const;
	bool				Contains( Vec2i pt );
	bool				Intersects( const RectXYWHi& rhs );
	// cuts the area outside of clampToThis
	void				IntersectWith( const RectXYWHi& clampToThis );
};

class RectXYWH {
public:
	float x, y, w, h;
	RectXYWH();
	RectXYWH( float nx, float ny, float nw, float nh );
	RectXYWH( const RectXYWH& rectsrc );
	RectXYWH( const RectXYWHi rectsrc );
	// Grows to include "other"
	void				Union( RectXYWH other );
	void				Set( float nx, float ny, float nw, float nh );
	void				Move( float movex, float movey );
	void				Inflate( float scalar );
	const float			Bottom() const;
	const float			Right() const;
	const float			CenterX() const;
	const float			CenterY() const;
	const Vec2			Center() const;
};

class RectLTRB {
public:
	float left, top, right, bottom;
	RectLTRB();
	RectLTRB( RECT& rectSrc );
	RectLTRB( float nleft, float ntop, float nright, float nbottom );
	RectLTRB( const RectLTRB& rectsrc );
	RectLTRB( const RectXYWH rectsrc );

	void				Set( float nleft, float ntop, float nright, float nbottom );
	void				Move( float dx, float dy );
	// checks if rect intersects dest rect (borders touching are not considered intersections hence we use >= and not >)
	bool				Intersects( RectLTRB & dest );
	// checks if current rect contains dest rect
	bool				Contains( RectLTRB & dest );
	float				Width();
	float				Height();
	// finds intersection of two RECTLTRB_F returning true if they intersect
	static bool			Intersection( RectLTRB & a, RectLTRB & b, RectLTRB & retVal );

};

class RectXYXY {
public:
	float x1, y1, x2, y2;
	RectXYXY();
	RectXYXY( RECT& rectSrc );
	RectXYXY( float nx1, float ny1, float nx2, float ny2 );
	RectXYXY( const RectXYXY& rectsrc );
	RectXYXY( const RectXYWH rectsrc );
};

class RectXYXYi {
public:
	int x1, y1, x2, y2;
	RectXYXYi();
	RectXYXYi( RECT& rectSrc );
	RectXYXYi( int nx1, int ny1, int nx2, int ny2 );
	RectXYXYi( const RectXYXYi& rectsrc );
	RectXYXYi( const RectXYWHi rectsrc );

	// Clamps rectangle to limits (clip or intersect would be better)
	void				Clamp( int xmin, int ymin, int xmax, int ymax );
	void				Move( int dx, int dy );
};

namespace Rects {
	// barycentric coords for rectangles, changing coords from one to the other
	Vec2				FromRectToRect( Vec2 & point, RectXYWH & src, RectXYWH & dest );
	Vec2				FromRectToRect( Vec2 & point, RectXYXY & src, RectXYXY & dest );
	Vec2				FromRectToRect( Vec2 & point, RectLTRB& src, RectLTRB& dest );

	bool				PointInRect( Vec2 pt, RectXYWH rct );
	bool				PointInRect( int x, int y, int rx, int ry, int rw, int rh );
	bool				PointInRect( int x, int y, RectXYWHi *r );
	bool				PointInRect( float x, float y, RectXYWH *r );
	bool				PointInRect( POINT *pt, RectXYWHi *r );
}
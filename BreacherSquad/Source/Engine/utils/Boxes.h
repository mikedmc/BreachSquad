#pragma once

class RECTXYWH {
public:
	int x, y, w, h;
	RECTXYWH();
	RECTXYWH( int nx, int ny, int nw, int nh );
	RECTXYWH( const RECTXYWH& rectsrc );
	RECTXYWH( const RECT& rectsrc );

	bool operator==( const RECTXYWH& rhs );
	bool operator!=( const RECTXYWH& rhs );

	void				Inflate( int dx, int dy );
	void				Move( int movex, int movey );
	void				Set( int nx, int ny, int nw, int nh );
	const int			Bottom() const;
	const int			Right() const;
	const int			CenterX() const;
	const int			CenterY() const;
	const PointXYi		Center() const;
	bool				Contains( Vec2i pt );
	bool				Intersects( const RECTXYWH& rhs );
	// cuts the area outside of clampToThis
	void				IntersectWith( const RECTXYWH& clampToThis );
};

class RECTXYWH_F {
public:
	float x, y, w, h;
	RECTXYWH_F();
	RECTXYWH_F( float nx, float ny, float nw, float nh );
	RECTXYWH_F( const RECTXYWH_F& rectsrc );
	RECTXYWH_F( const RECTXYWH rectsrc );
	// Grows to include "other"
	void				Union( RECTXYWH_F other );
	void				Set( float nx, float ny, float nw, float nh );
	void				Move( float movex, float movey );
	void				Inflate( float scalar );
	const float			Bottom() const;
	const float			Right() const;
	const float			CenterX() const;
	const float			CenterY() const;
	const Vec2			Center() const;
};

class RECTLTRB_F {
public:
	float left, top, right, bottom;
	RECTLTRB_F();
	RECTLTRB_F( RECT& rectSrc );
	RECTLTRB_F( float nleft, float ntop, float nright, float nbottom );
	RECTLTRB_F( const RECTLTRB_F& rectsrc );
	RECTLTRB_F( const RECTXYWH_F rectsrc );

	void				Set( float nleft, float ntop, float nright, float nbottom );
	void				Move( float dx, float dy );
	// checks if rect intersects dest rect (borders touching are not considered intersections hence we use >= and not >)
	bool				Intersects( RECTLTRB_F & dest );
	// checks if current rect contains dest rect
	bool				Contains( RECTLTRB_F & dest );
	float				Width();
	float				Height();
	// finds intersection of two RECTLTRB_F returning true if they intersect
	static bool			Intersection( RECTLTRB_F & a, RECTLTRB_F & b, RECTLTRB_F & retVal );

};

class RECTXYXY_F {
public:
	float x1, y1, x2, y2;
	RECTXYXY_F();
	RECTXYXY_F( RECT& rectSrc );
	RECTXYXY_F( float nx1, float ny1, float nx2, float ny2 );
	RECTXYXY_F( const RECTXYXY_F& rectsrc );
	RECTXYXY_F( const RECTXYWH_F rectsrc );
};

class RECTXYXY {
public:
	int x1, y1, x2, y2;
	RECTXYXY();
	RECTXYXY( RECT& rectSrc );
	RECTXYXY( int nx1, int ny1, int nx2, int ny2 );
	RECTXYXY( const RECTXYXY& rectsrc );
	RECTXYXY( const RECTXYWH rectsrc );

	// Clamps rectangle to limits (clip or intersect would be better)
	void				Clamp( int xmin, int ymin, int xmax, int ymax );
	void				Move( int dx, int dy );
};

namespace Rects {
	// barycentric coords for rectangles, changing coords from one to the other
	Vec2				FromRectToRect( Vec2 & point, RECTXYWH_F & src, RECTXYWH_F & dest );
	Vec2				FromRectToRect( Vec2 & point, RECTXYXY_F & src, RECTXYXY_F & dest );
	Vec2				FromRectToRect( Vec2 & point, RECTLTRB_F& src, RECTLTRB_F& dest );

	bool				PointInRect( Vec2 pt, RECTXYWH_F rct );
	bool				PointInRect( int x, int y, int rx, int ry, int rw, int rh );
	bool				PointInRect( int x, int y, RECTXYWH *r );
	bool				PointInRect( float x, float y, RECTXYWH_F *r );
	bool				PointInRect( POINT *pt, RECTXYWH *r );
}
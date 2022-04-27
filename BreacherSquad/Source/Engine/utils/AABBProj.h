#pragma once

///----------------------------------------------------------------------------------
/// AABB class that encapsulates fake projection computation for fake 3d games
/// Precomputes projected boxes for fast access
///----------------------------------------------------------------------------------
class CAABBProj
{
public:
	Vec3	vHalfSize;		//don't set manually!
	Vec3	vCenter;		//don't set manually!
	Vec3	vMin, vMax;		//don't set manually!
	Vec3	vSize;			//don't set manually!
	CAABB	box_xy;			//don't set manually! Floor plane projection
	CAABB	box_proj;		//don't set manually! Screen plane projection

public:
	CAABBProj();
	// Specify min and max world coords. Doesn't check min<max
	CAABBProj( Vec3 min, Vec3 max );

	CAABBProj( CAABBProj& src );

	// Completely initializes a bbox. specify min and max corners in world space.
	void				Set( Vec3 min, Vec3 max );

	// Returns projection on screen plane
	inline CAABB		GetProjection() {
		return box_proj;
	}

	// Returns projection on XY axis (floor axis usually)
	inline CAABB		GetProjectionXY() {
		return box_xy;
	}
};


///----------------------------------------------------------------------------------
/// Extended CAABBProj class that can save initial states
///----------------------------------------------------------------------------------
class CAABBProjEx : public CAABBProj
{
private:
	Vec3		vMin_ini;
	Vec3		vMax_ini;
public:
	CAABBProjEx();
	CAABBProjEx( const CAABBProj & box );
	// saves a snapshot of the current box
	void				SaveSnapshot();
	// Forcefully sets the backup copy
	void				SetSnapshot( Vec3 min, Vec3 max );
	// restores from snapshot into current box data (offset is optional)
	void				RestoreSnapshot( Vec3 vOffset = { 0.0f, 0.0f, 0.0f } );
	// returns saved snapshot without changing current box
	inline CAABBProj	GetSnapshot() {
		CAABBProj retb( vMin_ini, vMax_ini );
		return retb;
	}
};
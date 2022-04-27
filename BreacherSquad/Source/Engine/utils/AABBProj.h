#pragma once

///----------------------------------------------------------------------------------
/// AABB class that encapsulates fake projection computation for fake 3d games
///----------------------------------------------------------------------------------
class CAABBProj
{
public:
	Vec3 vHalfSize;			//don't set manually!
	Vec3 vCenter;			//don't set manually!
	Vec3 vMin, vMax;		//don't set manually!
	Vec3 vSize;				//don't set manually!

public:
	CAABBProj();
	// Specify min and max world coords. Doesn't check min<max
	CAABBProj( Vec3 min, Vec3 max );

	CAABBProj( CAABBProj& src );

	// Specify floor box and box height in world coords
	void				Set( Vec3 min, Vec3 max );

	// Returns projection on screen plane
	inline CAABB		GetProjection() {
		return CAABB( Vec3ProjVec2( vMin ), Vec3ProjVec2( vMax ) );
	}

	// Returns projection on XY axis (floor axis usually)
	inline CAABB		GetProjectionXY() {
		return CAABB( Vec3XY( vMin ), Vec3XY( vMax ) );
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
	// restores from snapshot into current box data (offset is optional)
	void				RestoreSnapshot( Vec3 vOffset = { 0.0f, 0.0f, 0.0f } );
	// returns saved snapshot without changing current box
	inline CAABBProj	GetSnapshot() {
		return CAABBProj( vMin_ini, vMax_ini );
	}
};
#pragma once
// Methods that handle AABB collision (entry time - exit time method)
// Method desc: https://www.gamedev.net/tutorials/programming/general-and-gameplay-programming/swept-aabb-collision-detection-and-response-r3084/
// Inspired from JS version: https://gamedev.stackexchange.com/questions/185664/swept-aabb-collision-detection-conflict-with-tile-sized-gaps/185711#185711
// JSFiddle version: https://jsfiddle.net/2bnv510x/

#define K_AABBSWEEP_INF	100000.0f

// Simple struct that will hold data about a given swept collision detection
struct SweepData {
	CAABB					aabb;					// checked bbox values
	float					fCollisionTime;
	Vec2					vNormal;
	int						eSide;					// use K_SIDE_LEFT, etc
	float					fDistance;

	SweepData(CAABB nAABB, float collisionTime, Vec2 normal, int side, float distance) :
		aabb(nAABB), fCollisionTime(collisionTime), vNormal(normal), eSide(side), fDistance(distance)
	{
	}
};

namespace AABBSweep {

	// Perform the sweep calculation and return it as a SweepData struct.
	// -- it could handle moving targets too by computing the relative movement
	SweepData CalculateSweepData(CAABB & movingbox, Vec2 movement, CAABB & staticbox);

};
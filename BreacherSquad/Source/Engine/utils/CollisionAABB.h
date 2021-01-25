#pragma once
// Methods that handle AABB collision (entry time - exit time method)
// Method desc: https://www.gamedev.net/tutorials/programming/general-and-gameplay-programming/swept-aabb-collision-detection-and-response-r3084/
// Inspired from JS version: https://gamedev.stackexchange.com/questions/185664/swept-aabb-collision-detection-conflict-with-tile-sized-gaps/185711#185711
// JSFiddle version: https://jsfiddle.net/2bnv510x/

#define K_AABBSWEEP_INF	100000.0f
#define K_AABBSWEEP_EPS 0.01f

class SweepAABB : public CAABB {
public:
	bool					bDisabled;

	SweepAABB() : bDisabled(false)
	{
		vHalfSize = Vec2(0.0f, 0.0f);
		vCenter = Vec2(0.0f, 0.0f);
		vMin = Vec2(0.0f, 0.0f);
		vMax = Vec2(0.0f, 0.0f);
		vSize = Vec2(0.0f, 0.0f);
	}

	SweepAABB(CAABB & src) 
	{
		vHalfSize = src.vHalfSize;
		vCenter = src.vCenter;
		vMin = src.vMin;
		vMax = src.vMax;
		vSize = src.vSize;
		// defaults to not disabled
		bDisabled = false;
	}
};

// Simple struct that will hold data about a given swept collision detection
struct SweepData {
	bool					bIsValid;				// is collision data valid?
	float					fCollisionTime;
	Vec2					vNormal;
	int						eSide;					// use K_SIDE_LEFT, etc
	float					fDistance;

	SweepData(bool isValid, float collisionTime, Vec2 normal, int side, float distance) :
		bIsValid(isValid), fCollisionTime(collisionTime), 
		vNormal(normal), eSide(side), fDistance(distance)
	{
	}
};

namespace AABBSweep {

	// Perform the sweep calculation and return it as a SweepData struct.
	// -- it could handle moving targets too by computing the relative movement
	SweepData CalculateSweepData(CAABB & movingbox, Vec2 movement, CAABB & staticbox);

};

/*
	// original fn for fixing the entry of tile sized moving box in one tile wide holes
fixEqualSizedHoleCollision(hit, potentialMovement, time, collisionStack) {
	if (collisionStack.length > 0) {
		// The tile/entity the player currently stands on
		let standingOn = hit.aabb;

		// The tile/entity assumed to be next to the player-sized hole
		let nextToHole = collisionStack[0].aabb;

		// Signed integer expressing the player's movement direction
		let movementDir;

		// Signed integer expressing what direction the points to the surface the player touches
		let touchDir;

		// Distance from the player and the surface he's standing on used to determine if it's walked on
		let requiredSurfaceDist;

		// Boolean control variables that speak for themselves. They're not really needed,
		// but make the code understandable.
		let areBothUnderMe;
		let isHoleBetween;

		// Distance between each entity expected to form the player's size
		let requiredHoleSize;

		// Offset vector to displace a hypothetical "barrier" that blocks the player in order
		// make it fall into the hole smoothly.
		let barrierOffset = createVector(0, 0);

		// Compute the above variables to detect and resolve the hole problem.
		// The next else-if statement is essentially the same as this if-statement only
		// that it works for sliding along walls.
		if (hit.normal.x != = 0) {
			movementDir = Math.sign(hit.normal.x);
			touchDir = Math.sign(potentialMovement.y);
			requiredSurfaceDist = (standingOn.extents.y + this.extents.y) * touchDir;
			areBothUnderMe =
				(standingOn.center.y - this.center.y) == = requiredSurfaceDist &&
				(nextToHole.center.y - this.center.y) == = requiredSurfaceDist;
			requiredHoleSize = (this.extents.x * 2 + standingOn.extents.x + nextToHole.extents.x) * movementDir;
			isHoleBetween = (nextToHole.center.x - standingOn.center.x) == = requiredHoleSize;
			barrierOffset.y = this.extents.y * 2 * touchDir;
			barrierOffset.x = (this.extents.x - nextToHole.extents.x) * -movementDir;
			if (areBothUnderMe && isHoleBetween) {
				// Create a simulated barrier that will block the player and make him fall 
				// into the hole.
				let barrier = new AABB(nextToHole.center.copy(), this.extents.copy());
				barrier.center.sub(barrierOffset);
				this.center.x = barrier.center.x + (barrier.extents.x + this.extents.x) * (-movementDir);
				hit.normal.x = 0; // The player should now fall into the hole and no longer walk
				hit.normal.y = potentialMovement.y; // The sliding should now be applied to the hole wall
			}
		}
		else if (hit.normal.y != = 0) {
			movementDir = Math.sign(hit.normal.y);
			touchDir = Math.sign(potentialMovement.x);
			requiredSurfaceDist = (standingOn.extents.x + this.extents.x) * touchDir;
			areBothUnderMe =
				(standingOn.center.x - this.center.x) == = requiredSurfaceDist &&
				(nextToHole.center.x - this.center.x) == = requiredSurfaceDist;
			requiredHoleSize = (this.extents.y * 2 + standingOn.extents.y + nextToHole.extents.y) * movementDir;
			isHoleBetween = (nextToHole.center.y - standingOn.center.y) == = requiredHoleSize;
			barrierOffset.x = this.extents.x * 2 * touchDir;
			barrierOffset.y = (this.extents.y - nextToHole.extents.y) * -movementDir;
			if (areBothUnderMe && isHoleBetween) {
				let barrier = new AABB(nextToHole.center.copy(), this.extents.copy());
				barrier.center.sub(barrierOffset);
				this.center.y = barrier.center.y + (barrier.extents.y + this.extents.y) * (-movementDir);
				hit.normal.y = 0;
				hit.normal.x = potentialMovement.x;
			}
		}
	}
}
*/
#include "dxstdafx.h"
#include "CollisionAABB.h"

SweepData AABBSweep::CalculateSweepData(CAABB & movingbox, Vec2 movement, CAABB & staticbox)
{
	Vec2 deltaEntry(0.0f, 0.0f);
	Vec2 deltaExit(0.0f, 0.0f);

	Vec2 velocity = movement;

	if (velocity.x > 0.0f)
	{
		deltaEntry.x = staticbox.vMin.x - movingbox.vMax.x;
		deltaExit.x = staticbox.vMax.x - movingbox.vMin.x;
	}
	else
	{
		deltaEntry.x = staticbox.vMax.x - movingbox.vMin.x;
		deltaExit.x = staticbox.vMin.x - movingbox.vMax.x;
	}

	if (velocity.y > 0.0f)
	{
		deltaEntry.y = staticbox.vMin.y - movingbox.vMax.y;
		deltaExit.y = staticbox.vMax.y - movingbox.vMin.y;
	}
	else
	{
		deltaEntry.y = staticbox.vMax.y - movingbox.vMin.y;
		deltaExit.y = staticbox.vMin.y - movingbox.vMax.y;
	}

	Vec2 entryTime(0.0f, 0.0f);
	Vec2 exitTime(0.0f, 0.0f);

	if (velocity.x == 0.0f)
	{
		entryTime.x = -K_AABBSWEEP_INF;
		exitTime.x = K_AABBSWEEP_INF;
	}
	else
	{
		entryTime.x = deltaEntry.x / velocity.x;
		exitTime.x = deltaExit.x / velocity.x;
	}

	if (velocity.y == 0.0f)
	{
		entryTime.y = -K_AABBSWEEP_INF;
		exitTime.y = K_AABBSWEEP_INF;
	}
	else
	{
		entryTime.y = deltaEntry.y / velocity.y;
		exitTime.y = deltaExit.y / velocity.y;
	}

	if (entryTime.y > 1.0f) entryTime.y = -K_AABBSWEEP_INF;
	if (entryTime.x > 1.0f) entryTime.x = -K_AABBSWEEP_INF;

	float maxEntryTime = max(entryTime.x, entryTime.y);
	float minExitTime = min(exitTime.x, exitTime.y);

	float resultTime = 1.0f;
	Vec2 normal(0.0f, 0.0f);  // this is really the tangent
	int side = K_SIDE_NONE;
	bool bValid = false;

	if (!(maxEntryTime > minExitTime
		|| (entryTime.x < 0.0f && entryTime.y < 0.0f)
		|| (entryTime.x < 0.0f && ((movingbox.vMax.x + velocity.x * maxEntryTime < staticbox.vMin.x) || (movingbox.vMin.x + velocity.x * maxEntryTime > staticbox.vMax.x)))
		|| (entryTime.y < 0.0f && ((movingbox.vMax.y + velocity.y * maxEntryTime < staticbox.vMin.y) || (movingbox.vMin.y + velocity.y * maxEntryTime > staticbox.vMax.y)))
		))
	{
		//DMC: remove a small fraction so it doesn't penetrate the box or it will detect collisions at tile borders
		resultTime = max(maxEntryTime - K_AABBSWEEP_EPS, 0.0f);
		if (entryTime.x < entryTime.y)
		{
			normal = Vec2(SIGN(deltaEntry.x), 0.0f);
			if (velocity.y > 0.0f)
				side = K_SIDE_BOTTOM;
			else if (velocity.y < 0.0f)
				side = K_SIDE_TOP;
		}
		else if (entryTime.x > entryTime.y)
		{
			normal = Vec2(0.0f, SIGN(deltaEntry.y));
			if (velocity.x > 0.0f)
				side = K_SIDE_RIGHT;
			else if (velocity.x < 0.0f)
				side = K_SIDE_LEFT;
		}
		else
		{
			// Both axes now have equal intersection depth (direct and perfect corner collision).
			// We need to use the velocity to determine the correct collision normal
			Vec2 absVel(fabs(velocity.x), fabs(velocity.y));

			// If the x-velocity is stronger than the y-velocity, perform a y-collision like we did previously and vice-versa
			if (absVel.x >= absVel.y)
			{
				normal = Vec2(SIGN(velocity.x), 0.0f);
				if (velocity.y > 0.0f)
					side = K_SIDE_BOTTOM;
				else if (velocity.y < 0.0f)
					side = K_SIDE_TOP;
			}
			else
			{
				normal = Vec2(0.0f, SIGN(velocity.y));
				if (velocity.x > 0.0f)
					side = K_SIDE_RIGHT;
				else if (velocity.x < 0.0f)
					side = K_SIDE_LEFT;
			}
		}
		// all data computed, output is valid
		bValid = true;
	}

	float fDistSq = MUVec2LenSq(&(movingbox.vCenter - staticbox.vCenter));
	SweepData retdata(
		bValid,
		resultTime,		// Retrieve the time of impact
		normal,			// Return the collision normal vector to apply sliding effect
		side,			// Know what side of the player has been touched (used for controlling logic like jumping and walking)
		fDistSq			// Calculate the distance (squared for performance) used to resolve the closest collision subject
	);

	return retdata;
}

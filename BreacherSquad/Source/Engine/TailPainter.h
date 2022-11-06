#pragma once

namespace Tails
{
	// max tail length
	const int K_MAX_TAIL_POINTS = 30;

	// gets a median normalized vector that points to the same direction
	Vec2 GetJointMedian( Vec2 pJoint, Vec2 pAfter);

	// arrPos[0] is the most recent point. Returns mesh idx.
	int BuildTail( CBufferedPainter* pPainter, Vec2 * arrPos, int nPoints, float fWidth );
}

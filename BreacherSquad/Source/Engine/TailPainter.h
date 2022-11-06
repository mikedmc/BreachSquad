#pragma once

namespace Tails
{
	// max tail length
	const int K_MAX_TAIL_POINTS = 30;

	// arrPos[0] is the most recent point. Returns mesh idx.
	int BuildTail( CBufferedPainter* pPainter, Vec2 * arrPos, int nPoints, float fWidth );
}

#pragma once

class CTails
{
private:

	// gets a median normalized vector that points to the same direction
	Vec2 GetJointMedian( Vec2 pJoint, Vec2 pAfter);

public:
	// arrPos[0] is the most recent point. Returns mesh idx.
	// uses texRect actual texcoords to map the tail (art must be horizontal, pointing to the left eg: <o==-- )
	int BuildTail( CBufferedPainter* pPainter, Vec2 * arrPos, int nPoints, float fWidth, RectLTRB & texRect );
};

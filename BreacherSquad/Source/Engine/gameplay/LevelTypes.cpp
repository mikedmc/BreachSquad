#include "dxstdafx.h"



///--------------------------------------------------------------------------
/// MISC OBJECTS - diverse obiecte speciale exportate din editor (RAILS, etc)
///--------------------------------------------------------------------------

Vec2 CMiscObjectRail::GetPosNormalized(float fCursorNormalized, Vec2 * retDir)
{
	return GetPos(fLength * fCursorNormalized, retDir);
}

Vec2 CMiscObjectRail::GetPos(float fDistFromStart, Vec2 * retDir)
{
	if (fDistFromStart < 0.0f)
		return arrPoints[0];
	if (fDistFromStart > fLength)
		return arrPoints[arrPoints.nCount - 1];

	int selidx = 0;
	for (int kk = 1; kk < arrLenghts.nCount; kk++)
	{
		if (arrLenghts.m_pData[kk] >= fDistFromStart)
		{
			selidx = kk;
			break;
		}
	}
	//nu ar trebui sa nu gaseasca nod	
	assert(selidx > 0);
	//daca nu am gasit nod mai mare inseamna ca e in afara
	float percent = (fDistFromStart - arrLenghts.m_pData[selidx - 1]) / (arrLenghts.m_pData[selidx] - arrLenghts.m_pData[selidx - 1]);
	//directia
	if (retDir != nullptr)
	{
		D3DXVec2Normalize(retDir, &(arrPoints.m_pData[selidx] - arrPoints.m_pData[selidx - 1]));
	}
	//interpolare liniara
	return arrPoints.m_pData[selidx] * percent + arrPoints.m_pData[selidx - 1] * (1.0f - percent);
}



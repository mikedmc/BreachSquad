#include "dxstdafx.h"
#include "IngameGUI.h"

void CIngameGUI::Init(CLevel* pLevel, CCameraTransform* pCamera)
{
	_ASSERT(pLevel != nullptr && pCamera != nullptr);

	fLocalTimeline = 0.0f;
	m_pLevel = pLevel;
	m_pCamera = pCamera;
	// hard link sprite collection for easy access
	m_pSprite = &pLevel->m_sprInterface;
}

void CIngameGUI::Update(float dTime)
{
	fLocalTimeline += dTime;
}

void CIngameGUI::Paint(PDEVICE pDevice)
{
	if (m_pLevel == nullptr || m_pCamera == nullptr)
		return;

	CCameraTransform::SetActiveCamera(pDevice, m_pCamera);
	//#TODO: set UTPainter matrices - needs special method in UTPainter
	///--- player 1
	CActor* pl1 = m_pLevel->pPlayerActor[0];

	///#TODO: --- player 2

	__Painter().Flush();
}

void CIngameGUI::Release()
{
	m_pCamera = nullptr;
	m_pLevel = nullptr;
}

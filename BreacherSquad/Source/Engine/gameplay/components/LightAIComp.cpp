#include "dxstdafx.h"
#include "LightAIComp.h"

CLightAIComponent::CLightAIComponent()
{

}

CLightAIComponent::~CLightAIComponent()
{

}

bool CLightAIComponent::Update( CLight& active, float dTime, CLevel& level )
{
	if ( active.AIstate == K_AI_STATE_UNDEFINED )
		return true;
	//update timeline
	fTimelineAI += dTime;

	switch ( active.AIstate )
	{
		case K_AI_STATE_FN_LIGHT_FLICKER1:
		{
			//params: f_timeMul, f_threshold
			float timeMul = active.varAIparams.GetVariantByName( L"f_timeMul" )->m_asFloat;
			float fThreshold = active.varAIparams.GetVariantByName( L"f_threshold" )->asFloat();
			float falpha = UTPerlin::PerlinNoise1D( fTimelineAI * timeMul, 2.0f, 3.0f, 0.8f, 0.25f, 2 );
			if ( falpha > fThreshold )
				falpha = 1.0f;
			else
				falpha = falpha / fThreshold;
			//falpha = (falpha < fThreshold) ? 0.0f : 1.0f;
			active.color = DW_COLORALPHA( active.color_ini, falpha );
		}
		break;
		case K_AI_STATE_FN_LIGHT_ANG_CONE_XZ_TIME:
		{
			//fvar1 - height, fvar2 - radius, timer1 - timeMul, timer2 - timeAdd
			Vec3 conepoint( 0.0f, -mem.AIfvar1, 0.0f );
			Vec3 ppos = Vec3( mem.AIfvar2 * sin( (fTimelineAI + mem.AItimer2) * mem.AItimer1 ), 0.0f, mem.AIfvar2 * cos( (fTimelineAI + mem.AItimer2) * mem.AItimer1 ) );
			MUVec3Norm( &active.vnDir, &(ppos - conepoint) );
		}
		break;
		default:
		{
			// call base update if not handled
			if ( !CActiveAIComponent::Update( active, dTime, level ) )
			{
				ErrorBox( K_ERR_WARNING, L"LightAIComponent::Update - AIstate not handled: %d", EAIstate_names[ active.AIstate ] );
			}
		}
		break;
	}
}

#include "dxstdafx.h"

//--- static members ---
CCameraTransform* CCameraTransform::g_currentCamera = NULL;

void CCameraTransform::SetActiveCamera(PDEVICE pDevice, CCameraTransform *camera)
{
	if ((pDevice == NULL) || (camera == NULL))
	{
		ErrorBox(K_ERR_WARNING, L"SetViewCamera: Device or Camera is NULL!");
		return;
	}

	pDevice->SetTransform( D3DTS_VIEW, &(camera->GetViewTransform()) );
	g_currentCamera = camera;
}

CCameraTransform* CCameraTransform::GetActiveCamera()
{
	return g_currentCamera;
}

void CCameraTransform::SetActiveCameraIdentity(PDEVICE pDevice)
{
	if (pDevice == NULL)
	{
		ErrorBox(K_ERR_WARNING, L"SetViewIdentity: Device or Camera is NULL!");
		return;
	}

	pDevice->SetTransform(D3DTS_VIEW, &g_matIdentity);
	g_currentCamera = NULL;
}

/*----------------------------------*\
*  Constructor/Destructor
\*----------------------------------*/

CCameraTransform::CCameraTransform()
{
	m_Viewport = RECTXYWH_F(0.0f, 0.0f, 0.0f, 0.0f);

	m_animType = K_CAMTRANS_ANIM_NONE;

	fLocalTimeLine = 0.0f;
	D3DXMatrixIdentity(&m_matView);

	m_worldAABB = RECTXYWH_F(0.0f, 0.0f, 0.0f, 0.0f);

	m_camScreenSize = 0;
	m_camScreenAxis = K_CAMTRANS_AXIS_NONE;
	//look at
	m_vecLookAt = Vec3(0.0f, 0.0f, 1.0f);
	m_vecLookAtSpeed = Vec3(0.0f, 0.0f, 0.0f);
	m_vecRealLookAt = Vec3(0.0f, 0.0f, 1.0f);

	m_vecHW = Vec2(1.0f, 0.0f);
	m_vecHH = Vec2(0.0f, 1.0f);

	m_veck1 = m_veck2 = Vec2(0.0f, 0.0f);

	m_camWorldAABB = RECTXYWH_F(0.0f, 0.0f, 0.0f, 0.0f);
	m_constraintAxis = K_CAMTRANS_AXIS_NONE;
	m_minAxisSize = 0.0f;
    m_maxAxisSize = 0.0f;
    m_bHardWorldEdges = false;

	m_bAxisLockedX = m_bAxisLockedY = m_bAxisLockedZoom = false;
	m_bPixelPerfect = false;

	m_shakeAmplitude = m_shakeAttenuationPerSec = 0.0f;
}

void CCameraTransform::SetAxisLock(bool lockXaxis, bool lockYaxis, bool lockZoom)
{
	m_bAxisLockedX = lockXaxis;
	m_bAxisLockedY = lockYaxis;
	m_bAxisLockedZoom = lockZoom;
}

void CCameraTransform::SetWorldBounds(RECTXYWH_F worldAABB, bool bHardWorldEdges, ECamAxisType constraintAxis, float minAxisSize, float maxAxisSize)
{
    m_bHardWorldEdges = bHardWorldEdges;
    m_worldAABB = worldAABB;
    if((m_worldAABB.w <= 0.0f) || (m_worldAABB.h <= 0.0f))
    {
        m_bHardWorldEdges = false;
    }

	m_constraintAxis = constraintAxis;
	m_minAxisSize = minAxisSize;
    m_maxAxisSize = maxAxisSize;
}

void CCameraTransform::SetCamPos(Vec2 *vecLookAt, float fZoom, bool forced)
{
	if (vecLookAt != NULL)
	{
		if (!m_bAxisLockedX)
		{
			m_vecLookAt.x = vecLookAt->x;
			if (forced)
				m_vecRealLookAt.x = m_vecLookAt.x;
		}
		if (!m_bAxisLockedY)
		{
			m_vecLookAt.y = vecLookAt->y;
			if (forced)
				m_vecRealLookAt.y = m_vecLookAt.y;
		}
	}

	//zoom
	if (!m_bAxisLockedZoom)
		m_vecLookAt.z = fZoom;
	if (forced)
	{
		m_vecRealLookAt.z = m_vecLookAt.z;
		m_vecLookAtSpeed = Vec3(0.0f, 0.0f, 0.0f);
	}
}

void CCameraTransform::MoveCamPos(Vec2 vDelta, bool forced /*= false*/)
{
	m_vecLookAt.x += vDelta.x;
	if (forced)
		m_vecRealLookAt.y += vDelta.y;
}

void CCameraTransform::SetViewport(RECTXYWH_F viewport)
{
	m_Viewport = viewport;
}

void CCameraTransform::InitCamera(RECTXYWH_F viewport, int camScreenSize, ECamAxisType eSizeAxis, Vec2 vecLookAt, float fZoom)
{
	m_Viewport = viewport;
	m_camScreenSize = abs(camScreenSize);
	m_camScreenAxis = eSizeAxis;
	m_vecLookAt = m_vecRealLookAt = Vec3(vecLookAt.x, vecLookAt.y, fZoom);
	m_vecLookAtSpeed = Vec3(0.0f, 0.0f, 0.0f);
}

void CCameraTransform::ZoomToFitWorld()
{
	//TODO: de implementat
}

Vec2 CCameraTransform::ScreenToWorld(Vec2 inPt, RECTXYWH_F *srcViewportOverride)
{
	RECTXYWH_F *view;
	if (srcViewportOverride != NULL)
		view = srcViewportOverride;
	else
		view = &m_Viewport;

	double percX, percY;
	//aflu procente intre -1 si 1 pt ca originea este centrul ecranului
	percX = 2.0f * (((inPt.x - view->x) / view->w) - 0.5f);
	percY = 2.0f * (((inPt.y - view->y) / view->h) - 0.5f);
	Vec2 vecLookAtXY(m_vecRealLookAt.x, m_vecRealLookAt.y);
	return vecLookAtXY + Vec2(percX * m_vecHW.x, percX * m_vecHW.y) + Vec2(percY * m_vecHH.x, percY * m_vecHH.y);
}

RECTXYWH_F CCameraTransform::ScreenToWorld(RECTXYWH_F inRect)
{
	double percX, percY;
	//aflu procente intre -1 si 1 pt ca originea este centrul ecranului
	percX = 2.0f * (((inRect.x - m_Viewport.x) / m_Viewport.w) - 0.5f);
	percY = 2.0f * (((inRect.y - m_Viewport.y) / m_Viewport.h) - 0.5f);
	Vec2 vecLookAtXY(m_vecRealLookAt.x, m_vecRealLookAt.y);
	Vec2 retpos = vecLookAtXY + Vec2(percX * m_vecHW.x, percX * m_vecHW.y) + Vec2(percY * m_vecHH.x, percY * m_vecHH.y);
	//aflu procente intre 0 si 1 din jumatatile de vectori de directie
	percX = inRect.w / m_Viewport.w;
	percY = inRect.h / m_Viewport.h;
	Vec2 nscale = percX * 2.0f * m_vecHW + percY * 2.0f * m_vecHH;
	return RECTXYWH_F(retpos.x, retpos.y, nscale.x, nscale.y);
}

Vec2 CCameraTransform::WorldToScreen(Vec2 inPT, RECTXYWH_F *srcViewportOverride)
{
	RECTXYWH_F *view;
	if (srcViewportOverride != NULL)
		view = srcViewportOverride;
	else
		view = &m_Viewport;

	double percX, percY;
	//procente intre -1 si 1 in fn de lungimea axelor vecHW si vecHH
	//TODO: daca adaug rotatie aici trebuie facut cu vectori si proiectii!
	percX = ((inPT.x - m_vecRealLookAt.x) / m_vecHW.x) / 2.0f;
	percY = ((inPT.y - m_vecRealLookAt.y) / m_vecHH.y) / 2.0f;
	return Vec2(view->w / 2.0f + percX * view->w + view->x, view->h / 2.0f + percY * view->h + view->y);
}

RECTXYWH_F CCameraTransform::WorldToScreen(RECTXYWH_F inRect)
{
	double percX, percY;
	//procente intre -1 si 1 in fn de lungimea axelor vecHW si vecHH
	//TODO: daca adaug rotatie aici trebuie facut cu vectori si proiectii!
	percX = ((inRect.x - m_vecRealLookAt.x) / m_vecHW.x) / 2.0f;
	percY = ((inRect.y - m_vecRealLookAt.y) / m_vecHH.y) / 2.0f;
	Vec2 vpos(m_Viewport.w / 2.0f + percX * m_Viewport.w + m_Viewport.x, m_Viewport.h / 2.0f + percY * m_Viewport.h + m_Viewport.y);
	percX = (inRect.w / m_vecHW.x) / 2.0f;
	percY = (inRect.h / m_vecHH.y) / 2.0f;
	return RECTXYWH_F(vpos.x, vpos.y, percX * m_Viewport.w, percY * m_Viewport.h);
}

Vec2 CCameraTransform::ScreenToViewport(Vec2 inPt)
{
	return Vec2(inPt.x - m_Viewport.x, inPt.y - m_Viewport.y);
}

Vec2 CCameraTransform::ViewportToScreen(Vec2 inPt)
{
	return Vec2(inPt.x + m_Viewport.x, inPt.y + m_Viewport.y);
}


SIZEWH_F CCameraTransform::ScreenToWorld(SIZEWH_F inSZ)
{
	double percX, percY;
	//aflu procente intre 0 si 1 din jumatatile de vectori de directie
	percX = inSZ.w / m_Viewport.w;
	percY = inSZ.h / m_Viewport.h;
	Vec2 nscale = percX * 2.0f * m_vecHW + percY * 2.0f * m_vecHH;
	return SIZEWH_F(nscale.x, nscale.y);
}

SIZEWH_F CCameraTransform::WorldToScreen(SIZEWH_F inSZ)
{
	double percX, percY;
	//procente intre -1 si 1 in fn de lungimea axelor vecHW si vecHH
	//TODO: daca adaug rotatie aici trebuie facut cu vectori si proiectii!
	percX = (inSZ.w / m_vecHW.x) / 2.0f;
	percY = (inSZ.h / m_vecHH.y) / 2.0f;
	return SIZEWH_F(percX * m_Viewport.w, percY * m_Viewport.h);
}

Vec2	CCameraTransform::ViewportToViewport(Vec2 inPt, CCameraTransform &destCam)
{
	RECTXYWH_F newView = destCam.GetViewport();
	//le aduce in ecranul default dupa care le duce in noul viewport
	return Vec2(inPt.x + m_Viewport.x - newView.x, inPt.y + m_Viewport.y - newView.y);
}


Vec2	CCameraTransform::WorldToWorld(Vec2 inPt, CCameraTransform &destCam)
{
	RECTXYWH_F newView = destCam.GetViewport();
	//aduce punctul in viewport
	Vec2 viewPos = WorldToScreen(inPt);
	//trece din destView in destWorld
	return (destCam.ScreenToWorld(viewPos));
}

///--- functii pentru schimbarea modului de animare a camerei ---
void CCameraTransform::SetCamAnimationNone()
{
	m_animType = K_CAMTRANS_ANIM_NONE;
}

void CCameraTransform::SetCamAnimationSpring(float springKs, float dampingKd)
{
	m_animType = K_CAMTRANS_ANIM_SPRING;
	m_k1 = springKs;
	m_k2 = dampingKd;
}

void CCameraTransform::SetCamAnimationInertial(Vec2 elasticBorderExtension, float frictionK, float springKd, float zoomMin, float zoomMax)
{
	m_animType = K_CAMTRANS_ANIM_INERTIAL;
	m_k1 = frictionK;
	m_k2 = springKd;
	m_veck1 = elasticBorderExtension;
	m_veck2 = Vec2(zoomMin, zoomMax); //salvez constantele springului pt zoom
}

///--- screen shake ---
void CCameraTransform::ShakeScreen(float maxAmplitude, float attenuationPerSecond, Vec2 * vShakeSource)
{
	if (UTApp().m_Settings.bScreenShakes == false)
		return;
	if (maxAmplitude == 0.0f)
	{
		m_shakeAmplitude = 0.0f;
		m_shakeAttenuationPerSec = 0.0f;
	}
	//when source is specified check to see if onscreen
	RECTXYWH_F bboxExtended = m_camWorldAABB;
	bboxExtended.Inflate(m_camWorldAABB.h / 4.0f);
	if (vShakeSource != null)
	{
		if (!Rects::PointInRect(vShakeSource->x, vShakeSource->y, &bboxExtended))
			return;
	}

	if (maxAmplitude > m_shakeAmplitude)
		m_shakeAmplitude = maxAmplitude;
	m_shakeAttenuationPerSec = attenuationPerSecond;
}

ECamMoveStatus CCameraTransform::Update(float dTime, bool userHasInput, Vec3 inputDelta)
{
	ECamMoveStatus retval = K_CAMTRANS_STILL;
	//update local timeline
	fLocalTimeLine += dTime;
	///--- calculeaza vectori initiali ecran ---
	float fPixelSize = 1.0f;
	//daca nu este setata marimea zonei virtuale a camerei o seteaza aici cat cea a ecranului final
	Vec2 vScreenSize(m_Viewport.w, m_Viewport.h);
	if (m_camScreenSize != 0)
	{
		if (m_camScreenAxis == K_CAMTRANS_AXIS_V)
		{
			float fAspect = (float)m_Viewport.w / (float)m_Viewport.h;
			vScreenSize.y = m_camScreenSize;
			vScreenSize.x = (m_camScreenSize * fAspect);
			//ma asigur ca am mereu dimensiune para
			//if ((((int)vScreenSize.x) % 2) == 1)
//				vScreenSize.x += 1.0f;
			fPixelSize = m_Viewport.h / m_camScreenSize;
		}
		else
		{
			float fAspect = (float)m_Viewport.h / (float)m_Viewport.w;
			vScreenSize.x = m_camScreenSize;
			vScreenSize.y = (m_camScreenSize * fAspect);
			//ma asigur ca am mereu dimensiune para
			//if ((((int)vScreenSize.y) % 2) == 1)
				//vScreenSize.y += 1.0f;
			fPixelSize = m_Viewport.w / m_camScreenSize;
		}
	}

	m_vecHW = Vec2(vScreenSize.x * 0.5f, 0.0f);
	m_vecHH = Vec2(0.0f, vScreenSize.y * 0.5f);
	//trateaza zoom
	m_vecHH /= m_vecRealLookAt.z;
	m_vecHW /= m_vecRealLookAt.z;
	//TODO: daca bag rotatii probabil o sa dispara verificarea axelor? adica ar fi aiurea sa stabilesti axa minima pe ecran rotit...
	//verificare sa respecte minimul afisabil daca este setat
	if ((m_constraintAxis != K_CAMTRANS_AXIS_NONE) && (m_minAxisSize > 0.0f))
	{
		if (m_constraintAxis == K_CAMTRANS_AXIS_H)
		{
			float scale = 1.0f;
			if (2.0f * m_vecHW.x < m_minAxisSize)
			{
				scale = m_minAxisSize / (2.0f * m_vecHW.x);
			}
			else if (2.0f * m_vecHW.x > m_maxAxisSize)
			{
				scale = m_maxAxisSize / (2.0f * m_vecHW.x);
			}

			m_vecHW *= scale;
			m_vecHH *= scale;
		}
		else if (m_constraintAxis == K_CAMTRANS_AXIS_V)
		{
			float scale = 1.0f;
			if (2.0f * m_vecHH.y < m_minAxisSize)
			{
				scale = m_minAxisSize / (2.0f * m_vecHH.y);
			}
			else if (2.0f * m_vecHH.y > m_maxAxisSize)
			{
				scale = m_maxAxisSize / (2.0f * m_vecHH.y);
			}

			m_vecHW *= scale;
			m_vecHH *= scale;
		}
	}
	//TODO: aici se va adauga rotatia vectorilor si se va complica putin la verificarea limitarilor pentru ca va trebui verificat dreptunghiul rotit
	//verificare sa respecte dimensiunea maxima, adica sa nu iasa cu zoom din ea
	RECTXYWH_F	limitRect = m_worldAABB;
	if (m_bHardWorldEdges)
	{
		if (m_vecHW.x * 2.0f > limitRect.w)
		{
			float scale = limitRect.w / (m_vecHW.x * 2.0f);
			m_vecHW *= scale;
			m_vecHH *= scale;
		}
		if (m_vecHH.y * 2.0f > limitRect.h)
		{
			float scale = limitRect.h / (m_vecHH.y * 2.0f);
			m_vecHW *= scale;
			m_vecHH *= scale;
		}
	}
	///--- animatie camera ---
	switch (m_animType)
	{
		case K_CAMTRANS_ANIM_NONE:
		{
			m_vecRealLookAt = m_vecLookAt;
		}
			break;
		case K_CAMTRANS_ANIM_SPRING:
		{
			//blocarea axelor este tratata in setCamPos
			//pozitie
			Vec3 deltaP = m_vecLookAt - m_vecRealLookAt;
			float dist = D3DXVec3Length(&deltaP);

			Vec3 springForce, force;
			D3DXVec3Normalize(&springForce, &deltaP);
			springForce *= dist * m_k1; //konstanta hook
			force = springForce - m_vecLookAtSpeed * m_k2; //aici face damping

			m_vecLookAtSpeed += force * dTime;
			m_vecRealLookAt += m_vecLookAtSpeed * dTime;

			if (dist > 1.0f)
				retval = K_CAMTRANS_MOVING;
		}
			break;
		case K_CAMTRANS_ANIM_INERTIAL:
		{
			//set speed
			if (userHasInput)
			{
				//x si y sunt inmultite cu dTime deci inputul este invers fata de dTime si zoom
				m_vecLookAtSpeed.x = (inputDelta.x / dTime) / m_vecRealLookAt.z;
				m_vecLookAtSpeed.y = (inputDelta.y / dTime) / m_vecRealLookAt.z;
				m_vecLookAtSpeed.z = (inputDelta.z / dTime);// / m_vecRealLookAt.z; //<-- zoom-ul nu trebuie scalat in fn de zoom
			}

			//elasticitate limite
			//TODO: aici probabil va trebui schimbat daca bagam rotatii
			//calculeaza dreptunghiul in care are voie centrul camerei micsorand aabb-ul lumii cu limita data de noi (extensia elastica)
			RECTLTRB_F centerRect(m_worldAABB.x + m_vecHW.x + m_veck1.x, m_worldAABB.y + m_vecHH.y + m_veck1.y,
				m_worldAABB.x + m_worldAABB.w - m_vecHW.x - m_veck1.x, m_worldAABB.y + m_worldAABB.h - m_vecHH.y - m_veck1.y);

			Vec2 springOff(0.0f, 0.0f);
			//verificare elasticitate centru
			if (m_veck1.x > 0.0f)
			{
				if (m_vecRealLookAt.x < centerRect.left)
				{
					springOff.x = -((centerRect.left - m_vecRealLookAt.x) / m_veck1.x);
				}
				else if (m_vecRealLookAt.x > centerRect.right)
				{
					springOff.x = ((m_vecRealLookAt.x - centerRect.right) / m_veck1.x);
				}
			}
			if (m_veck1.y > 0.0f)
			{
				if (m_vecRealLookAt.y < centerRect.top)
				{
					springOff.y = -((centerRect.top - m_vecRealLookAt.y) / m_veck1.y);
				}
				else if (m_vecRealLookAt.y > centerRect.bottom)
				{
					springOff.y = ((m_vecRealLookAt.y - centerRect.bottom) / m_veck1.y);
				}
			}
			//daca ai iesit pe zona extinsa inmulteste cu elasticul (intre -1 si 1 la capete)
			if ((springOff.x != 0.0f) && (SIGN(m_vecLookAtSpeed.x) == SIGN(springOff.x)))
				m_vecLookAtSpeed.x *= 1.0f - fabs(springOff.x);
			if ((springOff.y != 0.0f) && (SIGN(m_vecLookAtSpeed.y) == SIGN(springOff.y)))
				m_vecLookAtSpeed.y *= 1.0f - fabs(springOff.y);
			//daca esti in zona extra te aduce inapoi in ecran
			if (!userHasInput)
			{
				m_vecLookAtSpeed.x -= springOff.x * dTime * m_veck1.x * m_k2;
				m_vecLookAtSpeed.y -= springOff.y * dTime * m_veck1.y * m_k2;
			}

			if (!userHasInput)
			{
				//add friction
				m_vecLookAtSpeed -= m_vecLookAtSpeed * m_k1 * dTime;
			}

			//update position
			if(!m_bAxisLockedX)
				m_vecRealLookAt.x += m_vecLookAtSpeed.x * dTime;
			if (!m_bAxisLockedY)
				m_vecRealLookAt.y += m_vecLookAtSpeed.y * dTime;

			if (!m_bAxisLockedZoom)
			{
				//update zoom
				m_vecRealLookAt.z += m_vecLookAtSpeed.z * dTime;
				//clamp zoom and stop zoom speed
				if (m_vecRealLookAt.z < m_veck2.x) //zoom minim
				{
					m_vecRealLookAt.z = m_veck2.x;
					m_vecLookAtSpeed.z = 0.0f;
				}
				else if (m_vecRealLookAt.z > m_veck2.y) //zoom maxim
				{
					m_vecRealLookAt.z = m_veck2.y;
					m_vecLookAtSpeed.z = 0.0f;
				}
			}

			//moving state
			if(D3DXVec3LengthSq(&m_vecLookAtSpeed) > 1.0f)
				retval = K_CAMTRANS_MOVING;
		}
			break;
	}

	///--- Calculeaza limitarile si matricele ---
	//camera shake - se cheama aici ca sa nu iasa din ecran nici pe hard edges
	if (m_shakeAmplitude > 0.0f)
	{
		//TODO: oare ar trebui luat in considerare zoom-ul si poate adaugate amplitudini diferite pe axe?
		//fac cu sin si cos ca sa nu depinda de framerate
		m_vecRealLookAt.x += m_shakeAmplitude * sin(fLocalTimeLine * 131);
		m_vecRealLookAt.y += m_shakeAmplitude * cos(fLocalTimeLine * 71);
		
		dec_limit(m_shakeAmplitude, m_shakeAttenuationPerSec * dTime, 0.0f);
	}
	//calculeaza camera AABB ca sa il limitez si se scot din el din nou vectorii
	//TODO: aici se schimba cand vom trata si rotatiile. Va trebui verificat dreptunghiul rotit sa intre inapoi in dreptunghiul lumii
	//se va face AABB-ul camerei dupa dreptunghiul rotit si se va incadra acela in cel al lumii
	m_camWorldAABB = RECTXYWH_F(m_vecRealLookAt.x - m_vecHW.x, m_vecRealLookAt.y - m_vecHH.y, m_vecHW.x * 2.0f, m_vecHH.y * 2.0f);
	//verificare cu outer world bounds doar daca avem hard world edges
	if (m_bHardWorldEdges)
	{
		//left
		if (m_camWorldAABB.x < limitRect.x)
		{
			m_camWorldAABB.x = limitRect.x;
			m_vecLookAtSpeed.x = 0.0f;
		}
		//right	- doar daca nu a iesit prin stanga (optimizare)
		else if (m_camWorldAABB.x + m_camWorldAABB.w > limitRect.x + limitRect.w)
		{
			m_camWorldAABB.x -= m_camWorldAABB.x + m_camWorldAABB.w - (limitRect.x + limitRect.w);
			m_vecLookAtSpeed.x = 0.0f;
		}
		//up
		if (m_camWorldAABB.y < limitRect.y)
		{
			m_camWorldAABB.y = limitRect.y;
			m_vecLookAtSpeed.y = 0.0f;
		}
		//bottom - doar daca nu a iesit pe sus (optimizare)
		else if (m_camWorldAABB.y + m_camWorldAABB.h > limitRect.y + limitRect.h)
		{
			m_camWorldAABB.y -= m_camWorldAABB.y + m_camWorldAABB.h - (limitRect.y + limitRect.h);
			m_vecLookAtSpeed.y = 0.0f;
		}
	}
	//scot din nou vectorii din bbox (vectorii de HW si HH sunt corecti, doar pozitia de look at se schimba)
	m_vecRealLookAt = Vec3(m_camWorldAABB.x + m_camWorldAABB.w / 2.0f, m_camWorldAABB.y + m_camWorldAABB.h / 2.0f, m_vecRealLookAt.z);
	//needs to be aligned to pixel edges? - se poate mai bine aici, adica ar trebui tratat si in partea cu paint, poate sa ia din tex de la 0.5f
	Vec2 vFinalTranslate(m_Viewport.x + m_Viewport.w / 2.0f, m_Viewport.y + m_Viewport.h / 2.0f);
	if (m_bPixelPerfect)
	{
		m_vecRealLookAt.x = ROUND_FLOAT(m_vecRealLookAt.x);
		m_vecRealLookAt.y = ROUND_FLOAT(m_vecRealLookAt.y);

		vFinalTranslate.x = ROUND_FLOAT(vFinalTranslate.x) - 1.0f / fPixelSize - 1.0f /*needed to keep perfect X alignment*/;
		vFinalTranslate.y = ROUND_FLOAT(vFinalTranslate.y) - 1.0f / fPixelSize;
	}
	//calculare matrice scalare finala ca sa incapa pe ecran
	float fFinalZoom = (float)m_Viewport.h / (float)m_camWorldAABB.h;
	//aici se taie din ultimele zecimale din float
	//fFinalZoom = floor(fFinalZoom * 100.0f) / 100.0f;

	D3DXMATRIXA16 m1;
	D3DXMatrixTranslation(&m_matView, -m_vecRealLookAt.x, -m_vecRealLookAt.y, 0.0f);
	D3DXMatrixScaling(&m1, fFinalZoom, fFinalZoom, 1.0f);
	m_matView = m_matView * m1;
	D3DXMatrixTranslation(&m1, vFinalTranslate.x, vFinalTranslate.y, 0.0f);
	m_matView = m_matView * m1;

	return retval;
}



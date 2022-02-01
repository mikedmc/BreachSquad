#pragma once

enum ECamAxisType {
	K_CAMTRANS_AXIS_NONE = 0,
	K_CAMTRANS_AXIS_H = 1,
	K_CAMTRANS_AXIS_V = 2,
};

enum ECamAnimType {
	K_CAMTRANS_ANIM_NONE = 0,
	K_CAMTRANS_ANIM_SPRING = 1,		// sprin animation
	K_CAMTRANS_ANIM_INERTIAL,		// inertial camera (iOS style)
};

enum ECamMoveStatus {
	K_CAMTRANS_MOVING = 0,
	K_CAMTRANS_STILL = 1,
};

//TODO: - de adaugat rotatia ecranului. Daca ingreuneaza update-ul ar trebui facuta clasa separata
//TODO: - de adaugat camera follow si zoom follow cu tipuri diferite de animatie (spring, inertial, etc)

class CCameraTransform
{
private:
	static CCameraTransform*	g_currentCamera; //camera curenta
private:
	RectXYWH					m_Viewport;			// viewport in screen coords (rectangle on screen where we show the contents)
	float						fLocalTimeLine;
	Mat							m_matView;

	ECamAnimType				m_animType;			// type of camera animation
	float						m_k1, m_k2;			// animation constants
	Vec2						m_veck1, m_veck2;	// vector anim constants

	RectXYWH					m_worldAABB;		// camera world bbox limits in world coords. If 0 then not set.
	//datele din care se construieste dreptunghiul vizibil pe camera in world coords
	Vec3		m_vecLookAt;		//(x, y, zoom) punctul unde se doreste pozitionata camera (vine spre acest punct cu animatie)
	Vec3		m_vecRealLookAt;	//(x, y, zoom) look at real - punctul spre care priveste acum camera, se duce catre punctul m_vecLookAt cu animatie
	Vec3		m_vecLookAtSpeed;	//(x, y, zoom) viteza cu care se deplaseaza look at catre destinatie

	Vec2		m_vecHW, m_vecHH;	//vectorii care pornesc din lookat (centru) si se duc pe jumatate din latimea/inaltimea ecranului - world space
	RectXYWH		m_camWorldAABB;		//camera view rectangle in world coords

	int				m_camScreenSize;	//marimea ecranului virtual vazut de camera. cealalta axa se calculeaza in fn de rezolutia ecranului
	ECamAxisType	m_camScreenAxis;	//axa pe care e setat ScreenSize

	bool            m_bHardWorldEdges;  //daca este true nu am voie sa vad nimic in afara worldAABB (deci scaleaza ca sa umple ecranul)
	//axa si dimensiunea minima ce trebuie afisata. ex: la un fundal vei dori sa se vada tot pe inaltime deci se seteaza axa=verticala si limita = inaltimea fundalului in coord world
	ECamAxisType	m_constraintAxis;
	float			m_minAxisSize, m_maxAxisSize;
	//proprietati diverse
	bool			m_bAxisLockedX; //axa X este blocata deci va fi mereu egala cu pozitia initiala LookAt. In Update nu se updateaza X
	bool			m_bAxisLockedY;	//axa Y este blocata deci va fi mereu egala cu pozitia initiala LookAt. In Update nu se updateaza Y
	bool			m_bAxisLockedZoom;

	bool			m_bPixelPerfect;	//camera needs to be aligned to pixel edges (int)
	//screen shake
	float			m_shakeAmplitude;  //amplitudinea maxima in world coord
	float			m_shakeAttenuationPerSec; //cat scade amplitudinea pe secunda
public:
	CCameraTransform();

	///--- UTILS ---
	//functii care seteaza transformarea curenta si iti spune ultima transformare setata
	static void SetActiveCameraIdentity(PDEVICE pDevice);
	static void SetActiveCamera(PDEVICE pDevice, CCameraTransform *camera);
	static CCameraTransform* GetActiveCamera();
	///--- SET ---
    //Params SetWorldBounds
    //worldAABB - dreptunghiul in care se incadreaza lumea pe care o priveste camera
    //bHardWorldEdges - true - camera nu are voie sa afiseze nimic din afara lumii deci va scala in asa fel incat sa umple ecranul
    //constraintAxis - axa de constrangere la zoom (vertical sau orizontal). Pe axa respectiva nu poti sa vezi mai putin de minAxisSize sau mai mult de maxAxisSize
	void SetWorldBounds(RectXYWH worldAABB, bool bHardWorldEdges = true, ECamAxisType constraintAxis = K_CAMTRANS_AXIS_NONE, float minAxisSize = 0.0f, float maxAxisSize = 100000.0f);
	//initializeaza camera
	//viewport - viewportul in care face transformarile (coordonate ecran)
	//camScreenSize - dimensiunea ecranului virtual (pe axa eSizeAxis) al camerei raportat la viewport, cand zoom este 1.0f; 0 - size of viewport
	void InitCamera(RectXYWH viewport, int camScreenSize, ECamAxisType eSizeAxis, Vec2 vecLookAt, float fZoom = 1.0f);
	//se cheama dupa initCamera ca sa iti afiseze tot dreptunghiul lumii in dreptunghiul camerei. Seteaza hardWorldEdges pe false
	void ZoomToFitWorld();
	
	// Sets camera target position
	// \param pos==NULL sets only zoom-ul. Nu se aplica pe camera inertiala unde controlul este al utilizatorului. forced face repozitionare imediata
	void SetCamPos(Vec2 *vecLookAt, float fZoom = 1.0f, bool forced = false);
	
	// Moves camera look at vector with delta
	// \param forced - moves immediately, doesn't wait for animation (moves real position too)
	void MoveCamPos(Vec2 vDelta, bool forced = false);

	void SetViewport(RectXYWH viewport);
	//daca incui o axa nu va mai fi modificata din update. Practic ramane ce setezi in initCamera
	void SetAxisLock(bool lockXaxis, bool lockYaxis, bool lockZoom);
	/*!
	 * \brief Use when the camera needs to be pixel perfect (eg. thin lines appear on interfaces)
	 * Rounds the translates to the nearest integer
	 */
	FORCEINLINE void SetPixelPerfect(bool bIsPixelPerfect) { m_bPixelPerfect = bIsPixelPerfect; }
	//seteaza tipurile de animatii, fiecare cu parametrii ei
	void SetCamAnimationNone(); //default animation - optional
	void SetCamAnimationSpring(float springKs = 5.0f, float dampingKd = 4.0f); //springKS - puterea arcului, springKd - damping
	void SetCamAnimationInertial(Vec2 elasticBorderExtension, float frictionK = 4.0f, float springKd = 20.0f, float zoomMin = 1.0f, float zoomMax = 1.0f); //specifici daca il lasi sa iasa din limitele lumii si cu cat pe fiecare axa

	//screen shake
	void ShakeScreen(float maxAmplitude, float attenuationPerSecond, Vec2 * vShakeSource = null);
	//calculeaza toti parametrii interni ca sa ii poti lua prin fns Get (userHasInput, inputDelta(x,y,zoom) sunt folosite doar pe animatia INERTIAL)
	ECamMoveStatus  Update(float dTime, bool userHasInput = false, Vec3 inputDelta = Vec3(0.0f, 0.0f, 0.0f) );
	///--- GET ---
	//functiile GET trebuiesc chemate dupa Update
	FORCEINLINE Mat & GetViewTransform() { return m_matView; }
	FORCEINLINE const Vec3 & GetCamPos() const { return m_vecRealLookAt; }
	/*!
	 * \brief Gets the visible rectangle in world coordinates
	 */
	FORCEINLINE const RectXYWH &	GetCamWorldAABB() const { return m_camWorldAABB; }
	//void				GetCamVectors(Vec2 *LookAtPt, Vec2 *vecRightHW, Vec2 *vecDownHH);

	FORCEINLINE const RectXYWH &	GetWorldAABB() const { return m_worldAABB; }
	FORCEINLINE const RectXYWH & GetViewport() const { return m_Viewport; }

	//trece din coord ecran in coord World
	//param: inPt - punctul cerut ca input
	//param: srcViewportOverride - daca inPt este in coordonate diferite de cele ale ecranului real se vor specifica aici. De exemplu touch-ul de la iOS are alte dimensiuni
	Vec2		ScreenToWorld(Vec2 inPt, RectXYWH *srcViewportOverride = NULL);
	Vec2		WorldToScreen(Vec2 inPT, RectXYWH *srcViewportOverride = NULL);
	Vec2		ViewportToScreen(Vec2 inPt);
	Vec2		ScreenToViewport(Vec2 inPT);
	SIZEWH_F		ScreenToWorld(SIZEWH_F inSZ);
	SIZEWH_F		WorldToScreen(SIZEWH_F inSZ);
	RectXYWH		ScreenToWorld(RectXYWH inRect);
	RectXYWH		WorldToScreen(RectXYWH inRect);
	///--- transformari intre 2 camere ---

	//Transforma un punct din viewportul camerei curente in viewportul camerei destCam
	Vec2		ViewportToViewport(Vec2 inPt, CCameraTransform &destCam);
	//Transforma un punct din Lumea camerei curente in lumea camerei destCam
	Vec2		WorldToWorld(Vec2 inPt, CCameraTransform &destCam);
};
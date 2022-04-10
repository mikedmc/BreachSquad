#pragma once

// structure that holds AI variables for fast access
//#TODO: change with fast map with names
struct AImem {
	UINT32					AItargetUID;					// enemy UID (not the one set from the editor!!!)
	float					AItimer1, AItimer2;
	float					AIfvar1, AIfvar2, AIfvar3;
	int						AIvar1, AIvar2;
	Vec2					AIvec1;
	bool					AIvarBool1, AIvarBool2;
	CStringHash				AIstrvar1, AIstrvar2;

	AImem() : AItargetUID(0), AItimer1(0.0f), AItimer2(0.0f),
		AIfvar1(0.0f), AIfvar2(0.0f), AIfvar3(0.0f),AIvar1(0), 
		AIvar2(0), AIvec1( {0.0f, 0.0f} ), AIvarBool1(false), AIvarBool2(false)
	{}
};
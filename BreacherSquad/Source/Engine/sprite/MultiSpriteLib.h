#pragma once
/************************************************************************/
/* Loads many sprite libraries into a single place.
/************************************************************************/
class CMultiSpriteLib : public IDeviceRes
{
private:
	struct SpriteLibInstance 
	{
		CStringHash					shID;
		CSpriteLib					spriteLib;
	};

public: 
	CArray<SpriteLibInstance*>		arrLibs;

public:
	CMultiSpriteLib();
	~CMultiSpriteLib();
	// Adds a new sprite
	OPRESULT						AddSprites( WCHAR* wcsFullPath, int & retLibIdx );
	// Releases all sprite libs
	void							Release();
	// Returns index of library or -1 if not found. strLibId is the lib path
	int								GetLibIndex( WCHAR* wcsFullPath );
	CSpriteLib*						GetLib( int nLibIdx );


public:
	OPRESULT OnCreateDevice( PDEVICE pDevice, const SURFACE_DESC* pBBDesc = nullptr ) override;
	OPRESULT OnResetDevice( PDEVICE pDevice, const SURFACE_DESC* pBBDesc = nullptr ) override;
	OPRESULT OnLostDevice() override;
	OPRESULT OnDestroyDevice() override;
};

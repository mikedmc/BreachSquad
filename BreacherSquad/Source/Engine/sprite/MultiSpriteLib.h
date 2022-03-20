#pragma once
/************************************************************************/
/* Loads many sprite libraries into a single place.
/************************************************************************/
class CMultiSpriteLib : public IDeviceRes
{
private:
	struct SpriteLibInstance 
	{
		CStringHash					shNickname;				// Nickname can be used instead of lib index so we don't store lib index if not necessary
		CStringHash					shFullPath;
		CSpriteLib					spriteLib;
	};

public: 
	CArray<SpriteLibInstance*>		arrLibs;

public:
	CMultiSpriteLib();
	~CMultiSpriteLib();
	// Adds a new sprite
	OPRESULT						AddSprites( WCHAR* wcsFullPath, int & retLibIdx, WCHAR* nickname = nullptr );
	// Releases all sprite libs
	void							Release();
	// Returns index of library or -1 if not found. wcsFullPath is the lib full path.
	int								GetLibIndex( WCHAR* wcsFullPath );
	// Returns index of library or -1 if not found. Searches by nickname given on loading.
	int								GetLibIndexByNick( WCHAR* nickname );
	CSpriteLib*						GetLib( int nLibIdx );


public:
	OPRESULT OnCreateDevice( PDEVICE pDevice, const SURFACE_DESC* pBBDesc = nullptr ) override;
	OPRESULT OnResetDevice( PDEVICE pDevice, const SURFACE_DESC* pBBDesc = nullptr ) override;
	OPRESULT OnLostDevice() override;
	OPRESULT OnDestroyDevice() override;
};

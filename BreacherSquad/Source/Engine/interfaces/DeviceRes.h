#pragma once

// All classes that keep device connected resources (textures and meshes) must implement the callbacks below
class IDeviceRes {
protected:
	PDEVICE				m_pDevice;		// pointer to current device gets saved here
public:
	IDeviceRes() {
		m_pDevice = nullptr;
	};

	virtual OPRESULT OnCreateDevice( PDEVICE pDevice, const SURFACE_DESC* pBBDesc = nullptr) = 0;
	virtual OPRESULT OnResetDevice( PDEVICE pDevice, const SURFACE_DESC* pBBDesc = nullptr) = 0;
	virtual OPRESULT OnLostDevice() = 0;
	virtual OPRESULT OnDestroyDevice() = 0;
};


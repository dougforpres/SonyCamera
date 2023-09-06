#pragma once
#include "pch.h"
#include <PortableDeviceApi.h>
#include "Camera.h"

class DeviceEventHandler : public IPortableDeviceEventCallback
{
public:
    DeviceEventHandler(IPortableDevice* device, Camera* camera);
    ~DeviceEventHandler();

    HRESULT __stdcall QueryInterface(REFIID  riid, LPVOID* ppvObj);
    ULONG __stdcall AddRef();
    ULONG __stdcall Release();
    HRESULT __stdcall OnEvent(IPortableDeviceValues* pEventParameters);

private:
    unsigned long m_cRef = 0;
    IPortableDevice* m_device = nullptr;
    Camera* m_camera;
};


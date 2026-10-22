#pragma once
#include "pch.h"
#include <PortableDeviceApi.h>
#include <cfgmgr32.h>
#include "Camera.h"

// COM callback for WPD device events.
//
// Lifetime: the object is reference counted.  The creator owns the initial
// reference and WPD takes its own reference(s) when Register() calls Advise().
// WPD dispatches events on its own worker thread and may still do so after
// Unregister() returns (or if Unadvise() fails because the device is already
// gone), so the owner must call Unregister() and then Release() - never delete.
//
// Unregister() does not block (it may run on the camera worker thread, which an
// in-flight event can be waiting on).  Call WaitForCallbacks() before the Camera
// passed to Register() is deleted.
//
// WPD only reports removal the next time the driver talks to the device, which can be
// the end of a long exposure, so removal is also watched through Plug and Play, which
// reports it as soon as the cable is pulled.
class DeviceEventHandler : public IPortableDeviceEventCallback
{
public:
    DeviceEventHandler();

    HRESULT Register(IPortableDevice* device, Camera* camera, const std::wstring& deviceId);
    void Unregister();
    bool WaitForCallbacks(DWORD timeout);

    HRESULT __stdcall QueryInterface(REFIID  riid, LPVOID* ppvObj);
    ULONG __stdcall AddRef();
    ULONG __stdcall Release();
    HRESULT __stdcall OnEvent(IPortableDeviceValues* pEventParameters);

private:
    ~DeviceEventHandler();

    Camera* AcquireCamera();
    void ReleaseCamera();
    void NotifyDeviceRemoved();
    static DWORD CALLBACK OnPnpNotification(HCMNOTIFICATION notification, PVOID context, CM_NOTIFY_ACTION action, PCM_NOTIFY_EVENT_DATA eventData, DWORD eventDataSize);

    HCMNOTIFICATION m_pnpNotification = nullptr;
    std::wstring m_deviceId;

    volatile long m_cRef = 1;
    volatile long m_inFlight = 0;
    SRWLOCK m_cameraLock = SRWLOCK_INIT;
    IPortableDevice* m_device = nullptr;
    Camera* m_camera = nullptr;
    std::wstring m_cookie;
};

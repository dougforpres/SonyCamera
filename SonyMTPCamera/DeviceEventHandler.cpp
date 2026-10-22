#include "pch.h"
#include "DeviceEventHandler.h"
#include <PortableDevice.h>
#include <PortableDeviceTypes.h>
#include <WpdMtpExtensions.h>
#include "Logger.h"
#include <wtypes.h>
#include <objbase.h>
#include "ResourceLoader.h"

#define DUMP_EVENT_PARAMS

constexpr auto CALLBACK_WAIT_SLEEP = 10;

// GUID_DEVINTERFACE_WPD - the interface class our device ids (symbolic links) belong to
static const GUID WPD_DEVICE_INTERFACE_CLASS = { 0x6ac27878, 0xa6fa, 0x4155, { 0xba, 0x85, 0xf9, 0x8f, 0x49, 0x1d, 0x4f, 0x33 } };

DeviceEventHandler::DeviceEventHandler()
{
    LOGINFO(L"DeviceEventHandler::DeviceEventHandler x%p", (void*)this);
}

DeviceEventHandler::~DeviceEventHandler()
{
    LOGINFO(L"DeviceEventHandler::~DeviceEventHandler x%p", (void*)this);

    // Unregister() should already have been called by the owner, this is a safety net
    if (m_device)
    {
        m_device->Release();
        m_device = nullptr;
    }
}

HRESULT
DeviceEventHandler::Register(IPortableDevice* device, Camera* camera, const std::wstring& deviceId)
{
    LOGINFO(L"DeviceEventHandler::Register with device x%p, camera x%p", (void*)device, (void*)camera);
    LPWSTR wszEventCookie = nullptr;

    if (device == nullptr)
    {
        return E_POINTER;
    }

    if (m_device)
    {
        LOGWARN(L"DeviceEventHandler x%p already registered with cookie '%ws'", (void*)this, m_cookie.c_str());

        return S_OK;
    }

    // Set camera before Advise, as WPD can deliver an event before Advise returns
    AcquireSRWLockExclusive(&m_cameraLock);
    m_camera = camera;
    ReleaseSRWLockExclusive(&m_cameraLock);

    HRESULT hr = device->Advise(0, this, nullptr, &wszEventCookie);

    if (SUCCEEDED(hr) && wszEventCookie != nullptr)
    {
        m_cookie = wszEventCookie;
        m_device = device;
        m_device->AddRef();

        LOGINFO(L"Registered for device event notifications, cookie '%ws'", m_cookie.c_str());

        // Best effort: without it removal is still reported by WPD, just later
        m_deviceId = deviceId;

        CM_NOTIFY_FILTER filter = {};
        filter.cbSize = sizeof(filter);
        filter.FilterType = CM_NOTIFY_FILTER_TYPE_DEVICEINTERFACE;
        filter.u.DeviceInterface.ClassGuid = WPD_DEVICE_INTERFACE_CLASS;

        CONFIGRET cr = CM_Register_Notification(&filter, this, OnPnpNotification, &m_pnpNotification);

        if (cr != CR_SUCCESS)
        {
            LOGWARN(L"Unable to register for Plug and Play removal notifications, cr = %d", cr);
            m_pnpNotification = nullptr;
        }
    }
    else
    {
        AcquireSRWLockExclusive(&m_cameraLock);
        m_camera = nullptr;
        ReleaseSRWLockExclusive(&m_cameraLock);

        LOGERROR(L"! Failed to register for device events, hr = 0x%lx", hr);

        if (SUCCEEDED(hr))
        {
            hr = E_UNEXPECTED;
        }
    }

    if (wszEventCookie != nullptr)
    {
        CoTaskMemFree(wszEventCookie);
    }

    return hr;
}

void
DeviceEventHandler::Unregister()
{
    // From here on, events WPD is still dispatching (or dispatches later, if Unadvise
    // fails) will not reach the camera.  Events already inside the camera are not
    // waited for here - see WaitForCallbacks().
    AcquireSRWLockExclusive(&m_cameraLock);
    m_camera = nullptr;
    ReleaseSRWLockExclusive(&m_cameraLock);

    if (m_pnpNotification)
    {
        // Waits for a callback in progress; those only take short, bounded camera locks
        CM_Unregister_Notification(m_pnpNotification);
        m_pnpNotification = nullptr;
    }

    if (m_device == nullptr)
    {
        return;
    }

    HRESULT hr = m_device->Unadvise(m_cookie.c_str());

    if (SUCCEEDED(hr))
    {
        LOGINFO(L"Unregistered from device event notifications, cookie '%ws'", m_cookie.c_str());
    }
    else
    {
        // Typically the device has already been removed.  WPD may keep its reference to us,
        // which is safe because we are reference counted and no longer have a camera.
        LOGERROR(L"! Failed to unregister for device events using registration cookie '%ws', hr = 0x%lx", m_cookie.c_str(), hr);
    }

    m_cookie.clear();
    m_device->Release();
    m_device = nullptr;
}

bool
DeviceEventHandler::WaitForCallbacks(DWORD timeout)
{
    ULONGLONG start = GetTickCount64();

    while (InterlockedCompareExchange(&m_inFlight, 0, 0) != 0)
    {
        if (GetTickCount64() - start >= timeout)
        {
            LOGERROR(L"DeviceEventHandler x%p - timed out waiting for %d event callback(s) to finish", (void*)this, m_inFlight);

            return false;
        }

        Sleep(CALLBACK_WAIT_SLEEP);
    }

    return true;
}

Camera*
DeviceEventHandler::AcquireCamera()
{
    // Count the callback while holding the lock, so Unregister + WaitForCallbacks
    // cannot miss a callback that picked up the camera just before it was cleared
    AcquireSRWLockShared(&m_cameraLock);

    Camera* camera = m_camera;

    if (camera)
    {
        InterlockedIncrement(&m_inFlight);
    }

    ReleaseSRWLockShared(&m_cameraLock);

    return camera;
}

void
DeviceEventHandler::ReleaseCamera()
{
    InterlockedDecrement(&m_inFlight);
}

void
DeviceEventHandler::NotifyDeviceRemoved()
{
    Camera* camera = AcquireCamera();

    if (camera)
    {
        camera->OnDeviceRemoved();
        ReleaseCamera();
    }
}

DWORD CALLBACK
DeviceEventHandler::OnPnpNotification(HCMNOTIFICATION notification, PVOID context, CM_NOTIFY_ACTION action, PCM_NOTIFY_EVENT_DATA eventData, DWORD eventDataSize)
{
    DeviceEventHandler* handler = (DeviceEventHandler*)context;

    if (action == CM_NOTIFY_ACTION_DEVICEINTERFACEREMOVAL && handler && eventData &&
        _wcsicmp(eventData->u.DeviceInterface.SymbolicLink, handler->m_deviceId.c_str()) == 0)
    {
        LOGINFO(L"Plug and Play reports the device was removed");
        handler->NotifyDeviceRemoved();
    }

    return ERROR_SUCCESS;
}

HRESULT __stdcall
DeviceEventHandler::QueryInterface(REFIID  riid, LPVOID* ppvObj)
{
    HRESULT hr = S_OK;

    if (ppvObj == NULL)
    {
        hr = E_INVALIDARG;
        return hr;
    }

    if ((riid == IID_IUnknown) ||
        (riid == IID_IPortableDeviceEventCallback))
    {
        AddRef();
        *ppvObj = this;
    }
    else
    {
        hr = E_NOINTERFACE;
    }

    return hr;
}

ULONG __stdcall
DeviceEventHandler::AddRef()
{
    return (ULONG)InterlockedIncrement(&m_cRef);
}

ULONG __stdcall
DeviceEventHandler::Release()
{
    long refCount = InterlockedDecrement(&m_cRef);

    if (refCount == 0)
    {
        LOGINFO(L"DeviceEventHandler::Release() - refcount is 0, deleting");
        delete this;
    }

    return (ULONG)refCount;
}

HRESULT __stdcall
DeviceEventHandler::OnEvent(IPortableDeviceValues* pEventParameters)
{
    LOGTRACE(L"DeviceEventHandler::OnEvent() - in");

    if (pEventParameters != NULL)
    {
        LPWSTR pStrValue = nullptr;
        GUID guid = GUID_NULL;
        RPC_WSTR guidString = nullptr;

        if (SUCCEEDED(pEventParameters->GetStringValue(WPD_EVENT_PARAMETER_PNP_DEVICE_ID, &pStrValue)) && pStrValue != nullptr)
        {
            LOGTRACE(L"New event from PNP Device ID: %s", pStrValue);
            CoTaskMemFree(pStrValue);
            pStrValue = nullptr;
        }
        else
        {
            LOGTRACE(L"New event from unknown PNP Device ID");
        }

        // May be pointless, as the event-type always seems to be 0x0000 in USB packet
        if (FAILED(pEventParameters->GetGuidValue(WPD_EVENT_PARAMETER_EVENT_ID, &guid)))
        {
            LOGTRACE(L"Event Type: unavailable");
        }
        else if (IsEqualGUID(guid, WPD_EVENT_NOTIFICATION))
        {
            LOGTRACE(L"Event Type: Event Notification");
        }
        else if (IsEqualGUID(guid, WPD_EVENT_OBJECT_ADDED))
        {
            LOGTRACE(L"Event Type: Object Added");
        }
        else if (IsEqualGUID(guid, WPD_EVENT_OBJECT_REMOVED))
        {
            LOGTRACE(L"Event Type: Object Removed");
        }
        else if (IsEqualGUID(guid, WPD_EVENT_OBJECT_UPDATED))
        {
            LOGTRACE(L"Event Type: Object Updated");
        }
        else if (IsEqualGUID(guid, WPD_EVENT_DEVICE_RESET))
        {
            LOGTRACE(L"Event Type: Device Reset");
        }
        else if (IsEqualGUID(guid, WPD_EVENT_DEVICE_CAPABILITIES_UPDATED))
        {
            LOGTRACE(L"Event Type: Device Capabilities Updated");
        }
        else if (IsEqualGUID(guid, WPD_EVENT_STORAGE_FORMAT))
        {
            LOGTRACE(L"Event Type: Storage Format");
        }
        else if (IsEqualGUID(guid, WPD_EVENT_OBJECT_TRANSFER_REQUESTED))
        {
            LOGTRACE(L"Event Type: Object Transfer Requested");
        }
        else if (IsEqualGUID(guid, WPD_EVENT_DEVICE_REMOVED))
        {
            LOGTRACE(L"Event Type: Device Removed");
            NotifyDeviceRemoved();
        }
        else if (IsEqualGUID(guid, WPD_EVENT_SERVICE_METHOD_COMPLETE))
        {
            LOGTRACE(L"Event Type: Service Method Complete");
        }
        else
        {
            // See if it's a Sony event
            // 1st two bytes
            DWORD sonyEventId = guid.Data1;

            if ((sonyEventId & 0xc2000000) == 0xc2000000 )
            {
                DWORD sonyId = sonyEventId >> 16;
                Camera* camera = AcquireCamera();

                if (camera == nullptr)
                {
                    LOGTRACE(L"Sony event 0x%04x ignored, handler is no longer registered", sonyId);
                }
                else
                {
                    switch (sonyId)
                    {
                    case 0xc201:
                        LOGTRACE(L"0xc201 - photo(s) available");
                        camera->OnImageBufferStatus(ImageBufferStatus::ImageReady);
                        break;

                    case 0xc202:
                        LOGTRACE(L"0xc202 - no more photo(s) available");
                        camera->OnImageBufferStatus(ImageBufferStatus::ImageNotReady);
                        break;

                    case 0xc203:
                        LOGTRACE(L"0xc203 - property updated");
                        camera->OnPropertiesUpdated();
                        break;

                    default:
                        LOGTRACE(L"Unknown sony event: 0x%04x", sonyId);
                        break;
                    }

                    ReleaseCamera();
                }
            }
            else if (UuidToString(&guid, &guidString) == RPC_S_OK)
            {
                LOGTRACE(L"Event Type: Unknown - %s 0x%08x", guidString, sonyEventId);
                RpcStringFree(&guidString);
            }
            else
            {
                LOGTRACE(L"Event Type: Unknown - 0x%08x", sonyEventId);
            }
        }

#ifdef DUMP_EVENT_PARAMS
        // Not every event has MTP parameters (e.g. Device Removed), so only touch the
        // collection when it was actually returned
        IPortableDevicePropVariantCollection* pdpvc = nullptr;

        if (SUCCEEDED(pEventParameters->GetIPortableDevicePropVariantCollectionValue(WPD_PROPERTY_MTP_EXT_EVENT_PARAMS, &pdpvc)) && pdpvc != nullptr)
        {
            DWORD count = 0;
            pdpvc->GetCount(&count);

            for (DWORD i = 0; i < count; i++)
            {
                LOGTRACE(L"--- Property %d", i);
                PROPVARIANT val;

                PropVariantInit(&val);

                if (SUCCEEDED(pdpvc->GetAt(i, &val)))
                {
                    if (val.vt == VT_UI4)
                    {
                        LOGTRACE(L"    UI4 => x%08x (%s)", val.ulVal, ResourceLoader::GetString(val.ulVal).c_str());
                    }
                    else
                    {
                        LOGTRACE(L"    type = %d", val.vt);
                    }
                }

                PropVariantClear(&val);
            }

            pdpvc->Release();
        }
#endif
    }

    LOGTRACE(L"DeviceEventHandler::OnEvent() - out");

    return S_OK;
}

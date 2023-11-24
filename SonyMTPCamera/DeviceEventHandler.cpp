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

std::wstring g_strEventRegistrationCookie;

DeviceEventHandler::DeviceEventHandler(IPortableDevice* device, Camera* camera)
    : m_device(device),
      m_camera(camera)
{
    LOGINFO(L"DeviceEventHandler::DeviceEventHandler with x%p", (void*)device);
    HRESULT                        hr = S_OK;
    LPWSTR                         wszEventCookie = NULL;

    if (device == NULL)
    {
        return;
    }

    // Check to see if we already have an event registration cookie.  If so,
    // then avoid registering again.
    // NOTE: An application can register for events as many times as they want.
    //       This sample only keeps a single registration cookie around for
    //       simplicity.
    if (!g_strEventRegistrationCookie.empty())
    {
        LOGERROR(L"This application has already registered to receive device events.");

        return;
    }

    // Call Advise to register the callback and receive events.
    if (hr == S_OK)
    {
        hr = device->Advise(0, this, NULL, &wszEventCookie);

        if (FAILED(hr))
        {
            LOGERROR(L"! Failed to register for device events, hr = 0x%lx", hr);
        }
    }

    // Save the event registration cookie if event registration was successful.
    if (hr == S_OK)
    {
        g_strEventRegistrationCookie = wszEventCookie;
    }

    // Free the event registration cookie, if one was returned.
    if (wszEventCookie != NULL)
    {
        CoTaskMemFree(wszEventCookie);
        wszEventCookie = NULL;
    }

    if (hr == S_OK)
    {
        LOGINFO(L"This application has registered for device event notifications and was returned the registration cookie '%ws'", g_strEventRegistrationCookie.c_str());
    }

    // If a failure occurs, remember to delete the allocated callback object, if one exists.
    //if (pCallback != NULL)
    //{
    //    this->Release();
    //}
}

DeviceEventHandler::~DeviceEventHandler()
{
    LOGINFO(L"DeviceEventHandler::~DeviceEventHandler()");
    HRESULT hr = S_OK;

    if (m_device == NULL)
    {
        return;
    }

    hr = m_device->Unadvise(g_strEventRegistrationCookie.c_str());

    if (FAILED(hr))
    {
        LOGERROR(L"! Failed to unregister for device events using registration cookie '%ws', hr = 0x%lx", g_strEventRegistrationCookie.c_str(), hr);
    }

    if (hr == S_OK)
    {
        LOGINFO(L"This application used the registration cookie '%ws' to unregister from receiving device event notifications", g_strEventRegistrationCookie.c_str());
    }

    g_strEventRegistrationCookie = L"";
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
//    LOGTRACE(L"DeviceEventHandler::AddRef()");

    InterlockedIncrement((long*)&m_cRef);

//    LOGTRACE(L"DeviceEventHandler::AddRef() - ref count now %d", m_cRef);

    return m_cRef;
}

ULONG __stdcall 
DeviceEventHandler::Release()
{
//    LOGTRACE(L"DeviceEventHandler::Release()");
    ULONG ulRefCount = m_cRef - 1;

    if (InterlockedDecrement((long*)&m_cRef) == 0)
    {
//        delete this;
        LOGINFO(L"DeviceEventHandler::Release() - refcount is 0");

        return 0;
    }

//    LOGTRACE(L"DeviceEventHandler::Release() - refcount is now %d", m_cRef);

    return ulRefCount;
}

HRESULT __stdcall 
DeviceEventHandler::OnEvent(IPortableDeviceValues* pEventParameters)
{
    LOGTRACE(L"DeviceEventHandler::OnEvent() - in");
    HRESULT hr = S_OK;

    if (pEventParameters != NULL)
    {
        LPWSTR pStrValue = nullptr;
        GUID guid;
        RPC_WSTR guidString;

        pEventParameters->GetStringValue(WPD_EVENT_PARAMETER_PNP_DEVICE_ID, &pStrValue);
        LOGTRACE(L"New event from PNP Device ID: %s", pStrValue);

        // May be pointless, as the event-type always seems to be 0x0000 in USB packet
        pEventParameters->GetGuidValue(WPD_EVENT_PARAMETER_EVENT_ID, &guid);

        if (IsEqualGUID(guid, WPD_EVENT_NOTIFICATION))
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

                switch (sonyId)
                {
                case 0xc201:
                    LOGTRACE(L"0xc201 - photo(s) available");
                    m_camera->OnImageBufferStatus(ImageBufferStatus::ImageReady);
                    break;

                case 0xc202:
                    LOGTRACE(L"0xc202 - no more photo(s) available");
                    m_camera->OnImageBufferStatus(ImageBufferStatus::ImageNotReady);
                    break;

                case 0xc203:
                    LOGTRACE(L"0xc203 - property updated");
                    m_camera->OnPropertiesUpdated();
                    break;

                default:
                    LOGTRACE(L"Unknown sony event: 0x%04x", sonyId);
                    break;
                }
            }
            else {
                UuidToString(&guid, &guidString);
                LOGTRACE(L"Event Type: Unknown - %s 0x%08x", guidString, sonyEventId);
                RpcStringFree(&guidString);
            }
        }

#ifdef DUMP_EVENT_PARAMS
        IPortableDevicePropVariantCollection* pdpvc;

        hr = pEventParameters->GetIPortableDevicePropVariantCollectionValue(WPD_PROPERTY_MTP_EXT_EVENT_PARAMS, &pdpvc);

        if (SUCCEEDED(hr))
        {
            pdpvc->AddRef();
            DWORD count = 0;
            pdpvc->GetCount(&count);

            for (int i = 0; i < (int)count; i++)
            {
                LOGTRACE(L"--- Property %d", i);
                PROPVARIANT val;

                pdpvc->GetAt(i, &val);

                if (val.vt == VT_UI4)
                {
                    LOGTRACE(L"    UI4 => x%08x (%s)", val.uiVal, ResourceLoader::GetString(val.uiVal).c_str());
                }
                else
                {
                    LOGTRACE(L"    type = %d", val.vt);
                }
            }
        }

        pdpvc->Release();
#endif
    }

    LOGTRACE(L"DeviceEventHandler::OnEvent() - out, returning %08x", hr);

    return hr;
}
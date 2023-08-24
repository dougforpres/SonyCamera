#include "pch.h"
#include "SonyCamera.h"
#include "Logger.h"
#include "CameraException.h"
#include "Locker.h"

#define SONY_GET_NEXT_HANDLE_ENABLE
#define SONY_GET_NEXT_HANDLE_EXTRA_PARAMS

SonyCamera::SonyCamera(Device* device)
    : Camera(device)
{
    LOGTRACE(L"In: SonyCamera::SonyCamera");
    LOGTRACE(L"Out: SonyCamera::SonyCamera");
}

SonyCamera::~SonyCamera()
{
    LOGTRACE(L"In: SonyCamera::~SonyCamera");
    LOGTRACE(L"Out: SonyCamera::~SonyCamera");
}

bool
SonyCamera::Initialize()
{
    LOGTRACE(L"In: SonyCamera::Initialize()");
    bool result = true;
    Message* tx;
    Message* rx;

    if (GetDevice()->NeedsSession())
    {
        tx = new Message(COMMAND_OPEN_SESSION);

        tx->AddParam(0x00000001);

        this->m_device->Send(tx);

        delete tx;
    }

    tx = new Message(COMMAND_GET_STORAGE_IDS);

    rx = m_device->Receive(tx);
    //  Ignore result of this method, a7siii (one sample) seems to return a non-success result
    //    result &= rx->IsSuccess();

    LOGTRACE(L"Get Storage ID's >> %s", rx->Dump().c_str());

    delete tx;
    delete rx;

#ifdef SONY_GET_NEXT_HANDLE_ENABLE
    tx = new Message(COMMAND_SONY_GET_NEXT_HANDLE);

    tx->AddParam(0x00000001);
#ifdef SONY_GET_NEXT_HANDLE_EXTRA_PARAMS
    tx->AddParam(0x00000000);  // Test only sending one param (which is what MTP spec says it should be)
    tx->AddParam(0x00000000);
#endif

    rx = m_device->Receive(tx);
    result &= rx->IsSuccess();

    LOGTRACE(L"Get Next Handle (1) >> %s", rx->Dump().c_str());

    delete tx;
    delete rx;

    tx = new Message(COMMAND_SONY_GET_NEXT_HANDLE);

    tx->AddParam(0x00000002);
#ifdef SONY_GET_NEXT_HANDLE_EXTRA_PARAMS
    tx->AddParam(0x00000000);  // Test only sending one param (which is what MTP spec says it should be)
    tx->AddParam(0x00000000);
#endif

    rx = m_device->Receive(tx);
    result &= rx->IsSuccess();

    LOGTRACE(L"Get Next Handle (2) >> %s", rx->Dump().c_str());

    delete tx;
    delete rx;
#endif

    tx = new Message(COMMAND_SONY_GET_PROPERTY_LIST);

    tx->AddParam(0x000000c8);

    rx = m_device->Receive(tx);
    result &= rx->IsSuccess();

    m_supportedProperties = new CameraSupportedProperties(rx);
    LOGTRACE(L"Get Supported Properties (1) >> %s", m_supportedProperties->AsString().c_str());

    delete tx;
    delete rx;

    tx = new Message(COMMAND_SONY_GET_NEXT_HANDLE);

    tx->AddParam(0x00000003);
    tx->AddParam(0x00000000);
    tx->AddParam(0x00000000);

    rx = m_device->Receive(tx);
    result &= rx->IsSuccess();

    LOGTRACE(L"Get Next Handle (3) >> %s", rx->Dump().c_str());

    delete tx;
    delete rx;

    tx = new Message(COMMAND_SONY_GET_PROPERTY_LIST);

    tx->AddParam(0x000000c8);

    rx = m_device->Receive(tx);
    result &= rx->IsSuccess();

    LOGTRACE(L"Get Supported Properties (2) >> %s", rx->Dump().c_str());

    delete tx;
    delete rx;

    GetSettings(true);

    LOGTRACE(L"Out: SonyCamera::Initialize() - result %d", result);

    return result;
}

CameraSettings*
SonyCamera::GetSettings(bool refresh)
{
    LOGTRACE(L"In: SonyCamera::GetSettings(%s)", refresh ? L"true": L"false");

    if (refresh)
    {
        Message* tx;
        Message* rx;

        tx = new Message(COMMAND_READ_SETTINGS);
        rx = m_device->Receive(tx);

        if (rx->IsSuccess())
        {
            Locker lock(settingsLock);

            CameraSettings* cs = new CameraSettings(rx);

            delete tx;
            delete rx;

            if (m_settings)
            {
                delete m_settings;
            }

            m_settings = cs;

            LoadFakeProperties(m_settings);
        }
        else
        {
            throw CameraException(L"Error reading settings from camera");
        }
    }

    LOGTRACE(L"Out: SonyCamera::GetSettings()");

    return m_settings;
}



bool
SonyCamera::SetProperty(Property id, PropertyValue* value)
{
    LOGTRACE(L"In: SonyCamera::SetProperty(x%08x, %s)", (int)id, value->ToString().c_str());

    bool result = false;

    Message* tx;

    // Depending on the property type, we need to send different command
    CameraProperty* prop = GetSettings(false)->GetProperty(id);

    if (prop)
    {
        Locker lock(settingsLock);
        DWORD messageType;
        PropertyInfo* info = prop->GetInfo();

        if (info->GetAccess() == Accessibility::READ_WRITE)
        {
            messageType = COMMAND_SONY_SET_PROPERTY;
        }
        else if (info->GetSonySpare() != 0)
        {
            messageType = COMMAND_SONY_SET_PROPERTY2;
        }
        else
        {
            throw CameraException(L"Property is not writable");
        }

        tx = new Message(messageType);

        tx->AddParam((DWORD)id);

        size_t size = value->GetDataSize();

        if (size & 1) {
            size++;
        }

        BYTE* txdata = new BYTE[size];

        memset(txdata, 0, size);

        value->Write(txdata, size);
        tx->SetData(txdata, size);

        if (m_device->Send(tx))
        {
            result = true;
        }

        delete[] txdata;
        delete tx;
    }

    LOGTRACE(L"Out: SonyCamera::SetProperty(x%08x, %s), - result = %d", (int)id, value->ToString().c_str(), result);

    return result;
}
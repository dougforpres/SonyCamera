#include "pch.h"
#include "SonyCamera.h"
#include "Logger.h"
#include "CameraException.h"

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

/*    this->GetDeviceInfo();
    tx = new Message(COMMAND_GET_DEVICE_INFO);
        rx = this->_device->Receive(tx);

        wprintf(L"Get Device Info:\n%s", rx->Dump().c_str());

        this->_deviceInfo = new DeviceInfo(rx);

        delete tx;
        delete rx;

    tx = new Message(COMMAND_OPEN_SESSION);
    this->_device->Send(tx);

    delete tx;
*/
    tx = new Message(COMMAND_GET_STORAGE_IDS);
    rx = m_device->Receive(tx);
    result &= rx->IsSuccess();

    LOGTRACE(L"Get Storage ID's >> %s", rx->Dump().c_str());
    //    LOGINFO(L"Asked for storage Id's\n%s", rx->Dump().c_str());

    delete tx;
    delete rx;

    tx = new Message(COMMAND_SONY_GET_NEXT_HANDLE);

    tx->AddParam(0x00000001);
    tx->AddParam(0x00000000);
    tx->AddParam(0x00000000);

    rx = m_device->Receive(tx);
    result &= rx->IsSuccess();

    delete tx;
    delete rx;

    tx = new Message(COMMAND_SONY_GET_NEXT_HANDLE);

    tx->AddParam(0x00000002);
    tx->AddParam(0x00000000);
    tx->AddParam(0x00000000);

    rx = m_device->Receive(tx);
    result &= rx->IsSuccess();

    delete tx;
    delete rx;

    tx = new Message(COMMAND_SONY_GET_PROPERTY_LIST);

    tx->AddParam(0x000000c8);

    rx = m_device->Receive(tx);
    result &= rx->IsSuccess();

    m_supportedProperties = new CameraSupportedProperties(rx);
    LOGTRACE(L"Get Supported Properties >> %s", m_supportedProperties->AsString().c_str());

    delete tx;
    delete rx;

    tx = new Message(COMMAND_SONY_GET_NEXT_HANDLE);

    tx->AddParam(0x00000003);
    tx->AddParam(0x00000000);
    tx->AddParam(0x00000000);

    rx = m_device->Receive(tx);
    result &= rx->IsSuccess();

    delete tx;
    delete rx;

    tx = new Message(COMMAND_SONY_GET_PROPERTY_LIST);

    tx->AddParam(0x000000c8);

    rx = m_device->Receive(tx);
    result &= rx->IsSuccess();

    LOGTRACE(L"Get Supported Properties >> %s", rx->Dump().c_str());

    delete tx;
    delete rx;

    GetSettings(true);

    LOGTRACE(L"Out: SonyCamera::Initialize() - result %d", result);

    return result;
}

CameraSettings*
SonyCamera::GetSettings(bool refresh)
{
    LOGTRACE(L"In: SonyCamera::GetSettings()");

    if (refresh)
    {
        Message* tx;
        Message* rx;

        tx = new Message(COMMAND_READ_SETTINGS);
        tx->AddParam(0x025c);
        tx->AddParam(0x0000);
        rx = m_device->Receive(tx);

        if (rx->IsSuccess())
        {
            CameraSettings* cs = new CameraSettings(rx);

            delete tx;
            delete rx;

            if (m_settings)
            {
                delete m_settings;
                m_settings = nullptr;
            }

            m_settings = cs;

            if (cs && m_propertyInfos.empty())
            {
                std::list<CameraProperty*> properties = cs->GetProperties();

                for (std::list<CameraProperty*>::iterator it = properties.begin(); it != properties.end(); it++)
                {
                    m_propertyInfos.insert(std::pair<Property, PropertyInfo*>((*it)->GetId(), new PropertyInfo(*(*it)->GetInfo())));
                    LOGDEBUG(L"Added new property: %s", (*it)->ToString().c_str());
                }
            }
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
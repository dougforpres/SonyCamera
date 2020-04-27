#pragma once
#include "pch.h"
#include <PortableDeviceApi.h>
#include "DeviceEventHandler.h"

#include "Message.h"

#define CLIENT_NAME L"Retro.kiwi Sony Camera"
#define CLIENT_MAJOR_VER 0
#define CLIENT_MINOR_VER 1
#define CLIENT_REVISION 1

class Device
{
public:
    enum class Op
    {
        CommandOnly,
        SendData,
        ReceiveData
    };

    /* Constructor */
    Device(IPortableDeviceManager* PortableDeviceManager, std::wstring DeviceId);
    Device(const Device &rhs);

    /* Destructor */
    ~Device();

    virtual Device* Clone();

    virtual HANDLE Open();
    virtual bool Close();

    virtual std::wstring GetId();
    virtual std::wstring GetFriendlyName();
    virtual std::wstring GetManufacturer();
    virtual std::wstring GetDescription();

    virtual bool Send(Message* out);
    virtual Message* Receive(Message* out);

    virtual HANDLE GetHandle();
    virtual bool StartNotifications();
    virtual bool StopNotifications();

protected:
    virtual Message* InternalSend(Op kind, Message* out);

    std::wstring m_id;
    std::wstring m_friendlyName;
    std::wstring m_manufacturer;
    std::wstring m_description;

private:
    void CreateClientInformation();
//    bool LogIfFailed(HRESULT hr, std::wstring message);
    bool IsSuccess(HRESULT hr, const wchar_t * message);

    HANDLE m_handle = INVALID_HANDLE_VALUE;
    short m_openCount = 0;
    IPortableDevice* m_device = nullptr;
    IPortableDeviceManager* m_manager = nullptr;
    IPortableDeviceValues* m_clientInformation = nullptr;
    DeviceEventHandler* m_eventHandler = nullptr;
};


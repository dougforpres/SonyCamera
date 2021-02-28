#pragma once
#include "pch.h"
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
    Device(std::wstring DeviceId);
    Device(const Device &rhs);

    /* Destructor */
    ~Device();

//    virtual Device* Clone() = 0;

    virtual HANDLE Open() = 0;
    virtual bool Close() = 0;

    virtual std::wstring GetId();
    virtual std::wstring GetFriendlyName();
    virtual std::wstring GetManufacturer();
    virtual std::wstring GetDescription();

    virtual bool Send(Message* out) = 0;
    virtual Message* Receive(Message* out) = 0;

    virtual HANDLE GetHandle();
    virtual bool StartNotifications();
    virtual bool StopNotifications();

    bool IsSupported();
    virtual std::wstring GetRegistryPath() = 0;
    virtual bool NeedsSession();

protected:
    std::wstring m_id;
    std::wstring m_friendlyName;
    std::wstring m_manufacturer;
    std::wstring m_description;
    HANDLE m_handle = INVALID_HANDLE_VALUE;
    short m_openCount = 0;

private:
};


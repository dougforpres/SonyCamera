#pragma once
#include "pch.h"
#include "Device.h"
#include <PortableDeviceApi.h>
#include "DeviceEventHandler.h"

class MTPDevice : public Device
{
public:
    /* Constructor */
    MTPDevice(IPortableDeviceManager* PortableDeviceManager, std::wstring DeviceId);
    MTPDevice(const MTPDevice& rhs);

    /* Destructor */
    ~MTPDevice();

//    virtual Device* Clone();

    virtual HANDLE Open();
    virtual bool Close();

    virtual bool Send(Message* out);
    virtual Message* Receive(Message* out);

    virtual bool StartNotifications(Camera* camera);
    virtual bool StopNotifications();
    virtual bool DrainNotifications(DWORD timeout);

    virtual std::wstring GetRegistryPath();

protected:
    virtual Message* InternalSend(Op kind, Message* out);

private:
    void InitManufacturer();
    void InitDescription();
    void InitFriendlyName();
    void CreateClientInformation();
    bool IsSuccess(HRESULT hr, const wchar_t* message);

    IPortableDevice* m_device = nullptr;
    IPortableDeviceManager* m_manager = nullptr;
    IPortableDeviceValues* m_clientInformation = nullptr;
    DeviceEventHandler* m_eventHandler = nullptr;
    DeviceEventHandler* m_retiredEventHandler = nullptr;
    // One MTP transaction (command, data phase, response) at a time: interleaving two
    // threads' transfers made the camera drop off USB.
    SRWLOCK m_transactionLock = SRWLOCK_INIT;
};

#pragma once
#include <list>
#include "LibUSBKEnumerator.h"

class DeviceManager
{
public:
    DeviceManager();
    ~DeviceManager();
    Device* GetDevice(std::wstring id);

    size_t RefreshDevices();
    std::list<Device*> GetAllDevices();
    std::list<Device*> GetFilteredDevices();

protected:

private:
    void ClearDeviceList();

    std::list<Device*> m_allDevices;

    LibUSBKEnumerator* m_libusbkEnumerator = nullptr;
};


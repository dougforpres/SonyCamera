#pragma once
#include <list>
#include "Device.h"

class DeviceManager
{
public:
    DeviceManager();
    ~DeviceManager();
    Device* GetDevice(std::wstring id);

    std::list<Device*> GetAllDevices();
    std::list<Device*> GetFilteredDevices();

private:
    void ClearDeviceList();

    std::list<Device*> m_allDevices;
};


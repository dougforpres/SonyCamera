#pragma once
#include "DeviceEnumerator.h"
#include "LibUSBKDevice.h"

class LibUSBKEnumerator : public DeviceEnumerator
{
public:
    LibUSBKEnumerator();
    ~LibUSBKEnumerator();

    std::list<Device*> EnumerateDevices();

private:
    KLST_HANDLE m_deviceList;
};
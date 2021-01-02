#pragma once
#include "pch.h"
#include "Device.h"
#include <list>

class DeviceEnumerator
{
public:
    DeviceEnumerator();
    ~DeviceEnumerator();

    virtual std::list<Device*>EnumerateDevices() = 0;
};
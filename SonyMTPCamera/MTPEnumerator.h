#pragma once
#include "pch.h"
#include "DeviceEnumerator.h"

class MTPEnumerator : public DeviceEnumerator
{
public:
    MTPEnumerator();
    ~MTPEnumerator();

    std::list<Device*>EnumerateDevices();
};
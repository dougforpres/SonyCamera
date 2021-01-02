#include "pch.h"
#include "Device.h"
#include "Logger.h"
#include "Registry.h"

Device::Device(std::wstring DeviceId)
  : m_id(DeviceId)
{
    LOGTRACE(L"In: Device::Device(devicdId='%s')", m_id.c_str());

    LOGTRACE(L"Out: Device::Device(deviceId='%s')", DeviceId.c_str());
}

Device::Device(const Device& rhs)
    : m_id(rhs.m_id),
      m_description(rhs.m_description),
      m_friendlyName(rhs.m_friendlyName),
      m_manufacturer(rhs.m_manufacturer)
{
    LOGTRACE(L"In: Device::Device[copy](deviceId='%s')", m_id.c_str());

    LOGTRACE(L"Out: Device::Device[copy](deviceId='%s')", m_id.c_str());
}

Device::~Device()
{
    LOGTRACE(L"In: Device::~Device()");

    LOGTRACE(L"Out: Device::~Device()");
}

HANDLE
Device::GetHandle()
{
    return m_handle;
}

std::wstring
Device::GetId()
{
    return m_id;
}

std::wstring
Device::GetFriendlyName()
{
    return m_friendlyName;
}

std::wstring
Device::GetDescription()
{
    return m_description;
}

std::wstring
Device::GetManufacturer()
{
    return m_manufacturer;
}

bool
Device::StopNotifications()
{
    // Stub, Override
    return false;
}

bool
Device::StartNotifications()
{
    // Stub, Override
    return false;
}

bool
Device::IsSupported()
{
    bool result = false;
    std::wostringstream builder;

    builder << L"Cameras\\" << GetRegistryPath();

    registry.Open();

    result = registry.DoesKeyExist(builder.str().c_str());

    registry.Close();

    LOGINFO(L"Device(%s)::IsSupported = %d", GetFriendlyName().c_str(), result);

    return result;
}

bool
Device::NeedsSession()
{
    return false;
}

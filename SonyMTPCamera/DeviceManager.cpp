#include "pch.h"
#include "DeviceManager.h"
#include "CameraManager.h"
#include "MTPEnumerator.h"
#include "LibUSBKEnumerator.h"
#include "Logger.h"
#include "DummyDevice.h"
#include "Registry.h"

DeviceManager::DeviceManager()
{
    m_libusbkEnumerator = new LibUSBKEnumerator();
}

DeviceManager::~DeviceManager()
{
    ClearDeviceList();
    delete m_libusbkEnumerator;
    m_libusbkEnumerator = nullptr;
}

void
DeviceManager::ClearDeviceList()
{
    while (!m_allDevices.empty())
    {
        delete m_allDevices.front();
        m_allDevices.pop_front();
    }
}

Device*
DeviceManager::GetDevice(std::wstring id)
{
    std::list<Device*> devices = GetAllDevices();

    Device* result = nullptr;

    for (std::list<Device*>::iterator it = devices.begin(); it != devices.end() && result == NULL; it++)
    {
        if ((*it)->GetId() == id)
        {
            result = *it;
        }
    }

    return result;
}

size_t
DeviceManager::RefreshDevices()
{
    MTPEnumerator* mtpEnumerator = new MTPEnumerator();

    std::list<Device*> foundDevices = mtpEnumerator->EnumerateDevices();
    foundDevices.splice(foundDevices.end(), m_libusbkEnumerator->EnumerateDevices());

    delete mtpEnumerator;

    mtpEnumerator = nullptr;

    // Add dummy device if there is a registry key for it
    registry.Open();

    if (registry.DoesKeyExist(L"Cameras\\retro.kiwi\\Test Camera"))
    {
        Device* device = new DummyDevice(L"test", L"Test Camera", L"retro.kiwi", L"Dummy Camera for Testing");

        LOGTRACE(L"Adding dummy device for testing");
        foundDevices.push_back(device);
    }

    registry.Close();

    std::list<Device*> newAllDevices = std::list<Device*>();

    for (std::list<Device*>::iterator newit = foundDevices.begin(); newit != foundDevices.end(); newit++)
    {
        // If the current item is already in "m_allDevices" then move the current one over
        // If it's not, then move the new one in
        // At the end we'll have a list of devices that have been removed
        bool found = false;
        std::list<Device*>::iterator it = m_allDevices.begin();

        while (it != m_allDevices.end() && !found)
        {
            if ((*it)->GetId() == (*newit)->GetId())
            {
                found = true;
                LOGINFO(L"Existing device '%s' being copied over", (*it)->GetId().c_str());
                newAllDevices.push_back(*it);
                it = m_allDevices.erase(it);
            }
            else
            {
                it++;
            }
        }

        if (!found)
        {
            LOGINFO(L"New device '%s' being added", (*newit)->GetId().c_str());
            newAllDevices.push_back(*newit);
        }
    }

    for (std::list<Device*>::iterator it = m_allDevices.begin(); it != m_allDevices.end(); it++)
    {
        delete* it;
    }

    m_allDevices = newAllDevices;

    return m_allDevices.size();
}

std::list<Device*>
DeviceManager::GetAllDevices()
{
    if (m_allDevices.empty())
    {
        RefreshDevices();
    }

    return m_allDevices;
}

std::list<Device*>
DeviceManager::GetFilteredDevices()
{
    std::list<Device*> allDevices = GetAllDevices();
    std::list<Device*> result;

    for (std::list<Device*>::iterator it = allDevices.begin(); it != allDevices.end(); it++)
    {
        if ((*it)->IsSupported())
        {
            result.push_back(*it);
        }
    }

    return result;
}
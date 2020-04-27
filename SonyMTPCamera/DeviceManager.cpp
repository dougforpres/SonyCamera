#include "pch.h"
#include "DeviceManager.h"
#include "CameraManager.h"
#include "Logger.h"
#include "DummyDevice.h"
#include "Registry.h"

DeviceManager::DeviceManager()
{
}

DeviceManager::~DeviceManager()
{
    ClearDeviceList();
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

std::list<Device*>
DeviceManager::GetAllDevices()
{
    HRESULT hr;
    IPortableDeviceManager* pPortableDeviceManager = nullptr;
    DWORD cPnPDeviceIDs = 0;

    std::list<Device*> foundDevices;

    hr = CoCreateInstance(CLSID_PortableDeviceManager,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pPortableDeviceManager));
    if (FAILED(hr))
    {
        LOGERROR(L"! Failed to CoCreateInstance CLSID_PortableDeviceManager, hr = 0x%lx", hr);
    }

    if (SUCCEEDED(hr))
    {
        hr = pPortableDeviceManager->GetDevices(NULL, &cPnPDeviceIDs);
        if (FAILED(hr))
        {
            LOGERROR(L"! Failed to get number of devices on the system, hr = 0x%lx", hr);
        }
    }

    // Report the number of devices found.  NOTE: we will report 0, if an error
    // occured.

    LOGERROR(L"%d Windows Portable Device(s) found on the system", cPnPDeviceIDs);

    PWSTR* pPnpDeviceIDs = NULL;
    DWORD dwIndex = 0;

    if (SUCCEEDED(hr) && (cPnPDeviceIDs > 0))
    {
        pPnpDeviceIDs = new (std::nothrow) PWSTR[cPnPDeviceIDs];

        if (pPnpDeviceIDs != NULL)
        {
            ZeroMemory((void*)pPnpDeviceIDs, sizeof(PWSTR) * cPnPDeviceIDs);

            hr = pPortableDeviceManager->GetDevices(pPnpDeviceIDs, &cPnPDeviceIDs);
            if (SUCCEEDED(hr))
            {
                // For each device found, display the devices friendly name,
                // manufacturer, and description strings.
                for (dwIndex = 0; dwIndex < cPnPDeviceIDs; dwIndex++)
                {
                    std::wstring deviceId = (WCHAR*)pPnpDeviceIDs[dwIndex];

                    if (!deviceId.empty())
                    {
                        // If device is already listed, do nothing
                        Device* exists = nullptr;

                        for (std::list<Device*>::iterator it = m_allDevices.begin(); it != m_allDevices.end() && !exists; it++)
                        {
                            if ((*it)->GetId() == deviceId)
                            {
                                exists = *it;
                                LOGTRACE(L"Device '%s' already known", (*exists).GetFriendlyName().c_str());
                            }
                        }

                        foundDevices.push_back(exists ? exists : new Device(pPortableDeviceManager, deviceId));
                    }
                }

                for (dwIndex = 0; dwIndex < cPnPDeviceIDs; dwIndex++)
                {
                    CoTaskMemFree(pPnpDeviceIDs[dwIndex]);
                    pPnpDeviceIDs[dwIndex] = nullptr;
                }
            }
            else
            {
                LOGERROR(L"! Failed to get the device list from the system, hr = 0x%lx", hr);
            }
        }

        // Delete the array of PWSTR pointers
        delete[] pPnpDeviceIDs;
        pPnpDeviceIDs = NULL;
    }

    // Add dummy device if there is a registry key for it
    registry.Open();

    if (registry.DoesKeyExist(L"Cameras\\retro.kiwi\\Test Camera"))
    {
        Device* device = new DummyDevice(L"test", L"Test Camera", L"retro.kiwi", L"Dummy Camera for Testing");

        LOGTRACE(L"Adding dummy device for testing");
        foundDevices.push_back(device);
    }

    registry.Close();

    // We now have a list of _currently_ connected and available devices
    // Compare with the current list and remove any that have disappeared.
    for (std::list<Device*>::iterator it = m_allDevices.begin(); it != m_allDevices.end(); it++)
    {
        bool found = false;

        for (std::list<Device*>::iterator newit = foundDevices.begin(); newit != foundDevices.end() && !found; newit++)
        {
            if ((*it)->GetId() == (*newit)->GetId())
            {
                found = true;
            }
        }

        if (!found)
        {
            // Device has disappeared.. we need to remove/delete it
            delete* it;
        }
    }

    m_allDevices = foundDevices;

    return foundDevices;
}

std::list<Device*>
DeviceManager::GetFilteredDevices()
{
    std::list<Device*> allDevices = GetAllDevices();
    std::list<Device*> result;

    for (std::list<Device*>::iterator it = allDevices.begin(); it != allDevices.end(); it++)
    {
        if (CameraManager::IsSupportedDevice(*it))
        {
            result.push_back(*it);
        }
    }

    return result;
}
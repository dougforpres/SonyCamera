#include "pch.h"
#include "MTPEnumerator.h"
#include "MTPDevice.h"
#include "Logger.h"

MTPEnumerator::MTPEnumerator()
{

}

MTPEnumerator::~MTPEnumerator()
{

}

std::list<Device*>
MTPEnumerator::EnumerateDevices()
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
                        foundDevices.push_back(new MTPDevice(pPortableDeviceManager, deviceId));
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

    return foundDevices;
}
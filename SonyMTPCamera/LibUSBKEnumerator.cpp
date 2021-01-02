#include "pch.h"
#include "LibUSBKEnumerator.h"
#include "CameraException.h"

#define SONY_VENDOR_ID 0x054c

LibUSBKEnumerator::LibUSBKEnumerator()
{
    if (!LstK_Init(&m_deviceList, KLST_FLAG_NONE))
    {
        throw new CameraException(L"Unable to initialize list of libusbK devices");
    }
}

LibUSBKEnumerator::~LibUSBKEnumerator()
{
    // Always free the device list if LstK_Init returns TRUE
    LstK_Free(m_deviceList);
}

std::list<Device*>
LibUSBKEnumerator::EnumerateDevices()
{
    std::list<Device*> foundDevices;

    KLST_DEVINFO_HANDLE deviceInfo = NULL;
    DWORD errorCode = ERROR_SUCCESS;
    UINT count = 0;

    // Get the number of devices contained in the device list.
    LstK_Count(m_deviceList, &count);

    if (count)
    {
        while (LstK_MoveNext(m_deviceList, &deviceInfo))
        {
            if (deviceInfo)
            {
                if (deviceInfo->Common.Vid == SONY_VENDOR_ID)
                {
                    std::wostringstream builder;

                    builder << deviceInfo->SerialNumber;

                    foundDevices.push_back(new LibUSBKDevice(deviceInfo, builder.str()));

                    //BOOL success = LibK_SetContext(deviceInfo, KLIB_HANDLE_TYPE_LSTINFOK, 1);

                    //if (success)
                    //{
                    //    UINT_PTR myValue = LibK_GetContext(deviceInfo, KLIB_HANDLE_TYPE_LSTINFOK);
                    //    printf("MyContextValue = %u\n", myValue);
                    //}

                }
            }
        }

        errorCode = GetLastError();

        if (errorCode == ERROR_NO_MORE_ITEMS)
        {
            // End of list
        }
        else
        {
            // Error
        }
    }
    else
    {
        printf("No devices connected.\n");
    }

    return foundDevices;
}
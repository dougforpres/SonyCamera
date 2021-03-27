// LetsGoHunting.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define WIN32_LEAN_AND_MEAN
// Windows Header Files
#include <windows.h>
#include <iostream>
#include <iomanip>
#include "libusbK/libusbk.h"

#define SONY_VENDOR_ID 0x054c

#define DEVICE_UUID_COUNT 9
#define CLASS_UUID_COUNT 2

char device_uuids[DEVICE_UUID_COUNT][40] = {
    "{1EC3985C-A341-4724-96B3-DB0D5CE52709}",
    "{18A73032-DA01-4D12-A8DD-2182CD690F4E}",
    "{6DC1B412-14D1-40D8-831E-0BD5212CE780}",
    "{9D47F2B5-D6AC-4DB9-ACDC-6D2AC6F38E22}",
    "{571854E4-20E8-45A8-91A6-FCDFC4DDA204}",
    "{43137B7F-D3ED-4353-BAFF-D5F2FCA3CD25}",
    "{83B70C84-54AE-427F-B3DC-4545893FB007}",
    "{36988ED5-BF8C-43BF-8F61-3DAE3700B84B}",
    "{A5A9EEF3-CC95-EA06-F565-53FC144341D9}"
};

char class_uuids[CLASS_UUID_COUNT][40] = {
    "{ECFB0CFD-74C4-4f52-BBF7-343461CD72AC}",  // "libusbK Usb Device" From sony inf file
    "{EEC5AD98-8080-425F-922A-DABF3DE3F69A}"   // "WDP" When associated with wpdmtp - doesn't appear to find regardless
};

void
dumpDevices(KLST_HANDLE& deviceList)
{
    KLST_DEVINFO_HANDLE deviceInfo = NULL;
    DWORD errorCode = ERROR_SUCCESS;
    UINT count = 0;

    LstK_Count(deviceList, &count);

    if (count > 0)
    {
        std::cerr << count << " libusbK devices found" << std::endl;

        while (LstK_MoveNext(deviceList, &deviceInfo))
        {
            if (deviceInfo)
            {
                std::cout << "Scanning libusbK device: VID=" << std::hex << std::setw(4) << std::setfill('0') << deviceInfo->Common.Vid << ", PID=" << std::hex << std::setw(4) << std::setfill('0') << deviceInfo->Common.Pid << std::endl;

                if (deviceInfo->Common.Vid == SONY_VENDOR_ID)
                {
                    std::cout << "  Bus Number:          " << deviceInfo->BusNumber << std::endl;
                    std::cout << "  ClassGUID:           " << deviceInfo->ClassGUID << std::endl;
                    std::cout << "  Connected:           " << deviceInfo->Connected << std::endl;
                    std::cout << "  DeviceAddress:       " << deviceInfo->DeviceAddress << std::endl;
                    std::cout << "  DeviceDesc:          " << deviceInfo->DeviceDesc << std::endl;
                    std::cout << "  DeviceID:            " << deviceInfo->DeviceID << std::endl;
                    std::cout << "  DeviceInterfaceGUID: " << deviceInfo->DeviceInterfaceGUID << std::endl;
                    std::cout << "  DevicePath:          " << deviceInfo->DevicePath << std::endl;
                    std::cout << "  DriverID:            " << deviceInfo->DriverID << std::endl;
                    std::cout << "  LUsb0FilterIndex:    " << deviceInfo->LUsb0FilterIndex << std::endl;
                    std::cout << "  Mfg:                 " << deviceInfo->Mfg << std::endl;
                    std::cout << "  Serial Number:       " << deviceInfo->SerialNumber << std::endl;
                    std::cout << "  Service:             " << deviceInfo->Service << std::endl;
                    std::cout << "  SyncFlags:           " << deviceInfo->SyncFlags << std::endl;
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
            std::cerr << "Got an error enumerating devices " << errorCode << std::endl;
        }
    }
}

void
phase1()
{
    KLST_HANDLE deviceList;

    std::cout << "Phase 1 - Ask for ALL devices" << std::endl;

    if (!LstK_Init(&deviceList, KLST_FLAG_NONE))
    {
        std::cerr << "Unable to initialize list of libusbK devices" << std::endl;

        return;
    }

    dumpDevices(deviceList);

    // Always free the device list if LstK_Init returns TRUE
    LstK_Free(deviceList);

    std::cout << "---------------------------------" << std::endl;
}

void
phase2search(char* uuid)
{
    KLST_HANDLE deviceList;
    KLST_PATTERN_MATCH pattern;

    memset(&pattern, 0, sizeof(KLST_PATTERN_MATCH));
    strncpy_s(pattern.DeviceID, uuid, KLST_STRING_MAX_LEN);

    if (!LstK_InitEx(&deviceList, KLST_FLAG_NONE, &pattern))
    {
        std::cerr << "Unable to initialize list of libusbK devices for " << uuid << std::endl;

        return;
    }

    dumpDevices(deviceList);

    // Always free the device list if LstK_Init returns TRUE
    LstK_Free(deviceList);
}

void
phase2()
{
    std::cout << "Phase 2 - Ask for specific device GUIDs" << std::endl;

    for (int i = 0; i < DEVICE_UUID_COUNT; i++)
    {
        phase2search(device_uuids[i]);
    }

    std::cout << "---------------------------------" << std::endl;
}

void
phase3search(char* uuid)
{
    KLST_HANDLE deviceList;
    KLST_PATTERN_MATCH pattern;

    memset(&pattern, 0, sizeof(KLST_PATTERN_MATCH));
    strncpy_s(pattern.DeviceInterfaceGUID, uuid, KLST_STRING_MAX_LEN);

    if (!LstK_InitEx(&deviceList, /*KLST_FLAG_INCLUDE_RAWGUID |*/ KLST_FLAG_INCLUDE_DISCONNECT, &pattern))
    {
        std::cerr << "Unable to initialize list of libusbK devices for " << uuid << std::endl;

        return;
    }

    dumpDevices(deviceList);

    // Always free the device list if LstK_Init returns TRUE
    LstK_Free(deviceList);
}

void
phase3()
{
    std::cout << "Phase 3 - Ask for specific device interface GUIDs" << std::endl;

    for (int i = 0; i < DEVICE_UUID_COUNT; i++)
    {
        phase3search(device_uuids[i]);
    }

    std::cout << "---------------------------------" << std::endl;
}
void
phase4search(char* uuid)
{
    KLST_HANDLE deviceList;
    KLST_PATTERN_MATCH pattern;

    memset(&pattern, 0, sizeof(KLST_PATTERN_MATCH));
    strncpy_s(pattern.ClassGUID, uuid, KLST_STRING_MAX_LEN);

    if (!LstK_InitEx(&deviceList, KLST_FLAG_INCLUDE_RAWGUID, &pattern))
    {
        std::cerr << "Unable to initialize list of libusbK devices for class GUID" << std::endl;

        return;
    }

    dumpDevices(deviceList);

    // Always free the device list if LstK_Init returns TRUE
    LstK_Free(deviceList);
}

void
phase4()
{
    std::cout << "Phase 4 - Ask for class GUID" << std::endl;

    for (int i = 0; i < CLASS_UUID_COUNT; i++)
    {
        phase4search(class_uuids[i]);
    }

    std::cout << "---------------------------------" << std::endl;
}

int main()
{
    phase1();
    phase2();
    phase3();
    phase4();

    std::cout << std::endl << "Did we find any wabbits?" << std::endl << std::endl << "Press <enter> to exit";
    getchar();
}
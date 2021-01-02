// SonyCameraInfo.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include <Windows.h>
#include <string>
#include "SonyMTPCamera.h"

bool
ensureShutterSpeed(HANDLE hCamera)
{
    // Get settings...
    HRESULT hr = ERROR_SUCCESS;
    PROPERTYVALUE pv;
    double speed = 10000.0;
    bool speedOk = false;
    std::wstring shutterSpeed;

    while (!speedOk && hr == ERROR_SUCCESS)
    {
        hr = GetSinglePropertyValue(hCamera, 0xd20d, &pv);

        if (hr == ERROR_SUCCESS)
        {
            shutterSpeed = pv.text;

            switch (pv.value)
            {
            case 0x00000000:
                // BULB
                // This is ok, as code will take 1 sec exposure
                // Fall thru

            case 0xffffffff:
                // AUTO
                speed = DBL_MIN;
                break;

            default:
                speed = (pv.value & 0xffff0000 >> 16) / (double)(pv.value & 0x0000ffff);
            }

            if (speed <= 5.0)
            {
                speedOk = true;
            }
            else
            {
                std::wcout << L"\n    Shutter speed currently set to " << pv.text << L", please set to a value less than 5 seconds";
                Sleep(2500);
            }
        }
        else
        {
            std::wcout << L"Error getting shutter speed from camera - error " << hr << L"\n";
        }
    }

    std::wcout << "\n    Shutter speed set to: " << shutterSpeed.c_str();

    return speedOk;
}

bool
ensureStorageMode(HANDLE hCamera)
{
    // Get settings...
    HRESULT hr = ERROR_SUCCESS;
    PROPERTYVALUE pv;
    bool modeOk = false;
    std::wstring smode;

    while (!modeOk && hr == ERROR_SUCCESS)
    {
        hr = GetSinglePropertyValue(hCamera, 0x5004, &pv);

        if (hr == ERROR_SUCCESS)
        {
            smode = pv.text;

            if (smode == L"RAW" || smode == L"RAW+JPEG")
            {
                modeOk = true;
            }
            else
            {
                std::wcout << L"\n    Storage mode currently set to '" << pv.text << L"', please set to either 'RAW' or 'RAW+JPEG'";
                Sleep(2500);
            }
        }
        else
        {
            std::wcout << L"Error getting shutter speed from camera - error " << hr << L"\n";
        }
    }

    std::wcout << "\n    Storage mode set to:  " << smode.c_str();

    return modeOk;
}

int main()
{
    std::cout << "Sony Camera Info\n~~~~~~~~~~~~~~~~\n\n";

    std::cout << "Enumerating connected devices...";

    HRESULT comhr = CoInitialize(nullptr);

    int portableDeviceCount = GetPortableDeviceCount();

    std::cout << portableDeviceCount << " portable devices found\n\n";
    PORTABLEDEVICEINFO pdinfo;

    for (int p = 0; p < portableDeviceCount; p++)
    {
        std::cout << "--------------------------------------\nDevice #" << p + 1 << "\n";
        memset(&pdinfo, 0, sizeof(pdinfo));

        if (GetPortableDeviceInfo(p, &pdinfo) == ERROR_SUCCESS)
        {
            std::wcout << L"  Manufacturer: " << pdinfo.manufacturer << L"\n";
            std::wcout << L"  Model:        " << pdinfo.model << L"\n";
            std::wcout << L"  Connection:   ";

            try
            {
                HANDLE hCamera = OpenDeviceEx(pdinfo.id, OPENDEVICEEX_OPEN_ANY_DEVICE);

                if (hCamera != INVALID_HANDLE_VALUE)
                {
                    // If we were able to open the device that is a super great start
                    std::wcout << L"Connected\n";
                    std::wcout << L"  Get Info:     ";

                    CAMERAINFO cameraInfo;
                    HRESULT hr = GetCameraInfo(hCamera, &cameraInfo, 0);

                    if (hr == ERROR_SUCCESS)
                    {
                        if (cameraInfo.pixelWidth == 0.0 || cameraInfo.pixelHeight == 0.0)
                        {
                            std::wcout << L"Currently unknown device: ";

                            if (cameraInfo.imageWidthPixels == 0 || cameraInfo.imageHeightPixels == 0 || cameraInfo.previewWidthPixels == 0 || cameraInfo.previewHeightPixels == 0)
                            {
                                std::wcout << L" - attempt to determine image resolution...";

                                bool shutterSpeedSet = ensureShutterSpeed(hCamera);
                                bool storageModeOk = ensureStorageMode(hCamera);

                                if (shutterSpeedSet && storageModeOk)
                                {
                                    std::wcout << L"\n    Asking camera to take test shots (1 x preview, 1 x full-resolution)...";

                                    hr = GetCameraInfo(hCamera, &cameraInfo, 1);

                                    if (hr == ERROR_SUCCESS)
                                    {
                                        std::wcout << L"\n\nPlease email the following info to: retrodotkiwi@gmail.com\n\n";
                                        std::wcout << L"   Manufacturer:           " << pdinfo.manufacturer << "\n";
                                        std::wcout << L"   Model:                  " << pdinfo.model << "\n";
                                        std::wcout << L"   Device Path:            " << pdinfo.devicePath << "\n";
                                        std::wcout << L"   Preview image width:    " << cameraInfo.previewWidthPixels << "px\n";
                                        std::wcout << L"   Preview image height:   " << cameraInfo.previewHeightPixels << "px\n";
                                        std::wcout << L"   Full-size image width:  " << cameraInfo.imageWidthPixels << "px\n";
                                        std::wcout << L"   Full-size image height: " << cameraInfo.imageHeightPixels << "px\n";
                                    }
                                    else
                                    {
                                        std::wcout << L"Error taking test photos - error " << hr << L"\n";
                                    }
                                }
                            }
                            else
                            {
                                // Already detected
                                std::wcout << L" - already taken test shots";

                                std::wcout << L"\n\nPlease email the following info to: retrodotkiwi@gmail.com\n\n";
                                std::wcout << L"   Manufacturer:           " << pdinfo.manufacturer << "\n";
                                std::wcout << L"   Model:                  " << pdinfo.model << "\n";
                                std::wcout << L"   Device Path:            " << pdinfo.devicePath << "\n";
                                std::wcout << L"   Preview image width:    " << cameraInfo.previewWidthPixels << "px\n";
                                std::wcout << L"   Preview image height:   " << cameraInfo.previewHeightPixels << "px\n";
                                std::wcout << L"   Full-size image width:  " << cameraInfo.imageWidthPixels << "px\n";
                                std::wcout << L"   Full-size image height: " << cameraInfo.imageHeightPixels << "px\n";
                            }
                        }
                        else
                        {
                            std::wcout << L"This device is already known\n";
                        }
                    }
                    else
                    {
                        std::wcout << L"Failed getting camera info - error " << hr << L"\n";
                    }
                }
                else
                {
                    std::wcout << L"Unable to open device\n";
                }
            }
            catch (...)
            {
                std::wcout << L"Failed connecting/negotiating with device\n";
            }
        }
    }
}

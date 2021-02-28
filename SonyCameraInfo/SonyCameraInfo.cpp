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

void
checkLogging()
{
    HKEY key;

    if (RegCreateKeyEx(HKEY_CURRENT_USER, L"Software\\Retro.kiwi\\SonyMTPCamera.dll", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ, NULL, &key, NULL) == ERROR_SUCCESS)
    {
        // Get Log Level (use same default as driver)
        DWORD dword;
        DWORD dataSize = sizeof(DWORD);

        LSTATUS regresult = RegGetValue(key, nullptr, L"Log Level", RRF_RT_REG_DWORD, nullptr, &dword, &dataSize);

        if (regresult == ERROR_SUCCESS)
        {
            std::wcout << L"Logging level set to " << dword << L" (1 = everything... 5 = errors only)\n";
        }
        else
        {
            std::wcout << L"Logging level not set, DLL will attempt to log ERRORS only\n";
        }

        dataSize = 0;
        regresult = RegGetValue(key, nullptr, L"Logfile Name", RRF_RT_REG_SZ, nullptr, nullptr, &dataSize);

        if (regresult == ERROR_SUCCESS && dataSize)
        {
            LPWSTR str = (LPWSTR)new BYTE[dataSize];

            regresult = RegGetValue(key, nullptr, L"Logfile Name", RRF_RT_REG_SZ, nullptr, str, &dataSize);

            // Deal with ENV expansion first
            WCHAR* buffer = new WCHAR[1024];

            if (ExpandEnvironmentStrings(str, buffer, 1024))
            {
                delete[] str;
                str = buffer;
            }

            std::wcout << L"Driver DLL IS configured to log to a file (" << str << L")\n";

            // We cannot write to the file as the driver dll will have it locked for writing, but we can get its size and call a method that will write to the log
            // Attempt to get size of the file, then write to it, and finally get size again and see if it got bigger
            HANDLE hLog = CreateFile(str, 0, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

            if (hLog != INVALID_HANDLE_VALUE)
            {
                DWORD sizeLow = GetFileSize(hLog, nullptr);

                TestFunc(nullptr);

                DWORD biggerSizeLow = GetFileSize(hLog, nullptr);

                if (biggerSizeLow == sizeLow)
                {
                    std::wcerr << L"Unable to write to the log file (error = " << GetLastError() << ")\n";
                }

                CloseHandle(hLog);
            }
            else
            {
                DWORD err = GetLastError();

                std::wcerr << L"Unable to open the log file";

                switch (err)
                {
                case ERROR_PATH_NOT_FOUND:
                    std::wcerr << L" - Cannot find the directory\n";
                    break;

                case ERROR_FILE_NOT_FOUND:
                    std::wcerr << L" - File cannot be found\n";
                    break;

                case ERROR_FILE_READ_ONLY:
                    std::wcerr << L" - File is marked READ-ONLY\n";
                    break;

                case ERROR_SHARING_VIOLATION:
                    std::wcerr << L" - Another program is preventing the file from being written to\n";
                    break;

                default:
                    std::wcerr << L" (error = " << GetLastError() << L")\n";
                    break;
                }
            }
        }
        else
        {
            std::wcout << L"Driver DLL IS NOT configured to log to a file\n";
        }

        RegCloseKey(key);
    }
    else
    {
        std::wcerr << L"Problem opening the registry for the camera driver... not ideal.  (Error " << GetLastError() << L")\n";
    }

    std::wcout << L"\n";
}

int main()
{
    std::cout << "Sony Camera Info\n~~~~~~~~~~~~~~~~\n\n";

    // Test logging if a logfile is specified
    checkLogging();

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
                                        std::wcout << L"   Manufacturer:                     " << pdinfo.manufacturer << "\n";
                                        std::wcout << L"   Model:                            " << pdinfo.model << "\n";
                                        std::wcout << L"   Device Path:                      " << pdinfo.devicePath << "\n";
                                        std::wcout << L"   Preview image width:              " << cameraInfo.previewWidthPixels << "px\n";
                                        std::wcout << L"   Preview image height:             " << cameraInfo.previewHeightPixels << "px\n";
                                        std::wcout << L"   Full-size image width:            " << cameraInfo.imageWidthPixels << "px\n";
                                        std::wcout << L"   Full-size image height:           " << cameraInfo.imageHeightPixels << "px\n";
                                        std::wcout << L"   Full-size image width (Cropped):  " << cameraInfo.imageWidthCroppedPixels << "px\n";
                                        std::wcout << L"   Full-size image height (Cropped): " << cameraInfo.imageHeightCroppedPixels << "px\n";
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
                                std::wcout << L"   Manufacturer:                     " << pdinfo.manufacturer << "\n";
                                std::wcout << L"   Model:                            " << pdinfo.model << "\n";
                                std::wcout << L"   Device Path:                      " << pdinfo.devicePath << "\n";
                                std::wcout << L"   Preview image width:              " << cameraInfo.previewWidthPixels << "px\n";
                                std::wcout << L"   Preview image height:             " << cameraInfo.previewHeightPixels << "px\n";
                                std::wcout << L"   Full-size image width:            " << cameraInfo.imageWidthPixels << "px\n";
                                std::wcout << L"   Full-size image height:           " << cameraInfo.imageHeightPixels << "px\n";
                                std::wcout << L"   Full-size image width (Cropped):  " << cameraInfo.imageWidthPixels << "px\n";
                                std::wcout << L"   Full-size image height (Cropped): " << cameraInfo.imageHeightPixels << "px\n";
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

                    CloseDevice(hCamera);
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

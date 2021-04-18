// SonyCameraInfo.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include <Windows.h>
#include <string>
#include <sstream>
#include "SonyMTPCamera.h"

typedef void (*F)(int deviceId, HANDLE hCamera, PORTABLEDEVICEINFO *pdinfo, int value);

bool
ensureExposureMode(HANDLE hCamera)
{
    // Get settings...
    HRESULT hr = ERROR_SUCCESS;
    PROPERTYVALUE pv;
    bool modeOk = false;
    std::wstring smode;

    while (!modeOk && hr == ERROR_SUCCESS)
    {
        RefreshPropertyList(hCamera);
        hr = GetSinglePropertyValue(hCamera, 0x500e, &pv);

        if (hr == ERROR_SUCCESS)
        {
            smode = pv.text;

            if (smode == L"M")
            {
                modeOk = true;
            }
            else
            {
                std::wcout << L"\n    Exposure mode currently set to '" << pv.text << L"', please set to 'M'";
                Sleep(2500);
            }
        }
        else
        {
            std::wcout << L"Error getting exposure mode from camera - error " << hr << L"\n";
        }
    }

    std::wcout << "\n  Exposure mode set to:  " << smode.c_str();

    return modeOk;
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
        RefreshPropertyList(hCamera);
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

    std::wcout << "\n  Storage mode set to:   " << smode.c_str();

    return modeOk;
}

void
setLogging(short debugLevel)
{
    HKEY key;

    if (RegCreateKeyEx(HKEY_CURRENT_USER, L"Software\\Retro.kiwi\\SonyMTPCamera.dll", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &key, NULL) == ERROR_SUCCESS)
    {
        // Set Log Level
        DWORD dword = debugLevel;
        DWORD dataSize = sizeof(DWORD);

        LSTATUS regresult = RegSetValueEx(key, L"Log Level", 0, REG_DWORD, (BYTE*)&dword, sizeof(DWORD));

        if (regresult == ERROR_SUCCESS)
        {
            std::wcout << L"Debug level is set to " << dword << L" (1 = everything... 5 = errors only)\n";
        }
        else
        {
            std::wcerr << L"Debug level could not be set\n";
        }

        std::wstring filename = debugLevel > 0 ? std::wstring(L"%HOMEDRIVE%%HOMEPATH%\\sonycamera.txt") : std::wstring(L"");

        regresult = RegSetValueEx(key, L"Logfile Name", 0, REG_SZ, (BYTE *)filename.c_str(), filename.length() * sizeof(wchar_t));

        if (regresult == ERROR_SUCCESS)
        {
            std::wcout << L"Log file set to '" << filename.c_str() << "'";
        }
        else
        {
            std::wcerr << L"Log file could not be set";
        }

        RegCloseKey(key);
    }
    else
    {
        std::wcerr << L"Unable to open registry to update debug setting";
    }
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
            std::wcout << L"Debug level is set to " << dword << L" (1 = everything... 5 = errors only)\n";
        }
        else
        {
            std::wcout << L"Debug level is not set, DLL will attempt to log ERRORS only\n";
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

void
checkCrop(int deviceId, HANDLE hCamera, PORTABLEDEVICEINFO *pdinfo, int value)
{
    DEVICEINFO deviceInfo;

    memset(&deviceInfo, 0, sizeof(DEVICEINFO));

    deviceInfo.version = 1;

    HRESULT hr = GetDeviceInfo(deviceId, &deviceInfo);

    if (hr == ERROR_SUCCESS)
    {
        std::wcout << L"  Crop Mode:    " << deviceInfo.cropMode << "\n";
    }
    else
    {
        std::wcout << L"Failed getting device info - error " << hr << L"\n";
    }
}

void
setCrop(int deviceId, HANDLE hCamera, PORTABLEDEVICEINFO *pdinfo, int cropMode)
{
    if (cropMode == 1)
    {
        CAMERAINFO cameraInfo;
        HRESULT hr = GetCameraInfo(hCamera, &cameraInfo, 0);

        if (hr == ERROR_SUCCESS)
        {
            if (cameraInfo.imageWidthCroppedPixels == 0 || cameraInfo.imageHeightCroppedPixels == 0)
            {
                std::wcerr << L"Cannot change crop mode to AUTO until crop image size is known." << std::endl;
                std::wcerr << L"please re-run this tool with '/s' flag first (or no flags at all)." << std::endl;

                return;
            }
        }
    }

    DEVICEINFO deviceInfo;

    memset(&deviceInfo, 0, sizeof(DEVICEINFO));

    deviceInfo.version = 1;

    HRESULT hr = GetDeviceInfo(deviceId, &deviceInfo);

    if (hr == ERROR_SUCCESS)
    {
        std::wcout << L"  Existing Crop Mode:   " << deviceInfo.cropMode << "\n";

        if (deviceInfo.cropMode == (DWORD)cropMode)
        {
            std::wcout << L"  Not updating, already set to desired value\n";
        }
        else
        {
            HKEY key;
            std::wostringstream builder;

            builder << L"Software\\Retro.kiwi\\SonyMTPCamera.dll\\Cameras\\" << pdinfo->devicePath;

            if (RegCreateKeyEx(HKEY_CURRENT_USER, builder.str().c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &key, NULL) == ERROR_SUCCESS)
            {
                // Set Log Level
                DWORD dword = (DWORD)cropMode;
                DWORD dataSize = sizeof(DWORD);

                LSTATUS regresult = RegSetValueEx(key, L"Crop Mode", 0, REG_DWORD, (BYTE*)&dword, sizeof(DWORD));

                if (regresult == ERROR_SUCCESS)
                {
                    std::wcout << L"  New Crop Mode:        " << dword << L"\n";
                }
                else
                {
                    std::wcerr << L"Debug level could not be set\n";
                }

                RegCloseKey(key);
            }
            else
            {
                std::wcerr << L"Unable to update crop mode\n";
            }
        }
    }
    else
    {
        std::wcout << L"Failed getting device info - error " << hr << L"\n";
    }
}

void
printHelp()
{
    std::wcout << L"\n/h        Print this help\n/s        Scan for connected cameras (default)\n/l        List supported cameras\n/d        Show debug setting (see /d[0-5] to set)\n/c        Show crop setting for connected cameras\n/e        Show list of exposure times supported by device\n";
    std::wcout << L"/c[0-2]   Change crop-mode of images for connected cameras\n          0 = No Crop (all pixels)\n          1 = Auto Crop (To what image editing apps would see)\n          2 = User Crop (Manual, requires registry editing)\n";
    std::wcout << L"/d[0-5]   Change debug logging of driver\n          0 = Off (no logging)\n          1 = Log everything\n          ...\n          5 = Errors Only\n          Note that this option will write debugging to a file called\n          'sonycamera.txt' that will appear on your desktop\n";
}

void
printExposureTimes(int deviceId, HANDLE hCamera, PORTABLEDEVICEINFO* pdinfo, int shortForm)
{
    PROPERTYDESCRIPTOR descriptor;

    HRESULT hr = GetPropertyDescriptor(hCamera, 0xffff, &descriptor);

    if (descriptor.valueCount > 0)
    {
        PROPERTYVALUEOPTION option;

        std::wcout << "   Available Exposure Times:" << std::endl;
        
        if (shortForm)
        {
            std::wcout << "     ";
        }

        int c = 0;

        for (int i = 0; i < descriptor.valueCount; i++)
        {
            hr = GetPropertyValueOption(hCamera, 0xffff, &option, i);

            if (shortForm)
            {
                c++;
                std::wcout << option.value << L", ";

                if (c > 9)
                {
                    std::wcout << std::endl << "     ";;
                    c = 0;
                }
            }
            else
            {
                std::wcout << L"    " << i + 1 << L" - " << option.name << L" (" << option.value << L")" << std::endl;
            }
        }

        std::wcout << std::endl;
    }
}

void
printISOs(int deviceId, HANDLE hCamera, PORTABLEDEVICEINFO* pdinfo, int shortForm)
{
    PROPERTYDESCRIPTOR descriptor;

    HRESULT hr = GetPropertyDescriptor(hCamera, 0xfffe, &descriptor);

    if (descriptor.valueCount > 0)
    {
        PROPERTYVALUEOPTION option;

        std::wcout << "   Available ISO Values:" << std::endl;

        if (shortForm)
        {
            std::wcout << "     ";
        }

        int c = 0;

        for (int i = 0; i < descriptor.valueCount; i++)
        {
            hr = GetPropertyValueOption(hCamera, 0xfffe, &option, i);

            if (shortForm)
            {
                c++;
                std::wcout << option.value << L", ";

                if (c > 9)
                {
                    std::wcout << std::endl << "     ";;
                    c = 0;
                }
            }
            else
            {
                std::wcout << L"    " << i + 1 << L" - " << option.name << L" (" << option.value << L")" << std::endl;
            }
        }

        std::wcout << std::endl;
    }
}

void
performScan(int deviceId, HANDLE hCamera, PORTABLEDEVICEINFO* pdinfo, int shortForm)
{
    std::wcout << L"  Get Info:\n";

    CAMERAINFO cameraInfo;
    HRESULT hr = GetCameraInfo(hCamera, &cameraInfo, 0);

    if (hr == ERROR_SUCCESS)
    {
        // Check ISO and exposure times have > 0 items
        bool exposureTimesOk;
        bool isosOk;
        bool previewOk = cameraInfo.previewWidthPixels != 0 && cameraInfo.previewHeightPixels != 0;
        bool fullImageOk = cameraInfo.imageWidthPixels != 0 && cameraInfo.imageHeightPixels != 0;
        bool croppedImageOk = cameraInfo.imageWidthCroppedPixels != 0 && cameraInfo.imageHeightCroppedPixels != 0;

        PROPERTYDESCRIPTOR descriptor;
        HRESULT hr = GetPropertyDescriptor(hCamera, 0xffff, &descriptor);

        exposureTimesOk = (descriptor.valueCount > 0);
        hr = GetPropertyDescriptor(hCamera, 0xfffe, &descriptor);
        isosOk = (descriptor.valueCount > 0);


        if (!previewOk || !fullImageOk || !croppedImageOk || !exposureTimesOk || !isosOk)
        {
            ensureExposureMode(hCamera);
            ensureStorageMode(hCamera);

            std::wcout << std::endl << std::endl << L"Performing the following actions." << std::endl;

            if (!exposureTimesOk)
            {
                std::wcout << L" - Detect camera supported exposure times (30-45 seconds)" << std::endl;
                std::wcout << L"   During this time, you will see the exposure time changing on your camera" << std::endl;
            }

            if (!isosOk)
            {
                std::wcout << L" - Detect camera supported ISO values (30-45 seconds)" << std::endl;
                std::wcout << L"   During this time, you will see the ISO value changing on your camera" << std::endl;
            }

            if (!previewOk)
            {
                std::wcout << L" - Detect preview support & resolution (less than 1 second)" << std::endl;
            }

            if (!fullImageOk || !croppedImageOk)
            {
                std::wcout << L" - Detect full (uncropped and cropped) image size (2-3 seconds)" << std::endl;
                std::wcout << L"   Your camera will take a photo" << std::endl;
            }

            std::wcout << std::endl << L"  ** PLEASE DO NOT INTERACT WITH THE CAMERA UNTIL DONE **" << std::endl << std::endl;

            hr = GetCameraInfo(hCamera, &cameraInfo, INFOFLAG_ACTIVE | INFOFLAG_INCLUDE_SETTINGS);

            if (hr == ERROR_SUCCESS)
            {
                std::wcout << L"\n\nPlease email the following info to: retrodotkiwi@gmail.com\n\n";
                std::wcout << L"   Manufacturer:                     " << pdinfo->manufacturer << "\n";
                std::wcout << L"   Model:                            " << pdinfo->model << "\n";
                std::wcout << L"   Device Path:                      " << pdinfo->devicePath << "\n";
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
        else
        {
            // Already detected
            if (cameraInfo.pixelWidth == 0.0 || cameraInfo.pixelHeight == 0.0)
            {
                std::wcout << L"Currently unknown device: ";

                std::wcout << L"\n\nPlease email the following info to: retrodotkiwi@gmail.com\n\n";
            }

            std::wcout << L"   Manufacturer:                     " << pdinfo->manufacturer << "\n";
            std::wcout << L"   Model:                            " << pdinfo->model << "\n";
            std::wcout << L"   Device Path:                      " << pdinfo->devicePath << "\n";
            std::wcout << L"   Preview image width:              " << cameraInfo.previewWidthPixels << "px\n";
            std::wcout << L"   Preview image height:             " << cameraInfo.previewHeightPixels << "px\n";
            std::wcout << L"   Full-size image width:            " << cameraInfo.imageWidthPixels << "px\n";
            std::wcout << L"   Full-size image height:           " << cameraInfo.imageHeightPixels << "px\n";
            std::wcout << L"   Full-size image width (Cropped):  " << cameraInfo.imageWidthCroppedPixels << "px\n";
            std::wcout << L"   Full-size image height (Cropped): " << cameraInfo.imageHeightCroppedPixels << "px\n";
        }
    }
    else
    {
        std::wcout << L"Failed getting camera info - error " << hr << L"\n";
    }

    printExposureTimes(deviceId, hCamera, pdinfo, 1);
    printISOs(deviceId, hCamera, pdinfo, 1);
}

void
listCameras()
{
    std::wcout << L"The following is a list of cameras that have been tested by individual users.\n\n";

    HKEY key;

    if (RegCreateKeyEx(HKEY_CURRENT_USER, L"Software\\Retro.kiwi\\SonyMTPCamera.dll\\Cameras\\Sony Corporation", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ, NULL, &key, NULL) == ERROR_SUCCESS)
    {
        // List MTP Cameras first
        std::wcout << L"MTP Connection\n--------------\n";

        // Look for all keys
        DWORD index = 0;
        LSTATUS enumStatus;
        TCHAR keyName[64];

        do
        {
            enumStatus = RegEnumKey(key, index, keyName, 64);

            if (enumStatus == ERROR_SUCCESS)
            {
                std::wcout << L"  " << keyName;

                DWORD dataSize = 0;
                LSTATUS regresult = RegGetValue(key, keyName, nullptr, RRF_RT_REG_SZ, nullptr, nullptr, &dataSize);

                if (regresult == ERROR_SUCCESS && dataSize)
                {
                    LPWSTR str = (LPWSTR)new BYTE[dataSize];

                    regresult = RegGetValue(key, keyName, nullptr, RRF_RT_REG_SZ, nullptr, str, &dataSize);

                    std::wcout << L" (" << str << L")";

                    delete[] str;
                }

                std::wcout << L"\n";
            }

            index++;
        } while (enumStatus == ERROR_SUCCESS);

        std::wcout << L"\n";
    }

    RegCloseKey(key);

    if (RegCreateKeyEx(HKEY_CURRENT_USER, L"Software\\Retro.kiwi\\SonyMTPCamera.dll\\Cameras", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ, NULL, &key, NULL) == ERROR_SUCCESS)
    {
        // List libuskK Cameras next
        std::wcout << L"libusbK Connection\n------------------\n";

        // Look for keys starting with "VID_"
                // Look for all keys
        DWORD index = 0;
        LSTATUS enumStatus;
        TCHAR keyName[64];

        do
        {
            enumStatus = RegEnumKey(key, index, keyName, 64);

            if (enumStatus == ERROR_SUCCESS)
            {
                std::wstring name = std::wstring(keyName);

                if (name.compare(0, 4, L"VID_") == 0)
                {
                    std::wcout << L"  " << keyName;

                    DWORD dataSize = 0;
                    LSTATUS regresult = RegGetValue(key, keyName, nullptr, RRF_RT_REG_SZ, nullptr, nullptr, &dataSize);

                    if (regresult == ERROR_SUCCESS && dataSize)
                    {
                        LPWSTR str = (LPWSTR)new BYTE[dataSize];

                        regresult = RegGetValue(key, keyName, nullptr, RRF_RT_REG_SZ, nullptr, str, &dataSize);

                        std::wcout << L" (" << str << L")";

                        delete[] str;
                    }

                    std::wcout << L"\n";
                }
            }

            index++;
        } while (enumStatus == ERROR_SUCCESS);
    }

    RegCloseKey(key);
}

void
iterateDevices(std::string message, F func, int value)
{
    std::wcout << message.c_str() << std::endl << std::endl;

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

                    try
                    {
                        func(p, hCamera, &pdinfo, value);
                    }
                    catch (...)
                    {
                        std::wcerr << L"Problem executing task\n";
                    }

                    CloseDevice(hCamera);
                }
                else
                {
                    std::wcerr << L"Unable to open device\n";
                }
            }
            catch (...)
            {
                std::wcerr << L"Failed connecting/negotiating with device\n";
            }
        }
    }

    CoUninitialize();
}

int
main(int argc, char* argv[])
{
    bool error = false;
    bool option_specified = false;
    bool do_help = false;
    bool do_scan = false;
    bool do_list = false;
    bool do_exposures = false;
    bool do_isos = false;
    bool do_show_crop = false;
    bool do_show_debug = false;
    bool do_crop = false;
    short crop_option = 0;
    bool do_debug = false;
    short debug_option = 0;

    for (int i = 1; i < argc; i++)
    {
        std::string v = std::string(argv[i]);

        if (v.size() > 1 && v[0] == '/')
        {
            switch (v.size())
            {
            case 2:
                switch (v[1])
                {
                case 'h':
                    // Help
                    option_specified = true;
                    do_help = true;
                    break;

                case 's':
                    // Scan for cameras (default)
                    option_specified = true;
                    do_scan = true;
                    break;

                case 'l':
                    // List supported cameras
                    option_specified = true;
                    do_list = true;
                    break;

                case 'd':
                    // Show/test debug file
                    option_specified = true;
                    do_show_debug = true;
                    break;

                case 'c':
                    // Show crop mode
                    option_specified = true;
                    do_show_crop = true;
                    break;

                case 'e':
                    // Show exposure values
                    option_specified = true;
                    do_exposures = true;
                    break;

                case 'i':
                    // Show iso values
                    option_specified = true;
                    do_isos = true;
                    break;

                default:
                    std::cerr << "'" << v.c_str() << "' is not a valid option\n";
                    error = true;
                    break;
                }
                break;

            case 3:
                switch (v[1])
                {
                case 'c':
                    // Change crop-mode for connected devices
                    switch (v[2])
                    {
                    case '0':
                    case '1':
                    case '2':
                        crop_option = (short)(atoi(v.substr(2).c_str()));
                        option_specified = true;
                        do_crop = true;
                        break;

                    default:
                        std::cerr << "Valid options are 0, 1, or 2";
                        error = true;
                    }
                    break;

                case 'd':
                    // Change logging mode
                    // Change crop-mode for connected devices
                    switch (v[2])
                    {
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                        debug_option = (short)(atoi(v.substr(2).c_str()));
                        option_specified = true;
                        do_debug = true;
                        break;

                    default:
                        std::cerr << "Valid options are 0, 1, 2, 3, 4, or 5";
                        error = true;
                    }
                    break;

                default:
                    std::cerr << "'" << v.c_str() << "' is not a valid option\n";
                    error = true;
                    break;
                }
            }
        }
    }

    std::cout << "Sony Camera Info\n~~~~~~~~~~~~~~~~\n\n";

    if (error || do_help)
    {
        printHelp();

        return error ? ERROR_INVALID_OPERATION : ERROR_SUCCESS;
    }

    if (!option_specified)
    {
        do_scan = true;
    }

    if (do_scan)
    {
        iterateDevices("Scanning for Cameras", performScan, 0);
    }

    if (do_list)
    {
        // TODO
        listCameras();
    }

    if (do_show_debug)
    {
        // Test logging if a logfile is specified
        checkLogging();
    }

    if (do_debug)
    {
        setLogging(debug_option);
    }

    if (do_show_crop)
    {
        iterateDevices("Getting Crop Settings", checkCrop, 0);
    }

    if (do_crop)
    {
        iterateDevices("Setting Crop", setCrop, crop_option);
    }

    if (do_exposures)
    {
        iterateDevices("Listing supported Exposure times", printExposureTimes, 0);
    }

    if (do_isos)
    {
        iterateDevices("Listing supported ISO values", printISOs, 0);
    }

    return ERROR_SUCCESS;
}

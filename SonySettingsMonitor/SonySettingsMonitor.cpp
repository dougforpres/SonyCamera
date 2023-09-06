// SonySettingsMonitor.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include <iostream>
#include <Windows.h>
#include "SonyMTPCamera.h"

void
watchSettings(HANDLE h, bool loop)
{
    DWORD count = 0;
    IMAGEINFO iinfo;

    GetPropertyList(h, nullptr, &count);

    PROPERTYVALUE* pv1 = new PROPERTYVALUE[count];
    PROPERTYVALUE* pv2 = nullptr;

    // Dump current state of all properties
    RefreshPropertyList(h);
    GetAllPropertyValues(h, pv1, &count);

    for (int i = 0; i < count; i++)
    {
        PROPERTYVALUE* v = (pv1 + i);
        PROPERTYDESCRIPTOR d;

        GetPropertyDescriptor(h, v->id, &d);

        printf("x%04x (%S), type = x%04x, flags = x%04x, value = x%04x (%S)\n", v->id, d.name, d.type, d.flags, v->value, v->text);
    }

    if (loop)
    {
        pv2 = pv1;

        for (int i = 0; i < 20; i++)
        {
            printf("---\n");
            Sleep(500);

//            RefreshPropertyList(h);
            GetAllPropertyValues(h, pv1, &count);

            if (pv2)
            {
                // Compare
                for (int i = 0; i < count; i++)
                {
                    PROPERTYVALUE* t1 = (pv1 + i);
                    PROPERTYVALUE* t2 = (pv2 + i);

                    if (t1->id == t2->id)
                    {
                        if (t1->value != t2->value)
                        {
                            printf("Property x%04x value changed from x%04x to x%04x (%S -> %S)\n", t1->id, t2->value, t1->value, t2->text, t1->text);
                        }
                    }
                    else
                    {
                        // Expectation is same order for return data
                        printf("Id's don't match! x%04x != x%04x\n", t1->id, t2->id);
                    }
                }
            }

            pv2 = pv1;
            pv1 = new PROPERTYVALUE[count];
        }
    }
}

void
dumpExposureOptions(HANDLE h)
{
    PROPERTYDESCRIPTOR descriptor;
    PROPERTYVALUEOPTION option;
    DWORD count = 0;

//    GetPropertyValueOption(h, 0xd24e, &option, 0);

    // Ask how many options there are
    HRESULT hr = GetPropertyDescriptor(h, 0xffff, &descriptor);

    count = descriptor.valueCount;

    for (int i = 0; i < count; i++)
    {
        hr = GetPropertyValueOption(h, 0xffff, &option, i);

        if (hr != ERROR_SUCCESS)
        {
            std::cerr << "Error reading number of options " << hr;
        }

        std::wcout << "Option #" << i + 1 << " - '" << option.name << "' (" << option.value << ")" << std::endl;
    }
}

void
testExposure(HANDLE h)
{
    // Device is open
    // Get exposure time info
    PROPERTYDESCRIPTOR descriptor;
    #define SHUTTERSPEED 0xd20d
    HRESULT hr = GetPropertyDescriptor(h, SHUTTERSPEED, &descriptor);

    if (hr != ERROR_SUCCESS)
    {
        std::cerr << "Error reading property descriptor " << hr;
    }

    PROPERTYVALUE value;
    hr = GetSinglePropertyValue(h, SHUTTERSPEED, &value);

    if (hr != ERROR_SUCCESS)
    {
        std::cerr << "Error reading property value " << hr;
    }

    std::wcout << "Shutter speed set to '" << value.text << "' (" << value.value << ")" << std::endl;

    std::cout << "Setting shutter speed to 1/100..." << std::endl;

    SetExposureTime(h, 0.01, &value);

    if (hr != ERROR_SUCCESS)
    {
        std::cerr << "Error setting property value " << hr;
    }

    std::wcout << "Shutter speed now set to '" << value.text << "' (" << value.value << ")" << std::endl;

    std::cout << "Setting shutter speed to 0.75..." << std::endl;

    SetExposureTime(h, 0.75, &value);

    if (hr != ERROR_SUCCESS)
    {
        std::cerr << "Error setting property value " << hr;
    }

    std::wcout << "Shutter speed now set to '" << value.text << "' (" << value.value << ")" << std::endl;

    std::cout << "Setting shutter speed to 1/3..." << std::endl;

    SetExposureTime(h, 0.333, &value);

    if (hr != ERROR_SUCCESS)
    {
        std::cerr << "Error setting property value " << hr;
    }

    std::wcout << "Shutter speed now set to '" << value.text << "' (" << value.value << ")" << std::endl;

    std::cout << "Setting shutter speed to 1/23..." << std::endl;

    SetExposureTime(h, 1.0/23.0, &value);

    if (hr != ERROR_SUCCESS)
    {
        std::cerr << "Error setting property value " << hr;
    }

    std::wcout << "Shutter speed now set to '" << value.text << "' (" << value.value << ")" << std::endl;

    std::cout << "Setting shutter speed to 1/4001..." << std::endl;

    SetExposureTime(h, 1.0/4001.0, &value);

    if (hr != ERROR_SUCCESS)
    {
        std::cerr << "Error setting property value " << hr;
    }

    std::wcout << "Shutter speed now set to '" << value.text << "' (" << value.value << ")" << std::endl;


    std::cout << "Setting shutter speed to 32..." << std::endl;

    SetExposureTime(h, 32.0, &value);

    if (hr != ERROR_SUCCESS)
    {
        std::cerr << "Error setting property value " << hr;
    }

    std::wcout << "Shutter speed now set to '" << value.text << "' (" << value.value << ")" << std::endl;

    std::cout << "Setting shutter speed to BULB..." << std::endl;

    SetExposureTime(h, 0, &value);

    if (hr != ERROR_SUCCESS)
    {
        std::cerr << "Error setting property value " << hr;
    }

    std::wcout << "Shutter speed now set to '" << value.text << "' (" << value.value << ")" << std::endl;

    std::cout << "Got it";
}

void
testSetISO(HANDLE h)
{
    SetPropertyValue(h, 0xd21e, 640);
}

int main()
{
    HRESULT comhr = CoInitialize(nullptr);

    int portableDeviceCount = GetPortableDeviceCount();

    std::cout << portableDeviceCount << " portable devices found\n\n";
    PORTABLEDEVICEINFO pdinfo;
    std::wstring firstDevice;

    for (int p = 0; p < portableDeviceCount; p++)
    {
        std::cout << "\nDevice #" << p + 1 << "\n";
        memset(&pdinfo, 0, sizeof(pdinfo));

        if (GetPortableDeviceInfo(p, &pdinfo) == ERROR_SUCCESS)
        {
            if (firstDevice.empty())
            {
                firstDevice = pdinfo.id;
            }

            std::wcout << L"  Manufacturer: " << pdinfo.manufacturer << L"\n";
            std::wcout << L"  Model:        " << pdinfo.model << L"\n";
        }
    }

//    DEVICEINFO info;

//    info.version = 1;
//    GetDeviceInfo(0, &info);

    HANDLE h = OpenDevice((LPWSTR)firstDevice.c_str());

    // Uncomment to just keep pulling settings looking for changes
//    watchSettings(h, true);

    // Uncomment to try state-machiney thing to change exposure time
    testExposure(h);
//    testSetISO(h);
//    dumpExposureOptions(h);

    CloseDevice(h);
    _CrtDumpMemoryLeaks();
}

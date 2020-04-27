// SonySettingsMonitor.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include "SonyMTPCamera.h"

int main()
{
    HRESULT comhr = CoInitialize(nullptr);

    int portableDeviceCount = GetPortableDeviceCount();

    std::cout << portableDeviceCount << " portable devices found\n\n";
    PORTABLEDEVICEINFO pdinfo;

    for (int p = 0; p < portableDeviceCount; p++)
    {
        std::cout << "\nDevice #" << p + 1 << "\n";
        memset(&pdinfo, 0, sizeof(pdinfo));

        if (GetPortableDeviceInfo(p, &pdinfo) == ERROR_SUCCESS)
        {
            std::wcout << L"  Manufacturer: " << pdinfo.manufacturer << L"\n";
            std::wcout << L"  Model:        " << pdinfo.model << L"\n";
        }
    }

    DEVICEINFO info;

    info.version = 1;
    GetDeviceInfo(0, &info);

    HANDLE h = OpenDevice(info.deviceName);
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

    pv2 = pv1;

    for (int i = 0; i < 120; i++)
    {
        printf("---\n");
        Sleep(500);

        RefreshPropertyList(h);
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

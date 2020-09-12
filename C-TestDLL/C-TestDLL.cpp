// C-TestDLL.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include "SonyMTPCamera.h"

PROPERTYVALUE*
FindProperty(DWORD id, PROPERTYVALUE* list, DWORD count)
{
    PROPERTYVALUE* value = nullptr;

    for (int i = 0; i < count && !value; i++)
    {
        if ((list + i)->id == id)
        {
            value = list + i;
        }
    }

    return value;
}

enum class CaptureStatus
{
    Created = 0x0000,
    Capturing = 0x0001,
    Failed = 0x0002,
    Cancelled = 0x0003,
    Complete = 0x0004,
    Starting = 0x8001,
    Reading = 0x8002,
    Processing = 0x8003,
};

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

    memset(&info, 0, sizeof(info));
    info.version = 1;
    GetDeviceInfo(0, &info);

    HANDLE h = OpenDevice(info.deviceName);

    // Try to close device, then reopen it
    CloseDevice(h);
    h = OpenDevice(info.deviceName);
    DWORD count = 0;
    IMAGEINFO iinfo;

    bool previewOnly = false;

    CAMERAINFO cameraInfo;

    memset(&cameraInfo, 0, sizeof(cameraInfo));
    GetCameraInfo(h, &cameraInfo, 0);

    while (1)
    {
        memset(&iinfo, 0, sizeof(IMAGEINFO));

        if (previewOnly)
        {
            iinfo.imageMode = 1;// 2;
            GetPreviewImage(h, &iinfo);
        }
        else
        {
            iinfo.duration = 0.3;
            iinfo.imageMode = 1;

            StartCapture(h, &iinfo);

            for (int i = 0; i < 100 && (iinfo.status != 4 && iinfo.status != 3 && iinfo.status != 2); i++)
            {
                GetCaptureStatus(h, &iinfo);
                Sleep(50);
            }
        }

        count++;
        printf("Capture #%d complete\n", count);

        if (iinfo.status == 4)
        {
            CoTaskMemFree(iinfo.data);
        }
    }
//
//    GetPropertyList(h, nullptr, &count);
//
//    PROPERTYVALUE* pv1 = new PROPERTYVALUE[count];
//    PROPERTYVALUE* pv2 = nullptr;
//
//    // Dump current state of all properties
//    RefreshPropertyList(h);
//    GetAllPropertyValues(h, pv1, &count);
//
//    for (int i = 0; i < count; i++)
//    {
//        PROPERTYVALUE* v = (pv1 + i);
//        PROPERTYDESCRIPTOR d;
//
//        GetPropertyDescriptor(h, v->id, &d);
//
//        printf("x%04x (%S), type = x%04x, flags = x%04x, value = x%04x (%S)\n", v->id, d.name, d.type, d.flags, v->value, v->text);
//    }
//
//    pv2 = pv1;
//
////    SetPropertyValue(h, 0x5004, 0x10);
////    RefreshPropertyList(h);
////    SetPropertyValue(h, 0xd22d, 1);
//    SetPropertyValue(h, 0xd2cb, 0x02);
//    SetPropertyValue(h, 0xd2cb, 0x01);
//    //    RefreshPropertyList(h);
////    RefreshPropertyList(h);
//    Sleep(5000);
//    SetPropertyValue(h, 0xd2cc, 0x02);
//    SetPropertyValue(h, 0xd2cc, 0x01);
//    //    SetPropertyValue(h, 0xd22d, 0);
////    SetPropertyValue(h, 0xd22f, 0x00);
////    SetPropertyValue(h, 0x5004, 0x10);
////    RefreshPropertyList(h);
////    Sleep(5000);
//
//    for (int i = 0; i < 120; i++)
//    {
//        printf("---\n");
//        Sleep(500);
//
//        RefreshPropertyList(h);
//        GetAllPropertyValues(h, pv1, &count);
//
//        if (pv2)
//        {
//            // Compare
//            for (int i = 0; i < count; i++)
//            {
//                PROPERTYVALUE* t1 = (pv1 + i);// *sizeof(PROPERTYVALUE));
//                PROPERTYVALUE* t2 = (pv2 + i);// *sizeof(PROPERTYVALUE));
//
//                if (t1->id == t2->id)
//                {
//                    if (t1->value != t2->value)
//                    {
//                        printf("Property x%04x value changed from x%04x to x%04x (%S -> %S)\n", t1->id, t2->value, t1->value, t2->text, t1->text);
//                    }
//                }
//                else
//                {
//                    // Expectation is same order for return data
//                    printf("Id's don't match! x%04x != x%04x\n", t1->id, t2->id);
//                }
//            }
//        }
//
//        pv2 = pv1;
//        pv1 = new PROPERTYVALUE[count];
//     }
}

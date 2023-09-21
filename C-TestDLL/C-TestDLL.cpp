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

void CompareBytes(BYTE* data1, DWORD size1, BYTE* data2, DWORD size2)
{
    if (size1 != size2)
    {
        printf("Cannot compare, sizes are different\n");

        return;
    }
    //else
    //{
    //    if (memcmp(data1, data2, size1) == 0)
    //    {
    //        printf("Unchanged\n");
    //    }
    //}

    // Do a custom dump
    for (int row = 0; row < size1; row += 16)
    {
        printf("%04x:  ", row);

        for (int col = 0; col < min(size1 - row, 16); col++)
        {
            printf("%02x ", data1[row + col]);

            if (col % 8 == 7)
            {
                printf(" ");
            }
        }

        for (int col = 0; col < min(size1 - row, 16); col++)
        {
            if (data1[row + col] == data2[row + col])
            {
                printf("   ");
            }
            else
            {
                printf("%02x ", data2[row + col]);
            }

            if (col % 8 == 7)
            {
                printf(" ");
            }
        }

        printf("\n");
    }
}

void HexDump(BYTE* data, DWORD size)
{
    for (int row = 0; row < size; row += 16)
    {
        printf("%04x:  ", row);

        for (int col = 0; col < min(size - row, 16); col++)
        {
            printf("%02x ", data[row + col]);

            if (col % 8 == 7)
            {
                printf(" ");
            }
        }

        printf("\n");
    }
}

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
    GetPortableDeviceInfo(0, &pdinfo);

    HANDLE h = OpenDevice(pdinfo.id);

    // Try to close device, then reopen it
//    CloseDevice(h);
//    h = OpenDevice(pdinfo.id);
    DWORD count = 0;
    IMAGEINFO iinfo;

    bool previewOnly = false;

    CAMERAINFO cameraInfo;

    memset(&cameraInfo, 0, sizeof(cameraInfo));
    GetCameraInfo(h, &cameraInfo, 0);
    GetPropertyList(h, nullptr, &count);

    DWORD* ids = new DWORD[count];
    GetPropertyList(h, ids, &count);

    PROPERTYVALUE* pv1 = new PROPERTYVALUE[count];
    PROPERTYVALUE* pv2 = nullptr;

    // Dump current state of all properties
    //RefreshPropertyList(h);
    //GetAllPropertyValues(h, pv1, &count);

    for (int i = 0; i < count; i++)
    {
//        PROPERTYVALUE* v = (pv1 + i);
        PROPERTYDESCRIPTOR d;

        GetPropertyDescriptor(h, ids[i], &d);

        printf("x%04x (%S), type = x%04x, flags = x%04x\n", ids[i], d.name, d.type, d.flags);
    }

    DWORD lastSize = 0;
    BYTE* lastMetaData = nullptr;;

    for (int j = 0; j < 10; j++)
    {
        memset(&iinfo, 0, sizeof(IMAGEINFO));

        if (previewOnly)
        {
            iinfo.imageMode = 2;
            GetPreviewImage(h, &iinfo);

            if (iinfo.metaDataSize)
            {
                printf("\n\n");

                if (lastMetaData)
                {
                    CompareBytes(lastMetaData, lastSize, iinfo.metaData, iinfo.metaDataSize);
                }
                else
                {
                    HexDump(iinfo.metaData, iinfo.metaDataSize);
                }

                lastSize = iinfo.metaDataSize;

                if (lastMetaData)
                {
                    delete[] lastMetaData;
                }

                lastMetaData = new BYTE[lastSize];
                memcpy(lastMetaData, iinfo.metaData, lastSize);
                CoTaskMemFree(iinfo.metaData);
//                CoTaskMemFree(iinfo.data);
            }

            Sleep(2000);
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

            PROPERTYVALUE v;

            GetSinglePropertyValue(h, 0xfffd, &v);
//            if (iinfo.data)
//            {
//                CoTaskMemFree(iinfo.data);
////                delete[] iinfo.data;
//            }
        }

        count++;
        printf("Capture #%d complete\n", count);

        if (iinfo.status == 4)
        {
            CoTaskMemFree(iinfo.data);
        }
    }

    CloseDevice(h);

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

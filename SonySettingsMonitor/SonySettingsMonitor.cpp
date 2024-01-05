// SonySettingsMonitor.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>

#include <iostream>
#include <Windows.h>
#include <conio.h>
#include <map>
#include <math.h>
#include "SonyMTPCamera.h"

constexpr int FOCUS_PROP = 0xd2d1;

constexpr int FOCUS_FULL_CLOSE = 0xfff9;
constexpr int FOCUS_BIG_CLOSE = 0xfffd;
constexpr int FOCUS_MEDIUM_CLOSE = 0xfffe;
constexpr int FOCUS_SMALL_CLOSE = 0xffff;
constexpr int FOCUS_SMALL_FAR = 0x0001;
constexpr int FOCUS_MEDIUM_FAR = 0x0002;
constexpr int FOCUS_BIG_FAR = 0x0003;
constexpr int FOCUS_FULL_FAR = 0x0007;

static void
scanFocus(HANDLE h, DWORD stepSize, int steps, bool slow)
{
    // First, set focus to infinite
    printf("Setting full near\n");
    TestFunc(h, FOCUS_FULL_CLOSE);
    SetPropertyValue(h, FOCUS_PROP, FOCUS_FULL_CLOSE);

    Sleep(slow ? 5000 : 1000);

    printf("Stepping in %d steps of %d\n", steps, stepSize);

    for (int i = 0; i < steps; i++)
    {
        printf("%d/%d of %d\n", i + 1, steps, stepSize);

        TestFunc(h, i);
        SetPropertyValue(h, FOCUS_PROP, stepSize);
        Sleep(150);
        if (slow)
        {
            printf("Press a key\n");
            if (_getch() != 32)
            {
                TestFunc(h, 9999);
            }
        }
    }

    printf("Sleeping before next run...\n");
    Sleep(5000);

    //TestFunc(h, FOCUS_FULL_FAR);
    //SetPropertyValue(h, FOCUS_PROP, FOCUS_FULL_FAR);

    //Sleep(1000);

    //for (int small_steps = 0; small_steps < 270; small_steps++)
    //{
    //    printf("%d - small step in\n", small_steps);
    //    TestFunc(h, small_steps);
    //    SetPropertyValue(h, FOCUS_PROP, FOCUS_SMALL_CLOSE);
    //    Sleep(100);
    //    //if (_getch() != 32)
    //    //{
    //    //    TestFunc(h, 9999);
    //    //}
    //}
}

const short min_position = 0;
const short max_position = 9999;
const double step_growth_rate = 2.53;
const unsigned short max_step_size = 7;

bool always_reset_to_infinite = true;

short current_position = -1;

std::map<unsigned short, short> steps;

static double calculate_step(unsigned short step_size)
{
    double scale = double(max_position) / pow(step_growth_rate, max_step_size - 1);

    return pow(step_growth_rate, step_size) * scale;
}

static void populate_sizes()
{
    for (int i = 0; i < max_step_size; i++)
    {
        steps[i + 1] = (short)calculate_step(i);
    }
}

static void move(HANDLE hCamera, short step)
{
    short diff = steps[abs(step)];

    printf("Moving focus by %d (current = %d, step_size = %d).. ", step, current_position, diff);
    SetPropertyValue(hCamera, FOCUS_PROP, step);

    if (abs(step) == max_step_size)
    {
        current_position = step < 0 ? min_position : max_position;
    }
    else
    {
        if (step < 0)
        {
            current_position -= diff;
        }
        else
        {
            current_position += diff;
        }

        if (current_position < min_position)
        {
            current_position = min_position;
        }

        if (current_position > max_position)
        {
            current_position = max_position;
        }
    }

    printf(" new position = %d\n", current_position);

    Sleep(200);
}

static short nearest_step(short size)
{
    size = abs(size);

    short best_id = 1;
    short best_diff = abs(size - steps[best_id]);

    for (short test_id = best_id + 1; test_id < max_step_size; test_id += 1)
    {
        short diff = abs(size - steps[test_id]);

        if (diff < best_diff)
        {
            best_id = test_id;
            best_diff = diff;
        }
    }

    return best_id;
}

static void set_focus(HANDLE hCamera, short position)
{
    if (current_position == -1 || always_reset_to_infinite)
    {
        move(hCamera, max_step_size);
    }

    while (abs(current_position - position) >= steps[1])
    {
        // Calculate steps to get from here to there
        short diff = position - current_position;

        // We could just find largest step LESS than desired position,
        // but it would be more efficient to find largest step CLOSEST
        // to desired position, then repeat
        short ideal_step = nearest_step(diff);

        if (diff < 0)
        {
            ideal_step = -ideal_step;
        }

        move(hCamera, ideal_step);
    }
}

static void
testFocus(HANDLE h)
{
//    populate_sizes();

    for (int t = 0; t < 10; t++)
    {
        DWORD pos = rand() % (max_position + 1);

        printf("About to test random move to %d\n", pos);
//        set_focus(h, pos);
        SetFocusPosition(h, &pos);
    }
}

static void
resetFocus(HANDLE h, int count)
{
    for (int i = 0; i < count; i++)
    {
        SetPropertyValue(h, FOCUS_PROP, 7);
        Sleep(500);
    }
}

static void
calcFocusRanges(HANDLE h)
{
    std::map<int, float> values;
    PROPERTYDESCRIPTOR pd;
    double growth = 2.0;

    GetPropertyDescriptor(h, FOCUS_PROP, &pd);

    printf("Scanning focus, largest step first!\nAfter that, assuming a > %.1fx growth rate\n\n", growth);

    for (int size = 7; size > 0; size--)
    {
        printf("\n-----\nSTEP SIZE %d\nResetting focus to far-right\n", size);
        resetFocus(h, size < 7 ? (int)values[7] + 2 : 25);
        Sleep(1000);

        float steps = 0.0;

        if (size < 7)
        {
            double pre = values[size + 1] * growth;

            printf("I think this size will require around %.2f steps\n", pre);

            // the pre value is very accurate, and we don't want to go over, so we'll trim say 5% off
            pre = pre * 0.95;

            // Move some portion of distance to assumed next step
            printf("Based on previous setting, pre-moving %d steps (so you don't have to)\n", (int)pre);

            for (int s = 0; s < (int)pre; s++)
            {
                printf("\r%d/%d...", s+1, (int)pre);
                SetPropertyValue(h, FOCUS_PROP, -size);
                Sleep(200);
                steps++;
            }
        }

        printf("\nGoing to repeatedly move in steps of size %d - press a key when ready to start...", size);
        _getch();
        printf("\n");

        do
        {
            steps++;
            SetPropertyValue(h, FOCUS_PROP, -size);
            Sleep(200);
            printf("\nStep %d... press 'Y' if focus is at left-hand side, else press something else", (int)steps);
        } while (toupper(_getch()) != 'Y');

        printf("\nIf the last move was only a partial move (i.e. %d.2) enter that here, otherwise enter %d : ", (int)(steps - 1), (int)steps);

        scanf_s("%f", &steps);
        values[size] = steps;

        if (size < 7)
        {
            // Recalculate growth
            double new_growth = steps / values[size + 1];
            double avg = (growth + new_growth) / 2;

            printf("Growth rate was %.2f, updating to %.2f\n", growth, avg);
            growth = avg;
        }
    }

    printf("Results:\n");

    for (int i = 1; i <= 7; i++)
    {
        printf("Step size %d - count %.1f\n", i, values[i]);
    }

    printf("\n\nPress 'X' or 'x' to exit: ");

    do
    {
    } while (toupper(_getch()) != 'X');

    //scanFocus(h, 7, 1, false);  // All the way - 1 step end-to-end
    //scanFocus(h, 6, 3, false);  // 45% - not super useful - 3 steps end-to-end
    //scanFocus(h, 5, 6, false);  // 16% - not super useful - 6 steps end-to-end
    //scanFocus(h, 4, 16, false);  // 6.25% - not super useful - 16 steps end-to-end
    //scanFocus(h, 3, 41, false);   // 41 steps end-to-end
    //scanFocus(h, 2, 103, false);   // sony, wtf, 103 steps?
    //scanFocus(h, 1, 264, false);
}

static void testFocusAPI(HANDLE h)
{
    DWORD count = GetLensCount();

    printf("There appear to be %d lenses available to choose from\n", count);

    LENSINFO lense;

    // Test get info method
    int infoCount = (int)count;

    for (int i = 0; i < infoCount; i++)
    {
        GetLensInfo(i, &lense);

        if (std::wstring(lense.lensPath) == L"Lenses\\Sigma\\c017_16_14")
        {
            DWORD position = 500;

            SetAttachedLens(h, lense.id);
            SetFocusPosition(h, &position);
        }
        //if (i == 0)
        //{
        //    DWORD position;

        //    position = 0;

        //    // Test focus
        //    SetFocusPosition(h, &position);

        //    position = 300;

        //    SetFocusPosition(h, &position);

        //    position = 600;

        //    SetFocusPosition(h, &position);

        //    position = GetFocusLimit(h);

        //    SetFocusPosition(h, &position);
        //}
    }
}

static void
watchSettings(HANDLE h, bool loop)
{
    DWORD count = 0;
    IMAGEINFO iinfo{};
    
    GetPropertyList(h, nullptr, &count);

    PROPERTYVALUE* pv1 = new PROPERTYVALUE[count];
    PROPERTYVALUE* pv2 = nullptr;

    // Dump current state of all properties
//    RefreshPropertyList(h);
    GetAllPropertyValues(h, pv1, &count);

    for (int i = 0; i < (int)count; i++)
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
                for (int i = 0; i < (int)count; i++)
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

            // Clean up text
            for (int i = 0; i < (int)count; i++)
            {
                PROPERTYVALUE* t2 = (pv2 + i);

                if (t2->text)
                {
                    CoTaskMemFree(t2->text);
                }
            }

            pv2 = pv1;
            pv1 = new PROPERTYVALUE[count];
        }
    }
}

static void
dumpExposureOptions(HANDLE h)
{
    PROPERTYDESCRIPTOR descriptor;
    PROPERTYVALUEOPTION option;
    DWORD count = 0;

//    GetPropertyValueOption(h, 0xd24e, &option, 0);

    // Ask how many options there are
    HRESULT hr = GetPropertyDescriptor(h, 0xffff, &descriptor);

    count = descriptor.valueCount;

    for (int i = 0; i < (int)count; i++)
    {
        hr = GetPropertyValueOption(h, 0xffff, &option, i);

        if (hr != ERROR_SUCCESS)
        {
            std::cerr << "Error reading number of options " << hr;
        }

        std::wcout << "Option #" << i + 1 << " - '" << option.name << "' (" << option.value << ")" << std::endl;
    }
}

static void
testExposure(HANDLE h)
{
    // Device is open
    // Get exposure time info
    PROPERTYDESCRIPTOR descriptor;
    constexpr int SHUTTERSPEED = 0xd20d;
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

    CoTaskMemFree(value.text);
    hr = GetSinglePropertyValue(h, SHUTTERSPEED, &value);

    std::wcout << "Shutter speed set to '" << value.text << "' (" << value.value << ")" << std::endl;

    std::cout << "Setting shutter speed to 1/100..." << std::endl;

    SetExposureTime(h, (float)0.01, &value);

    if (hr != ERROR_SUCCESS)
    {
        std::cerr << "Error setting property value " << hr;
    }

    CoTaskMemFree(value.text);
    hr = GetSinglePropertyValue(h, SHUTTERSPEED, &value);

    std::wcout << "Shutter speed now set to '" << value.text << "' (" << value.value << ")" << std::endl;

    std::cout << "Setting shutter speed to 0.75..." << std::endl;

    SetExposureTime(h, 0.75, &value);

    if (hr != ERROR_SUCCESS)
    {
        std::cerr << "Error setting property value " << hr;
    }

    CoTaskMemFree(value.text);
    hr = GetSinglePropertyValue(h, SHUTTERSPEED, &value);

    std::wcout << "Shutter speed now set to '" << value.text << "' (" << value.value << ")" << std::endl;

    std::cout << "Setting shutter speed to 1/3..." << std::endl;

    SetExposureTime(h, (float)0.333, &value);

    if (hr != ERROR_SUCCESS)
    {
        std::cerr << "Error setting property value " << hr;
    }

    CoTaskMemFree(value.text);
    hr = GetSinglePropertyValue(h, SHUTTERSPEED, &value);

    std::wcout << "Shutter speed now set to '" << value.text << "' (" << value.value << ")" << std::endl;

    std::cout << "Setting shutter speed to 1/23..." << std::endl;

    SetExposureTime(h, (float)(1.0/23.0), &value);

    if (hr != ERROR_SUCCESS)
    {
        std::cerr << "Error setting property value " << hr;
    }

    CoTaskMemFree(value.text);
    hr = GetSinglePropertyValue(h, SHUTTERSPEED, &value);

    std::wcout << "Shutter speed now set to '" << value.text << "' (" << value.value << ")" << std::endl;

    std::cout << "Setting shutter speed to 1/4001..." << std::endl;

    SetExposureTime(h, (float)(1.0/4001.0), &value);

    if (hr != ERROR_SUCCESS)
    {
        std::cerr << "Error setting property value " << hr;
    }

    CoTaskMemFree(value.text);
    hr = GetSinglePropertyValue(h, SHUTTERSPEED, &value);

    std::wcout << "Shutter speed now set to '" << value.text << "' (" << value.value << ")" << std::endl;

    std::cout << "Setting shutter speed to 32..." << std::endl;

    SetExposureTime(h, (float)32.0, &value);

    if (hr != ERROR_SUCCESS)
    {
        std::cerr << "Error setting property value " << hr;
    }

    CoTaskMemFree(value.text);
    hr = GetSinglePropertyValue(h, SHUTTERSPEED, &value);

    std::wcout << "Shutter speed now set to '" << value.text << "' (" << value.value << ")" << std::endl;

    std::cout << "Setting shutter speed to BULB..." << std::endl;

    SetExposureTime(h, (float)(0), &value);

    if (hr != ERROR_SUCCESS)
    {
        std::cerr << "Error setting property value " << hr;
    }

    CoTaskMemFree(value.text);
    hr = GetSinglePropertyValue(h, SHUTTERSPEED, &value);

    std::wcout << "Shutter speed now set to '" << value.text << "' (" << value.value << ")" << std::endl;

    CoTaskMemFree(value.text);
    std::cout << "Got it";
}

static void
testSetISO(HANDLE h)
{
    SetPropertyValue(h, 0xd21e, 640);
}

int main()
{
//    HRESULT comhr = CoInitialize(nullptr);

    int portableDeviceCount = GetSupportedDeviceCount();

    std::cout << portableDeviceCount << " portable devices found\n\n";
    PORTABLEDEVICEINFO pdinfo;
    std::wstring firstDevice;

    for (int p = 0; p < portableDeviceCount; p++)
    {
        std::cout << "\nDevice #" << p + 1 << "\n";
        memset(&pdinfo, 0, sizeof(pdinfo));

        if (GetSupportedDeviceInfo(p, &pdinfo) == ERROR_SUCCESS)
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
//    testExposure(h);
// 
//     testFocus(h);
//    testFocusAPI(h);
    calcFocusRanges(h);
//    testSetISO(h);
//    dumpExposureOptions(h);

    CloseDevice(h);
//    CoUninitialize();
//    _CrtDumpMemoryLeaks();
    printf("Out\n");
}

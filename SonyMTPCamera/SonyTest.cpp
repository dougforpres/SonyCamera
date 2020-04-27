#include "pch.h"
#include <iostream>
#include <windows.h>
#include <time.h>
#include <map>

#include "DeviceManager.h"
#include "Device.h"
#include "SonyILCE6400.h"

int main()
{
    DeviceManager* manager = new DeviceManager();

    std::list<Device*> devices = manager->GetDevices();

    for (std::list<Device*>::iterator it = devices.begin(); it != devices.end(); it++)
    {
        Device* d = *it;

        printf("    %ws\n", d->GetFriendlyName().c_str());
        printf("    %ws\n", d->GetManufacturer().c_str());
        printf("    %ws\n", d->GetDescription().c_str());
        printf("    %ws\n", d->GetId().c_str());

        SonyILCE6400* camera = new SonyILCE6400(d);

        camera->Initialize();

        CameraSettings *cs = nullptr, * last = nullptr;
        DWORD sameCount = 0;
        bool isDiff = false;
        char tbuff[64];

//        camera->TakePhoto();

        int action = ' ';

        do
        {
            cs = new CameraSettings(*camera->GetSettings());

            if (cs != nullptr)
            {
                isDiff = false;

                if (last != nullptr)
                {
                    if (cs->_properties.size() != last->_properties.size())
                    {
                        printf("Lengths are different! %d vs %d\n", last->_properties.size(), cs->_properties.size());
                    }

                    for (CAMERAPROP::iterator it = cs->_properties.begin(); it != cs->_properties.end(); it++)
                    {
                        Property id = (*it).first;
                        CAMERAPROP::iterator lastIt = last->_properties.find(id);

                        if (lastIt != last->_properties.end())
                        {
                            // Its in there
                            if (!(*it).second->equals(*(*lastIt).second))
                            {
                                CameraProperty* p = (*it).second;
                                CameraProperty* p1 = (*lastIt).second;
                                wprintf(L"**CHANGED** x%04x - %s = %s  ==> x%04x - %s = %s\n", p1->GetId(), p1->GetName().c_str(), p1->AsString().c_str(), p->GetId(), p->GetName().c_str(), p->AsString().c_str());

                                isDiff = true;
                            }
                        }
                    }
                }
                else
                {
                    printf("First time thru\n");
                }

                if (isDiff || last == nullptr)
                {
                    sameCount = 0;
                    time_t t = time(NULL);
                    ctime_s(tbuff, 64, &t);
                    printf("%s\n", tbuff);
                    std::list<CameraProperty*> properties = cs->GetProperties();

                    for (std::list<CameraProperty*>::iterator it = properties.begin(); it != properties.end(); it++)
                    {
                        CameraProperty *p = *it;
                        std::wstring name = p->GetName();

             //           if (!name.empty())
                        {
                            wprintf(L"x%04x - %-32s = %s\n", p->GetId(), name.c_str(), p->AsString().c_str());
                        }
                    }
                    printf("\n-------------------------------------\n");
                }
                else
                {
                    sameCount++;
                }

                CameraProperty* p = cs->GetProperty(Property::PhotoBufferStatus);

                if (p->AsString() == L"Ready")
                {
                    //  ffffc001 = image buffer
                    //  ffffc002 = preview
                    ObjectInfo* info = camera->GetPreviewImageInfo(0xffffc001);
                    Image* image = camera->GetImage(0xffffc001);
                    std::wstring filename = std::wstring(L"C:\\Users\\dougf\\") + info->GetFilename();

                    wprintf(L"Writing image to: %s\n", filename.c_str());
                    image->WriteToFile(filename);
                    delete image;
                    delete info;
                }
            }
            
            if (last)
            {
                delete last;
            }

            last = cs;
            action = getchar();

            if (action == 'p')
            {
                camera->TakePhoto();
            }
        } while (action != 'x');// sameCount < 1500);// getchar() != 'x');

        if (last)
        {
            delete last;
        }

        printf("Stopping...\n");
        delete camera;
    }
}
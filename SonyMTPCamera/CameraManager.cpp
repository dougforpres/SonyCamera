#include "pch.h"
#include "CameraManager.h"
#include "SonyCamera.h"
#include "Logger.h"
#include "Registry.h"

CameraManager::CameraManager()
{

}

CameraManager::~CameraManager()
{
    // TODO Clean out camera map and close each if required
}

void
CameraManager::SetupSupportedDevices()
{
    LOGINFO(L"In: CameraManager::SetupSupportedDevices()");

    // Set up defaults for cameras
    registry.Open();

    // a5000
    std::wstring key = L"Cameras\\Sony Corporation\\ILCE-5000";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"Sensor Name", L"CMOS");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 4.22);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 4.22);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 5472);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 3656);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 0);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 0);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 0);

    // a6400
    key = L"Cameras\\Sony Corporation\\ILCE-6400";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"Sensor Name", L"EXMOR");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 3.91);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 3.91);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 6024);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 4024);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 1024);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 680);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 1);

    // a6000
    key = L"Cameras\\Sony Corporation\\ILCE-6000";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"Sensor Name", L"CMOS");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 3.91);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 3.91);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 6024);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 4024);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 1024);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 680);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 0);

    // a6300
    key = L"Cameras\\Sony Corporation\\ILCE-6300";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"Sensor Name", L"EXMOR");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 3.91);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 3.91);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 6024);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 4024);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 1024);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 680);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 1);

    // a6500
    key = L"Cameras\\Sony Corporation\\ILCE-6500";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"Sensor Name", L"EXMOR");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 3.91);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 3.91);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 6024);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 4024);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 1024);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 680);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 1);

    // a7
    key = L"Cameras\\Sony Corporation\\ILCE-7";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"Sensor Name", L"CMOS");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 5.95);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 5.95);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 6024);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 4024);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 0);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 0);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 0);

    // a7 II
    key = L"Cameras\\Sony Corporation\\ILCE-7M2";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"Sensor Name", L"CMOS");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 5.95);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 5.95);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 6024);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 4024);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 1024);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 680);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 1);

    // a7 III
    key = L"Cameras\\Sony Corporation\\ILCE-7M3";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"Sensor Name", L"CMOS");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 5.95);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 5.95);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 6024);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 4024);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 1024);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 680);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 1);

    // a7R
    key = L"Cameras\\Sony Corporation\\ILCE-7R";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"Sensor Name", L"CMOS");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 4.86);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 4.86);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 7362);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 4920);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 1024);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 680);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 1);

    // a7RM2
    key = L"Cameras\\Sony Corporation\\ILCE-7RM2";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"Sensor Name", L"CMOS");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 4.86);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 4.86);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 7968);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 5320);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 1024);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 680);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 1);

    // a7RM3
    key = L"Cameras\\Sony Corporation\\ILCE-7RM3";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"Sensor Name", L"CMOS");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 4.86);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 4.86);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 7968);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 5320);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 1024);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 680);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 1);

    // a7R IV
    key = L"Cameras\\Sony Corporation\\ILCE-7RM4";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"Sensor Name", L"CMOS");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 3.73);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 3.72);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 9600);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 6376);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 1024);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 680);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 1);

    // a7S
    key = L"Cameras\\Sony Corporation\\ILCE-7S";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"Sensor Name", L"CMOS");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 8.31);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 8.31);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 4256);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 2848);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 1024);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 680);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 1);

    // a7S II
    key = L"Cameras\\Sony Corporation\\ILCE-7SM2";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"Sensor Name", L"CMOS");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 8.31);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 8.31);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 4256);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 2848);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 1024);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 680);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 1);

    key = L"Cameras\\Sony Corporation\\ILCA-77M2";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"Sensor Name", L"CMOS");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 3.91);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 3.91);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 6024);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 4024);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 1024);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 680);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 1);

    registry.Close();

    LOGINFO(L"Out: CameraManager::SetupSupportedDevices()");
}

bool
CameraManager::IsSupportedDevice(Device* device)
{
    bool result = false;
    std::wostringstream builder;

    builder << L"Cameras\\" << device->GetManufacturer() << "\\" << device->GetFriendlyName();

    registry.Open();

    result = registry.DoesKeyExist(builder.str().c_str());

    registry.Close();

    LOGINFO(L"CameraManager::IsSupportedDevice(%s) = %d", device->GetFriendlyName().c_str(), result);

    return result;
}

HANDLE
CameraManager::CreateCamera(Device* device, DWORD flags)
{
    LOGTRACE(L"In: CameraManager::CreateCamera(device='%s', flags=x%08x)", device->GetFriendlyName().c_str(), flags);
    Camera* camera = nullptr;
    HANDLE hResult = INVALID_HANDLE_VALUE;

    // See if we already have this device
    for (std::unordered_map<HANDLE, Camera*>::iterator it = m_cameraMap.begin(); it != m_cameraMap.end() && hResult == INVALID_HANDLE_VALUE; it++)
    {
        if ((*it).second->GetId() == device->GetId())
        {
            camera = (*it).second;
            LOGTRACE(L"CameraManager::CreateCamera: Found existing camera");
            hResult = camera->Open();
        }
    }

    if (hResult == INVALID_HANDLE_VALUE)
    {
        // As long as the camera entry is in the registry, we're good to go
        std::wostringstream builder;

        LOGTRACE(L"CameraManager::CreateCamera: Not an existing camera, looking to see if '%s' is supported", device->GetFriendlyName().c_str());

        if ((flags & OPEN_OVERRIDE) || IsSupportedDevice(device))
        {
            // Make a new camera...
            LOGTRACE(L"CameraManager::CreateCamera: Creating new camera object for '%s'", device->GetFriendlyName().c_str());

            camera = new SonyCamera(device);

            hResult = camera->Open();

            m_cameraMap.insert(std::pair<HANDLE, Camera*>(hResult, camera));
        }
        else
        {
            LOGWARN(L"CameraManager::CreateCamera: Couldn't find what camera to make for '%s'", device->GetFriendlyName().c_str());
        }
    }

    if (camera)
    {
        camera->GetDevice()->StartNotifications();
    }

    LOGTRACE(L"Out: CameraManager::CreateCamera - returning x%08x", hResult);

    return hResult;
}

void
CameraManager::AddCamera(HANDLE hCamera, Camera* camera)
{
    m_cameraMap.insert(std::pair<HANDLE, Camera*>(hCamera, camera));
}

void
CameraManager::RemoveCamera(HANDLE hCamera)
{
    m_cameraMap.erase(hCamera);
}

Camera*
CameraManager::GetCameraForHandle(HANDLE hCamera)
{
    std::unordered_map<HANDLE, Camera*>::iterator it = m_cameraMap.find(hCamera);

    if (it != m_cameraMap.end())
    {
        return (*it).second;
    }
    else
    {
        LOGWARN(L"Unable to find camera for handle x%08x", hCamera);
        return nullptr;
    }
}

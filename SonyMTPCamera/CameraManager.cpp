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

    // a5100
    key = L"Cameras\\Sony Corporation\\ILCE-5100";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"Sensor Name", L"CMOS");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 3.91);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 3.91);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 6024);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 4024);
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

    // a6600
    key = L"Cameras\\Sony Corporation\\ILCE-6600";

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

    // SLT Alpha 58
    key = L"Cameras\\Sony Corporation\\SLT-A58";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"Sensor Name", L"CMOS");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 4.27);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 4.27);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 5472);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 3656);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 0);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 0);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 0);

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
            LOGTRACE(L"CameraManager::CreateCamera: Found existing camera with handle x%08x", (*it).first);
            hResult = CompatibleHandle(camera->Open());
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

            hResult = AddCamera(camera->Open(), camera);
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

    LOGTRACE(L"Out: CameraManager::CreateCamera - returning x%p", hResult);

    return hResult;
}

HANDLE
CameraManager::AddCamera(HANDLE hCamera, Camera* camera)
{
    LOGTRACE(L"In: CameraManager::AddCamera(x%p, x%p)", hCamera, camera);

    HANDLE shortHandle = CompatibleHandle(hCamera);
    // In 64-bit windows we could get a > 32-bit handle.  The ASCOM code is expecting a 32-bit value.
    // According to MSDN, only the bottom 32-bits are valid, so we can trim the top 32-bits
    m_cameraMap.insert(std::pair<HANDLE, Camera*>(shortHandle, camera));

    LOGTRACE(L"Out: CameraManager::AddCamera(x%p, x%p), added handle x%p", hCamera, camera, shortHandle);

    return shortHandle;
}

HANDLE
CameraManager::CompatibleHandle(HANDLE handle)
{
#if _WIN64
    uint64_t temp = (uint64_t)handle & 0xffffffff;

    LOGTRACE(L"CameraManager::CompatibleHandle(x%p) = x%p", handle, temp);

    return (HANDLE)temp;
#else
    return handle;
#endif
}

void
CameraManager::RemoveCamera(HANDLE hCamera)
{
    LOGTRACE(L"In: CameraManager::RemoveCamera(x%p)", hCamera);

    m_cameraMap.erase(hCamera);

    LOGTRACE(L"Out: CameraManager::RemoveCamera(x%p)", hCamera);
}

Camera*
CameraManager::GetCameraForHandle(HANDLE hCamera)
{
    hCamera = CompatibleHandle(hCamera);

    std::unordered_map<HANDLE, Camera*>::iterator it = m_cameraMap.find(hCamera);

    if (it != m_cameraMap.end())
    {
        return (*it).second;
    }
    else
    {
        LOGWARN(L"Unable to find camera for handle x%p", hCamera);
        LOGWARN(L"I have %d cameras in my list", m_cameraMap.size());

        for (it = m_cameraMap.begin(); it != m_cameraMap.end(); it++)
        {
            LOGWARN(L"-- got a x%p with name %s", (it->first), (it->second)->GetDeviceInfo()->GetManufacturer().c_str());
        }

        return nullptr;
    }
}

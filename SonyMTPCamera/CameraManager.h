#pragma once
#include <list>
#include <unordered_map>
#include "Camera.h"
#include "Device.h"

#define OPEN_OVERRIDE 0x00000001

class CameraManager
{
public:
    CameraManager();
    ~CameraManager();
    HANDLE CreateCamera(Device* device, DWORD flags);
    void AddCamera(HANDLE hCamera, Camera* camera);
    void RemoveCamera(HANDLE hCamera);
    Camera* GetCameraForHandle(HANDLE hCamera);

    static void SetupSupportedDevices();
    static bool IsSupportedDevice(Device* device);

private:
    std::unordered_map<HANDLE, Camera*> m_cameraMap;
};

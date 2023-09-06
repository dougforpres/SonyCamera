#pragma once
#include "pch.h"
#include "SonyMTPCamera.h"
#include "Device.h"
#include "CameraSettings.h"
#include "DeviceInfo.h"
#include "Image.h"
#include "CameraSupportedProperties.h"
#include "Lockable.h"
#include "CameraWorker.h"

class CameraTask;
class CameraWorker;
class TakePhotoTask;
class DownloadAndProcessImageTask;
class TakePhotoTaskParams;

// Latest full-size image taken by camera
#define FULL_IMAGE                      0xffffc001

// Latest live-preview
#define PREVIEW_IMAGE                   0xffffc002

// Standard Commands
#define COMMAND_GET_DEVICE_INFO         0x1001
#define COMMAND_OPEN_SESSION            0x1002
#define COMMAND_CLOSE_SESSION           0x1003
#define COMMAND_GET_STORAGE_IDS         0x1004
#define COMMAND_GET_STORAGE_INFO        0x1005
#define COMMAND_GET_NUM_OBJECTS         0x1006
#define COMMAND_GET_OBJECT_HANDLES      0x1007
#define COMMAND_GET_OBJECT_INFO         0x1008
#define COMMAND_GET_OBJECT              0x1009
#define COMMAND_GET_THUMB               0x100a

typedef std::unordered_map<Property, PropertyInfo> PropertyInfoMap;

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

enum class ImageBufferStatus
{
    ImageNotReady,
    ImageReady,
};

class Camera : public Lockable
{
public:
    Camera(Device* device);
    ~Camera();

    const std::wstring GetId() const;
    HANDLE Open();
    bool Close();

    virtual bool RefreshDeviceInfo();
    const DeviceInfo GetDeviceInfo() const;
    virtual bool SetProperty(const Property id, PropertyValue* value) = 0;

    Device* GetDevice();
    virtual bool Initialize() = 0;
    virtual Image* GetImage(DWORD imageId);
    CameraSettings* GetSettings() const;
    CameraProperty* GetProperty(Property id) const;
    virtual bool RefreshSettings() = 0;
    void LoadFakeProperties(CameraSettings* settings);
//    virtual const PropertyInfoMap GetSupportedProperties() const;

    bool StartCapture(double duration, OutputMode outputMode, DWORD flags);
    const CaptureStatus GetCaptureStatus();
    void SetCaptureStatus(CaptureStatus status);
    Image* GetCapturedImage();
    bool CancelCapture();
    void CleanupCapture();
//    bool RunTask(CameraTask* task);
    bool IsInitialized() const;
    CameraWorker* GetWorker();
    void OnPropertiesUpdated();
    void OnImageBufferStatus(ImageBufferStatus status);

    Lockable settingsLock;

protected:
    void DoRefreshProperties();
    void SetInitialized(bool value);
    virtual ObjectInfo* GetImageInfo(DWORD imageId);
    bool ProcessDeviceInfoOverrides(DeviceInfo& deviceInfo);

    DeviceInfo m_deviceInfo;
    Device* m_device = nullptr;
    CameraSettings* m_settings;
    CameraSupportedProperties m_supportedProperties;
    TakePhotoTask* m_takePhotoTask = nullptr;
    DownloadAndProcessImageTask* m_downloadAndProcessImageTask = nullptr;
    HANDLE m_hBusyMutex = INVALID_HANDLE_VALUE;
    TakePhotoTaskParams* m_takePhotoTaskParams = nullptr;
    CaptureStatus m_captureStatus = CaptureStatus::Created;

private:
    bool m_shutdown = false;
    CameraWorker* pWorker = nullptr;
    bool initialized = false;
};

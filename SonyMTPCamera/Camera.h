#pragma once
#include "pch.h"
#include "SonyMTPCamera.h"
#include "Device.h"
#include "CameraSettings.h"
#include "DeviceInfo.h"
#include "Image.h"
#include "CameraSupportedProperties.h"
#include "Lockable.h"
//#include "CaptureThread.h"

// Special object handles for retrieval of images

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

typedef std::unordered_map<Property, PropertyInfo*> PropertyInfoMap;

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

class Camera : public Lockable
{
public:
    class CaptureThread
    {
    public:
        CaptureThread(Camera* camera, OutputMode outputMode);
        ~CaptureThread();

        bool StartCapture(double duration);
        bool CancelCapture();
        CaptureStatus GetStatus();
        Image* GetImage();
        double GetDuration();
        static DWORD WINAPI _run(LPVOID lpParameter);
        DWORD Run();

    private:
        void SetStatus(CaptureStatus status);
        bool ImageReady(CameraSettings* settings);

        OutputMode m_outputMode;
        Camera* m_camera = nullptr;
        Image* m_image = nullptr;
        HANDLE m_hThread = INVALID_HANDLE_VALUE;
        HANDLE m_hWakeupEvent = INVALID_HANDLE_VALUE;
        HANDLE m_hMutex = INVALID_HANDLE_VALUE;
        double m_duration = 0.0;
        DWORD m_threadId = 0;
        CaptureStatus m_status = CaptureStatus::Created;
        bool m_cancelled = false;
    };

    Camera(Device* device);
    ~Camera();

    const std::wstring GetId();
    HANDLE Open();
    bool Close();

    virtual DeviceInfo* GetDeviceInfo(bool refresh);
    virtual bool SetProperty(Property id, PropertyValue* value) = 0;

    Device* GetDevice();
    virtual bool Initialize() = 0;
    virtual Image* GetImage(DWORD imageId);
    virtual CameraSettings* GetSettings(bool refresh) = 0;
    void LoadFakeProperties(CameraSettings* settings);
    virtual PropertyInfoMap GetSupportedProperties();

    bool StartCapture(double duration, OutputMode outputMode, DWORD flags);
    CaptureStatus GetCaptureStatus();
    Image* GetCapturedImage();
    bool CancelCapture();
    void CleanupCapture();

    static DWORD WINAPI _runHandlerThread(LPVOID lpParameter);

    Lockable settingsLock;

protected:
    virtual ObjectInfo* GetImageInfo(DWORD imageId);
    bool ProcessDeviceInfoOverrides();

    DeviceInfo* m_deviceInfo = nullptr;
    Device* m_device = nullptr;
    CameraSettings* m_settings = nullptr;
    CameraSupportedProperties* m_supportedProperties = nullptr;
    CaptureThread* m_captureThread = nullptr;

private:
    DWORD RunHandlerThread();

    HANDLE m_hHandlerThread = INVALID_HANDLE_VALUE;
    HANDLE m_hHandlerMutex = INVALID_HANDLE_VALUE;
    HANDLE m_hBusyMutex = INVALID_HANDLE_VALUE;
    HANDLE m_hWakeEvent = INVALID_HANDLE_VALUE;
//    DWORD m_handlerThreadId = 0;
    bool m_shutdown = false;
};

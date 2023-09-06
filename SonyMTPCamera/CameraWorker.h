#pragma once
#include "pch.h"
#include "Camera.h"
#include <queue>
#include "CameraState.h"
#include "CameraTask.h"

#define WORKER_THREAD_WAIT_FOR_EXIT 1000
#define WORKER_THREAD_WAIT_FOR_EXIT_RETRIES 5

class Camera;

// When camera state changes, an callback is made to the worker thread so it can do its thing.
// Only one worker can be active at a time
class CameraWorker
{
public:
    CameraWorker(Camera* pCamera);
    ~CameraWorker();

    void OnCameraStateChange(CameraState newState);
    bool QueueTask(CameraTask* task, HANDLE waiter);

    static DWORD WINAPI _run(LPVOID lpParameter);
    DWORD Run();

protected:

private:
    HANDLE mutexHandle = INVALID_HANDLE_VALUE;
    HANDLE threadHandle = INVALID_HANDLE_VALUE;
    DWORD threadId = 0;
    Camera* camera = nullptr;
    CameraState lastCameraState = CameraState::Null;
    bool stopRequested = false;
    std::list<std::pair<CameraTask*, HANDLE>> queue;
};

// Gets current properties from camera
class 
    RefreshPropertiesTask : public CameraTask
{
public:
    RefreshPropertiesTask();
};

// Gets current deviceinfo
class RefreshDeviceInfoTask : public CameraTask
{
public:
    RefreshDeviceInfoTask();
};

// Gets preview image from camera
class GetPreviewTask : public CameraTask
{
public:
    GetPreviewTask();

    Image* GetImage();
};

class InitializeCameraTask : public CameraTask
{
public:
    InitializeCameraTask();
};

class OpenCameraTask : public CameraTask
{
public:
    OpenCameraTask();

    HANDLE GetHandle();
};

class CloseCameraTask : public CameraTask
{
public:
    CloseCameraTask(HANDLE hCamera);

    bool Closed();
};

class SetPropertyTaskParams
{
public:
    SetPropertyTaskParams(Property id, PropertyValue* value);

    Property id;
    PropertyValue* value = nullptr;
};

class SetPropertyTask : public CameraTask
{
public:
    SetPropertyTask(SetPropertyTaskParams* params);
};

class TakePhotoTaskParams
{
public:
    TakePhotoTaskParams(double exposureTime, OutputMode mode);

    double exposureTime;
    OutputMode mode;
};

class TakePhotoTask : public CameraTask
{
public:
    TakePhotoTask(TakePhotoTaskParams* params);
};

class DownloadAndProcessImageTask : public CameraTask
{
public:
    DownloadAndProcessImageTask(TakePhotoTaskParams* params);

    Image* GetImage();
};
#pragma once
#include "pch.h"
#include "CameraState.h"
#include <list>
#include <unordered_map>
#include "CameraWorkerFunctions.h"
#include "CameraTaskInfo.h"
#include "Camera.h"

constexpr auto CAMERA_TASK_NOT_STARTED = -1;
constexpr auto CAMERA_TASK_COMPLETE = -2;

class Camera;
class CameraTask;

typedef StateResult(*StateHandler)(CameraTaskInfo& info);

typedef struct {
    int id;
    std::wstring name;
    StateHandler handler;
    int successId;
    int failId;
    int cancelId;
    int maxRetries;
    bool isTerminal;
    bool isSuccess;
    bool isActive;
    int retryCount;
    StateResult lastResult;
} StateStepInfo;

typedef struct
{
    int userStatus;
    int currentStep;
    std::wstring currentStepName;
} CameraTaskStatus;

class CameraTask
{
public:
    CameraTask(std::wstring name);
    ~CameraTask();

    bool Step(CameraTaskInfo& info);
    void SetResult(void* result);
    void* GetResult();
    void SetParam1(void* param);
    void* GetParam1();
    void SetParam2(void* param);
    void* GetParam2();
    void SetUserStatus(int userStatus);
    bool Run(Camera* camera);
    std::wstring Name();
    bool SupportsCameraState(CameraState state);
    void Start(Camera* camera);
    bool QueueAndForget(Camera* camera);
    DWORD Cancel(DWORD timeout);
    DWORD WaitFor(DWORD timeout);
    HANDLE GetCancelEventHandle() const;
    bool IsCancelled() const;
    bool IsComplete() const;
    CameraTaskStatus GetTaskStatus();

protected:
    void SetSteps(StateStepInfo steps[]);
    bool AddStep(StateStepInfo stepInfo);

    bool success = false;
    std::list<CameraState> runStates;

private:
    std::wstring name;
    std::unordered_map<int, StateStepInfo> steps;
    void* result = nullptr;
    CameraTaskStatus status;
    void* param1 = nullptr;
    void* param2 = nullptr;
    HANDLE waiter = INVALID_HANDLE_VALUE;
    HANDLE cancel = INVALID_HANDLE_VALUE;
    bool cancelled = false;
};


// Gets current properties from camera
class
    RefreshPropertiesTask : public CameraTask
{
public:
    RefreshPropertiesTask(bool refreshFakePropertiesToo);
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
    SetPropertyTaskParams(Property id, PropertyValue& value);

    Property id;
    PropertyValue value;
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
    ~DownloadAndProcessImageTask();

    Image* GetImage();
};
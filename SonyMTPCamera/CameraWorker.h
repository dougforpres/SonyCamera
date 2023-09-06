#pragma once
#include "pch.h"
#include "Camera.h"
#include <queue>

#define WORKER_THREAD_WAIT_FOR_EXIT 1000
#define WORKER_THREAD_WAIT_FOR_EXIT_RETRIES 5

#define WORKER_TASK_NOT_STARTED -1
#define WORKER_TASK_COMPLETE -2

// Null -> Disconnected -> Connecting -> Connected -> [Idle -> Busy]* .... -> Disconnecting -> Disconnected
enum class CameraState {
    Null,           // Initial State
    Disconnected,   // Constructor/Disconnect would set this
    Connecting,     // Connect method would set this
    Connected,      // Connect method would set this
    Disconnecting,  // Disconnect method would set this
};

enum class StateResult
{
    Success,
    Fail,
    Cancel,
    Retry,
};

class Camera;
class CameraTask;

typedef struct {
    Camera* pCamera;
    CameraTask* pTask;
} CameraTaskInfo;

typedef StateResult (*StateHandler)(CameraTaskInfo& info);

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
    bool Run(Camera *camera);
    std::wstring Name();
    bool SupportsCameraState(CameraState state);
    void Start(Camera* camera);
    bool QueueAndForget(Camera* camera);
    DWORD Cancel(DWORD timeout);
    DWORD WaitFor(DWORD timeout);
    HANDLE GetCancelEventHandle();
    bool IsCancelled();
    bool IsComplete();
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
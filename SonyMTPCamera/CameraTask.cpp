#include "pch.h"
#include "CameraTask.h"
#include "CameraWorker.h"
#include "Camera.h"

class CameraWorker;

CameraTask::CameraTask(std::wstring name)
    : name(name)
{
#ifdef DEBUG
    LOGTRACE(L"CameraTask::CameraTask(%s)", name.c_str());
#endif
    status.currentStep = CAMERA_TASK_NOT_STARTED;

    runStates.push_back(CameraState::Connected);
}

CameraTask::~CameraTask()
{
#ifdef DEBUG
    LOGTRACE(L"CameraTask::~CameraTask() for %s", name.c_str());
#endif
    if (waiter != INVALID_HANDLE_VALUE)
    {
        CloseHandle(waiter);
    }

    if (cancel != INVALID_HANDLE_VALUE)
    {
        CloseHandle(cancel);
    }
}

void
CameraTask::SetSteps(StateStepInfo steps[])
{
    for (int i = 0; steps[i].handler != nullptr; i++)
    {
        AddStep(steps[i]);
    }
}

bool
CameraTask::AddStep(StateStepInfo info)
{
    steps.insert(std::pair<int, StateStepInfo>(info.id, info));

    return true;
}

bool
CameraTask::SupportsCameraState(CameraState state)
{
    return std::find(runStates.begin(), runStates.end(), state) != runStates.end();
}

bool
CameraTask::Run(Camera* camera)
{
#ifdef DEBUG
    LOGTRACE(L"In: CameraTask::Run(%s)", name.c_str());
#endif
    // Create semaphore, get camera worker, initate and wait for response
    CameraWorker* pWorker = camera->GetWorker();

    waiter = CreateSemaphore(nullptr, 0, 1, nullptr);

    pWorker->QueueTask(this, waiter);

    // Wait for task to complete
    DWORD waitResult = WaitForSingleObject(waiter, INFINITE);

    CloseHandle(waiter);
    waiter = INVALID_HANDLE_VALUE;
#ifdef DEBUG
    LOGTRACE(L"Out: CameraTask::Run(%s)", name.c_str());
#endif
    return this->success;
}

void
CameraTask::Start(Camera* camera)
{
    // Create semaphore, get camera worker, initate and wait for response
    CameraWorker* pWorker = camera->GetWorker();

    waiter = CreateSemaphore(nullptr, 1, 1, nullptr);
    cancel = CreateEvent(nullptr, TRUE, FALSE, nullptr);

    pWorker->QueueTask(this, waiter);
}

bool
CameraTask::QueueAndForget(Camera* camera)
{
    CameraWorker* pWorker = camera->GetWorker();

    return pWorker->QueueTask(this, INVALID_HANDLE_VALUE);
}

DWORD
CameraTask::Cancel(DWORD timeout)
{
    if (cancel != INVALID_HANDLE_VALUE)
    {
        SetEvent(cancel);

        return WaitForSingleObject(waiter, timeout);
    }
    else
    {
        return WAIT_OBJECT_0;
    }
}

DWORD
CameraTask::WaitFor(DWORD timeout)
{
    // Wait for task to complete
    return WaitForSingleObject(waiter, timeout);
}

void
CameraTask::SetResult(void* pResult)
{
    this->result = pResult;
}

void*
CameraTask::GetResult()
{
    return result;
}

HANDLE
CameraTask::GetCancelEventHandle()
{
    return cancel;
}

void
CameraTask::SetParam1(void* param)
{
    param1 = param;
}

void*
CameraTask::GetParam1()
{
    return param1;
}

void
CameraTask::SetParam2(void* param)
{
    param2 = param;
}

void*
CameraTask::GetParam2()
{
    return param2;
}

bool
CameraTask::IsCancelled()
{
    return cancelled;
}

bool
CameraTask::IsComplete()
{
    return this->status.currentStep == CAMERA_TASK_COMPLETE;
}

bool
CameraTask::Step(CameraTaskInfo& info)
{
    if (status.currentStep == CAMERA_TASK_NOT_STARTED) {
        status.currentStep = 0;
    }

    StateStepInfo stepInfo = steps[status.currentStep];

    status.currentStepName = stepInfo.name;

    if (stepInfo.handler == nullptr)
    {
        status.currentStep = CAMERA_TASK_COMPLETE;
    }
    else
    {
        if (stepInfo.isActive)
        {
            // Already started, this must be a retry - lets bump the retry counter
            stepInfo.retryCount++;
        }
        else
        {
            stepInfo.isActive = true;
        }

        // Should put this in try/catch
        std::wstring taskName = info.pTask->Name();
        std::wstring stepName = !stepInfo.name.empty() ? stepInfo.name : L"<unknown>";

#ifdef DEBUG
        LOGTRACE(L"Pre: %s.%s", taskName.c_str(), stepName.c_str());
#endif
        StateResult result = (stepInfo.handler)(info);
#ifdef DEBUG
        LOGTRACE(L"Post: %s.%s (result = %d)", info.pTask->Name().c_str(), stepName.c_str(), result);
#endif
        stepInfo.lastResult = result;

        if (!cancelled && cancel != INVALID_HANDLE_VALUE && result != StateResult::Cancel && stepInfo.cancelId >= 0)
        {
            // Test for cancel event
            if (WaitForSingleObject(cancel, 0) == WAIT_OBJECT_0)
            {
                result = StateResult::Cancel;
            }
        }

        if (!stepInfo.isTerminal)
        {
            switch (result)
            {
            case StateResult::Success:
                status.currentStep = stepInfo.successId;
                break;

            case StateResult::Fail:
                status.currentStep = stepInfo.failId >= 0 ? stepInfo.failId : stepInfo.successId;
                break;

            case StateResult::Cancel:
                status.currentStep = stepInfo.cancelId >= 0 ? stepInfo.cancelId : stepInfo.failId >= 0 ? stepInfo.failId : stepInfo.successId;
                cancelled = true;
                break;

            case StateResult::Retry:
                if (stepInfo.retryCount >= stepInfo.maxRetries)
                {
                    result == StateResult::Fail;
                    status.currentStep = stepInfo.failId;
                }
                break;
            }

            status.currentStepName = L"<pending>";
        }
        else
        {
            status.currentStep = CAMERA_TASK_COMPLETE;
            status.currentStepName = L"<complete>";
            success = result == StateResult::Success;
        }
    }

    return status.currentStep == CAMERA_TASK_COMPLETE;
}

std::wstring
CameraTask::Name()
{
    return name;
}

CameraTaskStatus
CameraTask::GetTaskStatus()
{
    return status;
}

void
CameraTask::SetUserStatus(int userStatus)
{
    status.userStatus = userStatus;
}

RefreshPropertiesTask::RefreshPropertiesTask()
    : CameraTask(L"RefreshPropertiesTask")
{
    StateStepInfo steps[] = {
        {
            0,
            L"RefreshSettings",
            refreshSettings,
            0,
            0,
            0,
            0,
            true,
            true,
        },
        {
            9999,       // step id
            L"",        // name
            nullptr,    // pointer to handler function
            0,          // step to move to on success
            0,          // step to move to on fail
            0,          // step to move to on cancel
            0,          // number of retries allowed
            true,       // this is a terminal step
            false,      // this terminal step is for success
        },
    };

    SetSteps(steps);
}

RefreshDeviceInfoTask::RefreshDeviceInfoTask()
    : CameraTask(L"RefreshDeviceInfoTask")
{
    StateStepInfo steps[] = {
        {
            0,
            L"RefreshDeviceInfo",
            refreshDeviceInfo,
            0,
            0,
            0,
            0,
            true,
            true,
        },
        {
            9999,       // step id
            L"",        // name
            nullptr,    // pointer to handler function
            0,          // step to move to on success
            0,          // step to move to on fail
            0,          // step to move to on cancel
            0,          // number of retries allowed
            true,       // this is a terminal step
            false,      // this terminal step is for success
        },
    };

    SetSteps(steps);
}

GetPreviewTask::GetPreviewTask()
    : CameraTask(L"GetPreviewTask")
{
    StateStepInfo steps[] = {
        {
            0,
            L"RefreshSettings",
            refreshSettings,
            1,
            0,
            0,
            0,
            false,
            false,
        },
        {
            1,
            L"GetPreview",
            getPreview,
            0,
            0,
            0,
            0,
            true,
            true,
        },
        {
            9999,       // step id
            L"",        // name
            nullptr,    // pointer to handler function
            0,          // step to move to on success
            0,          // step to move to on fail
            0,          // step to move to on cancel
            0,          // number of retries allowed
            true,       // this is a terminal step
            false,
        },
    };

    SetSteps(steps);
}

Image*
GetPreviewTask::GetImage()
{
    return (Image*)GetResult();
}

InitializeCameraTask::InitializeCameraTask()
    : CameraTask(L"InitializeCameraTask")
{
    StateStepInfo steps[] = {
        {
            0,
            L"InitializeCamera",
            initializeCamera,
            0,
            0,
            0,
            0,
            true,
            true,
        },
        {
            9999,       // step id
            L"",        // name
            nullptr,    // pointer to handler function
            0,          // step to move to on success
            0,          // step to move to on fail
            0,          // step to move to on cancel
            0,          // number of retries allowed
            true,       // this is a terminal step
            false,
        },
    };

    SetSteps(steps);
}

OpenCameraTask::OpenCameraTask()
    : CameraTask(L"OpenCameraTask")
{
    StateStepInfo steps[] = {
        {
            0,
            L"OpenCamera",
            openCamera,
            0,
            0,
            0,
            0,
            true,
            true,
        },
        {
            9999,       // step id
            L"",        // name
            nullptr,    // pointer to handler function
            0,          // step to move to on success
            0,          // step to move to on fail
            0,          // step to move to on cancel
            0,          // number of retries allowed
            true,       // this is a terminal step
            false,
        },
    };

    SetSteps(steps);
    runStates.push_back(CameraState::Null);
}

HANDLE OpenCameraTask::GetHandle()
{
    return (HANDLE)GetResult();
}

CloseCameraTask::CloseCameraTask(HANDLE hCamera)
    : CameraTask(L"CloseCameraTask")
{
    StateStepInfo steps[] = {
        {
            0,
            L"CloseCamera",
            closeCamera,
            0,
            0,
            0,
            0,
            true,
            true,
        },
        {
            9999,       // step id
            L"",        // name
            nullptr,    // pointer to handler function
            0,          // step to move to on success
            0,          // step to move to on fail
            0,          // step to move to on cancel
            0,          // number of retries allowed
            true,       // this is a terminal step
            false,
        },
    };

    SetParam1(hCamera);
    SetSteps(steps);
}

bool
CloseCameraTask::Closed()
{
    return GetResult() != nullptr;
}

SetPropertyTaskParams::SetPropertyTaskParams(Property id, PropertyValue* value)
    : id(id),
    value(value)
{

}

SetPropertyTask::SetPropertyTask(SetPropertyTaskParams* params)
    : CameraTask(L"SetPropertyTask")
{
    StateStepInfo steps[] = {
        {
            0,
            L"setProperty",
            setProperty,
            0,
            0,
            0,
            0,
            true,
            true,
        },
        {
            9999,       // step id
            L"",        // name
            nullptr,    // pointer to handler function
            0,          // step to move to on success
            0,          // step to move to on fail
            0,          // step to move to on cancel
            0,          // number of retries allowed
            true,       // this is a terminal step
            false,
        },
    };

    SetSteps(steps);
    SetParam1((void*)params);
}

TakePhotoTask::TakePhotoTask(TakePhotoTaskParams* params)
    : CameraTask(L"TakePhotoTask")
{
    // The most complicated task by far
    // Steps: (from the old CaptureThread class)
    // **STEP 1**
    // 1. Check whether auto-focus enabled, if so - we need to wait half-way thru pressing shutter button
    // 2. Get exposure time setting from camera - adjust exposure time to lower of
    //    requested or set
    // NOTE: Cancel prior to this point just exits
    // **STEP 2**
    // 3. Press shutter button half down
    // NOTE: Cancel at this point returns shutter button to up and exits
    // 4. Wait (if auto-focus)
    // 5. Press shutter button full down
    // NOTE: Cancel after this point just aborts the wait (if bulb mode) and continues processing (as camera will barf if we don't pull the image(s))
    // 6. Wait exposure time
    // 7. Release "full down"
    // 8. Release "half down"
    // **STEP 3**
    // 9. Loop, waiting for camera to indicate image(s) ready
    // 10. Read images until camera reports no more to send
    //     1st image is used, rest are saved or discarded based on save setting
    // 11. Process image
    StateStepInfo steps[] = {
        {
            0,
            L"refreshSettings",
            refreshSettings,
            1,
            5,
            5,
            0,
            false,
            false,
        },
        {
            1,
            L"doCapture",
            doCapture,
            0,
            2,
            2,
            0,
            true,
            false,
        },
        {
            2,
            L"abortDoCapture",
            refreshSettings,  // We just need to do something
            0,
            0,
            -1,
            0,
            true,
            true,
        },
        {
            9999,       // step id
            L"",        // name
            nullptr,    // pointer to handler function
            0,          // step to move to on success
            0,          // step to move to on fail
            0,          // step to move to on cancel
            0,          // number of retries allowed
            true,       // this is a terminal step
            false,
        },
    };

    SetSteps(steps);

    SetParam1((void*)params);
}

TakePhotoTaskParams::TakePhotoTaskParams(double exposureTime, OutputMode mode)
    : exposureTime(exposureTime),
    mode(mode)
{
}

DownloadAndProcessImageTask::DownloadAndProcessImageTask(TakePhotoTaskParams* params)
    : CameraTask(L"DownloadAndProcessImageTask")
{
    StateStepInfo steps[] = {
        {
            0,
            L"refreshSettings",
            refreshSettings,
            1,
            3,
            3,
            0,
            false,
            false,
        },
        {
            1,
            L"downloadImage",
            downloadImage,
            2,
            3,
            -1,
            0,
            false,
            false,
        },
        {
            2,
            L"processImage",
            processImage,
            0,
            0,
            -1,
            0,
            true,
            false,
        },
        {
            3,
            L"refreshSettings",
            refreshSettings,
            0,
            0,
            0,
            0,
            true,
            true,
        },
        {
            9999,       // step id
            L"",        // name
            nullptr,    // pointer to handler function
            0,          // step to move to on success
            0,          // step to move to on fail
            0,          // step to move to on cancel
            0,          // number of retries allowed
            true,       // this is a terminal step
            false,
        },
    };

    SetSteps(steps);
    SetParam1((void*)params);
}

Image*
DownloadAndProcessImageTask::GetImage()
{
    return (Image*)GetResult();
}
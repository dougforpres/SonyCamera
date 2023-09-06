#include "pch.h"
#include "CameraWorker.h"
#include "CameraManager.h"
#include "Logger.h"
#include "CameraException.h"

CameraWorker::CameraWorker(Camera* pCamera)
    : camera(pCamera)
{
    mutexHandle = CreateMutex(nullptr, FALSE, nullptr);
    threadHandle = CreateThread(NULL, 0, &_run, this, 0, &threadId);
}

CameraWorker::~CameraWorker()
{
    // We need to wait a long time as the camera may be in the middle of a long exposure (shouldn't be, but... users!)
    DWORD waitResult = WaitForSingleObject(mutexHandle, INFINITE);

    if (waitResult != WAIT_OBJECT_0)
    {
        // Need to error here
    }

    stopRequested = true;

    ReleaseMutex(mutexHandle);

    // Wait for run to exit
    DWORD exitCode = 0;
    BOOL ret = GetExitCodeThread(threadHandle, &exitCode);

    int waitCount = 0;

    while (exitCode == STILL_ACTIVE && waitCount < WORKER_THREAD_WAIT_FOR_EXIT_RETRIES)
    {
        waitCount++;

        LOGINFO(L"Waiting for camera handler thread to exit %d / %d loops", waitCount, WORKER_THREAD_WAIT_FOR_EXIT_RETRIES);

        Sleep(WORKER_THREAD_WAIT_FOR_EXIT);
        GetExitCodeThread(threadHandle, &exitCode);
    }

    if (exitCode == STILL_ACTIVE)
    {
        LOGERROR(L"Camera handler thread would not exit, terminating with prejudice");
        TerminateThread(threadHandle, -1);
    }

/*    if (currentTask)
    {
        // How do we go about stopping the task?
    }*/

    CloseHandle(mutexHandle);
    CloseHandle(threadHandle);
}

void
CameraWorker::OnCameraStateChange(CameraState newState)
{
    if (newState != lastCameraState)
    {
//        LOGTRACE(L"Camera State changed from %d to %d", lastCameraState, newState);
        lastCameraState = newState;
    }
}

DWORD WINAPI
CameraWorker::_run(LPVOID lpParameter)
{
    LOGTRACE(L"In: CameraWorker::_run(x%08x)", lpParameter);

    DWORD result = ((CameraWorker*)lpParameter)->Run();

    LOGTRACE(L"Out: CameraWorker::_run(x%08x) - returning %d", lpParameter, result);

    return result;
}

DWORD CameraWorker::Run()
{
    LOGTRACE(L"In: CameraWorker::Run");

    CameraTask* currentTask = nullptr;
    HANDLE waiter = INVALID_HANDLE_VALUE;

    HRESULT coInit = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    LOGTRACE(L"CoInitialize returned: %d", coInit);

    while (!stopRequested)
    {
        // Only reason mutex would be claimed is to set the exit flag, so while INFINITE may not be ideal,
        // we're expecting (essentially) an instant result
        DWORD waitResult = WaitForSingleObject(mutexHandle, INFINITE);

        if (waitResult != WAIT_OBJECT_0)
        {
            // Need to error here
        }

        if (!currentTask)
        {
            if (!queue.empty())
            {
                std::pair<CameraTask*, HANDLE> next = queue.front();
                queue.pop_front();
                currentTask = next.first;
                waiter = next.second;
#ifdef DEBUG
                LOGTRACE(L"Request pulled from queue");
#endif
            }
        }

        // We can start something
        if (!currentTask)
        {
#ifdef DEBUG
            LOGTRACE(L"Nothing to do, waiting a bit");
#endif

            Sleep(25);
        }

        if (currentTask && currentTask->SupportsCameraState(lastCameraState))
        {
            CameraTaskInfo info;

            info.pCamera = camera;
            info.pTask = currentTask;

            bool complete = currentTask->Step(info);

            if (complete)
            {
                if (waiter != INVALID_HANDLE_VALUE)
                {
#ifdef DEBUG
                    LOGTRACE(L"Releasing wait for task %s", currentTask->Name().c_str());
#endif
                    ReleaseSemaphore(waiter, 1, nullptr);
                }
                else
                {
                    // If there is no waiter, there is no way to indicate the task is complete.
                    // It is expected the only task that will fall into this category is the
                    // one auto-generated when there is nothing to do
                    LOGTRACE(L"Deleting un-waited-for task %s", currentTask->Name().c_str());
                    delete currentTask;
                }

                currentTask = nullptr;
                waiter = INVALID_HANDLE_VALUE;
            }
        }

        ReleaseMutex(mutexHandle);
    }

    CoUninitialize();

    LOGTRACE(L"Out: CameraWorker::Run");

    return 0;
}

bool
CameraWorker::QueueTask(CameraTask* task, HANDLE waiter)
{
    // Wait for runner thread to be idle
    DWORD waitResult = WaitForSingleObject(mutexHandle, INFINITE);

    if (waitResult != WAIT_OBJECT_0)
    {
        // Need to error here
    }

    // If there is a waiter, just queue it up - if there is no waiter, look thru
    // queue and only add if there isn't one of same type
    if (!waiter)
    {
        for (std::list<std::pair<CameraTask*, HANDLE>>::const_iterator it = queue.begin(); it != queue.end(); it++)
        {
            if ((*it).first->Name() == task->Name())
            {
                LOGINFO(L"Not queuing task %s as there is already one in queue", task->Name().c_str());
                task = nullptr;
            }
        }
    }

    if (task)
    {
        queue.push_back(std::pair<CameraTask*, HANDLE>(task, waiter));
    }

    // Let runner continue
    ReleaseMutex(mutexHandle);

    return task;
}

CameraTask::CameraTask(std::wstring name)
    : name(name)
{
#ifdef DEBUG
    LOGTRACE(L"CameraTask::CameraTask(%s)", name.c_str());
#endif
    status.currentStep = WORKER_TASK_NOT_STARTED;

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
CameraTask::Run(Camera *camera)
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

void *
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

void *
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
    return this->status.currentStep == WORKER_TASK_COMPLETE;
}

bool
CameraTask::Step(CameraTaskInfo& info)
{
    if (status.currentStep == WORKER_TASK_NOT_STARTED) {
        status.currentStep = 0;
    }

    StateStepInfo stepInfo = steps[status.currentStep];

    status.currentStepName = stepInfo.name;

    if (stepInfo.handler == nullptr)
    {
        status.currentStep = WORKER_TASK_COMPLETE;
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
            status.currentStep = WORKER_TASK_COMPLETE;
            status.currentStepName = L"<complete>";
            success = result == StateResult::Success;
        }
    }

    return status.currentStep == WORKER_TASK_COMPLETE;
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

StateResult
refreshSettings(CameraTaskInfo& info)
{
    try
    {
        info.pCamera->RefreshSettings();
    }
    catch (CameraException& ex)
    {
        LOGWARN(L"Error refreshing settings... going to ignore");
    }

    return StateResult::Success;
}

StateResult
getPreview(CameraTaskInfo& info)
{
    info.pTask->SetResult(info.pCamera->GetImage(PREVIEW_IMAGE));

    return StateResult::Success;
}

StateResult
initializeCamera(CameraTaskInfo& info)
{
    return info.pCamera->Initialize() ? StateResult::Success : StateResult::Fail;
}

StateResult
refreshDeviceInfo(CameraTaskInfo& info)
{
    return info.pCamera->RefreshDeviceInfo() ? StateResult::Success : StateResult::Fail;
}

StateResult
openCamera(CameraTaskInfo& info)
{
    info.pTask->SetResult((VOID *)info.pCamera->Open());

    return StateResult::Success;
}

StateResult
closeCamera(CameraTaskInfo& info)
{
    if (info.pCamera->Close())
    {
        info.pTask->SetResult((void *)1);
    }

    return StateResult::Success;
}

StateResult
setProperty(CameraTaskInfo& info)
{
    SetPropertyTaskParams* params = (SetPropertyTaskParams*)info.pTask->GetParam1();
    info.pCamera->SetProperty(params->id, params->value);

    return StateResult::Success;
}

StateResult
doCapture(CameraTaskInfo& info)
{
    PropertyValue up((WORD)(info.pCamera->GetDeviceInfo().GetButtonPropertiesInverted() ? 2 : 1));
    PropertyValue down((WORD)(info.pCamera->GetDeviceInfo().GetButtonPropertiesInverted() ? 1 : 2));

    std::unique_ptr<CameraSettings> cs(info.pCamera->GetSettings());

    // If auto-focus is enabled, we should add a short pause between 1/2 down and full down to let it
    // do its thing
    bool isManualFocus = cs->GetProperty(Property::FocusMode)->GetCurrentValue()->GetUINT16() == 1;
    double exposureTime = ((TakePhotoTaskParams*)info.pTask->GetParam1())->exposureTime;

    info.pCamera->SetCaptureStatus(CaptureStatus::Capturing);

    LOGINFO(L"Step 1: Shutter button down");
    info.pCamera->SetProperty(Property::ShutterHalfDown, &down);

    if (!isManualFocus)
    {
        LOGINFO(L"Step 1a: Pausing to allow auto-focus... why is it on?");
        Sleep(500);
    }

    info.pCamera->SetProperty(Property::ShutterFullDown, &down);

    WaitForSingleObject(info.pTask->GetCancelEventHandle(), (DWORD)(exposureTime * 1000));

    info.pCamera->SetProperty(Property::ShutterFullDown, &up);
    info.pCamera->SetProperty(Property::ShutterHalfDown, &up);
    LOGINFO(L"Step 2: Shutter button up");

    return StateResult::Success;
}

bool
isImageReady(CameraSettings* cs)
{
    CameraProperty* bufferStatus = cs->GetProperty(Property::PhotoBufferStatus);
    DWORD value = bufferStatus->GetCurrentValue()->GetUINT16();
    bool imageReady = false;

    switch (value)
    {
    case 0x0000:
        LOGINFO(L"  Nothing yet");
        break;

    case 0x0001:
        LOGINFO(L"  Camera processing");
        break;

    default:
        if (value & 0x8000)
        {
            LOGINFO(L"  %d Photo(s) ready for retrieval", value & 0xfff);
            imageReady = true;
        }
        else
        {
            LOGWARN(L"  No idea what this means x%08x", value);
        }
        break;
    }

    return imageReady;
}

StateResult
downloadImage(CameraTaskInfo& info)
{
    info.pCamera->SetCaptureStatus(CaptureStatus::Reading);

    // If we get here, there must be at least one image ready
    bool imageReady = true;
    int imageCount = 0;

    while (imageReady)
    {
        LOGINFO(L"Step 4: Retrieve image #%d", imageCount + 1);

        Image* image = info.pCamera->GetImage(FULL_IMAGE);

        if (image)
        {
            image->SaveFile();

            // Use the RAW image data and discard JPEG
            if (info.pTask->GetResult() == nullptr || (image->GetInputMode() == InputMode::ARW))
            {
                // Delete is happy with null ptr
                delete (Image*)info.pTask->GetResult();
                info.pTask->SetResult(image);
            }
            else
            {
                delete image;
            }

            imageCount++;

            refreshSettings(info);
            std::unique_ptr<CameraSettings> cs(info.pCamera->GetSettings());
            imageReady = isImageReady(cs.get());
        }
    }

    return imageCount > 0 ? StateResult::Success : StateResult::Fail;
}

StateResult
processImage(CameraTaskInfo& info)
{
    info.pCamera->SetCaptureStatus(CaptureStatus::Processing);
    StateResult result = StateResult::Success;

    Image* image = (Image*)info.pTask->GetResult();

    if (info.pTask->IsCancelled())
    {
        delete image;
        info.pTask->SetResult(nullptr);
        info.pCamera->SetCaptureStatus(CaptureStatus::Cancelled);
    }
    else
    {
        if (image->GetDataLength())
        {
            // Render as RGB or RGGB
            // We do it in here as there is a chance we'll get multiple hits on other threads that will cause... issues
            OutputMode mode = ((TakePhotoTaskParams*)info.pTask->GetParam1())->mode;

            image->SetOutputMode(mode);
            image->SetDuration(*(float*)(info.pTask->GetParam1()));

            // Tell the image how it should be cropped
            DeviceInfo deviceInfo = info.pCamera->GetDeviceInfo();

            switch (deviceInfo.GetCropMode())
            {
            case CropMode::AUTO:
                image->SetCrop(-1, -1, -1, -1);
                break;

            case CropMode::NONE:
                image->SetCrop(0, 0, 0, 0);
                break;

            case CropMode::USER:
                image->SetCrop(deviceInfo.GetTopCrop(), deviceInfo.GetLeftCrop(), deviceInfo.GetBottomCrop(), deviceInfo.GetRightCrop());
                break;

            default:
                // Image default is 0, 0, 0, 0
                break;
            }

            bool success = image->EnsurePixelsProcessed();

            if (success)
            {
                info.pCamera->SetCaptureStatus(CaptureStatus::Complete);
            }
            else
            {
                LOGERROR(L"Unable to generate pixel data, setting status failed");
                result = StateResult::Fail;
            }
        }
        else
        {
            info.pCamera->SetCaptureStatus(CaptureStatus::Failed);
            result = StateResult::Fail;
        }
    }

    return result;
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

//SetPropertyTask::~SetPropertyTask()
//{
//    delete (PropertyValue*)GetParam2();
//}

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
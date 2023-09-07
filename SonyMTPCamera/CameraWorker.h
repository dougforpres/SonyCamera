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
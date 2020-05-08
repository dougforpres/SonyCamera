#include "pch.h"
#include "Camera.h"
#include "CameraException.h"
#include "Logger.h"
#include "Registry.h"

#define IMAGE_WAIT_READY_SLEEP  250
#define IMAGE_WAIT_READY_LOOPS  100

#define THREAD_WAIT_EXIT_SLEEP  100
#define THREAD_WAIT_EXIT_LOOPS  105

DWORD WINAPI 
Camera::CaptureThread::_run(LPVOID lpParameter)
{
    LOGTRACE(L"In: CaptureThread::_run(x%08x)");

    DWORD result = ((CaptureThread*)lpParameter)->Run();

    LOGTRACE(L"Out: CaptureThread::_run(x%08x) - returning %d", result);

    return result;
}

Camera::CaptureThread::CaptureThread(Camera* camera, OutputMode outputMode)
    : m_camera(camera),
      m_outputMode(outputMode)
{
    LOGTRACE(L"In: CaptureThread::CaptureThread(x%p)", (LPVOID)camera);

    m_hWakeupEvent = CreateEvent(NULL, true, false, L"Capture Thread Wakeup Event");
    m_hMutex= CreateMutex(nullptr, false, nullptr);

    LOGTRACE(L"Out: CaptureThread::CaptureThread(x%p)", (LPVOID)camera);
}

Camera::CaptureThread::~CaptureThread()
{
    LOGTRACE(L"In: CaptureThread::~CaptureThread");

    CancelCapture();

    // Wait for exit
    DWORD exitCode = 0;
    DWORD waitCount = 0;

    GetExitCodeThread(m_hThread, &exitCode);

    while (exitCode == STILL_ACTIVE && waitCount < THREAD_WAIT_EXIT_LOOPS)
    {
        waitCount++;

        LOGINFO(L"Waiting for thread to exit %d / %d loops", waitCount, THREAD_WAIT_EXIT_LOOPS);

        // I know we called it above, but the thread may not have been capturing at that point
        // The Cancel method doesn't do anything that can't be done multiple times, so safe to call it
        // over and over
        if (m_status == CaptureStatus::Capturing)
        {
            CancelCapture();
        }

        Sleep(THREAD_WAIT_EXIT_SLEEP);
        GetExitCodeThread(&m_hThread, &exitCode);
    }

    CloseHandle(m_hWakeupEvent);
    CloseHandle(m_hThread);
    CloseHandle(m_hMutex);

    m_hWakeupEvent = INVALID_HANDLE_VALUE;
    m_hThread = INVALID_HANDLE_VALUE;
    m_hMutex = INVALID_HANDLE_VALUE;

    if (m_image)
    {
        delete[] m_image;
        m_image = nullptr;
    }

    LOGTRACE(L"Out: CaptureThread::~CaptureThread");
}

bool
Camera::CaptureThread::StartCapture(double duration)
{
    LOGTRACE(L"In: CaptureThread::StartCapture(%f)", duration);

    SetStatus(CaptureStatus::Starting);

    m_duration = duration;
    m_hThread = CreateThread(NULL, 0, &_run, this, 0, &m_threadId);

    LOGTRACE(L"Out: CaptureThread::StartCapture(%f)", duration);

    return true;
}

bool
Camera::CaptureThread::CancelCapture()
{
    LOGTRACE(L"In: CaptureThread::CancelCapture");

    if (m_hThread != INVALID_HANDLE_VALUE && m_hWakeupEvent != INVALID_HANDLE_VALUE)
    {
        DWORD exitCode = 0;

        LOGINFO(L"CaptureThread: Checking to ensure thread is not active");

        // Make sure no-one can change the status while we're doing the cancel thing
        if (WaitForSingleObject(m_hMutex, INFINITE) == WAIT_OBJECT_0)
        {
            if (m_status == CaptureStatus::Capturing || m_status == CaptureStatus::Reading)
            {
                LOGWARN(L"CaptureThread: Capturing active, attempting to cancel");

                SetEvent(m_hWakeupEvent);
                m_cancelled = true;
            }
            else
            {
                LOGINFO(L"Thread isn't actually capturing, so cannot cancel");
            }

            ReleaseMutex(m_hMutex);
        }
    }

    LOGTRACE(L"Out: CaptureThread::CancelCapture");

    return true;
}

CaptureStatus
Camera::CaptureThread::GetStatus()
{
    return m_status;
}

Image*
Camera::CaptureThread::GetImage()
{
    return m_image;
}

double
Camera::CaptureThread::GetDuration()
{
    return m_duration;
}

bool
Camera::CaptureThread::ImageReady(CameraSettings* settings)
{
    CameraProperty* bufferStatus = settings->GetProperty(Property::PhotoBufferStatus);
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

DWORD
Camera::CaptureThread::Run()
{
    LOGTRACE(L"In: CaptureThread::Run");
    PropertyValue up((WORD)(m_camera->GetDeviceInfo()->GetButtonPropertiesInverted() ? 2 : 1));
    PropertyValue down((WORD)(m_camera->GetDeviceInfo()->GetButtonPropertiesInverted() ? 1: 2));

    CameraSettings* settings = m_camera->GetSettings(true);

    // If auto-focus is enabled, we should add a short pause between 1/2 down and full down to let it
    // do its thing
    bool isManualFocus = settings->GetProperty(Property::FocusMode)->GetCurrentValue()->GetUINT16() == 1;
    SetStatus(CaptureStatus::Capturing);

    LOGINFO(L"Step 1: Shutter button down");
    m_camera->SetProperty(Property::ShutterHalfDown, &down);

    if (!isManualFocus)
    {
        LOGINFO(L"Step 1a: Pausing to allow auto-focus... why is it on?");
        Sleep(500);
    }

    m_camera->SetProperty(Property::ShutterFullDown, &down);

    WaitForSingleObject(m_hWakeupEvent, (DWORD)(m_duration * 1000));

    m_camera->SetProperty(Property::ShutterFullDown, &up);
    m_camera->SetProperty(Property::ShutterHalfDown, &up);
    LOGINFO(L"Step 2: Shutter button up");

    SetStatus(CaptureStatus::Reading);

    // Wait for settings to signal there is an image ready
    LOGINFO(L"Step 3: Wait for camera to indicate image ready");
    int waitLoop = 0;
    bool imageReady = false;

    // Wait up to 10 seconds for camera to report the image is ready for download
    do
    {
        settings = m_camera->GetSettings(true);
        imageReady = this->ImageReady(settings);

        if (!imageReady)
        {
            Sleep(IMAGE_WAIT_READY_SLEEP);
        }
    } while (waitLoop < IMAGE_WAIT_READY_LOOPS && !imageReady);

    int imageCount = 0;

    // Check to see if we're in RAW+JPEG mode... if so we need to pull 2 images from camera
    BYTE compressionSetting = settings->GetPropertyValue(Property::CompressionSetting)->GetUINT8();

    // Slurp all images in until there aren't any more
    while (imageReady)
    {
        LOGINFO(L"Step 4: Retrieve image #%d", imageCount + 1);

        Image* image = m_camera->GetImage(FULL_IMAGE);

        if (image)
        {
            image->SaveFile();

            // Use the RAW image data and discard JPEG
            if (m_image == nullptr || (image->GetInputMode() == InputMode::ARW))
            {
                delete m_image;
                m_image = image;
            }
            else
            {
                delete image;
            }

            imageCount++;

            settings = m_camera->GetSettings(true);
            imageReady = this->ImageReady(settings);
        }
    }

    if (imageCount > 0)
    {
        SetStatus(CaptureStatus::Processing);

        if (m_image->GetDataLength())
        {
            if (!m_cancelled)
            {
                // Render as RGB or RGGB
                // We do it in here as there is a chance we'll get multiple hits on other threads that will cause... issues
                m_image->SetOutputMode(m_outputMode);
                m_image->SetDuration(m_duration);

                // Tell the image how it should be cropped
                DeviceInfo* deviceInfo = m_camera->GetDeviceInfo();

                switch (deviceInfo->GetCropMode())
                {
                case CropMode::AUTO:
                    m_image->SetCrop(-1, -1, -1, -1);
                    break;

                case CropMode::NONE:
                    m_image->SetCrop(0, 0, 0, 0);
                    break;

                case CropMode::USER:
                    m_image->SetCrop(deviceInfo->GetTopCrop(), deviceInfo->GetLeftCrop(), deviceInfo->GetBottomCrop(), deviceInfo->GetRightCrop());
                    break;

                default:
                    // Image default is 0, 0, 0, 0
                    break;
                }

                bool success = m_image->EnsurePixelsProcessed();

                if (!success)
                {
                    LOGERROR(L"Unable to generate pixel data, setting status failed");
                }

                SetStatus(success ? CaptureStatus::Complete : CaptureStatus::Failed);
            }
            else
            {
                SetStatus(CaptureStatus::Cancelled);
            }
        }
        else
        {
            LOGWARN(L"Get Image failed");
            SetStatus(CaptureStatus::Failed);
        }
    }
    else
    {
        LOGERROR(L"Unable to get image data from camera");
        SetStatus(CaptureStatus::Failed);
    }

    LOGTRACE(L"Out: CaptureThread::Run - returning %d", (DWORD)m_status);

    return (DWORD)m_status;
}

void
Camera::CaptureThread::SetStatus(CaptureStatus status)
{
    // Need to only mess with status when we're allowed
    // The check for WAIT_OBJECT_0 allows the handle to be closed elsewhere (destructor)
    // and it will release but won't execute
    if (WaitForSingleObject(m_hMutex, INFINITE) == WAIT_OBJECT_0)
    {
        m_status = status;
        LOGTRACE(L"CaptureThread::SetStatus(%d)", status);
        ReleaseMutex(m_hMutex);
    }
}
#include "pch.h"
#include "Camera.h"
#include "Registry.h"
#include "Logger.h"
#include "CameraException.h"

#define THREAD_WAIT_EXIT_LOOPS 20
#define THREAD_WAIT_EXIT_SLEEP 100
#define MUTEX_TIMEOUT 10000

Camera::Camera(Device* device)
    : m_device(device)
{
    m_hBusyMutex = CreateMutex(nullptr, false, nullptr);
    m_hWakeEvent = CreateEvent(nullptr, false, false, nullptr);
}

Camera::~Camera()
{
    if (m_settings)
    {
        delete m_settings;
        m_settings = nullptr;
    }

    CloseHandle(m_hBusyMutex);
    CloseHandle(m_hWakeEvent);
    m_hBusyMutex = INVALID_HANDLE_VALUE;
    m_hWakeEvent = INVALID_HANDLE_VALUE;
}

const std::wstring
Camera::GetId()
{
    return m_device->GetId();
}

HANDLE
Camera::Open()
{
    HANDLE hCamera =  m_device->Open();

    if (hCamera != INVALID_HANDLE_VALUE && m_hHandlerThread == INVALID_HANDLE_VALUE)
    {
        // Start up handler thread
        m_shutdown = false;
        m_hHandlerThread = CreateThread(nullptr, 0, &_runHandlerThread, this, 0, nullptr);
    }

    return hCamera;
}

bool
Camera::Close()
{
    bool result = m_device->Close();

    if (result && m_hHandlerThread != INVALID_HANDLE_VALUE)
    {
        // Stop the handler thread when device reports "closed for reals"
        // Wait for exit
        DWORD exitCode = 0;
        DWORD waitCount = 0;

        m_shutdown = true;
        SetEvent(m_hWakeEvent);

        BOOL ret = GetExitCodeThread(m_hHandlerThread, &exitCode);

        waitCount = 0;

        while (exitCode == STILL_ACTIVE && waitCount < THREAD_WAIT_EXIT_LOOPS)
        {
            waitCount++;

            LOGINFO(L"Waiting for camera handler thread to exit %d / %d loops", waitCount, THREAD_WAIT_EXIT_LOOPS);

            Sleep(THREAD_WAIT_EXIT_SLEEP);
            GetExitCodeThread(m_hHandlerThread, &exitCode);
        }

        if (exitCode == STILL_ACTIVE)
        {
            LOGERROR(L"Camera handler thread would not exit, terminating with prejudice");
            TerminateThread(m_hHandlerThread, -1);
        }
    }

    return result;
}

DeviceInfo*
Camera::GetDeviceInfo(bool refresh)
{
    if (!m_deviceInfo || refresh)
    {
        m_device->Open();

        Message* tx;
        Message* rx;

        tx = new Message(COMMAND_GET_DEVICE_INFO);
        rx = m_device->Receive(tx);

        LOGTRACE(L"GetDeviceInfo() >> %s", rx->Dump().c_str());

        m_deviceInfo = new DeviceInfo(rx);

        delete tx;
        delete rx;

        m_device->Close();

        ProcessDeviceInfoOverrides();
        m_deviceInfo->DumpToLog();
    }

    return m_deviceInfo;
}

PropertyInfoMap
Camera::GetSupportedProperties()
{
    return m_propertyInfos;
}

Device *
Camera::GetDevice()
{
    return m_device;
}

bool
Camera::ProcessDeviceInfoOverrides()
{
    std::wostringstream builder;

    builder << L"Cameras\\" << m_device->GetRegistryPath();

    std::wstring cameraPath = builder.str();
    DeviceInfo* d = m_deviceInfo;

    registry.Open();

    d->SetSensorName(registry.GetString(cameraPath, L"Sensor Name", d->GetSensorName()));
    d->SetSensorPixelWidth(registry.GetDouble(cameraPath, L"Sensor X Size um", d->GetSensorPixelWidth()));
    d->SetSensorPixelHeight(registry.GetDouble(cameraPath, L"Sensor Y Size um", d->GetSensorPixelHeight()));
    d->SetSensorXResolution(registry.GetDWORD(cameraPath, L"Sensor X Resolution", d->GetSensorXResolution()));
    d->SetSensorYResolution(registry.GetDWORD(cameraPath, L"Sensor Y Resolution", d->GetSensorYResolution()));
    d->SetSensorXCroppedResolution(registry.GetDWORD(cameraPath, L"AutoCropped X Resolution", d->GetSensorXResolution()));
    d->SetSensorYCroppedResolution(registry.GetDWORD(cameraPath, L"AutoCropped Y Resolution", d->GetSensorYResolution()));
    d->SetPreviewXResolution(registry.GetDWORD(cameraPath, L"Preview X Resolution", d->GetPreviewXResolution()));
    d->SetPreviewYResolution(registry.GetDWORD(cameraPath, L"Preview Y Resolution", d->GetPreviewYResolution()));
    d->SetExposureTimeMin(registry.GetDouble(cameraPath, L"Exposure Time Min", d->GetExposureTimeMin()));
    d->SetExposureTimeMax(registry.GetDouble(cameraPath, L"Exposure Time Max", d->GetExposureTimeMax()));
    d->SetExposureTimeStep(registry.GetDouble(cameraPath, L"Exposure Time Step", d->GetExposureTimeStep()));
    d->SetSensorType((SensorType)registry.GetDWORD(cameraPath, L"Sensor Type", (DWORD)d->GetSensorType()));
    d->SetSupportsLiveview((bool)registry.GetDWORD(cameraPath, L"Supports Liveview", d->GetSupportsLiveview()));
    d->SetCropMode((CropMode)registry.GetDWORD(cameraPath, L"Crop Mode", (DWORD)d->GetCropMode()));
    d->SetLeftCrop(registry.GetDWORD(cameraPath, L"Crop Left", d->GetLeftCrop()));
    d->SetRightCrop(registry.GetDWORD(cameraPath, L"Crop Right", d->GetRightCrop()));
    d->SetTopCrop(registry.GetDWORD(cameraPath, L"Crop Top", d->GetTopCrop()));
    d->SetBottomCrop(registry.GetDWORD(cameraPath, L"Crop Bottom", d->GetBottomCrop()));
    d->SetButtonPropertiesInverted((bool)registry.GetDWORD(cameraPath, L"Button Properties Inverted", d->GetButtonPropertiesInverted()));

    registry.Close();

    return true;
}

ObjectInfo*
Camera::GetImageInfo(DWORD id)
{
    LOGTRACE(L"In: Camera::GetImageInfo(x%08x)", id);

    Message* tx;
    Message* rx;

    tx = new Message(COMMAND_GET_OBJECT_INFO);
    tx->AddParam(id);

    rx = m_device->Receive(tx);

    ObjectInfo* info = new ObjectInfo(rx);

    delete tx;
    delete rx;

    LOGTRACE(L"Out: Camera::GetImageInfo(x%08x)", id);

    return info;
}

Image*
Camera::GetImage(DWORD id)
{
    LOGTRACE(L"In: Camera::GetImage(x%08x)", id);

    Image* image = nullptr;
    ObjectInfo* info = GetImageInfo(id);

    if (info->GetCompressedSize())
    {
        Message* tx;
        Message* rx;

        tx = new Message(COMMAND_GET_OBJECT);
        tx->AddParam(id);
        rx = m_device->Receive(tx);

        image = new Image(info, rx);

        delete rx;
        delete tx;
    }
    else
    {
        // Failed
        LOGWARN(L"Unable to fetch image x%08x", id);
    }

    LOGTRACE(L"Out: Camera::GetImage(x%08x)", id);

    return image;
}

bool
Camera::StartCapture(double duration, OutputMode outputMode, DWORD flags)
{
    LOGTRACE(L"In: Camera::StartCapture(output = %d)", (DWORD)outputMode);

    bool result = false;

    if (m_captureThread)
    {
        LOGINFO(L"There's a pre-existing capture, attempting to clean up");
        CleanupCapture();
    }

    // Get camera settings... this serves two purposes
    // 1. Ensure camera is in a state that will allow a photo to be taken
    // 2. Allows us to clean out any "pending" images that could have been
    //    created by user pressing shutter button
    PropertyValue up((UINT16)(GetDeviceInfo(false)->GetButtonPropertiesInverted() ? 2 : 1));

    LOGTRACE(L"Up value is %d (%s)", up.GetUINT16(), up.ToString().c_str());
    LOGTRACE(L"Getting latest camera settings");

    CameraSettings* settings = GetSettings(true);

    // Ensure the shutter control properties are correct
    if (settings->GetPropertyValue(Property::ShutterFullDown)->GetUINT16() != up.GetUINT16())
    {
        LOGWARN(L"ShutterFullDown is set, clearing");

        SetProperty(Property::ShutterFullDown, &up);

        // Re-fetch settings
        settings = GetSettings(true);
    }

    if (settings->GetPropertyValue(Property::ShutterFullDown)->GetUINT16() != up.GetUINT16())
    {
        LOGWARN(L"ShutterHalfDown is set, clearing");
        PropertyValue up((WORD)1);

        SetProperty(Property::ShutterHalfDown, &up);

        // Re-fetch settings
        settings = GetSettings(true);
    }

    while (WORD bufferStatus = settings->GetPropertyValue(Property::PhotoBufferStatus)->GetUINT16() != 0)
    {
        switch (bufferStatus)
        {
        case 0x0001:
            // Camera is busy getting a photo ready for us
            LOGTRACE(L"  Waiting for camera to have previous photo ready");
            Sleep(100);
            break;

        case 0x8001:
            // Photo is ready... we don't actually want it, so we'll just toss whatever we get back
        {
            LOGTRACE(L"  Previous photo is ready to retrieve");
            Image* iimage = GetImage(FULL_IMAGE);

            if (iimage)
            {
                delete iimage;
            }

            LOGTRACE(L"  Previous photo retrieved and discarded");
        }
        break;

        default:
            LOGERROR(L"Unknown status of photo buffer x%08x, this is not good", bufferStatus);

            throw CameraException(L"Unknown status of photo buffer");
        }

        // Re-fetch settings
        settings = GetSettings(true);
    }

    if (settings->GetPropertyValue(Property::ShutterButtonStatus)->GetUINT8() == 1)
    {
        LOGTRACE(L"  Shutter button is UP");

        PropertyValue* exposureTime = settings->GetPropertyValue(Property::ShutterSpeed);

        if (!exposureTime)
        {
            throw CameraException(L"Could not fetch exposure time");
        }

        m_captureThread = new CaptureThread(this, outputMode);

        DWORD exposureTimeRaw = exposureTime->GetUINT32();

        if (exposureTimeRaw == 0)
        {
            LOGTRACE(L"  Shutter time is BULB, using supplied duration %f", duration);
        }
        else
        {
            double requestedDuration = duration;
            duration = ((double)(exposureTimeRaw >> 16) / (double)(exposureTimeRaw & 0x0000ffff));
            LOGTRACE(L"  Shutter time is %s (%f s), we wanted (%f s) - letting camera figure it out", exposureTime->ToString().c_str(), duration, requestedDuration);
        }

        result = m_captureThread->StartCapture(duration);
    }
    else
    {
        LOGWARN(L"StartCapture: Cannot start capture as someone has their finger on the shutter button (%s)", settings->GetPropertyValue(Property::ShutterButtonStatus)->ToString().c_str());
        throw CameraException(L"Cannot start capture as shutter already open");
    }

    LOGTRACE(L"Out: Camera::StartCapture() - returning %d", result);

    return result;
}

CaptureStatus
Camera::GetCaptureStatus()
{
    CaptureStatus result = CaptureStatus::Failed;
    CaptureThread* capture = m_captureThread;

    if (capture)
    {
        result = capture->GetStatus();

        switch (result)
        {
        case CaptureStatus::Complete:
            LOGTRACE(L"Camera::GetCaptureStatus() - Capture Complete");
            break;

        case CaptureStatus::Cancelled:
            LOGTRACE(L"Camera::GetCaptureStatus() - Capture Cancelled");
            break;

        case CaptureStatus::Capturing:
            LOGTRACE(L"Camera::GetCaptureStatus() - Capture Capturing");
            break;

        case CaptureStatus::Created:
            LOGTRACE(L"Camera::GetCaptureStatus() - Capture Created (not started yet)");
            break;

        case CaptureStatus::Failed:
            LOGTRACE(L"Camera::GetCaptureStatus() - Capture Failed");
            break;

        case CaptureStatus::Processing:
            LOGTRACE(L"Camera::GetCaptureStatus() - Capture Processing Data");
            break;

        case CaptureStatus::Reading:
            LOGTRACE(L"Camera::GetCaptureStatus() - Capture Reading From Camera");
            break;

        case CaptureStatus::Starting:
            LOGTRACE(L"Camera::GetCaptureStatus() - Capture Starting");
            break;

        default:
            LOGWARN(L"Camera::GetCaptureStatus() - Unknown Capture State %d", capture->GetStatus());
            break;
        }
    }
    else
    {
        LOGWARN(L"Asked for status of a capture that doesn't exist");

        throw CameraException(L"No capture exists for this camera");
    }

    return result;
}

Image*
Camera::GetCapturedImage()
{
    if (m_captureThread)
    {
        return m_captureThread->GetImage();
    }
    else
    {
        throw CameraException(L"No active capture, therefore no image to retrieve");
    }
}

bool
Camera::CancelCapture()
{
    if (m_captureThread)
    {
        m_captureThread->CancelCapture();
    }

    return true;
}

void
Camera::CleanupCapture()
{
    if (m_captureThread)
    {
        delete m_captureThread;
        m_captureThread = nullptr;
    }
}

DWORD WINAPI
Camera::_runHandlerThread(LPVOID lpParameter)
{
    LOGTRACE(L"In: Camera::_runHandlerThread(x%08x)", lpParameter);

    DWORD result = ((Camera*)lpParameter)->RunHandlerThread();

    LOGTRACE(L"Out: Camera::_runHandlerThread(x%08x) - returning %d", lpParameter, result);

    return result;
}

DWORD
Camera::RunHandlerThread()
{
    DWORD waitResult = 0;

    // Waits for work to do
    while (!m_shutdown)
    {
        if (WaitForSingleObject(m_hWakeEvent, INFINITE) == WAIT_OBJECT_0)
        {
            if (WaitForSingleObject(m_hBusyMutex, MUTEX_TIMEOUT) == WAIT_OBJECT_0)
            {
                // Process Queue

                ReleaseMutex(m_hBusyMutex);
            }
        }
    }

    return 0;
}
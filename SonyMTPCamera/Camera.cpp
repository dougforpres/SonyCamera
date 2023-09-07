#include "pch.h"
#include "Camera.h"
#include "Registry.h"
#include "Logger.h"
#include "CameraException.h"
#include <iostream>

#define THREAD_WAIT_EXIT_LOOPS 20
#define THREAD_WAIT_EXIT_SLEEP 100
#define MUTEX_TIMEOUT 10000

Camera::Camera(Device* device)
    : m_device(device)
{
    m_hBusyMutex = CreateMutex(nullptr, false, nullptr);
    pWorker = new CameraWorker(this);
}

Camera::~Camera()
{
    delete pWorker;
    pWorker = nullptr;

    if (m_settings)
    {
        delete m_settings;
        m_settings = nullptr;
    }

    CloseHandle(m_hBusyMutex);

    m_hBusyMutex = INVALID_HANDLE_VALUE;
}

CameraWorker*
Camera::GetWorker()
{
    return pWorker;
}

const std::wstring
Camera::GetId() const
{
    return m_device->GetId();
}

void
Camera::SetInitialized(bool value)
{
    initialized = value;
}

bool
Camera::IsInitialized() const
{
    return initialized;
}

void
Camera::OnPropertiesUpdated()
{
    RefreshPropertiesTask* task = new RefreshPropertiesTask();

    if (!task->QueueAndForget(this))
    {
        LOGTRACE(L"Removing unqueued task");
        // It didn't get queued
        delete task;
    }
}

void
Camera::OnImageBufferStatus(ImageBufferStatus status)
{
    LOGINFO(L"Image Buffer Status Update %d", status);

    if (status == ImageBufferStatus::ImageReady)
    {
        if (!m_takePhotoTaskParams) {
            m_takePhotoTaskParams = new TakePhotoTaskParams(0, OutputMode::PASSTHRU);
        }

        m_downloadAndProcessImageTask = new DownloadAndProcessImageTask(m_takePhotoTaskParams);

        m_downloadAndProcessImageTask->Start(this);
    }
}

HANDLE
Camera::Open()
{
    pWorker->OnCameraStateChange(CameraState::Connecting);

    HANDLE hCamera = m_device->Open();

    if (hCamera != INVALID_HANDLE_VALUE)
    {
        if (m_device->GetOpenCount() == 1)
        {
            m_device->StartNotifications(this);
        }

        pWorker->OnCameraStateChange(CameraState::Connected);
        RefreshDeviceInfo();
    }

    return hCamera;
}

bool
Camera::Close()
{
    if (m_device->GetOpenCount() == 1)
    {
        m_device->StopNotifications();
    }

    bool result = m_device->Close();

    if (result)
    {
        SetInitialized(false);
    }

    pWorker->OnCameraStateChange(CameraState::Disconnected);

    return result;
}

const DeviceInfo
Camera::GetDeviceInfo() const
{
    return m_deviceInfo;
}

bool
Camera::RefreshDeviceInfo()
{
    m_device->Open();

    Message* tx;
    Message* rx;

    tx = new Message(COMMAND_GET_DEVICE_INFO);
    rx = m_device->Receive(tx);

    LOGTRACE(L"GetDeviceInfo() >> %s", rx->Dump().c_str());

    DeviceInfo newDeviceInfo(rx);

    delete tx;
    delete rx;

    m_device->Close();

    ProcessDeviceInfoOverrides(newDeviceInfo);
    newDeviceInfo.DumpToLog();

    DWORD waitResult = WaitForSingleObject(m_hBusyMutex, 1000);

    if (waitResult == WAIT_OBJECT_0)
    {
        m_deviceInfo = newDeviceInfo;
    }

    ReleaseMutex(m_hBusyMutex);

    return true;
}

void
Camera::LoadFakeProperties(CameraSettings* settings)
{
    // Add Magic infos
    CameraPropertyFactory f;

    CameraProperty* p = f.Create(Property::PossibleExposureTimes);
    DeviceInfo deviceInfo = GetDeviceInfo();
    p->SetCurrentValue(new PropertyValue(0));

    PropertyInfo* inf = new PropertyInfo();

    inf->SetDefault(new PropertyValue((UINT32)0));
    inf->SetId(Property::PossibleExposureTimes);
    inf->SetType(DataType::UINT32);
    inf->SetFormMode(FormMode::ENUMERATION);

    std::list<PropertyValue*> values;
    std::list<DWORD> et = deviceInfo.GetExposureTimes();

    for (std::list<DWORD>::iterator ei = et.begin(); ei != et.end(); ei++)
    {
        values.push_back(new PropertyValue((UINT32)*ei));
    }

    inf->SetEnumeration(values);
    p->SetInfo(inf);

    settings->AddProperty(p);

    p = f.Create(Property::PossibleISOs);

    p->SetCurrentValue(new PropertyValue(0));

    inf = new PropertyInfo();

    inf->SetDefault(new PropertyValue((UINT32)0));
    inf->SetId(Property::PossibleISOs);
    inf->SetType(DataType::UINT32);
    inf->SetFormMode(FormMode::ENUMERATION);

    values.clear();

    et = deviceInfo.GetISOs();

    for (std::list<DWORD>::iterator ei = et.begin(); ei != et.end(); ei++)
    {
        values.push_back(new PropertyValue((UINT32)*ei));
    }

    inf->SetEnumeration(values);
    p->SetInfo(inf);
    settings->AddProperty(p);

    // Some cameras do not return info for the shutter control "buttons"
    // if they're missing, lets create some fake ones that always return "up"
    // but mimic the expected button-type so other pieces of the code can
    // continue to work
    if (!settings->GetProperty(Property::ShutterHalfDown))
    {
        p = f.Create(Property::ShutterHalfDown);

        p->SetCurrentValue(new PropertyValue((UINT8)0));

        inf = new PropertyInfo();

        inf->SetDefault(new PropertyValue((UINT8)0));
        inf->SetId(Property::ShutterHalfDown);
        inf->SetType(DataType::UINT8);
        inf->SetFormMode(FormMode::NONE);
        inf->SetAccess(Accessibility::WRITE_ONLY_BUTTON);
        inf->SetSonySpare(1);
        p->SetInfo(inf);

        settings->AddProperty(p);
    }

    if (!settings->GetProperty(Property::ShutterFullDown))
    {
        p = f.Create(Property::ShutterFullDown);

        p->SetCurrentValue(new PropertyValue((UINT8)0));

        inf = new PropertyInfo();

        inf->SetDefault(new PropertyValue((UINT8)0));
        inf->SetId(Property::ShutterFullDown);
        inf->SetType(DataType::UINT8);
        inf->SetFormMode(FormMode::NONE);
        inf->SetAccess(Accessibility::WRITE_ONLY_BUTTON);
        inf->SetSonySpare(1);
        p->SetInfo(inf);
        settings->AddProperty(p);
    }
}

Device *
Camera::GetDevice()
{
    return m_device;
}

bool
Camera::ProcessDeviceInfoOverrides(DeviceInfo& deviceInfo)
{
    std::wostringstream builder;

    builder << L"Cameras\\" << m_device->GetRegistryPath();

    std::wstring cameraPath = builder.str();

    registry.Open();

    deviceInfo.SetSensorName(registry.GetString(cameraPath, L"Sensor Name", deviceInfo.GetSensorName()));
    deviceInfo.SetSensorPixelWidth(registry.GetDouble(cameraPath, L"Sensor X Size um", deviceInfo.GetSensorPixelWidth()));
    deviceInfo.SetSensorPixelHeight(registry.GetDouble(cameraPath, L"Sensor Y Size um", deviceInfo.GetSensorPixelHeight()));
    deviceInfo.SetSensorXResolution(registry.GetDWORD(cameraPath, L"Sensor X Resolution", deviceInfo.GetSensorXResolution()));
    deviceInfo.SetSensorYResolution(registry.GetDWORD(cameraPath, L"Sensor Y Resolution", deviceInfo.GetSensorYResolution()));
    deviceInfo.SetSensorXCroppedResolution(registry.GetDWORD(cameraPath, L"AutoCropped X Resolution", deviceInfo.GetSensorXCroppedResolution()));
    deviceInfo.SetSensorYCroppedResolution(registry.GetDWORD(cameraPath, L"AutoCropped Y Resolution", deviceInfo.GetSensorYCroppedResolution()));
    deviceInfo.SetPreviewXResolution(registry.GetDWORD(cameraPath, L"Preview X Resolution", deviceInfo.GetPreviewXResolution()));
    deviceInfo.SetPreviewYResolution(registry.GetDWORD(cameraPath, L"Preview Y Resolution", deviceInfo.GetPreviewYResolution()));
    deviceInfo.SetExposureTimeMin(registry.GetDouble(cameraPath, L"Exposure Time Min", deviceInfo.GetExposureTimeMin()));
    deviceInfo.SetExposureTimeMax(registry.GetDouble(cameraPath, L"Exposure Time Max", deviceInfo.GetExposureTimeMax()));
    deviceInfo.SetExposureTimeStep(registry.GetDouble(cameraPath, L"Exposure Time Step", deviceInfo.GetExposureTimeStep()));
    deviceInfo.SetSensorType((SensorType)registry.GetDWORD(cameraPath, L"Sensor Type", (DWORD)deviceInfo.GetSensorType()));
    deviceInfo.SetSupportsLiveview((bool)registry.GetDWORD(cameraPath, L"Supports Liveview", deviceInfo.GetSupportsLiveview()));
    deviceInfo.SetCropMode((CropMode)registry.GetDWORD(cameraPath, L"Crop Mode", (DWORD)deviceInfo.GetCropMode()));
    deviceInfo.SetLeftCrop((UINT16)registry.GetDWORD(cameraPath, L"Crop Left", deviceInfo.GetLeftCrop()));
    deviceInfo.SetRightCrop((UINT16)registry.GetDWORD(cameraPath, L"Crop Right", deviceInfo.GetRightCrop()));
    deviceInfo.SetTopCrop((UINT16)registry.GetDWORD(cameraPath, L"Crop Top", deviceInfo.GetTopCrop()));
    deviceInfo.SetBottomCrop((UINT16)registry.GetDWORD(cameraPath, L"Crop Bottom", deviceInfo.GetBottomCrop()));
    deviceInfo.SetButtonPropertiesInverted((bool)registry.GetDWORD(cameraPath, L"Button Properties Inverted", deviceInfo.GetButtonPropertiesInverted()));
    deviceInfo.SetBitsPerPixel(registry.GetDWORD(cameraPath, L"Bits Per Pixel", deviceInfo.GetBitsPerPixel()));

    std::wistringstream exposureTimes(registry.GetString(cameraPath, L"Exposure Times", L""));
    std::list<DWORD> times;
    std::wstring s;

    while (std::getline(exposureTimes, s, L',')) {
        times.push_back(_wtoi(s.c_str()));
    }

    deviceInfo.SetExposureTimes(times);

    std::wistringstream isos(registry.GetString(cameraPath, L"ISOs", L""));
    std::list<DWORD> iso;

    while (std::getline(isos, s, L',')) {
        iso.push_back(_wtoi(s.c_str()));
    }

    deviceInfo.SetISOs(iso);

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

void
Camera::DoRefreshProperties()
{
    RefreshPropertiesTask rpt;

    rpt.Run(this);
}

bool
Camera::StartCapture(double duration, OutputMode outputMode, DWORD flags)
{
    LOGTRACE(L"In: Camera::StartCapture(output = %d)", (DWORD)outputMode);

    bool result = false;

    DWORD waitResult = WaitForSingleObject(m_hBusyMutex, 1000);

    if (waitResult == WAIT_OBJECT_0)
    {
        if (m_takePhotoTask)
        {
            {
                LOGINFO(L"There's a pre-existing capture, attempting to clean up");
                if (m_takePhotoTask->Cancel(20000) == WAIT_OBJECT_0)
                {
                    // Task successfully cancelled
                    delete m_takePhotoTask;
                    m_takePhotoTask = nullptr;
                }
                //                CleanupCapture();
            }
        }

        if (m_downloadAndProcessImageTask)
        {
            {
                LOGINFO(L"There's a pre-existing download, attempting to clean up");
                if (m_downloadAndProcessImageTask->Cancel(20000) == WAIT_OBJECT_0)
                {
                    // Task successfully cancelled
                    delete m_downloadAndProcessImageTask;;
                    m_downloadAndProcessImageTask = nullptr;
                }
                //                CleanupCapture();
            }
        }

        if (m_takePhotoTaskParams)
        {
            delete m_takePhotoTaskParams;
            m_takePhotoTaskParams = nullptr;
        }

        SetCaptureStatus(CaptureStatus::Created);
    }

    ReleaseMutex(m_hBusyMutex);

    // Get camera settings... this serves two purposes
    // 1. Ensure camera is in a state that will allow a photo to be taken
    // 2. Allows us to clean out any "pending" images that could have been
    //    created by user pressing shutter button
    PropertyValue up((UINT16)(GetDeviceInfo().GetButtonPropertiesInverted() ? 2 : 1));

    LOGTRACE(L"Up value is %d (%s)", up.GetUINT16(), up.ToString().c_str());
    LOGTRACE(L"Getting latest camera settings");

    DoRefreshProperties();

    std::unique_ptr<CameraSettings> cs(GetSettings());

    // Ensure the shutter control properties are correct

    // These null checks are necessary as some models advertise the property
    // but then don't actually return it.
    // If there are too many I may add some code to support default values
    // so we don't need all these checks.
    PropertyValue* v = cs->GetPropertyValue(Property::ShutterFullDown);

    if (!v || v->GetUINT16() != up.GetUINT16())
    {
        LOGWARN(L"ShutterFullDown is %s, clearing", v ? L"set" : L"unreported");

        SetProperty(Property::ShutterFullDown, &up);

        // Re-fetch settings
        DoRefreshProperties();
    }

    std::unique_ptr<CameraSettings> cs2(GetSettings());
    v = cs2->GetPropertyValue(Property::ShutterHalfDown);

    if (!v || v->GetUINT16() != up.GetUINT16())
    {
        LOGWARN(L"ShutterHalfDown is %s, clearing", v ? L"set" : L"unreported");
        PropertyValue up((WORD)1);

        SetProperty(Property::ShutterHalfDown, &up);

        // Re-fetch settings
        DoRefreshProperties();
    }

    while (WORD bufferStatus = cs->GetPropertyValue(Property::PhotoBufferStatus)->GetUINT16() != 0)
    {
        std::unique_ptr<CameraSettings> cs(GetSettings());

        if (bufferStatus == 0x0001)
        {
            // Camera is busy getting a photo ready for us
            LOGTRACE(L"  Waiting for camera to have previous photo ready");
            Sleep(100);
        }

        if (bufferStatus & 0x8000)
        {
            // Photo is ready... we don't actually want it, so we'll just toss whatever we get back
            LOGTRACE(L"  Previous photo is ready to retrieve");
            Image* iimage = GetImage(FULL_IMAGE);

            if (iimage)
            {
                delete iimage;
            }

            LOGTRACE(L"  Previous photo retrieved and discarded");
        }

        // Re-fetch settings
        DoRefreshProperties();
    }

    cs = std::unique_ptr<CameraSettings>(GetSettings());

    if (cs->GetPropertyValue(Property::ShutterButtonStatus)->GetUINT8() == 1)
    {
        LOGTRACE(L"  Shutter button is UP");

        PropertyValue* exposureTime = cs->GetPropertyValue(Property::ShutterSpeed);

        if (exposureTime->GetType() == DataType::UNKNOWN)
        {
            throw CameraException(L"Could not fetch exposure time");
        }

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

        m_takePhotoTaskParams = new TakePhotoTaskParams(duration, outputMode);
        m_takePhotoTask = new TakePhotoTask(m_takePhotoTaskParams);

        m_takePhotoTask->Start(this);

        result = true;
    }
    else
    {
        LOGWARN(L"StartCapture: Cannot start capture as someone has their finger on the shutter button (%s)", cs->GetPropertyValue(Property::ShutterButtonStatus)->ToString().c_str());
        throw CameraException(L"Cannot start capture as shutter already open");
    }

    LOGTRACE(L"Out: Camera::StartCapture() - returning %d", result);

    return result;
}

CameraSettings*
Camera::GetSettings() const
{
    DWORD waitResult = WaitForSingleObject(m_hBusyMutex, 1000);
    CameraSettings* result = nullptr;

    if (waitResult == WAIT_OBJECT_0)
    {
        result = new CameraSettings(*m_settings);
    }

    ReleaseMutex(m_hBusyMutex);

    return result;
}

CameraProperty*
Camera::GetProperty(Property id) const
{
    DWORD waitResult = WaitForSingleObject(m_hBusyMutex, 1000);
    CameraProperty* result = nullptr;

    if (waitResult == WAIT_OBJECT_0)
    {
        CameraProperty* existing = m_settings->GetProperty(id);

        if (existing)
        {
            result = existing->Clone();
        }
    }

    ReleaseMutex(m_hBusyMutex);

    return result;
}

void
Camera::SetCaptureStatus(CaptureStatus status)
{
    LOGTRACE(L"Set capture status to: %d", status);
    DWORD waitResult = WaitForSingleObject(m_hBusyMutex, 1000);

    if (waitResult == WAIT_OBJECT_0)
    {
        m_captureStatus = status;
    }

    ReleaseMutex(m_hBusyMutex);
}

const CaptureStatus
Camera::GetCaptureStatus()
{
    CaptureStatus result = CaptureStatus::Failed;

    DWORD waitResult = WaitForSingleObject(m_hBusyMutex, 1000);

    if (waitResult == WAIT_OBJECT_0)
    {
        result = m_captureStatus;
    }

    ReleaseMutex(m_hBusyMutex);

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
        LOGWARN(L"Camera::GetCaptureStatus() - Unknown Capture State %d", result);
        break;
    }

    return result;
}

Image*
Camera::GetCapturedImage()
{
    DWORD waitResult = WaitForSingleObject(m_hBusyMutex, 1000);
    Image* image = nullptr;

    if (waitResult == WAIT_OBJECT_0)
    {
        if (m_downloadAndProcessImageTask && GetCaptureStatus() == CaptureStatus::Complete)//m_downloadAndProcessImageTask && m_downloadAndProcessImageTask->IsComplete())
        {
            image = m_downloadAndProcessImageTask->GetImage();
        }
    }

    ReleaseMutex(m_hBusyMutex);

    if (image)
    {
        return image;
    }

    throw CameraException(L"No active capture, therefore no image to retrieve");
}

bool
Camera::CancelCapture()
{
    DWORD waitResult = WaitForSingleObject(m_hBusyMutex, 1000);

    if (waitResult == WAIT_OBJECT_0)
    {
        if (m_takePhotoTask)
        {
            m_takePhotoTask->Cancel(20000);
            delete m_takePhotoTask;
            m_takePhotoTask = nullptr;
        }
    }

    ReleaseMutex(m_hBusyMutex);

    return true;
}

void
Camera::CleanupCapture()
{
    CancelCapture();
}

#include "pch.h"
#include "Camera.h"
#include "Registry.h"
#include "Logger.h"
#include "CameraException.h"
#include <iostream>

constexpr auto THREAD_WAIT_EXIT_LOOPS = 20;
constexpr auto THREAD_WAIT_EXIT_SLEEP = 100;
constexpr auto MUTEX_TIMEOUT = 10000;
// Must stay below the 20 s callers wait for the camera lock (DEFAULT_LOCK_WAIT), so close/open can still get in
constexpr ULONGLONG PREVIOUS_PHOTO_TIMEOUT = 15000;
// 'Processing' (x0001) here is normally gone within a couple of seconds; when it is stuck
// (see below) waiting the full timeout only delays the exposure
constexpr ULONGLONG PROCESSING_STATUS_TIMEOUT = 3000;

// After USB is pulled mid-exposure an ILCE-7M3 keeps reporting 'processing' (x0001) even
// while taking and delivering new photos normally, across reconnects. Once a capture has
// had to go ahead regardless, stop waiting on that status, until the camera reports an
// empty buffer again.
static volatile bool s_processingStatusUnreliable = false;

constexpr DWORD TASK_CANCEL_TIMEOUT = 20000;

// Cancel a task the worker may still hold, and delete it once the worker has finished with it.
// If it does not finish in time, leak it rather than free memory the worker is still using.
// (Templated so the derived destructor runs; ~CameraTask is not virtual.)
template <typename T>
static bool DisposeTask(T* task)
{
    if (!task)
    {
        return true;
    }

    if (task->Cancel(TASK_CANCEL_TIMEOUT) != WAIT_OBJECT_0)
    {
        LOGERROR(L"Task %s did not finish within %d ms of being cancelled; not deleting it while it may still be in use", task->Name().c_str(), TASK_CANCEL_TIMEOUT);

        return false;
    }

    delete task;

    return true;
}

Camera::Camera(Device* device)
    : m_device(device)
{
    m_hBusyMutex = CreateMutex(nullptr, false, nullptr);
    pWorker = new CameraWorker(this);
}

Camera::~Camera()
{
    // A device event callback may still be using this camera (it can outlive Close)
    m_device->DrainNotifications(NOTIFICATION_DRAIN_TIMEOUT);

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
    RefreshPropertiesTask* task = new RefreshPropertiesTask(false);

    if (!task->QueueAndForget(this))
    {
        LOGTRACE(L"Removing unqueued task");
        // It didn't get queued
        delete task;
    }
}

void
Camera::OnDeviceRemoved()
{
    LOGWARN(L"Camera device removed - failing the current capture and any new ones until it is reopened");

    // Whatever is in progress can never complete now; let the caller stop waiting.
    // Read the status before setting the flag, as GetCaptureStatus reports Failed once it is set.
    CaptureStatus status = GetCaptureStatus();

    InterlockedExchange(&m_deviceRemoved, 1);

    if (status != CaptureStatus::Complete && status != CaptureStatus::Cancelled && status != CaptureStatus::Failed)
    {
        SetCaptureStatus(CaptureStatus::Failed);
    }
}

bool
Camera::IsDeviceRemoved() const
{
    return m_deviceRemoved != 0;
}

void
Camera::EndCaptureWindow()
{
    InterlockedExchange(&m_preparingCapture, 0);
}

void
Camera::OnImageBufferStatus(ImageBufferStatus status)
{
    LOGINFO(L"Image Buffer Status Update %d", status);

    if (status == ImageBufferStatus::ImageReady && IsDeviceRemoved())
    {
        LOGWARN(L"Ignoring image-ready notification, the device has been removed");
        return;
    }

    if (status == ImageBufferStatus::ImageReady && m_preparingCapture)
    {
        // Photos still waiting are picked up by the worker's idle check once the camera is free
        LOGINFO(L"Ignoring image-ready notification, a capture is being prepared or the shutter is open");
        return;
    }

    if (status == ImageBufferStatus::ImageReady)
    {
        // Called from the WPD event thread (and the worker), concurrently with StartCapture,
        // so only touch the task pointers under the lock.
        if (WaitForSingleObject(m_hBusyMutex, 1000) != WAIT_OBJECT_0)
        {
            LOGWARN(L"Camera busy, ignoring image-ready notification");
            return;
        }

        if (m_downloadAndProcessImageTask)
        {
            // A download is already queued, running, or holding an image not yet collected.
            // A stuck camera can send these notifications repeatedly; replacing the task here
            // lost the image or freed the task while it ran.
            ReleaseMutex(m_hBusyMutex);
            LOGINFO(L"Ignoring image-ready notification, a download is already in progress or waiting to be collected");
            return;
        }

        if (!m_takePhotoTaskParams) {
            m_takePhotoTaskParams = new TakePhotoTaskParams(0, OutputMode::PASSTHRU);
        }

        DownloadAndProcessImageTask* task = new DownloadAndProcessImageTask(m_takePhotoTaskParams);
        m_downloadAndProcessImageTask = task;

        ReleaseMutex(m_hBusyMutex);

        // Queue outside the lock: queuing waits for the worker, which can be mid-exposure
        task->Start(this);
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
            // A fresh open means the device is (back) on USB
            InterlockedExchange(&m_deviceRemoved, 0);
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
Camera::LoadFakeProperties(CameraSettings* settings) const
{
    // Add Magic infos
    CameraPropertyFactory f;

    CameraProperty* p = f.Create(Property::BatteryTemperature);
    p->SetCurrentValue(new PropertyValue((INT16)0xffff));

    PropertyInfo* inf = new PropertyInfo();

    inf->SetDefault(new PropertyValue((INT16)0xffff));
    inf->SetId(Property::BatteryTemperature);
    inf->SetType(DataType::INT16);
    inf->SetFormMode(FormMode::NONE);

    p->SetInfo(inf);

    settings->AddProperty(p);

    p = f.Create(Property::FocusPosition);
    p->SetCurrentValue(new PropertyValue((UINT16)0xffff));

    inf = new PropertyInfo();

    inf->SetDefault(new PropertyValue((UINT16)0xffff));
    inf->SetId(Property::FocusPosition);
    inf->SetType(DataType::UINT16);
    inf->SetFormMode(FormMode::NONE);

    p->SetInfo(inf);

    settings->AddProperty(p);

    p = f.Create(Property::PossibleExposureTimes);
    DeviceInfo deviceInfo = GetDeviceInfo();
    p->SetCurrentValue(new PropertyValue(0));

    inf = new PropertyInfo();

    inf->SetDefault(new PropertyValue((UINT32)0));
    inf->SetId(Property::PossibleExposureTimes);
    inf->SetType(DataType::UINT32);
    inf->SetFormMode(FormMode::ENUMERATION);

    std::list<PropertyValue*> values;
    std::list<DWORD> et = deviceInfo.GetExposureTimes();

    for (auto pv: et)
    {
        values.push_back(new PropertyValue((UINT32)pv));
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

    for (auto pv: et)
    {
        values.push_back(new PropertyValue((UINT32)pv));
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
        LOGTRACE(L"Adding fake 'ShutterHalfDown' property as camera forgot to enumerate");
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
        LOGTRACE(L"Adding fake 'ShutterFullDown' property as camera forgot to enumerate");
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

    // Some cameras don't return the focus-control property
    if (!settings->GetProperty(Property::FocusControl))
    {
        LOGTRACE(L"Adding fake 'ShutterControl' property as camera forgot to enumerate");
        p = f.Create(Property::FocusControl);

        p->SetCurrentValue(new PropertyValue((INT16)0));

        inf = new PropertyInfo();

        inf->SetDefault(new PropertyValue((UINT16)0));
        inf->SetId(Property::FocusControl);
        inf->SetType(DataType::INT16);
        inf->SetFormMode(FormMode::RANGE);
        inf->SetAccess(Accessibility::UNKNOWN_82);
        inf->SetSonySpare(1);
        inf->SetRange(new PropertyValue((UINT16)1), new PropertyValue((UINT16)7), new PropertyValue((UINT16)0xfff9));
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
//    deviceInfo.SetFocusLimit((UINT16)registry.GetDWORD(cameraPath, L"Focus Limit", deviceInfo.GetFocusLimit()));
//    deviceInfo.SetFocusMagicNumber(registry.GetDouble(cameraPath, L"Focus Magic Number", deviceInfo.GetFocusMagicNumber()));
    deviceInfo.SetFocusStartMode((FocusStartMode)registry.GetDWORD(cameraPath, L"Focus Start Mode", (DWORD)deviceInfo.GetFocusStartMode()));
//    deviceInfo.SetHandsOffFocus(registry.GetDWORD(cameraPath, L"Hands Off Focus", deviceInfo.GetHandsOffFocus() ? 1 : 0) ? true : false);

    std::wstring s;
    std::wistringstream exposureTimes(registry.GetString(cameraPath, L"Exposure Times", L""));
    std::list<DWORD> times;

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
        // Failed - the image did not take ownership of info, so clean it up here
        LOGWARN(L"Unable to fetch image x%08x", id);
        delete info;
    }

    LOGTRACE(L"Out: Camera::GetImage(x%08x)", id);

    return image;
}

void
Camera::DoRefreshProperties()
{
    RefreshPropertiesTask rpt(false);

    rpt.Run(this);
}

bool
Camera::StartCapture(double duration, OutputMode outputMode, DWORD flags)
{
    LOGTRACE(L"In: Camera::StartCapture(duration = %f, output = %d)", duration,(DWORD)outputMode);

    if (IsDeviceRemoved())
    {
        LOGERROR(L"  Not starting capture, the device has been removed");
        throw CameraException(L"Camera has been disconnected");
    }

    // Ignore image-ready events until the shutter is released. If no capture gets handed to the
    // worker (an exception, or the shutter is busy), reopen the window here instead.
    struct PreparingGuard
    {
        volatile long& flag;
        bool handedOff = false;
        explicit PreparingGuard(volatile long& f) : flag(f) { InterlockedExchange(&flag, 1); }
        ~PreparingGuard() { if (!handedOff) InterlockedExchange(&flag, 0); }
    } preparing(m_preparingCapture);

    bool result = false;

    // Take the previous tasks over under the lock, but wait for them outside it: the worker
    // needs the lock (with short timeouts) to finish them.
    TakePhotoTask* oldPhotoTask = nullptr;
    DownloadAndProcessImageTask* oldDownloadTask = nullptr;
    TakePhotoTaskParams* oldParams = nullptr;

    DWORD waitResult = WaitForSingleObject(m_hBusyMutex, 1000);

    if (waitResult == WAIT_OBJECT_0)
    {
        oldPhotoTask = m_takePhotoTask;
        oldDownloadTask = m_downloadAndProcessImageTask;
        oldParams = m_takePhotoTaskParams;
        m_takePhotoTask = nullptr;
        m_downloadAndProcessImageTask = nullptr;
        m_takePhotoTaskParams = nullptr;

        SetCaptureStatus(CaptureStatus::Created);

        ReleaseMutex(m_hBusyMutex);
    }

    if (oldPhotoTask)
    {
        LOGINFO(L"There's a pre-existing capture, attempting to clean up");
    }

    if (oldDownloadTask)
    {
        LOGINFO(L"There's a pre-existing download, attempting to clean up");
    }

    bool photoTaskDone = DisposeTask(oldPhotoTask);
    bool downloadTaskDone = DisposeTask(oldDownloadTask);

    // Both tasks point at the params, so only free them once neither can run any more
    if (photoTaskDone && downloadTaskDone)
    {
        delete oldParams;
    }

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

    // Re-read the status every pass and give up eventually - this holds the camera lock, so if the
    // camera has gone (e.g. USB unplugged) an endless wait here blocks close/open too
    PropertyValue* bufferStatusValue = cs->GetPropertyValue(Property::PhotoBufferStatus);
    WORD bufferStatus = bufferStatusValue ? bufferStatusValue->GetUINT16() : 0;
    ULONGLONG waitStart = GetTickCount64();

    if (bufferStatus == 0)
    {
        s_processingStatusUnreliable = false;
    }

    while (bufferStatus != 0)
    {
        if (bufferStatus == 0x0001 && s_processingStatusUnreliable)
        {
            LOGTRACE(L"  Camera reports 'processing' but that status has been stuck, not waiting");
            break;
        }

        if (IsDeviceRemoved())
        {
            LOGERROR(L"  Device removed while waiting for previous photo to clear, giving up");
            throw CameraException(L"Camera has been disconnected");
        }

        ULONGLONG waited = GetTickCount64() - waitStart;
        ULONGLONG timeout = bufferStatus == 0x0001 ? PROCESSING_STATUS_TIMEOUT : PREVIOUS_PHOTO_TIMEOUT;

        if (waited > timeout)
        {
            // Seen after USB was pulled mid-exposure: the reopened camera reports "processing"
            // indefinitely for the interrupted shot. Failing here failed every later exposure,
            // so carry on; a dead camera is caught by the device-removed flag instead.
            LOGWARN(L"  Camera still reports a previous photo (x%04x) after %d ms, starting the new capture anyway", bufferStatus, (int)waited);

            if (bufferStatus == 0x0001)
            {
                s_processingStatusUnreliable = true;
            }

            break;
        }

        // Do not try to fetch the photo while the camera reports 'processing' (x0001): on an
        // ILCE-7M3 that made the camera drop off USB within ~2 s.
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
        else
        {
            // Camera is busy getting a photo ready for us
            LOGTRACE(L"  Waiting for camera to have previous photo ready (x%04x)", bufferStatus);
            Sleep(100);
        }

        // Re-fetch settings
        DoRefreshProperties();

        std::unique_ptr<CameraSettings> csLoop(GetSettings());
        bufferStatusValue = csLoop->GetPropertyValue(Property::PhotoBufferStatus);
        bufferStatus = bufferStatusValue ? bufferStatusValue->GetUINT16() : 0;
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

        // doCapture reopens the image-ready window once the shutter is released
        preparing.handedOff = true;
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

    // Nothing still in progress can complete once the device is gone (an already
    // downloaded image can still be collected)
    if (IsDeviceRemoved() && result != CaptureStatus::Complete && result != CaptureStatus::Cancelled)
    {
        result = CaptureStatus::Failed;
    }

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
        // Update a couple of settings based on some EXIF data read from the image
        DWORD waitResult = WaitForSingleObject(m_hBusyMutex, 1000);
        CameraSettings* result = nullptr;

        if (waitResult == WAIT_OBJECT_0)
        {
            LOGTRACE(L"Updating temperature and focus properties from EXIF");

            CameraProperty* existing = m_settings->GetProperty(Property::BatteryTemperature);

            if (existing)
            {
                existing->SetCurrentValue(new PropertyValue((INT16)(image->GetBatteryTemperature() * 10)));
            }
            else
            {
                LOGERROR(L"Unable to find BatteryTemperature property");
            }

            existing = m_settings->GetProperty(Property::FocusPosition);

            if (existing)
            {
                existing->SetCurrentValue(new PropertyValue((UINT16)image->GetFocusPosition()));
            }
            else
            {
                LOGERROR(L"Unable to find FocusPosition property");
            }
        }

        ReleaseMutex(m_hBusyMutex);

        return image;
    }

    throw CameraException(L"No active capture, therefore no image to retrieve");
}

bool
Camera::CancelCapture()
{
    // As in StartCapture: take the task over under the lock, wait for it outside it
    TakePhotoTask* task = nullptr;

    if (WaitForSingleObject(m_hBusyMutex, 1000) == WAIT_OBJECT_0)
    {
        task = m_takePhotoTask;
        m_takePhotoTask = nullptr;
        ReleaseMutex(m_hBusyMutex);
    }

    if (!DisposeTask(task) && WaitForSingleObject(m_hBusyMutex, 1000) == WAIT_OBJECT_0)
    {
        // The task still uses the params; keep StartCapture from freeing them
        m_takePhotoTaskParams = nullptr;
        ReleaseMutex(m_hBusyMutex);
    }

    // A cancelled capture may never reach doCapture, which would normally do this
    EndCaptureWindow();

    SetCaptureStatus(CaptureStatus::Cancelled);

    return true;
}

void
Camera::CleanupCapture()
{
    CancelCapture();
}

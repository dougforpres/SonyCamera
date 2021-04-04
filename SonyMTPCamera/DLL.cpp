#include "pch.h"
#include "SonyMTPCamera.h"
#include <Ole2.h>
#include <algorithm>
#include "DeviceManager.h"
#include "CameraManager.h"
#include "SonyCamera.h"
#include "DeviceInfo.h"
#include "Logger.h"
#include <unordered_map>
#include "Registry.h"
#include "ResourceLoader.h"
#include "CameraException.h"
#include "Locker.h"

#define MAX_MANUFACTURER_SIZE   0x64
#define MAX_MODEL_SIZE          0x64
#define MAX_SERIAL_NUMBER_SIZE  0x64
#define MAX_DEVICE_NAME_SIZE    0x64

#define MAX_CAPTURE_MUTEX_WAIT 20000

#define MAX_EXPOSURE_SAME_COUNT 20

static DeviceManager* deviceManager = nullptr;
static CameraManager* cameraManager = nullptr;

void
Init()
{
    LOGINFO(L"Init() - Starting up");

    if (deviceManager == nullptr)
    {
        LOGINFO(L"GetDeviceManager: First time thru, creating new DeviceManager singleton");
        deviceManager = new DeviceManager();
    }
}

void
Shutdown()
{
    LOGINFO(L"Shutdown() - Shutting down");

    if (cameraManager)
    {
        LOGINFO(L"Destroying CameraManager");
        delete cameraManager;
        cameraManager = nullptr;
    }

    if (deviceManager)
    {
        LOGINFO(L"Destroying DeviceManager");
        delete deviceManager;
        deviceManager = nullptr;
    }
}

CameraManager*
GetCameraManager()
{
    if (cameraManager == nullptr)
    {
        LOGINFO(L"GetCameraManager: First time thru, creating new CameraManager singleton");
        cameraManager = new CameraManager();
    }

    return cameraManager;
}

LPWSTR
exportString(std::wstring input)
{
    /* you must use CoTaskMemAlloc to allocate the memory, not malloc, new, or anything else */
    LPWSTR returnedString = (LPWSTR)CoTaskMemAlloc((input.length() + 1) * sizeof(WCHAR));

    if (returnedString)
    {
        wcscpy_s(returnedString, input.length() + 1, input.c_str());
    }

    return returnedString;
}

BYTE*
exportBytes(BYTE* buffer, DWORD bufferLen)
{
    BYTE* returnedBytes = nullptr;
    
    if (bufferLen)
    {
        returnedBytes = (BYTE*)CoTaskMemAlloc(bufferLen);

        if (returnedBytes)
        {
            memcpy(returnedBytes, buffer, bufferLen);
        }
    }

    return returnedBytes;
}

void
RenderProperty(Property id, CameraProperty* property, PROPERTYVALUE* output)
{
    if (property)
    {
        PropertyValue* value = property->GetCurrentValue();

        switch (value->GetType())
        {
        case DataType::UINT8:
            output->value = value->GetUINT8();
            break;

        case DataType::UINT16:
            output->value = value->GetUINT16();
            break;

        case DataType::UINT32:
            output->value = value->GetUINT32();
            break;

        case DataType::INT8:
            output->value = value->GetINT8();
            break;

        case DataType::INT16:
            output->value = value->GetINT16();
            break;

        case DataType::INT32:
            output->value = value->GetINT32();
            break;

        default:
            output->value = 0;
            break;
        }

        output->id = (DWORD)id;
        output->text = exportString(property->AsString());
    }
}

Camera*
FindCamera(HANDLE hCamera)
{
    Camera *result = GetCameraManager()->GetCameraForHandle(hCamera);

    if (!result)
    {
        LOGWARN(L"FindCamera: Unable to find camera for handle x%p", hCamera);
    }

    return result;
}

DWORD
GetDeviceCount()
{
    LOGTRACE(L"In: GetDeviceCount()");

    deviceManager->RefreshDevices();

    DWORD count = deviceManager->GetFilteredDevices().size();

    LOGTRACE(L"Out: GetDeviceCount() = %d", count);

    return count;
}

HANDLE
OpenDevice(LPWSTR deviceName)
{
    return OpenDeviceEx(deviceName, 0);
}

HANDLE
OpenDeviceEx(LPWSTR deviceName, DWORD flags)
{
    LOGTRACE(L"In: OpenDeviceEx(%s, x%p)", deviceName, flags);

    HANDLE result = INVALID_HANDLE_VALUE;
    std::list<Device*> deviceList = (flags & OPENDEVICEEX_OPEN_ANY_DEVICE) ? deviceManager->GetAllDevices(false) : deviceManager->GetFilteredDevices();
    std::list<Device*>::iterator it;
    std::wstring deviceId = deviceName;

    LOGINFO(L"OpenDevice: Found %d devices... iterating...", deviceList.size());

    for (it = deviceList.begin(); it != deviceList.end() && result == INVALID_HANDLE_VALUE; it++)
    {
        std::wstring id = (*it)->GetId();
        LOGINFO(L"OpenCamera: Scanning %s (%s)", (*it)->GetFriendlyName().c_str(), (*it)->GetId().c_str());
        if ((*it)->GetId() == deviceId)
        {
            LOGINFO(L"OpenDevice: Found device... trying to create/open");
            result = GetCameraManager()->CreateCamera(*it, (flags & OPENDEVICEEX_OPEN_ANY_DEVICE) ? OPEN_OVERRIDE : 0);
            Camera* camera = GetCameraManager()->GetCameraForHandle(result);

            try
            {
                Locker lock(camera);

                if (!camera->Initialize())
                {
                    LOGERROR(L"Unable to initialize camera");
                    result = INVALID_HANDLE_VALUE;
                    GetCameraManager()->RemoveCamera(result);
                    camera = nullptr;
                }
            }
            catch (CameraException & gfe)
            {
                LOGERROR(L"Exception initializing camera: %s", gfe.GetMessage().c_str());

                result = INVALID_HANDLE_VALUE;
                GetCameraManager()->RemoveCamera(result);
                camera = nullptr;
            }
        }
    }

    LOGTRACE(L"Out: OpenDevice(%s) = x%08x", deviceName, result);

    return result;
}

void
CloseDevice(HANDLE hCamera)
{
    LOGTRACE(L"In: CloseDevice(x%p)", hCamera);

    Camera* camera = GetCameraManager()->GetCameraForHandle(hCamera);

    try
    {
        Locker lock(camera);

        if (camera->Close())
        {
            // This happens if the device is fully closed
            LOGINFO(L"Removing device x%p as it is no longer open", hCamera);

            GetCameraManager()->RemoveCamera(hCamera);
        }
        else
        {
            LOGINFO(L"Not removing device x%p as it is still open", hCamera);
        }
    }
    catch (CameraException & gfe)
    {
        LOGERROR(L"Exception closing camera: %s", gfe.GetMessage().c_str());
    }

    LOGTRACE(L"Out: CloseDevice(x%p)", hCamera);
}

HRESULT
GetDeviceInfo(DWORD deviceId, DEVICEINFO *info)
{
    LOGTRACE(L"In: GetDeviceInfo(x%p, @ x%p)", deviceId, info);

    if (!info || info->version != 1)
    {
        LOGWARN(L"Out: GetDeviceInfo(x%p, @ x%p) - Passed wrong version", deviceId, info);

        return ERROR_INCORRECT_SIZE;
    }

    std::list<Device*> deviceList = deviceManager->GetFilteredDevices();
    std::list<Device*>::iterator it = deviceList.begin();

    std::advance(it, deviceId);

    if (it != deviceList.end())
    {
        Device* device = (*it);// ->Clone();
        SonyCamera* camera = new SonyCamera(device);

        camera->Open();
        DeviceInfo* deviceInfo = camera->GetDeviceInfo(false);

        info->imageWidthPixels = deviceInfo->GetSensorXResolution();
        info->imageHeightPixels = deviceInfo->GetSensorYResolution();
        info->imageWidthCroppedPixels = deviceInfo->GetSensorXCroppedResolution();
        info->imageHeightCroppedPixels = deviceInfo->GetSensorYCroppedResolution();
        info->pixelWidth = deviceInfo->GetSensorPixelWidth();
        info->pixelHeight = deviceInfo->GetSensorPixelHeight();
        info->exposureTimeMin = deviceInfo->GetExposureTimeMin();
        info->exposureTimeMax = deviceInfo->GetExposureTimeMax();
        info->exposureTimeStep = deviceInfo->GetExposureTimeStep();
        info->bayerXOffset = deviceInfo->GetBayerXOffset();
        info->bayerYOffset = deviceInfo->GetBayerYOffset();
        info->cropMode = (DWORD)deviceInfo->GetCropMode();

        info->manufacturer = exportString(deviceInfo->GetManufacturer());
        info->model = exportString(deviceInfo->GetModel());
        info->serialNumber = exportString(deviceInfo->GetSerialNumber());
        info->deviceName = exportString((*it)->GetId());
        info->sensorName = exportString(deviceInfo->GetSensorName());
        info->deviceVersion = exportString(deviceInfo->GetVersion());

        delete deviceInfo;

        LOGTRACE(L"Out: GetDeviceInfo(x%p, @ x%p) - Returning data", deviceId, info);

        camera->Close();
        delete camera;

        return ERROR_SUCCESS;
    }
    else
    {
        LOGWARN(L"Out: GetDeviceInfo(x%p, @ x%p) - Device Not Found", deviceId, info);

        return ERROR_NOT_FOUND;
    }
}

HRESULT
GetPreviewImage(HANDLE hCamera, IMAGEINFO *info)
{
    LOGTRACE(L"In: GetPreviewImage(x%p, @ x%p)", hCamera, info);

    Camera* camera = GetCameraManager()->GetCameraForHandle(hCamera);

    try
    {
        Locker lock(camera);

        camera->GetSettings(true);

        // Set up some defaults
        info->data = nullptr;
        info->width = 0;
        info->height = 0;
        info->duration = 0.0;

        LOGDEBUG(L"Getting Preview");
        Image* image = camera->GetImage(PREVIEW_IMAGE);

        if (image)
        {
            // Always want RGB as preview doesn't support RAW
            image->SetOutputMode(OutputMode::RGB);

            info->size = image->GetImageDataSize();
            LOGDEBUG(L"Image Data Size = %d bytes", info->size);

            if (info->size)
            {
                info->data = exportBytes(image->GetImageData(), info->size);

                info->width = image->GetWidth();
                info->height = image->GetHeight();
                info->duration = 0.0;
                info->status = (DWORD)CaptureStatus::Complete;

                if (info->flags & IMAGEFLAG_SAVE_RAW_DATA)
                {
                    image->SaveFile();
                }
            }
            else
            {
                // This should not occur
                LOGWARN(L"Got an image, but it's size is 0 bytes");
                info->status = (DWORD)CaptureStatus::Failed;
            }

            delete image;
        }
        else
        {
            LOGWARN(L"Asked for liveview image, but none returned");

            info->status = (DWORD)CaptureStatus::Failed;
        }
    }
    catch (CameraException & gfe)
    {
        LOGERROR(L"Exception in GetPreviewImage: %s", gfe.GetMessage().c_str());
    }

    LOGTRACE(L"Out: GetPreviewImage(x%p, @ x%p) - Returning data", hCamera, info);

    return ERROR_SUCCESS;
}

HRESULT
StartCapture(HANDLE hCamera, IMAGEINFO* info)
{
    LOGTRACE(L"In: StartCapture(x%p, @ x%p)", hCamera, info);

    HRESULT result = ERROR_CANCELLED;

    Camera* camera = GetCameraManager()->GetCameraForHandle(hCamera);

    try
    {
        // Lock will be auto-released as soon as we leave the try
        Locker lock(camera);

        info->status = (DWORD)CaptureStatus::Starting;

        if (camera->StartCapture(info->duration, (OutputMode)info->imageMode, info->flags))
        {
            result = ERROR_SUCCESS;
        }
        else
        {
            info->status = (DWORD)CaptureStatus::Failed;
        }
    }
    catch (const CameraException &gfe)
    {
        const std::wstring s = gfe.GetMessage();
        LOGERROR(L"Exception starting capture: %s", gfe.GetMessage().c_str());
    }

    LOGTRACE(L"Out: StartCapture(x%08p, @ x%08p) - result = x%08x", hCamera, info, result);

    return result;
}

HRESULT
GetCaptureStatus(HANDLE hCamera, IMAGEINFO* info)
{
    LOGTRACE(L"In: GetCaptureStatus(x%08p)", hCamera);

    Camera* camera = GetCameraManager()->GetCameraForHandle(hCamera);

    try
    {
        Locker lock(camera);

        CaptureStatus status = camera->GetCaptureStatus();

        info->status = (DWORD)status;

        if (status == CaptureStatus::Complete)
        {
            Image* image = camera->GetCapturedImage();

            info->size = image->GetImageDataSize();
            info->data = exportBytes(image->GetImageData(), image->GetImageDataSize());

            info->width = image->GetWidth();
            info->height = image->GetHeight();
            info->duration = image->GetDuration();
            info->imageMode = (DWORD)image->GetOutputMode();

            LOGDEBUG(L"Cleaning up capture");

            // This deletes image too and deletes the capture
            camera->CleanupCapture();
        }
    }
    catch (const CameraException& gfe)
    {
        LOGERROR(L"Exception getting capture status: %s", gfe.GetMessage().c_str());

        info->status = (DWORD)CaptureStatus::Failed;
    }

    LOGTRACE(L"Out: GetCaptureStatus(x%08p)", hCamera);

    return ERROR_SUCCESS;
}

HRESULT
CancelCapture(HANDLE hCamera, IMAGEINFO* info)
{
    LOGTRACE(L"In: CancelCapture(x%08p)", hCamera);

    Camera* camera = GetCameraManager()->GetCameraForHandle(hCamera);

    try
    {
        Locker lock(camera);

        camera->CancelCapture();
    }
    catch (const CameraException& gfe)
    {
        LOGERROR(L"Exception cancelling capture: %s", gfe.GetMessage().c_str());
    }

    LOGTRACE(L"Out: CancelCapture(x%08p)", hCamera);

    return ERROR_SUCCESS;
}

std::list<DWORD>
GetAvailableExposureTimes(HANDLE hCamera, Camera* camera)
{

    PROPERTYVALUE exposureValue;
    std::list<DWORD> exposures;

    SetExposureTime(hCamera, 0.0, &exposureValue);

    exposures.push_back(exposureValue.value);

    short repeatCount = 0;
    bool done = false;
    DWORD exposure = 0;
    DWORD lastExposure = 0;

    do
    {
        SetPropertyValue(hCamera, (DWORD)Property::ShutterSpeed, 1);
        repeatCount = 0;

        do
        {
            Sleep(50);
            CameraSettings* cs = camera->GetSettings(true);
            CameraProperty* p = cs->GetProperty(Property::ShutterSpeed);
            exposure = p->GetCurrentValue()->GetUINT32();
            repeatCount++;
        } while (exposure == lastExposure && repeatCount < 50);

        if (exposure == lastExposure)
        {
            done = true;
        }
        else
        {
            exposures.push_back(exposure);
            lastExposure = exposure;
        }
    } while (!done);

    // Print list of exposure times
    for (std::list<DWORD>::iterator it = exposures.begin(); it != exposures.end(); it++)
    {
        LOGINFO(L"Got an exposure time of %d here", *it);
    }

    return exposures;
}

HRESULT
GetCameraInfo(HANDLE hCamera, CAMERAINFO* info, DWORD flags)
{
    LOGTRACE(L"In: GetCameraInfo(x%p)", hCamera);

    HRESULT result = ERROR_NOT_FOUND;
    Camera* camera = GetCameraManager()->GetCameraForHandle(hCamera);

    if (camera)
    {
        try
        {
            Locker lock(camera);

            result = ERROR_SUCCESS;

            DeviceInfo* deviceInfo = camera->GetDeviceInfo(false);
            std::wostringstream builder;

            builder << L"Cameras\\" << camera->GetDevice()->GetRegistryPath();

            std::wstring cameraPath = builder.str();

            registry.Open();

            if (deviceInfo->GetExposureTimes().empty() && (flags & INFOFLAG_INCLUDE_EXPOSURE_TIMES))
            {
                std::list<DWORD> exposureTimes = GetAvailableExposureTimes(hCamera, camera);

                // Generate a string with all values as CSV
                std::wostringstream regString;

                for (std::list<DWORD>::iterator it = exposureTimes.begin(); it != exposureTimes.end(); it++)
                {
                    regString << *it << L",";
                }

                // Trim off trailing ","
                std::wstring v = regString.str();

                if (v.back() == L',')
                {
                    v.pop_back();
                }

                registry.SetString(cameraPath, L"Exposure Times", v);
            }

            if (deviceInfo->GetPreviewXResolution() == 0 || deviceInfo->GetPreviewYResolution() == 0
                || deviceInfo->GetSensorXResolution() == 0 || deviceInfo->GetSensorYResolution() == 0
                || deviceInfo->GetSensorXCroppedResolution() == 0 || deviceInfo->GetSensorYCroppedResolution() == 0)
            {
                // Camera not fully specified
                LOGINFO(L"GetCameraInfo for '%s' is missing image size info", deviceInfo->GetModel().c_str());

                if (flags & INFOFLAG_ACTIVE)
                {
                    bool previewNeeded = deviceInfo->GetPreviewXResolution() == 0 || deviceInfo->GetPreviewYResolution() == 0;

                    if (previewNeeded)
                    {
                        // Need to get a preview image
                        LOGINFO(L"Getting preview image so we can determine size...");
                        IMAGEINFO iinfo;

                        memset((BYTE*)&iinfo, 0, sizeof(iinfo));

                        GetPreviewImage(hCamera, &iinfo);

                        if (iinfo.status == (DWORD)CaptureStatus::Complete)
                        {
                            registry.SetDWORD(cameraPath, L"Preview X Resolution", iinfo.width);
                            registry.SetDWORD(cameraPath, L"Preview Y Resolution", iinfo.height);

                            if (iinfo.width != 0)
                            {
                                registry.SetDWORD(cameraPath, L"Supports Liveview", 1);
                            }
                        }
                    }

                    // This flag allows us to fetch the info from the camera by changing settings and taking photos
                    if (deviceInfo->GetSensorXResolution() == 0 || deviceInfo->GetSensorYResolution() == 0
                        || deviceInfo->GetSensorXCroppedResolution() == 0 || deviceInfo->GetSensorYCroppedResolution() == 0)
                    {
                        if (previewNeeded)
                        {
                            Sleep(2000);
                        }

                        // Need to take a full-size image
                        LOGINFO(L"Getting full-size image so we can determine size...");
                        IMAGEINFO iinfo;

                        memset((BYTE*)&iinfo, 0, sizeof(iinfo));

                        // We should be on fastest exposure time, which is perfect for the test shot
                        camera->StartCapture(0.1, OutputMode::RGGB, 0);

                        bool done = false;
                        Image* image = nullptr;

                        while (!done)
                        {
                            switch (camera->GetCaptureStatus())
                            {
                            case CaptureStatus::Complete:
                                LOGDEBUG(L"  Capture complete...");

                                image = camera->GetCapturedImage();
                                image->GetImageDataSize();

                                registry.SetDWORD(cameraPath, L"Sensor X Resolution", image->GetRawWidth());
                                registry.SetDWORD(cameraPath, L"Sensor Y Resolution", image->GetRawHeight());
                                registry.SetDWORD(cameraPath, L"AutoCropped X Resolution", image->GetCroppedWidth());
                                registry.SetDWORD(cameraPath, L"AutoCropped Y Resolution", image->GetCroppedHeight());

                                // This deletes image too
                                camera->CleanupCapture();
                                done = true;
                                break;

                            case CaptureStatus::Cancelled:
                            case CaptureStatus::Failed:
                                done = true;
                                break;

                            default:
                                // Avoid spinning
                                Sleep(250);
                                break;
                            }
                        }
                    }
                }
            }

            registry.Close();

            // Fetch updated
            deviceInfo = camera->GetDeviceInfo(true);
            camera->GetSettings(true);

            info->flags = 0;
            info->flags |= deviceInfo->GetSupportsLiveview() ? CAMERAFLAGS_SUPPORTS_LIVEVIEW : 0;
            info->imageWidthPixels = deviceInfo->GetSensorXResolution();
            info->imageHeightPixels = deviceInfo->GetSensorYResolution();
            info->imageWidthCroppedPixels = deviceInfo->GetSensorXCroppedResolution();
            info->imageHeightCroppedPixels = deviceInfo->GetSensorYCroppedResolution();
            info->previewWidthPixels = deviceInfo->GetPreviewXResolution();
            info->previewHeightPixels = deviceInfo->GetPreviewYResolution();
            info->pixelWidth = deviceInfo->GetSensorPixelWidth();
            info->pixelHeight = deviceInfo->GetSensorPixelHeight();
            info->bayerXOffset = deviceInfo->GetBayerXOffset();
            info->bayerYOffset = deviceInfo->GetBayerYOffset();
        }
        catch (CameraException & gfe)
        {
            LOGERROR(L"Exception in GetCameraInfo: %s", gfe.GetMessage().c_str());
        }
    }
    else
    {
        LOGWARN(L"Unable to find camera x%p", hCamera);
    }

    LOGTRACE(L"Out: GetCameraInfo(x%08p)", hCamera);

    return result;
}

DWORD
GetPortableDeviceCount()
{
    LOGTRACE(L"In: GetPortableDeviceCount()");

    DWORD result = deviceManager->GetAllDevices(true).size();

    LOGTRACE(L"Out: GetPortableDeviceCount() - returning %d", result);

    return result;
}

HRESULT
GetPortableDeviceInfo(DWORD offset, PORTABLEDEVICEINFO* pdinfo)
{
    LOGTRACE(L"In: GetPortableDeviceInfo()");

    if (!pdinfo)
    {
        LOGWARN(L"Out: GetPortableDeviceInfo(x%08x, ...) pdinfo is null", offset);

        return ERROR_INCORRECT_SIZE;
    }

    std::list<Device*> deviceList = deviceManager->GetAllDevices(false);
    std::list<Device*>::iterator it = deviceList.begin();

    std::advance(it, offset);

    if (it != deviceList.end())
    {
        Device* device = (*it);// ->Clone();

        pdinfo->id = exportString((*it)->GetId());
        pdinfo->manufacturer = exportString(device->GetManufacturer());
        pdinfo->model = exportString(device->GetFriendlyName());
        pdinfo->devicePath = exportString(device->GetRegistryPath());

        LOGTRACE(L"Out: GetPortableDeviceInfo(x%08x, @ x%08p) - Returning data", offset, pdinfo);

        return ERROR_SUCCESS;
    }
    else
    {
        LOGWARN(L"Out: GetPortableDeviceInfo(x%08x, @ x%08p) - Device Not Found", offset, pdinfo);

        return ERROR_NOT_FOUND;
    }
}

HRESULT
RefreshPropertyList(HANDLE hCamera)
{
    LOGTRACE(L"In: RefreshPropertyList(x%p)", hCamera);

    Camera* camera = GetCameraManager()->GetCameraForHandle(hCamera);

    if (!camera)
    {
        LOGTRACE(L"Out: RefreshPropertyList(x%p) - camera not found", hCamera);

        return ERROR_INVALID_HANDLE;
    }

    try
    {
        Locker lock(camera);

        camera->GetSettings(true);
    }
    catch (CameraException & gfe)
    {
        LOGERROR(L"Exception refreshing property list: %s", gfe.GetMessage().c_str());
    }

    LOGTRACE(L"Out: RefreshPropertyList(x%p)", hCamera);

    return ERROR_SUCCESS;
}

HRESULT
GetPropertyList(HANDLE hCamera, DWORD* list, DWORD* listSize)
{
    LOGTRACE(L"In: GetPropertyList(x%p, ...)", hCamera);

    Camera* camera = GetCameraManager()->GetCameraForHandle(hCamera);

    if (!camera)
    {
        LOGWARN(L"Out: GetPropertyList(x%p, ...) - Camera not found", hCamera);

        return ERROR_INVALID_HANDLE;
    }

    try
    {
        Locker lock(camera);

        if (!list)
        {
            if (!listSize)
            {
                LOGWARN(L"Out: GetPropertyList(x%p, ...) - no list size pointer provided", hCamera);

                return ERROR_INVALID_PARAMETER;
            }

            // Get number of properties advertised, fill in listSize
            *listSize = camera->GetSupportedProperties().size();

            LOGTRACE(L"Out: GetPropertyList(x%p, ...) - list size set, retry", hCamera);

            return ERROR_RETRY;
        }

        PropertyInfoMap supportedProperties = camera->GetSupportedProperties();
        int offset = 0;

        for (std::unordered_map<Property, PropertyInfo*>::iterator it = supportedProperties.begin(); it != supportedProperties.end(); it++)
        {
            list[offset++] = (DWORD)((*it).first);
        }
    }
    catch (CameraException & gfe)
    {
        LOGERROR(L"Exception getting property list: %s", gfe.GetMessage().c_str());
    }

    LOGTRACE(L"Out: GetPropertyList(x%p, ...)", hCamera);

    return ERROR_SUCCESS;
}

HRESULT
GetPropertyDescriptor(HANDLE hCamera, DWORD propertyId, PROPERTYDESCRIPTOR* descriptor)
{
    LOGTRACE(L"In: GetPropertyDescriptor(x%p, x%04d, ...)");

    Camera* camera = GetCameraManager()->GetCameraForHandle(hCamera);

    if (!camera)
    {
        LOGWARN(L"Out: GetPropertyDescriptor(x%p, x%04d, ...) - unable to find camera");

        return ERROR_INVALID_HANDLE;
    }

    if (!descriptor)
    {
        LOGWARN(L"Out: GetPropertyDescriptor(x%p, x%04d, ...) - no descriptor pointer provided");

        return ERROR_INVALID_PARAMETER;
    }

    try
    {
        Locker lock(camera);

        PropertyInfoMap supportedProperties = camera->GetSupportedProperties();
        PropertyInfoMap::iterator it = supportedProperties.find((Property)propertyId);

        if (it != supportedProperties.end())
        {
            PropertyInfo* info = (*it).second;
            std::wstring name = ResourceLoader::GetString(propertyId);

            descriptor->id = propertyId;
            descriptor->type = (WORD)info->GetType();
            descriptor->flags = (WORD)((WORD)info->GetAccess() || ((WORD)info->GetSonySpare() << 8));
            descriptor->name = exportString(name.empty() ? L"?" : name);

            LOGTRACE(L"Out: GetPropertyDescriptor(x%p, x%04d, ...)");

            return ERROR_SUCCESS;
        }
        else
        {
            LOGWARN(L"Out: GetPropertyDescriptor(x%p, x%04d, ...) - property not found");

            return ERROR_NOT_FOUND;
        }
    }
    catch (CameraException & gfe)
    {
        LOGERROR(L"Exception getting property descriptor: %s", gfe.GetMessage().c_str());
    }

    return ERROR_CAN_NOT_COMPLETE;
}

HRESULT
GetPropertyValueOptions(HANDLE hCamera, DWORD propertyId, PROPERTYVALUEOPTION* options, DWORD* count)
{
    LOGTRACE(L"In: GetPropertyValueOptions(x%p, x%04d, ...)");

    Camera* camera = GetCameraManager()->GetCameraForHandle(hCamera);

    if (!camera)
    {
        LOGWARN(L"Out: GetPropertyValueOptions(x%p, x%04d, ...) - unable to find camera");

        return ERROR_INVALID_HANDLE;
    }

    if (!count)
    {
        LOGWARN(L"Out: GetPropertyValueOptions(x%p, x%04d, ...) - pointer to count not provided");

        return ERROR_INVALID_PARAMETER;
    }

    try
    {
        Locker lock(camera);

        PropertyInfoMap supportedProperties = camera->GetSupportedProperties();
        PropertyInfoMap::iterator it = supportedProperties.find((Property)propertyId);

        if (it != supportedProperties.end())
        {
            PropertyInfo* info = (*it).second;

            if (info->GetFormMode() == FormMode::ENUMERATION)
            {
                if (!options)
                {
                    *count = info->GetEnumeration().size();

                    LOGTRACE(L"Out: GetPropertyValueOptions(x%p, x%04d, ...) - count set, retry");

                    return ERROR_RETRY;
                }
                else
                {
                    if (*count < info->GetEnumeration().size())
                    {
                        *count = info->GetEnumeration().size();

                        LOGWARN(L"Out: GetPropertyValueOptions(x%p, x%04d, ...) - count was set too low, retry");

                        return ERROR_RETRY;
                    }

                    CameraPropertyFactory f;
                    CameraProperty* p = f.Create((Property)propertyId);
                    std::list<PropertyValue*> en = info->GetEnumeration();
                    PROPERTYVALUEOPTION* option = options;

                    for (std::list<PropertyValue*>::iterator it = en.begin(); it != en.end(); it++)
                    {
                        PropertyValue* v = *it;
                        option->name = exportString(p->AsString(v));

                        switch (v->GetType())
                        {
                        case DataType::UINT8:
                            option->value = (DWORD)v->GetUINT8();
                            break;

                        case DataType::UINT16:
                            option->value = (DWORD)v->GetUINT16();
                            break;

                        case DataType::UINT32:
                            option->value = (DWORD)v->GetUINT32();
                            break;

                        case DataType::INT8:
                            option->value = (DWORD)v->GetINT8();
                            break;

                        case DataType::INT16:
                            option->value = (DWORD)v->GetINT16();
                            break;

                        case DataType::INT32:
                            option->value = (DWORD)v->GetINT32();
                            break;

                        default:
                            option->value = 0xffffffff;
                            break;
                        }

                        option++;
                    }

                    delete p;
                }
            }

            LOGTRACE(L"Out: GetPropertyValueOptions(x%p, x%04d, ...)");

            return ERROR_SUCCESS;
        }
        else
        {
            LOGWARN(L"Out: GetPropertyValueOptions(x%p, x%04d, ...) - unable to find property");

            return ERROR_NOT_FOUND;
        }
    }
    catch (CameraException & gfe)
    {
        LOGERROR(L"Exception getting property value options: %s", gfe.GetMessage().c_str());
    }

    return ERROR_CAN_NOT_COMPLETE;
}

HRESULT
SetPropertyValue(HANDLE hCamera, DWORD propertyId, DWORD value)
{
    Camera* camera = GetCameraManager()->GetCameraForHandle(hCamera);

    if (!camera)
    {
        return ERROR_INVALID_HANDLE;
    }

    try
    {
        Locker lock(camera);

        CameraSettings* cs = camera->GetSettings(false);
        CameraProperty* v = cs->GetProperty((Property)propertyId);

        PropertyValue* val = nullptr;

        if (v)
        {
            switch (v->GetInfo()->GetType())
            {
            case DataType::INT8:
                val = new PropertyValue((INT8)value);
                break;

            case DataType::INT16:
                val = new PropertyValue((INT16)value);
                break;

            case DataType::INT32:
                val = new PropertyValue((INT32)value);
                break;

            case DataType::UINT8:
                val = new PropertyValue((UINT8)value);
                break;

            case DataType::UINT16:
                val = new PropertyValue((UINT16)value);
                break;

            case DataType::UINT32:
                val = new PropertyValue((UINT32)value);
                break;

            default:
                throw CameraException(L"Unable to set property, only properties with int/uint 8/16/32 supported");
            }

            camera->SetProperty((Property)propertyId, val);

            return ERROR_SUCCESS;
        }
        else
        {
            return ERROR_NOT_FOUND;
        }
    }
    catch (CameraException & gfe)
    {
        LOGERROR(L"Exception setting single property: %s", gfe.GetMessage().c_str());
    }

    return ERROR_CAN_NOT_COMPLETE;
}

HRESULT
GetSinglePropertyValue(HANDLE hCamera, DWORD propertyId, PROPERTYVALUE* value)
{
    Camera* camera = GetCameraManager()->GetCameraForHandle(hCamera);

    if (!camera)
    {
        return ERROR_INVALID_HANDLE;
    }

    try
    {
        Locker lock(camera);

        CameraSettings* cs = camera->GetSettings(false);
        CameraProperty* v = cs->GetProperty((Property)propertyId);

        if (v)
        {
            RenderProperty((Property)propertyId, v, value);

            return ERROR_SUCCESS;
        }
        else
        {
            return ERROR_NOT_FOUND;
        }
    }
    catch (CameraException & gfe)
    {
        LOGERROR(L"Exception getting single property: %s", gfe.GetMessage().c_str());
    }

    return ERROR_CAN_NOT_COMPLETE;
}

HRESULT
GetAllPropertyValues(HANDLE hCamera, PROPERTYVALUE* values, DWORD* count)
{
    Camera* camera = GetCameraManager()->GetCameraForHandle(hCamera);

    if (!camera)
    {
        return ERROR_INVALID_HANDLE;
    }

    try
    {
        Locker lock(camera);

        CameraSettings* cs = camera->GetSettings(false);
        std::list<CameraProperty*> properties = cs->GetProperties();

        if (!values)
        {
            *count = properties.size();

            return ERROR_SUCCESS;
        }

        DWORD offset = 0;

        for (std::list<CameraProperty*>::iterator it = properties.begin(); it != properties.end() && offset < *count; it++)
        {
            RenderProperty((*it)->GetId(), *it, &values[offset]);

            offset++;
        }
    }
    catch (CameraException & gfe)
    {
        LOGERROR(L"Exception getting all properties: %s", gfe.GetMessage().c_str());
    }

    return ERROR_SUCCESS;
}

float
ExposureDWORDToFloat(DWORD exposure, float bulbValue)
{
    DWORD num = 0;
    DWORD den = 0;
    float result = 0.0;

    num = (exposure & 0xffff0000) >> 16;
    den = exposure & 0x0000ffff;

    switch (num)
    {
    case 0:
        result = bulbValue;
        break;

    default:
        result = (float)num / (float)den;
        break;
    }

    return round(result * 100000) / 100000;
}

HRESULT
SetExposureTime(HANDLE hCamera, float desired, PROPERTYVALUE* valueOut)
{
    Camera* camera = GetCameraManager()->GetCameraForHandle(hCamera);
    Property propertyId = Property::ShutterSpeed;

    if (!camera)
    {
        return ERROR_INVALID_HANDLE;
    }

    try
    {
        Locker lock(camera);

        CameraSettings* cs = nullptr;
        CameraProperty* v = nullptr;
        DWORD currentExposure = 0;
        float lastRead = -9e9;
        float current = 0.0;
        std::list<DWORD> exposureTimes = camera->GetDeviceInfo(false)->GetExposureTimes();
        short sameCount = 0;
        float longestNonBulb = 0.0;

        if (exposureTimes.empty())
        {
            longestNonBulb = 30.0;
        }
        else
        {
            std::list<DWORD>::iterator i = exposureTimes.begin();

            i++;
            longestNonBulb = ExposureDWORDToFloat(*i, 9e9);
        }

        desired = desired == 0.0 ? 9e9 : round(desired * 100000) / 100000;

        if (desired > longestNonBulb)
        {
            desired = 9e9;
        }

        do
        {
            cs = camera->GetSettings(true);
            v = cs->GetProperty(propertyId);

            if (v)
            {
                RenderProperty(propertyId, v, valueOut);

                currentExposure = valueOut->value;
                current = ExposureDWORDToFloat(currentExposure, 9e9);

                LOGINFO(L"Setting exposure time to %f, current = %f (%s)", desired, current, valueOut->text);

                if (current == lastRead)
                {
                    sameCount++;
                    LOGTRACE(L"Unchanged...");
                    Sleep(100);
                }
                else
                {
                    LOGINFO(L"Changed from %f to %f", lastRead, current);

                    sameCount = 0;
                    lastRead = current;

                    if (lastRead == desired)
                    {
                        // Already set
                        LOGINFO(L"At desired exposure time");

                        return ERROR_SUCCESS;
                    }
                    else
                    {
                        // Do we need to go faster or slower?
                        DWORD direction = (lastRead > desired) ? 1 : -1;
                        LOGINFO(L"Still need to tweak %d", direction);

                        // If we jump to next value, will we be closer to desired value?
                        // (this avoids situation where someone asks for 19.9" exposure and it jumps back and forth from 20" to 15"
                        auto it = std::find(exposureTimes.begin(), exposureTimes.end(), currentExposure);

                        float next = 0.0;

                        if (!exposureTimes.empty())
                        {
                            if (direction == 1)
                            {
                                it++;
                            }
                            else
                            {
                                if (it == exposureTimes.begin())
                                {
                                    LOGINFO(L"At slowest exposure");

                                    return ERROR_SUCCESS;
                                }

                                it--;
                            }

                            if (it == exposureTimes.end())
                            {
                                LOGINFO(L"At fastest exposure %f", lastRead);

                                return ERROR_SUCCESS;
                            }

                            next = ExposureDWORDToFloat(*it, 9e9);
                        }

                        float nextDiff = abs(lastRead - next);
                        float desiredDiff = abs(lastRead - desired);

                        if (nextDiff > desiredDiff)
                        {
                            LOGINFO(L"As close as possible to %f (at %f)", desired, lastRead);

                            return ERROR_SUCCESS;
                        }

                        SetPropertyValue(hCamera, (DWORD)propertyId, direction);
                    }
                }
            }
            else
            {
                return ERROR_NOT_FOUND;
            }
        } while (lastRead != desired && sameCount < MAX_EXPOSURE_SAME_COUNT);

        return ERROR_SUCCESS;
    }
    catch (CameraException& gfe)
    {
        LOGERROR(L"Exception getting single property: %s", gfe.GetMessage().c_str());
    }

    return ERROR_CAN_NOT_COMPLETE;
}

HRESULT
TestFunc(HANDLE hCamera)
{
    LOGERROR(L"Test log file by writing an error");

    return ERROR_SUCCESS;
}

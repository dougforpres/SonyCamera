#include "pch.h"
#include "CameraException.h"
#include "CameraTask.h"
#include "Logger.h"
#include "Camera.h"

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
    info.pTask->SetResult((VOID*)info.pCamera->Open());

    return StateResult::Success;
}

StateResult
closeCamera(CameraTaskInfo& info)
{
    if (info.pCamera->Close())
    {
        info.pTask->SetResult((void*)1);
    }

    return StateResult::Success;
}

StateResult
setProperty(CameraTaskInfo& info)
{
    SetPropertyTaskParams* params = (SetPropertyTaskParams*)info.pTask->GetParam1();
    info.pCamera->SetProperty(params->id, &params->value);

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

static bool
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
            image->SetDuration(((TakePhotoTaskParams*)info.pTask->GetParam1())->exposureTime);

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

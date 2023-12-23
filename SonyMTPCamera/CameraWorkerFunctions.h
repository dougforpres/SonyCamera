#pragma once
#include "pch.h"
#include "CameraTask.h"
#include "CameraTaskInfo.h"
#include "CameraSettings.h"

enum class StateResult
{
    Success,
    Fail,
    Cancel,
    Retry,
};

bool isImageReady(CameraSettings* cs);

StateResult refreshSettings(CameraTaskInfo& info);
StateResult getPreview(CameraTaskInfo& info);
StateResult initializeCamera(CameraTaskInfo& info);
StateResult refreshDeviceInfo(CameraTaskInfo& info);
StateResult openCamera(CameraTaskInfo& info);
StateResult closeCamera(CameraTaskInfo& info);
StateResult setProperty(CameraTaskInfo& info);
StateResult doCapture(CameraTaskInfo& info);
StateResult downloadImage(CameraTaskInfo& info);
StateResult processImage(CameraTaskInfo& info);

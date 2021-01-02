#pragma once
#include "pch.h"
#include "MTPDevice.h"
#include "MessageWriter.h"
#include "ObjectInfo.h"
#include "PropertyValue.h"

class DummyDevice :
    public MTPDevice
{
public:
    DummyDevice(const std::wstring deviceId, const std::wstring friendlyName, const std::wstring manufacturer, const std::wstring description);

    Device* Clone();

    HANDLE Open();
    bool Close();

    HANDLE GetHandle();

protected:
    Message* InternalSend(Op kind, Message* out);

    bool StartNotifications();
    bool StopNotifications();

private:
    Message* GetDeviceInfo(Message* out);
    Message* GetStorageIds(Message* out);
    Message* GetNextHandle(Message* out);
    Message* GetPropertyList(Message* out);
    Message* ReadSettings(Message* out);
    Message* GetObjectInfo(Message* out);
    Message* GetObject(Message* out);
    Message* SetProperty(Message* out);

    void AddCameraProperty(MessageWriter& w, Property property, PropertyValue* value, bool writable);
    void WriteValue(MessageWriter& w, PropertyValue* value);
    std::wstring JustFilename(std::wstring fullpath);

    InputMode m_inputMode = InputMode::UNKNOWN;
    std::wstring m_ARWFileName;
    std::wstring m_JPEGFileName;
    DWORD m_ARWFileSize = 0;
    DWORD m_JPEGFileSize = 0;
    UINT16 m_shutterHalf = 1;
    UINT16 m_shutterFull = 1;
    bool m_photoReady = false;
};


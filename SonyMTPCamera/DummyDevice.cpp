#include "pch.h"
#include "DummyDevice.h"
#include "Logger.h"
#include "SonyCamera.h"
#include "MessageWriter.h"
#include "Registry.h"
#include "Camera.h"

DummyDevice::DummyDevice(const std::wstring deviceId, const std::wstring friendlyName, const std::wstring manufacturer, const std::wstring description)
    : Device(nullptr, deviceId)
{
    m_friendlyName = friendlyName;
    m_manufacturer = manufacturer;
    m_description = description;

    registry.Open();
    std::wostringstream builder;

    builder << L"Cameras\\" << m_manufacturer << L"\\" << m_friendlyName;
    std::wstring path = builder.str();

    m_ARWFileName = registry.GetString(path, L"ARW Image Name", L"ARW00001.ARW");
    m_JPEGFileName = registry.GetString(path, L"JPEG Image Name", L"JPEG0001.JPEG");

    registry.Close();

    // Try to read length of the two example files
    HANDLE hfile = CreateFile(m_ARWFileName.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (hfile != INVALID_HANDLE_VALUE)
    {
        m_ARWFileSize = GetFileSize(hfile, nullptr);

        CloseHandle(hfile);
    }
    else
    {
        LOGERROR(L"Unable to open sample file '%s', error %d", m_ARWFileName.c_str(), GetLastError());
    }

    hfile = CreateFile(m_JPEGFileName.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (hfile != INVALID_HANDLE_VALUE)
    {
        m_JPEGFileSize = GetFileSize(hfile, nullptr);

        CloseHandle(hfile);
    }
    else
    {
        LOGERROR(L"Unable to open sample file '%s', error %d", m_JPEGFileName.c_str(), GetLastError());
    }
}

Device*
DummyDevice::Clone()
{
    LOGTRACE(L"DummyDevice::Clone of %s", GetFriendlyName().c_str());

    return new DummyDevice(m_id, m_friendlyName, m_manufacturer, m_description);
}

HANDLE
DummyDevice::Open()
{
    return GetHandle();
}

bool
DummyDevice::Close()
{
    return true;
}

HANDLE
DummyDevice::GetHandle()
{
    return (HANDLE)this;
}

bool
DummyDevice::StartNotifications()
{
    return true;
}

bool
DummyDevice::StopNotifications()
{
    return true;
}

Message*
DummyDevice::InternalSend(Op kind, Message* out)
{
    // Here we emulate our desired device
    // We don't have to be super selective... just a switch on out->GetCommand()
    // and support the commands we know we send for real devices
    switch (out->GetCommand())
    {
    case COMMAND_GET_DEVICE_INFO:
        return GetDeviceInfo(out);

    case COMMAND_GET_STORAGE_IDS:
        return GetStorageIds(out);

    case COMMAND_SONY_GET_NEXT_HANDLE:
        return GetNextHandle(out);

    case COMMAND_SONY_GET_PROPERTY_LIST:
        return GetPropertyList(out);

    case COMMAND_READ_SETTINGS:
        // This will need to be somewhat state aware
        return ReadSettings(out);

    case COMMAND_GET_OBJECT_INFO:
        return GetObjectInfo(out);

    case COMMAND_GET_OBJECT:
        return GetObject(out);

    case COMMAND_SONY_SET_PROPERTY:
        return SetProperty(out);
        break;

    default:
        LOGERROR(L"Don't know how to process command x%04x", out->GetCommand());
        break;
    }
}

Message*
DummyDevice::GetDeviceInfo(Message* out)
{
    MessageWriter w(COMMAND_RESULT_SUCCESS);

    w.WriteWORD(100);                   // Standard Version: 1.00
    w.WriteDWORD(0);                    // Vendor Extension ID: 0
    w.WriteWORD(0);                     // Vendor Extension Version
    w.WriteString(L"");                 // Vendor Extension Desc
    w.WriteWORD(0);                     // Functional Mode: 0
    w.WriteWORDArray({});               // Operations Supported
    w.WriteWORDArray({});               // Events Supported
    w.WriteWORDArray({});               // Device Properties Supported
    w.WriteWORDArray({});               // Capture Formats Supported
    w.WriteWORDArray({});               // Image Formats Supported
    w.WriteString(m_manufacturer);      // Manufacturer
    w.WriteString(m_friendlyName);      // Model
    w.WriteString(m_description);       // Description
    w.WriteString(L"1");                // Serial Number

    return w.GetMessageObj();
}

Message*
DummyDevice::GetStorageIds(Message* out)
{
    // Result ignored at the moment, so no need to generate data
    return new Message(COMMAND_RESULT_SUCCESS);
}

Message*
DummyDevice::GetNextHandle(Message* out)
{
    // Result ignored at the moment, so no need to generate data
    return new Message(COMMAND_RESULT_SUCCESS);
}

Message*
DummyDevice::GetPropertyList(Message* out)
{
    MessageWriter w(COMMAND_RESULT_SUCCESS);

    w.WriteWORD(0);       // Unknown
    w.WriteWORDArray({}); // List A (unknown)
    w.WriteWORDArray({}); // List B (unknown)

    return w.GetMessageObj();
}

Message*
DummyDevice::ReadSettings(Message* out)
{
    MessageWriter w(COMMAND_RESULT_SUCCESS);

    w.WriteDWORD(7); // Number of properties
    w.WriteDWORD(0); // Some other value

    AddCameraProperty(w, Property::PhotoBufferStatus, new PropertyValue((UINT16)(m_photoReady ? 0x8001 : 0)), false); // Ready
    AddCameraProperty(w, Property::ShutterFullDown, new PropertyValue((UINT16)m_shutterHalf), true);
    AddCameraProperty(w, Property::ShutterHalfDown, new PropertyValue((UINT16)m_shutterFull), true);
    AddCameraProperty(w, Property::ShutterSpeed, new PropertyValue((UINT32)0), false);  // BULB
    AddCameraProperty(w, Property::CompressionSetting, new PropertyValue((UINT8)0x10), false); // RAW
    AddCameraProperty(w, Property::ShutterButtonStatus, new PropertyValue((UINT8)1), false); // Shutter button UP
    AddCameraProperty(w, Property::FocusMode, new PropertyValue((UINT16)1), false); // Focus Manual

    return w.GetMessageObj();
}

Message*
DummyDevice::GetObjectInfo(Message* out)
{
    MessageWriter w(COMMAND_RESULT_SUCCESS);

    // Depending on the output param, we return different data (JPEG/ARW)
    DWORD id = *(out->GetParams().begin());

    std::wstring filename = JustFilename(id == FULL_IMAGE ? m_ARWFileName.c_str() : m_JPEGFileName.c_str());

    w.WriteDWORD(0); // Storage ID
    w.WriteWORD((WORD)m_inputMode);
    w.WriteWORD(0); // Protection Status
    w.WriteDWORD(id == FULL_IMAGE ? m_ARWFileSize : m_JPEGFileSize + 4);
    w.WriteWORD(0); // Thumb Format
    w.WriteDWORD(0); // Thumb Compressed Size
    w.WriteDWORD(0); // Thumb width
    w.WriteDWORD(0); // Thumb height
    w.WriteDWORD(0); // Image width
    w.WriteDWORD(0); // Image height
    w.WriteDWORD(0); // Image bit depth
    w.WriteDWORD(0); // Image Parent
    w.WriteWORD(0);  // Association Code
    w.WriteDWORD(0); // Accociation Desc
    w.WriteDWORD(0); // Sequence
    w.WriteString(filename);
    w.WriteString(L""); // Capture Date
    w.WriteString(L""); // Modification Date
    w.WriteString(L""); // Keywords

    return w.GetMessageObj();
}

Message*
DummyDevice::GetObject(Message* out)
{
    MessageWriter w(COMMAND_RESULT_SUCCESS);

    // Depending on the output param, we return different data (JPEG/ARW)
    DWORD id = *(out->GetParams().begin());

    if (id == PREVIEW_IMAGE)
    {
        w.WriteDWORD(4);
    }

    HANDLE hfile = CreateFile(id == FULL_IMAGE ? m_ARWFileName.c_str() : m_JPEGFileName.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (hfile != INVALID_HANDLE_VALUE)
    {
        DWORD len = GetFileSize(hfile, nullptr);
        BYTE* temp = new BYTE[len];
        DWORD bytesRead = 0;

        if (ReadFile(hfile, temp, len, &bytesRead, nullptr))
        {
            w.Write(temp, len);
        }

        delete[] temp;
        CloseHandle(hfile);
    }

    m_photoReady = false;

    return w.GetMessageObj();
}

Message*
DummyDevice::SetProperty(Message* out)
{
    // Property to write
    Property id = (Property)*(out->GetParams().begin());
    WORD value = *((WORD*)out->GetData());

    // Value to write
    switch (id)
    {
    case Property::ShutterFullDown:
        m_shutterFull = value;
        break;

    case Property::ShutterHalfDown:
        if (m_shutterHalf != 1 && value != m_shutterHalf)
        {
            m_photoReady = true;
        }

        m_shutterHalf = value;
        break;

    default:
        LOGWARN(L"Attempting to set unknown property %d", (DWORD)id);
        break;
    }

    return new Message(COMMAND_RESULT_SUCCESS);
}

void
DummyDevice::AddCameraProperty(MessageWriter& w, Property property, PropertyValue* value, bool writable)
{
    w.WriteWORD((WORD)property);
    w.WriteWORD((WORD)value->GetType());
    w.WriteBYTE((BYTE)(writable ? Accessibility::READ_WRITE : Accessibility::READ_ONLY));
    w.WriteBYTE(0); // Sony Spare
    WriteValue(w, value);
    WriteValue(w, value);
    w.WriteBYTE((BYTE)FormMode::NONE);
}

void
DummyDevice::WriteValue(MessageWriter& w, PropertyValue* value)
{
    switch (value->GetType())
    {
    case DataType::UINT8:
        w.WriteBYTE(value->GetUINT8());
        return;

    case DataType::UINT16:
        w.WriteWORD(value->GetUINT16());
        return;

    case DataType::UINT32:
        w.WriteDWORD(value->GetUINT32());
        return;

    case DataType::INT8:
        w.WriteBYTE(value->GetINT8());
        return;

    case DataType::INT16:
        w.WriteWORD(value->GetINT16());
        return;

    case DataType::INT32:
        w.WriteDWORD(value->GetINT32());
        return;

    case DataType::STR:
        w.WriteString(value->GetString());
        return;
    }
}

std::wstring
DummyDevice::JustFilename(std::wstring fullpath)
{
    size_t lastSlash = fullpath.find_last_of(L"\\");

    if (lastSlash)
    {
        return fullpath.substr(lastSlash + 1);
    }
    else
    {
        return fullpath;
    }
}
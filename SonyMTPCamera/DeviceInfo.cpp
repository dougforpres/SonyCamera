#include "pch.h"
#include "DeviceInfo.h"
#include "Logger.h"

DeviceInfo::DeviceInfo()
    : MessageReader(nullptr)
{
#ifdef DEBUG
    LOGTRACE(L"In: DeviceInfo::DeviceInfo()");
#endif

#ifdef DEBUG
    LOGTRACE(L"Out: DeviceInfo::DeviceInfo()");
#endif
}

DeviceInfo::DeviceInfo(Message* message)
    : MessageReader(message)
{
#ifdef DEBUG
    LOGTRACE(L"In: DeviceInfo::DeviceInfo(this=0x%x)", this);
#endif
    DWORD offset = 0;

    m_standardVersion = this->GetWORD(offset) / 100.0;
    m_vendorExtensionId = this->GetDWORD(offset);
    m_vendorExtensionVersion = this->GetWORD(offset) / 100.0;
    m_vendorExtensionDesc = this->GetString(offset);
    m_functionalMode = this->GetWORD(offset);
    m_operationsSupported = this->GetWORDArray(offset);
    m_eventsSupported = this->GetWORDArray(offset);
    m_devicePropertiesSupported = this->GetWORDArray(offset);
    m_captureFormats = this->GetWORDArray(offset);
    m_imageFormats = this->GetWORDArray(offset);
    m_manufacturer = this->GetString(offset);
    m_model = this->GetString(offset);
    m_deviceVersion = this->GetString(offset);
    m_serialNumber = this->GetString(offset);
#ifdef DEBUG
    LOGTRACE(L"Out: DeviceInfo::DeviceInfo");
#endif
}

DeviceInfo::DeviceInfo(const DeviceInfo& rhs)
    : MessageReader(nullptr)
{
#ifdef DEBUG
    LOGTRACE(L"In: DeviceInfo::DeviceInfo((copy) rhs = x%08p) [this = %08p]", &rhs, this);
#endif
    Copy(rhs);
}

DeviceInfo
DeviceInfo::operator=(const DeviceInfo& rhs)
{
#ifdef DEBUG
    LOGTRACE(L"DeviceInfo::operator=(rhs = x%08p) [this = x%08p]", &rhs, this);
#endif
    Copy(rhs);

    return *this;
}

void
DeviceInfo::Copy(const DeviceInfo& rhs)
{
    m_standardVersion = rhs.m_standardVersion;
    m_vendorExtensionId = rhs.m_vendorExtensionId;
    m_vendorExtensionVersion = rhs.m_vendorExtensionVersion;
    m_vendorExtensionDesc = rhs.m_vendorExtensionDesc;
    m_functionalMode = rhs.m_functionalMode;
    m_operationsSupported = rhs.m_operationsSupported;
    m_eventsSupported = rhs.m_eventsSupported;
    m_devicePropertiesSupported = rhs.m_devicePropertiesSupported;
    m_captureFormats = rhs.m_captureFormats;
    m_imageFormats = rhs.m_imageFormats;
    m_manufacturer = rhs.m_manufacturer;
    m_model = rhs.m_model;
    m_deviceVersion = rhs.m_deviceVersion;
    m_serialNumber = rhs.m_serialNumber;
    m_sensorName = rhs.m_sensorName;
    m_sensorType = rhs.m_sensorType;
    m_sensorXResolution = rhs.m_sensorXResolution;
    m_sensorYResolution = rhs.m_sensorYResolution;
    m_sensorXCroppedResolution = rhs.m_sensorXCroppedResolution;
    m_sensorYCroppedResolution = rhs.m_sensorYCroppedResolution;
    m_previewXResolution = rhs.m_previewXResolution;
    m_previewYResolution = rhs.m_previewYResolution;
    m_bitsPerPixel = rhs.m_bitsPerPixel;
    m_leftCrop = rhs.m_leftCrop;
    m_rightCrop = rhs.m_rightCrop;
    m_topCrop = rhs.m_topCrop;
    m_bottomCrop = rhs.m_bottomCrop;
    m_cropMode = rhs.m_cropMode;
    m_bayerOffsetX = rhs.m_bayerOffsetX;
    m_bayerOffsetY = rhs.m_bayerOffsetY;
    m_sensorPixelWidth = rhs.m_sensorPixelWidth;
    m_sensorPixelHeight = rhs.m_sensorPixelHeight;
    m_exposureTimeMin = rhs.m_exposureTimeMin;
    m_exposureTimeMax = rhs.m_exposureTimeMax;
    m_exposureTimeStep = rhs.m_exposureTimeStep;
    m_supportsLiveview = rhs.m_supportsLiveview;
    m_buttonPropertiesInverted = rhs.m_buttonPropertiesInverted;
    m_exposureTimes = rhs.m_exposureTimes;
    m_isos = rhs.m_isos;
//    m_focusLimit = rhs.m_focusLimit;
//    m_focusSteps = rhs.m_focusSteps;
//    m_focusMagicNumber = rhs.m_focusMagicNumber;
    m_focusStartMode = rhs.m_focusStartMode;
//    m_handsOffFocus = rhs.m_handsOffFocus;
#ifdef DEBUG
    LOGTRACE(L"Out: DeviceInfo::DeviceInfo(copy)");
#endif
}

DeviceInfo::~DeviceInfo()
{
#ifdef DEBUG
    LOGTRACE(L"In: DeviceInfo::~DeviceInfo(this=0x%x)", this);
#endif
#ifdef DEBUG
    LOGTRACE(L"Out: DeviceInfo::~DeviceInfo");
#endif
}

void
DeviceInfo::DumpToLog() const
{
    LOGINFO(L"  Standard Version            %.02f", m_standardVersion);
    LOGINFO(L"  Vendor Extension ID         %d", m_vendorExtensionId);
    LOGINFO(L"  Vendor Extension Version    %.02f", m_vendorExtensionVersion);
    LOGINFO(L"  Vendor Extension Desc       %s", m_vendorExtensionDesc.c_str());

    std::wostringstream builder;

    switch (m_functionalMode)
    {
    case 0x0000:
        builder << "Standard Mode";
        break;

    case 0x0001:
        builder << "Sleep State";
        break;

    default:
        if (m_functionalMode & 0x8000)
        {
            builder << "Vendor defined";
        }
        else
        {
            builder << "Reserved";
        }

        builder << " x" << std::hex << std::setw(4) << std::setfill(L'0') << m_functionalMode;
    }

    LOGINFO(L"  Functional Mode             %s", builder.str().c_str());
    LOGINFO(L"  Operations Supported        %d operations", m_operationsSupported.size());
    this->DumpList(m_operationsSupported);
    LOGINFO(L"  Events Supported            %d events", m_eventsSupported.size());
    this->DumpList(m_eventsSupported);
    LOGINFO(L"  Device Properties Supported %d properties", m_devicePropertiesSupported.size());
    this->DumpList(m_devicePropertiesSupported);
    LOGINFO(L"  Capture Formats             %d formats", m_captureFormats.size());
    this->DumpList(m_captureFormats);
    LOGINFO(L"  Manufacturer                %s", m_manufacturer.c_str());
    LOGINFO(L"  Model                       %s", m_model.c_str());
    LOGINFO(L"  Device Version              %s", m_deviceVersion.c_str());
    LOGINFO(L"  Serial Number               %s", m_serialNumber.c_str());
    LOGINFO(L"Extras");
    LOGINFO(L"  Sensor Name                 %s", m_sensorName.c_str());
    LOGINFO(L"  Supported Exposure Times    %d values", m_exposureTimes.size());
    this->DumpList(m_exposureTimes);
    LOGINFO(L"  Supported ISO Values        %d values", m_isos.size());
    this->DumpList(m_isos);

    std::wstring sensorType;

    switch (m_sensorType)
    {
    case SensorType::CMYG:
        sensorType = L"CMYG";
        break;

    case SensorType::CMYG2:
        sensorType = L"CMYG2";
        break;

    case SensorType::COLOUR:
        sensorType = L"COLOUR";
        break;

    case SensorType::LRGB:
        sensorType = L"LRGB";
        break;

    case SensorType::MONOCHROME:
        sensorType = L"MONOCHROME";
        break;

    case SensorType::RGGB:
        sensorType = L"RGGB";
        break;

    default:
        sensorType = L"UNKNOWN";
        break;
    }

    LOGINFO(L"  Sensor Type                   %s", sensorType.c_str());
    LOGINFO(L"  Sensor Pixel Width            %f um", m_sensorPixelWidth);
    LOGINFO(L"  Sensor Pixel Height           %f um", m_sensorPixelHeight);
    LOGINFO(L"  Sensor X Resolution           %d pixels", m_sensorXResolution);
    LOGINFO(L"  Sensor Y Resolution           %d pixels", m_sensorYResolution);
    LOGINFO(L"  Sensor X Resolution (Cropped) %d pixels", m_sensorXCroppedResolution);
    LOGINFO(L"  Sensor Y Resolution (Cropped) %d pixels", m_sensorYCroppedResolution);
    LOGINFO(L"  Minimum Exposure Time         %f sec", m_exposureTimeMin);
    LOGINFO(L"  Maximum Exposure Time         %f sec", m_exposureTimeMax);
    LOGINFO(L"  Exposure Time Step            %f sec", m_exposureTimeStep);
    LOGINFO(L"  Bits Per Pixel                %d bits", m_bitsPerPixel);

    std::wstring startMode;

    switch (m_focusStartMode)
    {
    case FocusStartMode::RESET_EVERY_TIME:
        startMode = L"Every Time";
        break;

    case FocusStartMode::RESET_FIRST_TIME:
        startMode = L"First Focus Only";
        break;

    case FocusStartMode::RESET_BIGGER_MOVE:
        startMode = L"When change is higher than last time";
        break;
    }

    LOGINFO(L"  Reset Focus To Infinite       %s", startMode.c_str());
}

void
DeviceInfo::DumpList(std::list<WORD> list) const
{
    for (auto value: list)
    {
        LOGINFO(L"                              %d (x%04x)", value, value);
    }
}

void
DeviceInfo::DumpList(std::list<DWORD> list) const
{
    for (auto value : list)
    {
        LOGINFO(L"                              %d (x%08x)", value, value);
    }
}

std::wstring
DeviceInfo::GetManufacturer() const
{
    return m_manufacturer;
}

std::wstring
DeviceInfo::GetModel() const
{
    return m_model;
}

std::wstring
DeviceInfo::GetSerialNumber() const
{
    return m_serialNumber;
}

void
DeviceInfo::SetSensorName(std::wstring name)
{
    m_sensorName = name;
}

std::wstring
DeviceInfo::GetSensorName() const
{
    return m_sensorName;
}

void
DeviceInfo::SetSensorType(SensorType type)
{
    m_sensorType = type;
}

SensorType
DeviceInfo::GetSensorType() const
{
    return m_sensorType;
}

void
DeviceInfo::SetSensorXResolution(UINT32 resolution)
{
    m_sensorXResolution = resolution;
}

UINT32
DeviceInfo::GetSensorXResolution() const
{
    return m_sensorXResolution;
}

void
DeviceInfo::SetSensorYResolution(UINT32 resolution)
{
    m_sensorYResolution = resolution;
}

UINT32
DeviceInfo::GetSensorYResolution() const
{
    return m_sensorYResolution;
}

void
DeviceInfo::SetSensorXCroppedResolution(UINT32 resolution)
{
    m_sensorXCroppedResolution = resolution;
}

UINT32
DeviceInfo::GetSensorXCroppedResolution() const
{
    return m_sensorXCroppedResolution;
}

void
DeviceInfo::SetSensorYCroppedResolution(UINT32 resolution)
{
    m_sensorYCroppedResolution = resolution;
}

UINT32
DeviceInfo::GetSensorYCroppedResolution() const
{
    return m_sensorYCroppedResolution;
}

void
DeviceInfo::SetPreviewXResolution(UINT32 resolution)
{
    m_previewXResolution = resolution;
}

UINT32
DeviceInfo::GetPreviewXResolution() const
{
    return m_previewXResolution;
}

void
DeviceInfo::SetPreviewYResolution(UINT32 resolution)
{
    m_previewYResolution = resolution;
}

UINT32
DeviceInfo::GetPreviewYResolution() const
{
    return m_previewYResolution;
}

void
DeviceInfo::SetSensorPixelWidth(double width)
{
    m_sensorPixelWidth = width;
}

double
DeviceInfo::GetSensorPixelWidth() const
{
    return m_sensorPixelWidth;
}

void
DeviceInfo::SetSensorPixelHeight(double height)
{
    m_sensorPixelHeight = height;
}

double
DeviceInfo::GetSensorPixelHeight() const
{
    return m_sensorPixelHeight;
}

void
DeviceInfo::SetBayerXOffset(INT8 offset)
{
    m_bayerOffsetX = offset;
}

INT8
DeviceInfo::GetBayerXOffset() const
{
    return m_bayerOffsetX;
}

void
DeviceInfo::SetBayerYOffset(INT8 offset)
{
    m_bayerOffsetY = offset;
}

INT8
DeviceInfo::GetBayerYOffset() const
{
    return m_bayerOffsetY;
}

void
DeviceInfo::SetExposureTimeMin(double time)
{
    m_exposureTimeMin = time;
}

double
DeviceInfo::GetExposureTimeMin() const
{
    return m_exposureTimeMin;
}

void
DeviceInfo::SetExposureTimeMax(double time)
{
    m_exposureTimeMax = time;
}

double
DeviceInfo::GetExposureTimeMax() const
{
    return m_exposureTimeMax;
}

void
DeviceInfo::SetExposureTimeStep(double step)
{
    m_exposureTimeStep = step;
}

double
DeviceInfo::GetExposureTimeStep() const
{
    return m_exposureTimeStep;
}

std::wstring
DeviceInfo::GetVersion() const
{
    return m_deviceVersion;
}

bool
DeviceInfo::GetSupportsLiveview() const
{
    return m_supportsLiveview;
}

void
DeviceInfo::SetSupportsLiveview(bool support)
{
    m_supportsLiveview = support;
}

CropMode
DeviceInfo::GetCropMode() const
{
    return m_cropMode;
}

void
DeviceInfo::SetCropMode(CropMode mode)
{
    m_cropMode = mode;
}

UINT16
DeviceInfo::GetLeftCrop() const
{
    return m_leftCrop;
}

void
DeviceInfo::SetLeftCrop(UINT16 crop)
{
    m_leftCrop = crop;
}

UINT16
DeviceInfo::GetRightCrop() const
{
    return m_rightCrop;
}

void
DeviceInfo::SetRightCrop(UINT16 crop)
{
    m_rightCrop = crop;
}

UINT16
DeviceInfo::GetTopCrop() const
{
    return m_topCrop;
}

void
DeviceInfo::SetTopCrop(UINT16 crop)
{
    m_topCrop = crop;
}

UINT16
DeviceInfo::GetBottomCrop() const
{
    return m_bottomCrop;
}

void
DeviceInfo::SetBottomCrop(UINT16 crop)
{
    m_bottomCrop = crop;
}

bool
DeviceInfo::GetButtonPropertiesInverted() const
{
    return m_buttonPropertiesInverted;
}

void
DeviceInfo::SetButtonPropertiesInverted(bool invert)
{
    m_buttonPropertiesInverted = invert;
}

std::list<DWORD>
DeviceInfo::GetExposureTimes() const
{
    return m_exposureTimes;
}

void
DeviceInfo::SetExposureTimes(std::list<DWORD> exposureTimes)
{
    m_exposureTimes = exposureTimes;
}

std::list<DWORD>
DeviceInfo::GetISOs() const
{
    return m_isos;
}

void
DeviceInfo::SetISOs(std::list<DWORD> isos)
{
    m_isos = isos;
}

UINT32
DeviceInfo::GetBitsPerPixel() const
{
    return m_bitsPerPixel;
}

void
DeviceInfo::SetBitsPerPixel(UINT32 bpp)
{
    m_bitsPerPixel = bpp;
}

FocusStartMode
DeviceInfo::GetFocusStartMode() const
{
    return m_focusStartMode;
}

void
DeviceInfo::SetFocusStartMode(FocusStartMode mode)
{
    m_focusStartMode = mode;
}

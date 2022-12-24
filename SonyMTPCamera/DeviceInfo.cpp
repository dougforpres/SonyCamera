#include "pch.h"
#include "DeviceInfo.h"
#include "Logger.h"

DeviceInfo::DeviceInfo(Message* message)
    : MessageReader(message)
{
    LOGTRACE(L"In: DeviceInfo::DeviceInfo");

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

    LOGTRACE(L"Out: DeviceInfo::DeviceInfo");
}

DeviceInfo::~DeviceInfo()
{

}

void
DeviceInfo::DumpToLog()
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
}

void
DeviceInfo::DumpList(std::list<WORD> list)
{
    for (std::list<WORD>::iterator it = list.begin(); it != list.end(); it++)
    {
        LOGINFO(L"                              x%04x", *it);
    }
}

std::wstring
DeviceInfo::GetManufacturer()
{
    return m_manufacturer;
}

std::wstring
DeviceInfo::GetModel()
{
    return m_model;
}

std::wstring
DeviceInfo::GetSerialNumber()
{
    return m_serialNumber;
}

void
DeviceInfo::SetSensorName(std::wstring name)
{
    m_sensorName = name;
}

std::wstring
DeviceInfo::GetSensorName()
{
    return m_sensorName;
}

void
DeviceInfo::SetSensorType(SensorType type)
{
    m_sensorType = type;
}

SensorType
DeviceInfo::GetSensorType()
{
    return m_sensorType;
}

void
DeviceInfo::SetSensorXResolution(UINT32 resolution)
{
    m_sensorXResolution = resolution;
}

UINT32
DeviceInfo::GetSensorXResolution()
{
    return m_sensorXResolution;
}

void
DeviceInfo::SetSensorYResolution(UINT32 resolution)
{
    m_sensorYResolution = resolution;
}

UINT32
DeviceInfo::GetSensorYResolution()
{
    return m_sensorYResolution;
}

void
DeviceInfo::SetSensorXCroppedResolution(UINT32 resolution)
{
    m_sensorXCroppedResolution = resolution;
}

UINT32
DeviceInfo::GetSensorXCroppedResolution()
{
    return m_sensorXCroppedResolution;
}

void
DeviceInfo::SetSensorYCroppedResolution(UINT32 resolution)
{
    m_sensorYCroppedResolution = resolution;
}

UINT32
DeviceInfo::GetSensorYCroppedResolution()
{
    return m_sensorYCroppedResolution;
}

void
DeviceInfo::SetPreviewXResolution(UINT32 resolution)
{
    m_previewXResolution = resolution;
}

UINT32
DeviceInfo::GetPreviewXResolution()
{
    return m_previewXResolution;
}

void
DeviceInfo::SetPreviewYResolution(UINT32 resolution)
{
    m_previewYResolution = resolution;
}

UINT32
DeviceInfo::GetPreviewYResolution()
{
    return m_previewYResolution;
}

void
DeviceInfo::SetSensorPixelWidth(double width)
{
    m_sensorPixelWidth = width;
}

double
DeviceInfo::GetSensorPixelWidth()
{
    return m_sensorPixelWidth;
}

void
DeviceInfo::SetSensorPixelHeight(double height)
{
    m_sensorPixelHeight = height;
}

double
DeviceInfo::GetSensorPixelHeight()
{
    return m_sensorPixelHeight;
}

void
DeviceInfo::SetBayerXOffset(INT8 offset)
{
    m_bayerOffsetX = offset;
}

INT8
DeviceInfo::GetBayerXOffset()
{
    return m_bayerOffsetX;
}

void
DeviceInfo::SetBayerYOffset(INT8 offset)
{
    m_bayerOffsetY = offset;
}

INT8
DeviceInfo::GetBayerYOffset()
{
    return m_bayerOffsetY;
}

void
DeviceInfo::SetExposureTimeMin(double time)
{
    m_exposureTimeMin = time;
}

double
DeviceInfo::GetExposureTimeMin()
{
    return m_exposureTimeMin;
}

void
DeviceInfo::SetExposureTimeMax(double time)
{
    m_exposureTimeMax = time;
}

double
DeviceInfo::GetExposureTimeMax()
{
    return m_exposureTimeMax;
}

void
DeviceInfo::SetExposureTimeStep(double step)
{
    m_exposureTimeStep = step;
}

double
DeviceInfo::GetExposureTimeStep()
{
    return m_exposureTimeStep;
}

std::wstring
DeviceInfo::GetVersion()
{
    return m_deviceVersion;
}

bool
DeviceInfo::GetSupportsLiveview()
{
    return m_supportsLiveview;
}

void
DeviceInfo::SetSupportsLiveview(bool support)
{
    m_supportsLiveview = support;
}

CropMode
DeviceInfo::GetCropMode()
{
    return m_cropMode;
}

void
DeviceInfo::SetCropMode(CropMode mode)
{
    m_cropMode = mode;
}

UINT16
DeviceInfo::GetLeftCrop()
{
    return m_leftCrop;
}

void
DeviceInfo::SetLeftCrop(UINT16 crop)
{
    m_leftCrop = crop;
}

UINT16
DeviceInfo::GetRightCrop()
{
    return m_rightCrop;
}

void
DeviceInfo::SetRightCrop(UINT16 crop)
{
    m_rightCrop = crop;
}

UINT16
DeviceInfo::GetTopCrop()
{
    return m_topCrop;
}

void
DeviceInfo::SetTopCrop(UINT16 crop)
{
    m_topCrop = crop;
}

UINT16
DeviceInfo::GetBottomCrop()
{
    return m_bottomCrop;
}

void
DeviceInfo::SetBottomCrop(UINT16 crop)
{
    m_bottomCrop = crop;
}

bool
DeviceInfo::GetButtonPropertiesInverted()
{
    return m_buttonPropertiesInverted;
}

void
DeviceInfo::SetButtonPropertiesInverted(bool invert)
{
    m_buttonPropertiesInverted = invert;
}

std::list<DWORD>
DeviceInfo::GetExposureTimes()
{
    return m_exposureTimes;
}

void
DeviceInfo::SetExposureTimes(std::list<DWORD> exposureTimes)
{
    m_exposureTimes = exposureTimes;
}

std::list<DWORD>
DeviceInfo::GetISOs()
{
    return m_isos;
}

void
DeviceInfo::SetISOs(std::list<DWORD> isos)
{
    m_isos = isos;
}

UINT32
DeviceInfo::GetBitsPerPixel()
{
    return m_bitsPerPixel;
}

void
DeviceInfo::SetBitsPerPixel(UINT32 bpp)
{
    m_bitsPerPixel = bpp;
}
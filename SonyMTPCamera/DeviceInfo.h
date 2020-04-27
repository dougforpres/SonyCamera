#pragma once
#include "MessageReader.h"
#include <string>

enum class SensorType
{
    UNKNOWN = -1,
    MONOCHROME = 0,
    COLOUR = 1,
    RGGB = 2,
    CMYG = 3,
    CMYG2 = 4,
    LRGB = 5,
};

class DeviceInfo : public MessageReader
{
public:
    DeviceInfo(Message* message);
    virtual ~DeviceInfo();

    void DumpToLog();

    std::wstring GetManufacturer();
    std::wstring GetModel();
    std::wstring GetSerialNumber();

    void SetSensorType(SensorType type);
    SensorType GetSensorType();
    void SetSensorName(std::wstring name);
    std::wstring GetSensorName();
    void SetSensorXResolution(UINT32 resolution);
    UINT32 GetSensorXResolution();
    void SetSensorYResolution(UINT32 resolution);
    UINT32 GetSensorYResolution();
    void SetPreviewXResolution(UINT32 resolution);
    UINT32 GetPreviewXResolution();
    void SetPreviewYResolution(UINT32 resolution);
    UINT32 GetPreviewYResolution();
    void SetSensorPixelWidth(double width);
    double GetSensorPixelWidth();
    void SetSensorPixelHeight(double height);
    double GetSensorPixelHeight();
    void SetBayerXOffset(INT8 offset);
    INT8 GetBayerXOffset();
    void SetBayerYOffset(INT8 offset);
    INT8 GetBayerYOffset();
    void SetExposureTimeMin(double time);
    double GetExposureTimeMin();
    void SetExposureTimeMax(double time);
    double GetExposureTimeMax();
    void SetExposureTimeStep(double step);
    double GetExposureTimeStep();
    std::wstring GetVersion();
    bool GetSupportsLiveview();
    void SetSupportsLiveview(bool support);

private:
    void DumpList(std::list<WORD> list);

    double m_standardVersion = 0;
    DWORD m_vendorExtensionId = 0;
    double m_vendorExtensionVersion = 0;
    std::wstring m_vendorExtensionDesc;
    WORD m_functionalMode = 0;
    std::list<WORD> m_operationsSupported;
    std::list<WORD> m_eventsSupported;
    std::list<WORD> m_devicePropertiesSupported;
    std::list<WORD> m_captureFormats;
    std::list<WORD> m_imageFormats;
    std::wstring m_manufacturer;
    std::wstring m_model;
    std::wstring m_deviceVersion;
    std::wstring m_serialNumber;
    std::wstring m_sensorName;
    SensorType m_sensorType = SensorType::UNKNOWN;
    DWORD m_sensorXResolution = 0;
    DWORD m_sensorYResolution = 0;
    DWORD m_previewXResolution = 0;
    DWORD m_previewYResolution = 0;
    BYTE m_bayerOffsetX = 0;
    BYTE m_bayerOffsetY = 0;
    double m_sensorPixelWidth = 0.0;
    double m_sensorPixelHeight = 0.0;
    double m_exposureTimeMin = 0.0;
    double m_exposureTimeMax = 0.0;
    double m_exposureTimeStep = 0.0;
    bool m_supportsLiveview = false;
};


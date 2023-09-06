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

enum class CropMode
{
    NONE = 0,
    AUTO = 1,
    USER = 2,
};

class DeviceInfo : public MessageReader
{
public:
    DeviceInfo();
    DeviceInfo(Message* message);
    DeviceInfo(const DeviceInfo& rhs);
    virtual ~DeviceInfo();
    DeviceInfo operator=(const DeviceInfo& rhs);

    void DumpToLog() const;

    std::wstring GetManufacturer() const;
    std::wstring GetModel() const;
    std::wstring GetSerialNumber() const;

    void SetSensorType(SensorType type);
    SensorType GetSensorType() const;
    void SetSensorName(std::wstring name);
    std::wstring GetSensorName() const;
    void SetSensorXResolution(UINT32 resolution);
    UINT32 GetSensorXResolution() const;
    void SetSensorYResolution(UINT32 resolution);
    UINT32 GetSensorYResolution() const;
    void SetSensorXCroppedResolution(UINT32 resolution);
    UINT32 GetSensorXCroppedResolution() const;
    void SetSensorYCroppedResolution(UINT32 resolution);
    UINT32 GetSensorYCroppedResolution() const;
    void SetPreviewXResolution(UINT32 resolution);
    UINT32 GetPreviewXResolution() const;
    void SetPreviewYResolution(UINT32 resolution);
    UINT32 GetPreviewYResolution() const;
    void SetSensorPixelWidth(double width);
    double GetSensorPixelWidth() const;
    void SetSensorPixelHeight(double height);
    double GetSensorPixelHeight() const;
    void SetBayerXOffset(INT8 offset);
    INT8 GetBayerXOffset() const;
    void SetBayerYOffset(INT8 offset);
    INT8 GetBayerYOffset() const;
    void SetExposureTimeMin(double time);
    double GetExposureTimeMin() const;
    void SetExposureTimeMax(double time);
    double GetExposureTimeMax() const;
    void SetExposureTimeStep(double step);
    double GetExposureTimeStep() const;
    std::wstring GetVersion() const;
    bool GetSupportsLiveview() const;
    void SetSupportsLiveview(bool support);
    CropMode GetCropMode() const;
    void SetCropMode(CropMode mode);
    UINT16 GetLeftCrop() const;
    void SetLeftCrop(UINT16 crop);
    UINT16 GetBottomCrop() const;
    void SetBottomCrop(UINT16 crop);
    UINT16 GetRightCrop() const;
    void SetRightCrop(UINT16 crop);
    UINT16 GetTopCrop() const;
    void SetTopCrop(UINT16 crop);
    std::list<DWORD> GetExposureTimes() const;
    void SetExposureTimes(std::list<DWORD> exposureTimes);
    std::list<DWORD> GetISOs() const;
    void SetISOs(std::list<DWORD> isos);
    UINT32 GetBitsPerPixel() const;
    void SetBitsPerPixel(UINT32 bpp);

    bool GetButtonPropertiesInverted() const;
    void SetButtonPropertiesInverted(bool invert);

private:
    void Copy(const DeviceInfo& rhs);
    void DumpList(std::list<WORD> list) const;
    void DumpList(std::list<DWORD> list) const;

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
    DWORD m_sensorXCroppedResolution = 0;
    DWORD m_sensorYCroppedResolution = 0;
    DWORD m_previewXResolution = 0;
    DWORD m_previewYResolution = 0;
    DWORD m_bitsPerPixel = 14;
    UINT16 m_leftCrop = 0;
    UINT16 m_rightCrop = 0;
    UINT16 m_topCrop = 0;
    UINT16 m_bottomCrop = 0;
    // Why not AUTO here?
    // Well some apps (not mentioning names SharpCap) don't deal too well with
    // returned images being a different size than the camera originally reported
    // We'll add some extra fields later so we can use AUTO, but for now this is
    // here for backwards compatibility
    CropMode m_cropMode = CropMode::NONE;
    BYTE m_bayerOffsetX = 0;
    BYTE m_bayerOffsetY = 0;
    double m_sensorPixelWidth = 0.0;
    double m_sensorPixelHeight = 0.0;
    double m_exposureTimeMin = 0.0;
    double m_exposureTimeMax = 0.0;
    double m_exposureTimeStep = 0.0;
    bool m_supportsLiveview = false;
    bool m_buttonPropertiesInverted = false;
    std::list<DWORD> m_exposureTimes;
    std::list<DWORD> m_isos;
};


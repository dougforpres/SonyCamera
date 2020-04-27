#pragma once
#include "MessageReader.h"
#include "ObjectInfo.h"
#include "libraw/libraw.h"
#include "libjpeg//turbojpeg.h"
#include <string>


class Image : public MessageReader
{
public:
    Image(ObjectInfo* info, Message* message);
    virtual ~Image();

    DWORD GetDataLength();
    BYTE* GetData();
    DWORD GetImageDataSize();
    BYTE* GetImageData();
    DWORD GetWidth();
    DWORD GetHeight();
//    InputMode GetFormat();
    OutputMode GetOutputMode();
    void SetOutputMode(OutputMode mode);
    void SaveFile(std::wstring path = L"");
    InputMode GetInputMode();
    bool EnsurePixelsProcessed();
    void SetDuration(double duration);
    double GetDuration();

private:
    std::wstring WidenString(const char* shortString);
    std::wstring LibRawErrorToString(LibRaw* libraw, int librawError);
    std::wstring TurboJPEGErrorToString(tjhandle tj, int tjError);

    LibRaw* InitializeLibRaw(bool process);

    bool ProcessARWData();
    bool ProcessJPEGData();

    bool StringEndsWith(const std::wstring& mainStr, const std::wstring& toMatch);

    ObjectInfo* m_info = nullptr;
    double m_duration = 0.0;
    DWORD m_imageOffset = 0;
    DWORD m_filesize = 0;
    int m_pixelDataSize = 0;
    OutputMode m_outputMode = OutputMode::RGB;
    InputMode m_inputMode = InputMode::UNKNOWN;
    BYTE* m_pixelData = nullptr;
};

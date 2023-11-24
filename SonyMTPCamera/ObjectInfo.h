#pragma once
#include "MessageReader.h"
#include "Message.h"

enum class OutputMode
{
    RGGB = 0x001,
    RGB = 0x002,
    JPEG = 0x003,
    PASSTHRU = 0x100
};

enum class InputMode
{
    UNKNOWN = 0x0000,
    ARW     = 0xb101,
    JPEG    = 0x3801
};

class ObjectInfo : public MessageReader
{
public:
    ObjectInfo(InputMode formatCode);
    ObjectInfo(Message* message);

    std::wstring GetFilename();

    DWORD GetWidth() const;
    DWORD GetHeight() const;
    DWORD GetBitDepth() const;
    InputMode GetFormat() const;
    DWORD GetCompressedSize() const;

    void SetWidth(DWORD width);
    void SetHeight(DWORD height);

private:
    DWORD m_storageId = 0;
    InputMode m_objectFormatCode = InputMode::UNKNOWN;
    WORD m_protectionStatus = 0;
    DWORD m_objectCompressedSize = 0;
    WORD m_thumbFormat = 0;
    DWORD m_thumbCompressedSize = 0;
    DWORD m_thumbPixWidth = 0;
    DWORD m_thumbPixHeight = 0;
    DWORD m_imagePixWidth = 0;
    DWORD m_imagePixHeight = 0;
    DWORD m_imageBitDepth = 0;
    DWORD m_parentObject = 0;
    WORD m_associationCode = 0;
    DWORD m_associationDesc = 0;
    DWORD m_sequenceNumber = 0;
    std::wstring m_filename;
    std::wstring m_captureDate;
    std::wstring m_modificationDate;
    std::wstring m_keywords;
};
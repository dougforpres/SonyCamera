#include "pch.h"
#include "ObjectInfo.h"
#include "Logger.h"

ObjectInfo::ObjectInfo(InputMode formatCode)
    : MessageReader(nullptr),
      m_objectFormatCode(formatCode)
{

}

ObjectInfo::ObjectInfo(Message* message)
    : MessageReader(message)
{
    LOGTRACE(L"In: ObjectInfo::ObjectInfo");

    DWORD offset = 0;

    m_storageId = GetDWORD(offset);
    m_objectFormatCode = (InputMode)GetWORD(offset);
    m_protectionStatus = GetWORD(offset);
    m_objectCompressedSize = GetDWORD(offset);
    m_thumbFormat = GetWORD(offset);
    m_thumbCompressedSize = GetDWORD(offset);
    m_thumbPixWidth = GetDWORD(offset);
    m_thumbPixHeight = GetDWORD(offset);
    m_imagePixWidth = GetDWORD(offset);
    m_imagePixHeight = GetDWORD(offset);
    m_imageBitDepth = GetDWORD(offset);
    m_parentObject = GetDWORD(offset);
    m_associationCode = GetWORD(offset);
    m_associationDesc = GetDWORD(offset);
    m_sequenceNumber = GetDWORD(offset);
    m_filename = GetString(offset);
    m_captureDate = GetString(offset);
    m_modificationDate = GetString(offset);
    m_keywords = GetString(offset);

    LOGINFO(L"  Storage ID             x%08x", m_storageId);
    LOGINFO(L"  Object Format Code     x%04x", m_objectFormatCode);
    LOGINFO(L"  Protection Status      %d", m_protectionStatus);
    LOGINFO(L"  Object Compressed Size %d", m_objectCompressedSize);
    LOGINFO(L"  Thumb Format           %d", m_thumbFormat);
    LOGINFO(L"  Thumb Compressed Size  %d", m_thumbCompressedSize);
    LOGINFO(L"  Thumb Pix Width        %d", m_thumbPixWidth);
    LOGINFO(L"  Thumb Pix Height       %d", m_thumbPixHeight);
    LOGINFO(L"  Image Pix Width        %d", m_imagePixWidth);
    LOGINFO(L"  Image Pix Height       %d", m_imagePixHeight);
    LOGINFO(L"  Image Bit Depth        %d", m_imageBitDepth);
    LOGINFO(L"  Parent Object          %d", m_parentObject);
    LOGINFO(L"  Association Code       %d", m_associationCode);
    LOGINFO(L"  Association Desc       %d", m_associationDesc);
    LOGINFO(L"  Sequence Number        %d", m_sequenceNumber);
    LOGINFO(L"  Filename               %s", m_filename.c_str());
    LOGINFO(L"  Capture Date           %s", m_captureDate.c_str());
    LOGINFO(L"  Keywords               %s", m_keywords.c_str());

    LOGTRACE(L"Out: ObjectInfo::ObjectInfo");
}

std::wstring
ObjectInfo::GetFilename()
{
    return m_filename;
}

DWORD
ObjectInfo::GetWidth()
{
    return m_imagePixWidth;
}

void
ObjectInfo::SetWidth(DWORD width)
{
    m_imagePixWidth = width;
}

DWORD
ObjectInfo::GetHeight()
{
    return m_imagePixHeight;
}

void
ObjectInfo::SetHeight(DWORD height)
{
    m_imagePixHeight = height;
}

DWORD
ObjectInfo::GetBitDepth()
{
    return m_imageBitDepth;
}

InputMode
ObjectInfo::GetFormat()
{
    return m_objectFormatCode;
}

DWORD
ObjectInfo::GetCompressedSize()
{
    return m_objectCompressedSize;
}
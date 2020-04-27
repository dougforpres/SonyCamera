#pragma once
#include <PropIdl.h>
#include "Property.h"

// Base class intended to be a little like Object for Int/Str type values
class PropertyValue
{
public:
    PropertyValue(const PropertyValue& rhs);
    PropertyValue(INT8 value);
    PropertyValue(UINT8 value);
    PropertyValue(INT16 value);
    PropertyValue(UINT16 value);
    PropertyValue(INT32 value);
    PropertyValue(UINT32 value);
    PropertyValue(std::wstring value);

    virtual ~PropertyValue();

    UINT8  GetUINT8();
    INT8   GetINT8();
    UINT16 GetUINT16();
    INT16  GetINT16();
    UINT32 GetUINT32();
    INT32  GetINT32();
    std::wstring GetString();

    size_t GetDataSize();
    size_t Write(BYTE* buffer, DWORD bufferLen);
    DataType GetType();

    std::wstring ToString();

    PROPVARIANT ToPROPVARIANT();

protected:
    DataType m_type = DataType::UNKNOWN;
    UINT8  m_uint8 = 0;
    INT8   m_int8 = 0;
    UINT16 m_uint16 = 0;
    INT16  m_int16 = 0;
    UINT32 m_uint32 = 0;
    INT32  m_int32 = 0;
    std::wstring m_str;
};

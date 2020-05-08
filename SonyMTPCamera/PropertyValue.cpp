#include "pch.h"
#include "PropertyValue.h"
#include "Property.h"


PropertyValue::PropertyValue(const PropertyValue& rhs)
    : m_int8(rhs.m_int8),
      m_uint8(rhs.m_uint8),
      m_int16(rhs.m_int16),
      m_uint16(rhs.m_uint16),
      m_int32(rhs.m_int32),
      m_uint32(rhs.m_uint32),
      m_type(rhs.m_type)
{

}

PropertyValue::PropertyValue(INT8 value)
    : m_int8(value),
      m_type(DataType::INT8)
{

}

PropertyValue::PropertyValue(UINT8 value)
    : m_uint8(value),
      m_type(DataType::UINT8)
{

}

PropertyValue::PropertyValue(INT16 value)
    : m_int16(value),
      m_type(DataType::INT16)
{

}

PropertyValue::PropertyValue(UINT16 value)
    : m_uint16(value),
      m_type(DataType::UINT16)
{

}

PropertyValue::PropertyValue(INT32 value)
    : m_int32(value),
      m_type(DataType::INT32)
{

}

PropertyValue::PropertyValue(UINT32 value)
    : m_uint32(value),
      m_type(DataType::UINT32)
{

}

PropertyValue::PropertyValue(std::wstring value)
    : m_str(value),
    m_type(DataType::STR)
{

}

PropertyValue::~PropertyValue()
{

}

DataType
PropertyValue::GetType()
{
    return m_type;
}

UINT8
PropertyValue::GetUINT8()
{
    return m_uint8;
}

INT8
PropertyValue::GetINT8()
{
    return m_int8;
}

UINT16
PropertyValue::GetUINT16()
{
    return m_uint16;
}

INT16
PropertyValue::GetINT16()
{
    return m_uint16;
}

UINT32
PropertyValue::GetUINT32()
{
    return m_uint32;
}

INT32
PropertyValue::GetINT32()
{
    return m_uint32;
}

std::wstring
PropertyValue::GetString()
{
    return m_str;
}

PROPVARIANT
PropertyValue::ToPROPVARIANT()
{
    PROPVARIANT result = { 0 };

    switch (m_type)
    {
    case DataType::INT8:
        result.vt = VT_I1;
        result.iVal = m_int8;
        break;

    case DataType::UINT8:
        result.vt = VT_UI1;
        result.iVal = m_uint8;
        break;

    case DataType::INT16:
        result.vt = VT_I2;
        result.iVal = m_int16;
        break;

    case DataType::UINT16:
        result.vt = VT_UI2;
        result.iVal = m_uint16;
        break;

    case DataType::INT32:
        result.vt = VT_I4;
        result.iVal = m_int32;
        break;

    case DataType::UINT32:
        result.vt = VT_UI4;
        result.iVal = m_uint32;
        break;

    default:
        throw;
    }

    return result;
}

size_t
PropertyValue::Write(BYTE* buffer, DWORD bufferLen)
{
    size_t result = GetDataSize();

    if (bufferLen >= result)
    {
        switch (m_type)
        {
        case DataType::INT8:
            memcpy(buffer, &m_int8, result);
            break;

        case DataType::UINT8:
            memcpy(buffer, &m_uint8, result);
            break;

        case DataType::INT16:
            memcpy(buffer, &m_int16, result);
            break;

        case DataType::UINT16:
            memcpy(buffer, &m_uint16, result);
            break;

        case DataType::INT32:
            memcpy(buffer, &m_int32, result);
            break;

        case DataType::UINT32:
            memcpy(buffer, &m_uint32, result);
            break;

        case DataType::STR:
            memcpy(buffer, m_str.c_str(), result);
            break;

        default:
            throw;
        }
    }

    return result;
}

size_t
PropertyValue::GetDataSize()
{
    size_t result = 0;

    switch (m_type)
    {
    case DataType::INT8:
        result = sizeof(INT8);
        break;

    case DataType::UINT8:
        result = sizeof(UINT8);
        break;

    case DataType::INT16:
        result = sizeof(INT16);
        break;

    case DataType::UINT16:
        result = sizeof(UINT16);
        break;

    case DataType::INT32:
        result = sizeof(INT32);
        break;

    case DataType::UINT32:
        result = sizeof(UINT32);
        break;

    case DataType::STR:
        result = m_str.length() * sizeof(WCHAR);
        break;

    default:
        throw;
    }

    return result;
}

std::wstring
PropertyValue::ToString()
{
    std::wostringstream builder;

    switch (m_type)
    {
    case DataType::INT8:
        builder << L"(INT8)x" << std::hex << std::setw(2) << std::setfill(L'0') << m_int8;
        break;

    case DataType::UINT8:
        builder << L"(UINT8)x" << std::hex << std::setw(2) << std::setfill(L'0') << m_uint8;
        break;

    case DataType::INT16:
        builder << L"(INT16)x" << std::hex << std::setw(4) << std::setfill(L'0') << m_int16;
        break;

    case DataType::UINT16:
        builder << L"(UINT16)x" << std::hex << std::setw(4) << std::setfill(L'0') << m_uint16;
        break;

    case DataType::INT32:
        builder << L"(INT32)x" << std::hex << std::setw(8) << std::setfill(L'0') << m_int32;
        break;

    case DataType::UINT32:
        builder << L"(UINT32)x" << std::hex << std::setw(8) << std::setfill(L'0') << m_uint32;
        break;

    case DataType::STR:
        builder << L"(STR)'" << m_str << "'";
        break;

    default:
        throw;
    }

    return builder.str();
}
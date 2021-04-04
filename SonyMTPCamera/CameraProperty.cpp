#include "pch.h"
#include <unordered_map>
#include "Property.h"
#include "CameraProperty.h"
#include "ResourceLoader.h"

CameraProperty::CameraProperty()
{
    m_info = new PropertyInfo();
}

CameraProperty::CameraProperty(const CameraProperty& rhs)
{
    m_info = new PropertyInfo(*rhs.m_info);

    if (rhs.m_current)
    {
        SetCurrentValue(new PropertyValue(*rhs.m_current));
    }
}

CameraProperty::~CameraProperty()
{
    if (m_info)
    {
        delete m_info;
        m_info = nullptr;
    }

    if (m_current)
    {
        delete m_current;
        m_current = nullptr;
    }
}

void
CameraProperty::SetId(Property id)
{
    m_info->SetId(id);
}

Property
CameraProperty::GetId()
{
    return m_info->GetId();
}

std::wstring
CameraProperty::GetName()
{
    return ResourceLoader::GetString((UINT)GetId());
}

std::wstring
CameraProperty::AsString(PropertyValue* value)
{
    std::wostringstream builder;

    switch (m_info->GetType())
    {
    case DataType::INT8:
        builder << value->GetINT8();
        break;

    case DataType::UINT8:
        builder << value->GetUINT8();
        break;

    case DataType::INT16:
        builder << value->GetINT16();
        break;

    case DataType::UINT16:
        builder << value->GetUINT16();
        break;

    case DataType::INT32:
        builder << value->GetINT32();
        break;

    case DataType::UINT32:
        builder << value->GetUINT32();
        break;
        /*
            case DataType::INT64:
                builder << value->_INT64;
                v << value->_INT64;
                break;

            case DataType::UINT64:
                builder << value->_UINT64;
                v << value->_UINT64;
                break;
                */
    case DataType::STR:
        builder << value->GetString();
        break;
    }

    return builder.str();
}

std::wstring
CameraProperty::AsString()
{
    return AsString(m_current);
}

// Debug info
std::wstring
CameraProperty::ToString()
{
    std::wostringstream builder;

    builder << m_info->ToString() << L" = " << AsString();

    return builder.str();
}

PropertyInfo*
CameraProperty::GetInfo()
{
    return m_info;
}

bool
CameraProperty::equals(const CameraProperty& rhs)
{
    bool same = true;

    if (m_info->GetId() != rhs.m_info->GetId())
    {
        same = false;
    }

    if (m_info->GetAccess() != rhs.m_info->GetAccess())
    {
        same = false;
    }

    if (m_info->GetSonySpare() != rhs.m_info->GetSonySpare())
    {
        same = false;
    }

    if (m_info->GetType() != rhs.m_info->GetType())
    {
        same = false;
    }

    switch (m_info->GetType())
    {
    case DataType::INT8:
        if (m_current->GetINT8() != rhs.m_current->GetINT8())
        {
            same = false;
        }
        break;

    case DataType::UINT8:
        if (m_current->GetUINT8() != rhs.m_current->GetUINT8())
        {
            same = false;
        }
        break;

    case DataType::INT16:
        if (m_current->GetINT16() != rhs.m_current->GetINT16())
        {
            same = false;
        }
        break;

    case DataType::UINT16:
        if (m_current->GetUINT16() != rhs.m_current->GetUINT16())
        {
            same = false;
        }
        break;

    case DataType::INT32:
        if (m_current->GetINT32() != rhs.m_current->GetINT32())
        {
            same = false;
        }
        break;

    case DataType::UINT32:
        if (m_current->GetUINT32() != rhs.m_current->GetUINT32())
        {
            same = false;
        }
        break;

/*    case DataType::INT64:
        if (m_current->_INT64 != rhs.m_current->_INT64)
        {
            same = false;
        }
        break;

    case DataType::UINT64:
        if (m_current->_UINT64 != rhs.m_current->_UINT64)
        {
            same = false;
        }
        break;
        */
    }

    return same;
}

DWORD
CameraProperty::GetDWORD(BYTE* data, DWORD offset)
{
    DWORD result = 0;

    result = (DWORD)(data[offset] + (data[offset + 1] << 8) + (data[offset + 2] << 16) + (data[offset + 3] << 24));

    return result;
}

WORD
CameraProperty::GetWORD(BYTE* data, DWORD offset)
{
    WORD result = 0;

    result = (WORD)(data[offset] + (data[offset + 1] << 8));

    return result;
}

BYTE
CameraProperty::GetBYTE(BYTE* data, DWORD offset)
{
    BYTE result = 0;

    result = data[offset];

    return result;
}

DWORD
CameraProperty::Slurp(BYTE* data, DWORD offset)
{
    m_info->SetId((Property)GetWORD(data, offset));

    DataType type = (DataType)GetWORD(data, offset + 2);

    m_info->SetType(type);
    m_info->SetAccess((Accessibility)GetBYTE(data, offset + 4));
    m_info->SetSonySpare(GetBYTE(data, offset + 5));

    offset += 6;

    m_info->SetDefault(ReadData(data, offset, type));
    m_current = ReadData(data, offset, type);

    m_info->SetFormMode((FormMode)GetBYTE(data, offset++));

    switch (m_info->GetFormMode())
    {
    case FormMode::NONE:
        // No more
        break;

    case FormMode::RANGE:
        // Range
        m_info->SetRange(ReadData(data, offset, type), ReadData(data, offset, type), ReadData(data, offset, type));
        break;

    case FormMode::ENUMERATION:
        // Enum
        WORD numValues = GetWORD(data, offset);

        offset += 2;

        std::list<PropertyValue*> values;

        for (WORD i = 0; i < numValues; i++)
        {
            values.push_back(ReadData(data, offset, type));
        }

        m_info->SetEnumeration(values);

        break;
    }

    return offset;
}

PropertyValue*
CameraProperty::ReadData(BYTE* data, DWORD &offset, DataType type)
{
    PropertyValue* result = nullptr;

    switch (m_info->GetType())
    {
    case DataType::UINT8:
        result = new PropertyValue((UINT8)GetBYTE(data, offset++));
        break;

    case DataType::INT8:
        result = new PropertyValue((INT8)GetBYTE(data, offset++));
        break;

    case DataType::UINT16:
        result = new PropertyValue((UINT16)GetWORD(data, offset));
        offset += 2;
        break;

    case DataType::INT16:
        result = new PropertyValue((INT16)GetWORD(data, offset));
        offset += 2;
        break;

    case DataType::UINT32:
        result = new PropertyValue((UINT32)GetDWORD(data, offset));
        offset += 4;
        break;

    case DataType::INT32:
        result = new PropertyValue((INT32)GetDWORD(data, offset));
        offset += 4;
        break;

    default:
        throw;
        break;
    }

    return result;
}

PropertyValue *
CameraProperty::GetCurrentValue()
{
    return m_current;
}

void
CameraProperty::SetCurrentValue(PropertyValue* current)
{
    m_current = current;
}

ISOProperty::ISOProperty()
    : CameraProperty()
{

}

std::wstring
ISOProperty::AsString(PropertyValue *in)
{
    std::wostringstream builder;
    UINT32 value = in->GetUINT32();

    if (value != 0x00ffffff)
    {
        builder << value;
    }
    else
    {
        builder << "AUTO";
    }

    return builder.str();
}

ShutterTimingProperty::ShutterTimingProperty()
    : CameraProperty()
{

}

std::wstring
ShutterTimingProperty::AsString(PropertyValue *in)
{
    std::wostringstream builder;
    UINT32 value = in->GetUINT32();

    // Shutter
    WORD numerator = (WORD)((value & 0xffff0000) >> 16);
    WORD denominator = (WORD)(value & 0x0000ffff);

    switch (numerator)
    {
    case 0:
        builder << "BULB";
        break;

    case 1:
        builder << "1/" << denominator << "\"";

        break;

    default:
        builder << (float)(numerator / (float)denominator) << "\"";
        break;
    }

    return builder.str();
}

Div10Property::Div10Property()
    : CameraProperty()
{

}

std::wstring
Div10Property::AsString(PropertyValue *in)
{
    std::wostringstream builder;
    UINT16 value = in->GetUINT16();

    builder << value / 10.0;

    return builder.str();
}

Div100Property::Div100Property()
    : CameraProperty()
{

}

std::wstring
Div100Property::AsString(PropertyValue *in)
{
    std::wostringstream builder;
    UINT16 value = in->GetUINT16();

    builder << value / 100.0;

    return builder.str();
}

Div1000Property::Div1000Property()
    : CameraProperty()
{

}

std::wstring
Div1000Property::AsString(PropertyValue *in)
{
    std::wostringstream builder;
    INT16 value = in->GetINT16();

    builder << value / 1000.0;

    return builder.str();
}

PercentageProperty::PercentageProperty()
    : CameraProperty()
{

}

std::wstring
PercentageProperty::AsString(PropertyValue *in)
{
    std::wostringstream builder;

    builder << in->GetINT8() << L"%";

    return builder.str();
}

std::unordered_map<DWORD, WORD> StringLookupProperty::_resourceMap;

StringLookupProperty::StringLookupProperty()
    : CameraProperty()
{
    if (StringLookupProperty::_resourceMap.empty())
    {
        // Compression
        AddResource(Property::CompressionSetting, 0x0010, IDS_COMPRESSION_RAW);
        AddResource(Property::CompressionSetting, 0x0004, IDS_COMPRESSION_JPEG_XFINE);
        AddResource(Property::CompressionSetting, 0x0003, IDS_COMPRESSION_JPEG_FINE);
        AddResource(Property::CompressionSetting, 0x0002, IDS_COMPRESSION_JPEG_STD);
        AddResource(Property::CompressionSetting, 0x0014, IDS_COMPRESSION_RAWJPEG_XFINE);
        AddResource(Property::CompressionSetting, 0x0013, IDS_COMPRESSION_RAWJPEG_FINE);
        AddResource(Property::CompressionSetting, 0x0012, IDS_COMPRESSION_RAWJPEG_STD);

        // Metering Mode
        AddResource(Property::ExposureMeteringMode, 0x8001, IDS_METERINGMODE_MULTI);
        AddResource(Property::ExposureMeteringMode, 0x8002, IDS_METERINGMODE_CENTER);
        AddResource(Property::ExposureMeteringMode, 0x8003, IDS_METERINGMODE_ENTIRESCREEN);
        AddResource(Property::ExposureMeteringMode, 0x8004, IDS_METERINGMODE_SPOTSTANDARD);
        AddResource(Property::ExposureMeteringMode, 0x8005, IDS_METERINGMODE_SPOTLARGE);
        AddResource(Property::ExposureMeteringMode, 0x8006, IDS_METERINGMODE_HIGHLIGHT);

        // Focus Area
        AddResource(Property::FocusArea, 0x0001, IDS_FOCUSAREA_WIDE);
        AddResource(Property::FocusArea, 0x0002, IDS_FOCUSAREA_ZONE);
        AddResource(Property::FocusArea, 0x0003, IDS_FOCUSAREA_CENTER);
        AddResource(Property::FocusArea, 0x0101, IDS_FOCUSAREA_FLEXS);
        AddResource(Property::FocusArea, 0x0102, IDS_FOCUSAREA_FLEXM);
        AddResource(Property::FocusArea, 0x0103, IDS_FOCUSAREA_FLEXL);
        AddResource(Property::FocusArea, 0x0104, IDS_FOCUSAREA_EXPAND);

        // White Balance
        AddResource(Property::WhiteBalance, 0x0002, IDS_WHITEBALANCE_AUTO);
        AddResource(Property::WhiteBalance, 0x0004, IDS_WHITEBALANCE_DAYLIGHT);
        AddResource(Property::WhiteBalance, 0x8011, IDS_WHITEBALANCE_SHADE);
        AddResource(Property::WhiteBalance, 0x8010, IDS_WHITEBALANCE_CLOUDY);
        AddResource(Property::WhiteBalance, 0x0006, IDS_WHITEBALANCE_INCANDESCENT);
        AddResource(Property::WhiteBalance, 0x8001, IDS_WHITEBALANCE_FLOURWARM);
        AddResource(Property::WhiteBalance, 0x8002, IDS_WHITEBALANCE_FLOURCOOL);
        AddResource(Property::WhiteBalance, 0x8003, IDS_WHITEBALANCE_FLOURDAY);
        AddResource(Property::WhiteBalance, 0x8004, IDS_WHITEBALANCE_FLOURDAYLIGHT);
        AddResource(Property::WhiteBalance, 0x0007, IDS_WHITEBALANCE_FLASH);
        AddResource(Property::WhiteBalance, 0x8030, IDS_WHITEBALANCE_UNDERWATER);
        AddResource(Property::WhiteBalance, 0x8012, IDS_WHITEBALANCE_CTEMP);
        AddResource(Property::WhiteBalance, 0x8020, IDS_WHITEBALANCE_CUSTOM1);
        AddResource(Property::WhiteBalance, 0x8021, IDS_WHITEBALANCE_CUSTOM2);
        AddResource(Property::WhiteBalance, 0x8022, IDS_WHITEBALANCE_CUSTOM3);

        // Capture Mode
        AddResource(Property::StillCaptureMode, 0x0001, IDS_STILLCAPMODE_SINGLE);
        AddResource(Property::StillCaptureMode, 0x0002, IDS_STILLCAPMODE_CONTINUOUSHI);
        AddResource(Property::StillCaptureMode, 0x8015, IDS_STILLCAPMODE_CONTINUOUSMID);
        AddResource(Property::StillCaptureMode, 0x8012, IDS_STILLCAPMODE_CONTINUOUSLO);
        AddResource(Property::StillCaptureMode, 0x8010, IDS_STILLCAPMODE_CONTINUOUSHIPLUS);
        AddResource(Property::StillCaptureMode, 0x8004, IDS_STILLCAPMODE_SELFSINGLE_10);
        AddResource(Property::StillCaptureMode, 0x8003, IDS_STILLCAPMODE_SELFSINGLE_5);
        AddResource(Property::StillCaptureMode, 0x8005, IDS_STILLCAPMODE_SELFSINGLE_2);
        AddResource(Property::StillCaptureMode, 0x8008, IDS_STILLCAPMODE_SELFCONT_10_3);
        AddResource(Property::StillCaptureMode, 0x8009, IDS_STILLCAPMODE_SELFCONT_10_5);
        AddResource(Property::StillCaptureMode, 0x800c, IDS_STILLCAPMODE_SELFCONT_5_3);
        AddResource(Property::StillCaptureMode, 0x800d, IDS_STILLCAPMODE_SELFCONT_5_5);
        AddResource(Property::StillCaptureMode, 0x800e, IDS_STILLCAPMODE_SELFCONT_2_3);
        AddResource(Property::StillCaptureMode, 0x800f, IDS_STILLCAPMODE_SELFCONT_2_5);
        AddResource(Property::StillCaptureMode, 0x8337, IDS_STILLCAPMODE_CONTBRKT_DOT3_3);
        AddResource(Property::StillCaptureMode, 0x8537, IDS_STILLCAPMODE_CONTBRKT_DOT3_5);
        AddResource(Property::StillCaptureMode, 0x8937, IDS_STILLCAPMODE_CONTBRKT_DOT3_9);
        AddResource(Property::StillCaptureMode, 0x8357, IDS_STILLCAPMODE_CONTBRKT_DOT5_3);
        AddResource(Property::StillCaptureMode, 0x8557, IDS_STILLCAPMODE_CONTBRKT_DOT5_5);
        AddResource(Property::StillCaptureMode, 0x8957, IDS_STILLCAPMODE_CONTBRKT_DOT5_9);
        AddResource(Property::StillCaptureMode, 0x8377, IDS_STILLCAPMODE_CONTBRKT_DOT7_3);
        AddResource(Property::StillCaptureMode, 0x8577, IDS_STILLCAPMODE_CONTBRKT_DOT7_5);
        AddResource(Property::StillCaptureMode, 0x8977, IDS_STILLCAPMODE_CONTBRKT_DOT7_9);
        AddResource(Property::StillCaptureMode, 0x8311, IDS_STILLCAPMODE_CONTBRKT_1_3);
        AddResource(Property::StillCaptureMode, 0x8511, IDS_STILLCAPMODE_CONTBRKT_1_5);
        AddResource(Property::StillCaptureMode, 0x8911, IDS_STILLCAPMODE_CONTBRKT_1_9);
        AddResource(Property::StillCaptureMode, 0x8321, IDS_STILLCAPMODE_CONTBRKT_2_3);
        AddResource(Property::StillCaptureMode, 0x8521, IDS_STILLCAPMODE_CONTBRKT_2_5);
        AddResource(Property::StillCaptureMode, 0x8331, IDS_STILLCAPMODE_CONTBRKT_3_3);
        AddResource(Property::StillCaptureMode, 0x8351, IDS_STILLCAPMODE_CONTBRKT_3_5);
        AddResource(Property::StillCaptureMode, 0x8336, IDS_STILLCAPMODE_SINGLEBRKT_DOT3_3);
        AddResource(Property::StillCaptureMode, 0x8536, IDS_STILLCAPMODE_SINGLEBRKT_DOT3_5);
        AddResource(Property::StillCaptureMode, 0x8936, IDS_STILLCAPMODE_SINGLEBRKT_DOT3_9);
        AddResource(Property::StillCaptureMode, 0x8356, IDS_STILLCAPMODE_SINGLEBRKT_DOT5_3);
        AddResource(Property::StillCaptureMode, 0x8556, IDS_STILLCAPMODE_SINGLEBRKT_DOT5_5);
        AddResource(Property::StillCaptureMode, 0x8956, IDS_STILLCAPMODE_SINGLEBRKT_DOT5_9);
        AddResource(Property::StillCaptureMode, 0x8376, IDS_STILLCAPMODE_SINGLEBRKT_DOT7_3);
        AddResource(Property::StillCaptureMode, 0x8576, IDS_STILLCAPMODE_SINGLEBRKT_DOT7_5);
        AddResource(Property::StillCaptureMode, 0x8976, IDS_STILLCAPMODE_SINGLEBRKT_DOT7_9);
        AddResource(Property::StillCaptureMode, 0x8310, IDS_STILLCAPMODE_SINGLEBRKT_1_3);
        AddResource(Property::StillCaptureMode, 0x8510, IDS_STILLCAPMODE_SINGLEBRKT_1_5);
        AddResource(Property::StillCaptureMode, 0x8910, IDS_STILLCAPMODE_SINGLEBRKT_1_9);
        AddResource(Property::StillCaptureMode, 0x8320, IDS_STILLCAPMODE_SINGLEBRKT_2_3);
        AddResource(Property::StillCaptureMode, 0x8520, IDS_STILLCAPMODE_SINGLEBRKT_2_5);
        AddResource(Property::StillCaptureMode, 0x8330, IDS_STILLCAPMODE_SINGLEBRKT_3_3);
        AddResource(Property::StillCaptureMode, 0x8530, IDS_STILLCAPMODE_SINGLEBRKT_3_5);
        AddResource(Property::StillCaptureMode, 0x8018, IDS_STILLCAPMODE_WHITBALBRKT_LO);
        AddResource(Property::StillCaptureMode, 0x8028, IDS_STILLCAPMODE_WHITEBALBRKT_HI);
        AddResource(Property::StillCaptureMode, 0x8019, IDS_STILLCAPMODE_DROBRKT_LO);
        AddResource(Property::StillCaptureMode, 0x8029, IDS_STILLCAPMODE_DROBRKT_HI);

        // Functional Mode
        AddResource(Property::ExposureProgramMode, 0x8001, IDS_FUNCTIONALMODE_AUTO);
        AddResource(Property::ExposureProgramMode, 0x8015, IDS_FUNCTIONALMODE_SCENE);
        AddResource(Property::ExposureProgramMode, 0x8041, IDS_FUNCTIONALMODE_PANORAMA);
        AddResource(Property::ExposureProgramMode, 0x8084, IDS_FUNCTIONALMODE_SQ);
        AddResource(Property::ExposureProgramMode, 0x8050, IDS_FUNCTIONALMODE_MOVIE);
        AddResource(Property::ExposureProgramMode, 0x0001, IDS_FUNCTIONALMODE_M);
        AddResource(Property::ExposureProgramMode, 0x0004, IDS_FUNCTIONALMODE_S);
        AddResource(Property::ExposureProgramMode, 0x0003, IDS_FUNCTIONALMODE_A);
        AddResource(Property::ExposureProgramMode, 0x0002, IDS_FUNCTIONALMODE_P);

        // Shutter Button Status
        AddResource(Property::ShutterButtonStatus, 0x0001, IDS_SHUTTERSTATUS_UP);
        AddResource(Property::ShutterButtonStatus, 0x0002, IDS_SHUTTERSTATUS_HALF);
        AddResource(Property::ShutterButtonStatus, 0x0003, IDS_SHUTTERSTATUS_DOWN);

        // Photo Buffer Status
        AddResource(Property::PhotoBufferStatus, 0x0000, IDS_PHOTOBUFFER_EMPTY);
        AddResource(Property::PhotoBufferStatus, 0x0001, IDS_PHOTOBUFFER_PENDING);
        AddResource(Property::PhotoBufferStatus, 0x8001, IDS_PHOTOBUFFER_READY);

        // Zoom Assist Mode
        AddResource(Property::FocusAssistMode, 0x0000, IDS_FOCUSASSIST_OFF);
        AddResource(Property::FocusAssistMode, 0x0001, IDS_FOCUSASSIST_ACTIVE);
        AddResource(Property::FocusAssistMode, 0x0002, IDS_FOCUSASSIST_ZOOMED);

        // JPEG Image Size
        AddResource(Property::JPEGImageSize, 0x0001, IDS_JPEGIMAGESIZE_L);
        AddResource(Property::JPEGImageSize, 0x0002, IDS_JPEGIMAGESIZE_M);
        AddResource(Property::JPEGImageSize, 0x0003, IDS_JPEGIMAGESIZE_S);

        // Aspect Ratio
        AddResource(Property::AspectRatio, 0x0001, IDS_ASPECTRATIO_3X2);
        AddResource(Property::AspectRatio, 0x0002, IDS_ASPECTRATIO_16X9);
        AddResource(Property::AspectRatio, 0x0004, IDS_ASPECTRATIO_1X1);

        // Flash Mode
        AddResource(Property::FlashMode, 0x0003, IDS_FLASHMODE_FILL);
        AddResource(Property::FlashMode, 0x0005, IDS_FLASHMODE_FILL_REDEYE);
        AddResource(Property::FlashMode, 0x8003, IDS_FLASHMODE_REAR_SYNC);
        AddResource(Property::FlashMode, 0x8031, IDS_FLASHMODE_SLOW_SYNC_REDEYE);
        AddResource(Property::FlashMode, 0x8032, IDS_FLASHMODE_SLOW_SYNC);

        // DRO/Auto HDR
        AddResource(Property::DROAutoHDR, 0x0001, IDS_DROAUTOHDR_OFF);
        AddResource(Property::DROAutoHDR, 0x001f, IDS_DROAUTOHDR_AUTO);

        // LiveView Display
        AddResource(Property::LiveView, 0x0001, IDS_ON);
        AddResource(Property::LiveView, 0x0002, IDS_OFF);

        AddResource(Property::AutoExposureLock, 0x0001, IDS_OFF);
        AddResource(Property::AutoExposureLock, 0x0002, IDS_ON);

        AddResource(Property::AutoWhileBalanceLock, 0x0001, IDS_OFF);
        AddResource(Property::AutoWhileBalanceLock, 0x0002, IDS_ON);

        AddResource(Property::FlashExposureLock, 0x0001, IDS_OFF);
        AddResource(Property::FlashExposureLock, 0x0002, IDS_ON);

        // Focus Mode
        AddResource(Property::FocusMode, 0x0001, IDS_FOCUSMODE_MANUAL);
        AddResource(Property::FocusMode, 0x0002, IDS_FOCUSMODE_AF_S);
        AddResource(Property::FocusMode, 0x8004, IDS_FOCUSMODE_AF_C);
        AddResource(Property::FocusMode, 0x8005, IDS_FOCUSMODE_AF_A);
        AddResource(Property::FocusMode, 0x8006, IDS_FOCUSMODE_DMF);
    }
}

void
StringLookupProperty::AddResource(Property property, WORD value, WORD resid)
{
    StringLookupProperty::_resourceMap.insert(std::pair<DWORD, WORD>(((DWORD)property << 16) + (DWORD)value, resid));
}

std::wstring
StringLookupProperty::AsString(PropertyValue *in)
{
    // Currently this only works for unsigned 8, 16, 32 bit values
    DWORD lookup = -1;
    std::wstring result = L"<unable to lookup>";

    switch (m_info->GetType())
    {
    case DataType::UINT8:
        lookup = (DWORD)in->GetUINT8();
        break;

    case DataType::UINT16:
        lookup = (DWORD)in->GetUINT16();
        break;

    case DataType::UINT32:
        lookup = (DWORD)in->GetUINT32();
        break;

    default:
        throw;
        break;
    }

    if (lookup >= 0)
    {
        lookup = ((DWORD)m_info->GetId() << 16) + lookup;
        lookup = StringLookupProperty::_resourceMap[lookup];

        result = ResourceLoader::GetString(lookup);

        if (result.empty())
        {
            result = CameraProperty::AsString(in);
        }
    }

    return result;
}

CameraPropertyFactory::CameraPropertyFactory()
{
    AddCreator(Property::ExposureProgramMode, &pCreate<StringLookupProperty>);
    AddCreator(Property::ISO, &pCreate<ISOProperty>);
    AddCreator(Property::StillCaptureMode, &pCreate<StringLookupProperty>);
    AddCreator(Property::ShutterSpeed, &pCreate<ShutterTimingProperty>);
    AddCreator(Property::WhiteBalance, &pCreate<StringLookupProperty>);
    AddCreator(Property::FlashCompensation, &pCreate<Div1000Property>);
    AddCreator(Property::ExposureBiasCompensation, &pCreate<Div1000Property>);
    AddCreator(Property::CompressionSetting, &pCreate<StringLookupProperty>);
    AddCreator(Property::FocusArea, &pCreate<StringLookupProperty>);
    AddCreator(Property::ExposureMeteringMode, &pCreate<StringLookupProperty>);
    AddCreator(Property::ShutterButtonStatus, &pCreate<StringLookupProperty>);
    AddCreator(Property::PhotoBufferStatus, &pCreate<StringLookupProperty>);
    AddCreator(Property::FocusAssistMode, &pCreate<StringLookupProperty>);
    AddCreator(Property::FNumber, &pCreate<Div100Property>);
    AddCreator(Property::FocusAssistZoom, &pCreate<Div10Property>);
    AddCreator(Property::Battery, &pCreate <PercentageProperty>);
    AddCreator(Property::JPEGImageSize, &pCreate<StringLookupProperty>);
    AddCreator(Property::AspectRatio, &pCreate<StringLookupProperty>);
    AddCreator(Property::FlashMode, &pCreate<StringLookupProperty>);
    AddCreator(Property::DROAutoHDR, &pCreate<StringLookupProperty>);
    AddCreator(Property::LiveView, &pCreate<StringLookupProperty>);
    AddCreator(Property::FocusMode, &pCreate<StringLookupProperty>);
    AddCreator(Property::FlashExposureLock, &pCreate<StringLookupProperty>);
    AddCreator(Property::AutoExposureLock, &pCreate<StringLookupProperty>);
    AddCreator(Property::AutoWhileBalanceLock, &pCreate<StringLookupProperty>);
    AddCreator(Property::PossibleExposureTimes, &pCreate<ShutterTimingProperty>);
}

void
CameraPropertyFactory::AddCreator(Property id, pConstructor c)
{
    std::pair<std::unordered_map<Property, pConstructor>::iterator, bool> res = m_creators.insert(std::pair<Property, pConstructor>(id, nullptr));
    (*res.first).second = c;
}

CameraProperty*
CameraPropertyFactory::Create(Property id)
{
    std::unordered_map<Property, pConstructor>::iterator it = m_creators.find(id);

    if (it != m_creators.end())
    {
        CameraProperty* p = ((*it).second)();

        return p;
    }
    else
    {
        return new CameraProperty();
    }
}
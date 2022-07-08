#include "pch.h"
#include "CameraSettings.h"
#include "CameraProperty.h"

CameraSettings::CameraSettings(Message *message)
    : MessageReader(message)
{
    CameraPropertyFactory f;
    DWORD offset = 0;

    DWORD first = GetDWORD(offset);
    DWORD second = GetDWORD(offset);

    CameraProperty* prop;

    for (DWORD i = 0; i < first; i++)
    {
        prop = f.Create((Property)GetWORD(offset));
        offset = prop->Slurp(message->GetData(), offset - 2);
        AddProperty(prop);
    }
}

CameraSettings::CameraSettings(const CameraSettings& rhs)
    : MessageReader(nullptr)
{
    for (CAMERAPROP::const_iterator it = rhs.m_properties.begin(); it != rhs.m_properties.end(); it++)
    {
        AddProperty(new CameraProperty(*(*it).second));
    }
}

CameraSettings::~CameraSettings()
{
    for (CAMERAPROP::iterator it = m_properties.begin(); it != m_properties.end(); it++)
    {
        delete (*it).second;
        (*it).second = nullptr;
    }

    m_properties.clear();
}

void
CameraSettings::AddProperty(CameraProperty *property)
{
    m_properties.insert(CAMERAPROPERTY(property->GetId(), property));
}

std::list<CameraProperty*>
CameraSettings::GetProperties()
{
    std::list<CameraProperty*> result;

    for (CAMERAPROP::iterator it = m_properties.begin(); it != m_properties.end(); it++)
    {
        result.push_back((*it).second);
    }

    return result;
}

CameraProperty *
CameraSettings::GetProperty(Property id)
{
    CAMERAPROP::iterator it = m_properties.find(id);

    return (it != m_properties.end()) ? (*it).second : nullptr;
}

PropertyValue*
CameraSettings::GetPropertyValue(Property id)
{
    CameraProperty* property = m_properties[id];

    return property ? property->GetCurrentValue() : nullptr;
}

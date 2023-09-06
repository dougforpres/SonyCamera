#include "pch.h"
#include "CameraSettings.h"
#include "CameraProperty.h"
#include "Logger.h"

CameraSettings::CameraSettings()
    : MessageReader(nullptr)
{
#ifdef DEBUG
    LOGTRACE(L"CameraSettings::CameraSettings() [this = x%08p]", this);
#endif
}

CameraSettings::CameraSettings(Message *message)
    : MessageReader(message)
{
#ifdef DEBUG
    LOGTRACE(L"CameraSettings::CameraSettings(message=x%08p) [this = x%08p]", message, this);
#endif
    if (message)
    {
        CameraPropertyFactory f;
        DWORD offset = 0;

        DWORD first = GetDWORD(offset);
        DWORD second = GetDWORD(offset);

        for (DWORD i = 0; i < first; i++)
        {
            CameraProperty* prop = nullptr;
            prop = f.Create((Property)GetWORD(offset));
            offset = prop->Slurp(message->GetData(), offset - 2);
            AddProperty(prop);
        }
    }
}

CameraSettings::CameraSettings(const CameraSettings& rhs)
    : MessageReader(nullptr)
{
#ifdef DEBUG
    LOGTRACE(L"CameraSettings::CameraSettings(const CameraSettings = x%08p) [this = x%08p]", &rhs, this);
#endif
    Copy(rhs);
}

CameraSettings*
CameraSettings::operator=(CameraSettings* rhs)
{
#ifdef DEBUG
    LOGTRACE(L"CameraSettings::operator=(x%08p) [this = x%08p]", &rhs, this);
#endif

    Copy(*rhs);

    return this;
}

void
CameraSettings::Copy(const CameraSettings& rhs)
{
    for (CAMERAPROP::const_iterator it = rhs.m_properties.begin(); it != rhs.m_properties.end(); it++)
    {
        CAMERAPROP::const_iterator fit = find((*it).first);

        if (fit != end())
        {
            CameraProperty* oldProp = (*fit).second;
            CameraProperty* newProp = (*it).second;

            // Only update if changed
            if (!oldProp->equals(newProp))
            {
#ifdef DEBUG
                LOGTRACE(L"Updating value for property x%04x [this = x%08p]", oldProp->GetId(), this);
#endif
                (*fit).second->SetCurrentValue(new PropertyValue(*newProp->GetCurrentValue()));
            }
        }
        else
        {
#ifdef DEBUG
            LOGTRACE(L"Adding new property x%04x [this = x%08p]", (*it).first, this);
#endif
            m_properties.insert(CAMERAPROPERTY((*it).first, (*it).second->Clone()));
        }
    }
}

CameraSettings::~CameraSettings()
{
#ifdef DEBUG
    LOGTRACE(L"CameraSettings::~CameraSettings() [this = %08p]", this);
#endif

    for (CAMERAPROP::iterator it = m_properties.begin(); it != m_properties.end(); it++)
    {
        delete (*it).second;
    }

    m_properties.clear();
}

void
CameraSettings::AddProperty(CameraProperty* property)
{
    m_properties.insert(CAMERAPROPERTY(property->GetId(), property));
}

CAMERAPROP::const_iterator
CameraSettings::begin()
{
    return m_properties.begin();
}

CAMERAPROP::const_iterator
CameraSettings::end()
{
    return m_properties.end();
}

CAMERAPROP::const_iterator
CameraSettings::find(const Property id) const
{
    return m_properties.find(id);
}

int
CameraSettings::size() const
{
    return m_properties.size();
}

CameraProperty*
CameraSettings::GetProperty(Property id) const
{
    CAMERAPROP::const_iterator it = m_properties.find(id);

    return (it != m_properties.end()) ? (*it).second : nullptr;
//    return m_properties[id];// CAMERAPROP::const_iterator it = m_properties.find(id);
//
//    return (it != m_properties.end()) ? (*it).second : nullptr;
}

PropertyValue*
CameraSettings::GetPropertyValue(Property id) const
{
    CameraProperty* p = GetProperty(id);

    if (p->GetInfo()->GetType() != DataType::UNKNOWN)
    {
        return p->GetCurrentValue();
    }
    else
    {
        return nullptr;
    }
}

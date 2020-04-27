#include "pch.h"
#include "Property.h"
#include "PropertyInfo.h"
#include "ResourceLoader.h"

PropertyInfo::PropertyInfo()
{
}

PropertyInfo::PropertyInfo(const PropertyInfo& rhs)
{
    m_id = rhs.m_id;
    m_type = rhs.m_type;
    m_access = rhs.m_access;
    m_sonySpare = rhs.m_sonySpare;
    m_default = new PropertyValue(*rhs.m_default);
    m_form = rhs.m_form;

    if (rhs.m_rangeLo)
    {
        m_rangeLo = new PropertyValue(*rhs.m_rangeLo);
    }

    if (rhs.m_rangeHi)
    {
        m_rangeHi = new PropertyValue(*rhs.m_rangeHi);
    }

    if (rhs.m_rangeStep)
    {
        m_rangeStep = new PropertyValue(*rhs.m_rangeStep);
    }

    if (!rhs.m_enum.empty())
    {
        for (std::list<PropertyValue*>::const_iterator it = rhs.m_enum.begin(); it != rhs.m_enum.end(); it++)
        {
            m_enum.push_back(new PropertyValue(**it));
        }
    }
}

PropertyInfo::~PropertyInfo()
{
    if (m_default)
    {
        delete m_default;
        m_default = nullptr;
    }

    SetRange(nullptr, nullptr, nullptr);
    SetEnumeration(std::list<PropertyValue*>());
}

void
PropertyInfo::SetId(Property id)
{
    m_id = id;
}

Property
PropertyInfo::GetId()
{
    return m_id;
}

void
PropertyInfo::SetType(DataType type)
{
    m_type = type;
}

DataType
PropertyInfo::GetType()
{
    return m_type;
}

void
PropertyInfo::SetAccess(Accessibility access)
{
    m_access = access;
}

Accessibility
PropertyInfo::GetAccess()
{
    return m_access;
}

void
PropertyInfo::SetSonySpare(BYTE sonySpare)
{
    m_sonySpare = sonySpare;
}

BYTE
PropertyInfo::GetSonySpare()
{
    return m_sonySpare;
}

void
PropertyInfo::SetDefault(PropertyValue* def)
{
    m_default = def;
}

PropertyValue*
PropertyInfo::GetDefault()
{
    return m_default;
}

void
PropertyInfo::SetFormMode(FormMode formMode)
{
    m_form = formMode;
}

FormMode
PropertyInfo::GetFormMode()
{
    return m_form;
}

void
PropertyInfo::SetRange(PropertyValue* lo, PropertyValue* hi, PropertyValue* step)
{
    if (m_rangeLo)
    {
        delete m_rangeLo;
        m_rangeLo = nullptr;
    }

    if (m_rangeHi)
    {
        delete m_rangeHi;
        m_rangeHi = nullptr;
    }

    if (m_rangeStep)
    {
        delete m_rangeStep;
        m_rangeStep = nullptr;
    }

    if (lo)
    {
        m_rangeLo = lo;
    }

    if (hi)
    {
        m_rangeHi = hi;
    }

    if (step)
    {
        m_rangeStep = step;
    }
}

PropertyValue*
PropertyInfo::GetRangeLo()
{
    if (m_form == FormMode::RANGE)
    {
        return m_rangeLo;
    }
    else
    {
        throw;
    }
}

PropertyValue*
PropertyInfo::GetRangeHi()
{
    if (m_form == FormMode::RANGE)
    {
        return m_rangeHi;
    }
    else
    {
        throw;
    }
}

PropertyValue*
PropertyInfo::GetRangeStep()
{
    if (m_form == FormMode::RANGE)
    {
        return m_rangeStep;
    }
    else
    {
        throw;
    }
}

void
PropertyInfo::SetEnumeration(std::list<PropertyValue*> values)
{
    for (std::list<PropertyValue*>::iterator it = m_enum.begin(); it != m_enum.end(); it++)
    {
        delete* it;
    }

    m_enum.clear();

    for (std::list<PropertyValue*>::iterator it = values.begin(); it != values.end(); it++)
    {
        m_enum.push_back(*it);
    }
}

std::list<PropertyValue*>
PropertyInfo::GetEnumeration()
{
    return m_enum;
}

std::wstring
PropertyInfo::ToString()
{
    std::wostringstream builder;
    std::wostringstream formBuilder;
    std::wstring form;

    switch (m_form)
    {
    case FormMode::ENUMERATION:
        formBuilder << L"Enum [";

        for (std::list<PropertyValue*>::iterator it = m_enum.begin(); it != m_enum.end(); it++)
        {
            formBuilder << (it == m_enum.begin() ? L"" : L", ") << (*it)->ToString().c_str();
        }
        formBuilder << L"]";
        break;

    case FormMode::RANGE:
        formBuilder << L"Range (" << m_rangeLo->ToString().c_str() << L".." << m_rangeHi->ToString().c_str() << L" / " << m_rangeStep->ToString().c_str() << L")";
        break;

    default:
        formBuilder << L"None";
        break;
    }

    builder << L" " << ResourceLoader::GetString((DWORD)m_id) << L" (id = x" << std::hex << std::setw(4) << std::setfill(L'0') << (DWORD)m_id << ", type = x" << std::hex << std::setw(4) << std::setfill(L'0') << (DWORD)m_type << ", access = x" << std::setw(2) << (DWORD)m_access << ", sonySpare = x" << m_sonySpare << ", form = " << formBuilder.str() << L")" << std::dec;

    return builder.str();
}
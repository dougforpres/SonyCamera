#include "pch.h"
#include "Property.h"
#include "PropertyInfo.h"
#include "ResourceLoader.h"

PropertyInfo::PropertyInfo()
{
}

PropertyInfo::PropertyInfo(const PropertyInfo* rhs)
{
    Copy(rhs);
}

PropertyInfo*
PropertyInfo::operator= (const PropertyInfo* rhs)
{
    Copy(rhs);

    return this;
}

void
PropertyInfo::Copy(const PropertyInfo* rhs)
{
    m_id = rhs->m_id;
    m_type = rhs->m_type;
    m_access = rhs->m_access;
    m_sonySpare = rhs->m_sonySpare;
    SetDefault(new PropertyValue(*rhs->m_default));
    m_form = rhs->m_form;
    SetRange(rhs->m_rangeLo ? new PropertyValue(*rhs->m_rangeLo) : nullptr,
        rhs->m_rangeHi ? new PropertyValue(*rhs->m_rangeHi) : nullptr,
        rhs->m_rangeStep ? new PropertyValue(*rhs->m_rangeStep) : nullptr);

    for (std::list<PropertyValue*>::const_iterator it = m_enum.begin(); it != m_enum.end(); it++)
    {
        delete (*it);
    }

    m_enum.clear();

    for (std::list<PropertyValue*>::const_iterator it = rhs->m_enum.begin(); it != rhs->m_enum.end(); it++)
    {
        {
            m_enum.push_back(new PropertyValue(*(*it)));
        }
    }
}

PropertyInfo::~PropertyInfo()
{
    SetDefault(nullptr);
    SetRange(nullptr, nullptr, nullptr);
    SetEnumeration(std::list<PropertyValue*>());
}

void
PropertyInfo::SetId(Property id)
{
    m_id = id;
}

Property
PropertyInfo::GetId() const
{
    return m_id;
}

void
PropertyInfo::SetType(DataType type)
{
    m_type = type;
}

DataType
PropertyInfo::GetType() const
{
    return m_type;
}

void
PropertyInfo::SetAccess(Accessibility access)
{
    m_access = access;
}

Accessibility
PropertyInfo::GetAccess() const
{
    return m_access;
}

void
PropertyInfo::SetSonySpare(BYTE sonySpare)
{
    m_sonySpare = sonySpare;
}

BYTE
PropertyInfo::GetSonySpare() const
{
    return m_sonySpare;
}

void
PropertyInfo::SetDefault(PropertyValue* def)
{
    if (m_default)
    {
        delete m_default;
    }

    m_default = def;
}

PropertyValue*
PropertyInfo::GetDefault() const
{
    return m_default;
}

void
PropertyInfo::SetFormMode(FormMode formMode)
{
    m_form = formMode;
}

FormMode
PropertyInfo::GetFormMode() const
{
    return m_form;
}

void
PropertyInfo::SetRange(PropertyValue* lo, PropertyValue* hi, PropertyValue* step)
{
    if (m_rangeLo)
    {
        delete m_rangeLo;
    }

    m_rangeLo = lo;

    if (m_rangeHi)
    {
        delete m_rangeHi;
    }

    m_rangeHi = hi;

    if (m_rangeStep)
    {
        delete m_rangeStep;
    }

    m_rangeStep = step;
}

PropertyValue*
PropertyInfo::GetRangeLo() const
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
PropertyInfo::GetRangeHi() const
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
PropertyInfo::GetRangeStep() const
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
    for (std::list<PropertyValue*>::iterator dit = m_enum.begin(); dit != m_enum.end(); dit++)
    {
        delete (*dit);
    }

    m_enum.clear();

    for (std::list<PropertyValue*>::const_iterator it = values.begin(); it != values.end(); it++)
    {
        m_enum.push_back(*it);
    }
}

std::list<PropertyValue*>
PropertyInfo::GetEnumeration() const
{
    return m_enum;
}

std::wstring
PropertyInfo::ToString() const
{
    std::wostringstream builder;
    std::wostringstream formBuilder;
    std::wstring form;

    switch (m_form)
    {
    case FormMode::ENUMERATION:
        formBuilder << L"Enum [";

        for (std::list<PropertyValue*>::const_iterator it = m_enum.begin(); it != m_enum.end(); it++)
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
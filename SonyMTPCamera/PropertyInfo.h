#pragma once
#include "pch.h"
#include <list>
#include "Property.h"
#include "PropertyValue.h"

class PropertyInfo
{
public:
    PropertyInfo();
    PropertyInfo(const PropertyInfo& rhs);
    ~PropertyInfo();

    void SetId(Property id);
    Property GetId();
    void SetType(DataType type);
    DataType GetType();
    void SetAccess(Accessibility access);
    Accessibility GetAccess();
    void SetSonySpare(BYTE sonySpare);
    BYTE GetSonySpare();
    void SetDefault(PropertyValue* def);
    PropertyValue* GetDefault();
    void SetFormMode(FormMode mode);
    FormMode GetFormMode();
    void SetRange(PropertyValue* lo, PropertyValue* hi, PropertyValue* step);
    PropertyValue* GetRangeLo();
    PropertyValue* GetRangeHi();
    PropertyValue* GetRangeStep();
    void SetEnumeration(std::list<PropertyValue*> values);
    std::list<PropertyValue*> GetEnumeration();

    std::wstring ToString();

private:
    Property m_id = Property::Undefined;
    DataType m_type = DataType::UNKNOWN;
    Accessibility m_access = Accessibility::READ_ONLY;
    FormMode m_form = FormMode::NONE;
    BYTE m_sonySpare = 0;

    PropertyValue* m_default = nullptr;
    PropertyValue* m_rangeLo = nullptr;
    PropertyValue* m_rangeHi = nullptr;
    PropertyValue* m_rangeStep = nullptr;
    std::list<PropertyValue*> m_enum;
};


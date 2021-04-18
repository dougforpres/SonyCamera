#pragma once
#include "pch.h"
#include <stdint.h>
#include <list>
#include <unordered_map>
#include "Property.h"
#include "PropertyInfo.h"
#include "PropertyValue.h"

#include "resource.h"

class CameraProperty
{
public:
    CameraProperty();
    CameraProperty(const CameraProperty& rhs);
    virtual ~CameraProperty();

    virtual CameraProperty* Clone();
    void SetId(Property id);
    Property GetId();
    std::wstring GetName();
    virtual bool UpIsBigger();
    std::wstring AsString();
    virtual std::wstring AsString(PropertyValue *in);
    std::wstring ToString();
    DWORD Slurp(BYTE* data, DWORD dataLen);
    virtual bool equals(const CameraProperty& rhs);
    virtual int Compare(const CameraProperty& rhs);
    PropertyInfo* GetInfo();
    void SetInfo(PropertyInfo* info);
    PropertyValue* GetCurrentValue() const;
    void SetCurrentValue(PropertyValue* current);

protected:
    BYTE GetBYTE(BYTE* data, DWORD offset);
    WORD GetWORD(BYTE* data, DWORD offset);
    DWORD GetDWORD(BYTE* data, DWORD offset);
    PropertyValue* ReadData(BYTE* data, DWORD &offset, DataType type);

    PropertyInfo* m_info = nullptr;
    PropertyValue* m_current = nullptr;
};

class ISOProperty : virtual public CameraProperty
{
public:
    ISOProperty();

    CameraProperty* Clone();
    std::wstring AsString(PropertyValue *in);
    int Compare(const CameraProperty& rhs);
};

class ShutterTimingProperty : virtual public CameraProperty
{
public:
    ShutterTimingProperty();

    CameraProperty* Clone();
    bool UpIsBigger();
    std::wstring AsString(PropertyValue* in);
    int Compare(const CameraProperty& rhs);
};

class Div10Property : public CameraProperty
{
public:
    Div10Property();

    std::wstring AsString(PropertyValue* in);
};

class Div100Property : public CameraProperty
{
public:
    Div100Property();

    std::wstring AsString(PropertyValue* in);
};

class Div1000Property : public CameraProperty
{
public:
    Div1000Property();

    std::wstring AsString(PropertyValue* in);
};

class PercentageProperty : public CameraProperty
{
public:
    PercentageProperty();

    std::wstring AsString(PropertyValue* in);
};

class StringLookupProperty : public CameraProperty
{
public:
    StringLookupProperty();

    std::wstring AsString(PropertyValue* in);

protected:
    void AddResource(Property property, WORD value, WORD resid);

    static std::unordered_map<DWORD, WORD> _resourceMap;
};

template< typename T > CameraProperty* pCreate()
{
    return new T();
}

typedef CameraProperty* (*pConstructor)();

class CameraPropertyFactory
{
public:
    CameraPropertyFactory();

    CameraProperty* Create(Property id);

private:
    void AddCreator(Property id, pConstructor c);

    std::unordered_map<Property, pConstructor> m_creators;
};
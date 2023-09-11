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
    CameraProperty(CameraProperty& rhs);
    CameraProperty* operator=(CameraProperty* rhs);
    virtual ~CameraProperty();
    virtual CameraProperty* Clone();

    bool IsSet() const;

    void SetId(Property id);
    Property GetId() const;
    std::wstring GetName() const;
    virtual bool UpIsBigger() const;
    std::wstring AsString() const;
    virtual std::wstring AsString(PropertyValue* in) const;
    std::wstring ToString() const;
    DWORD Slurp(BYTE* data, DWORD dataLen);
    virtual bool equals(CameraProperty* rhs) const;
    virtual int Compare(CameraProperty* rhs) const;
    PropertyInfo* GetInfo() const;
    void SetInfo(PropertyInfo* info);
    PropertyValue* GetCurrentValue() const;
    void SetCurrentValue(PropertyValue* current);
    virtual bool CanNudge() const;

protected:
    BYTE GetBYTE(BYTE* data, DWORD offset) const;
    WORD GetWORD(BYTE* data, DWORD offset) const;
    DWORD GetDWORD(BYTE* data, DWORD offset) const;
    PropertyValue* ReadData(BYTE* data, DWORD &offset, DataType type);

    PropertyInfo* m_info;
    PropertyValue* m_current;
};

typedef std::unique_ptr<CameraProperty> CameraPropertyPointer;

class ISOProperty : virtual public CameraProperty
{
public:
    ISOProperty();

    virtual CameraProperty* Clone();
    virtual std::wstring AsString(PropertyValue* in) const;
    virtual int Compare(CameraProperty* rhs) const;
    virtual bool CanNudge() const;
};

class ShutterTimingProperty : virtual public CameraProperty
{
public:
    ShutterTimingProperty();

    virtual CameraProperty* Clone();
    virtual bool UpIsBigger() const;
    virtual std::wstring AsString(PropertyValue* in) const;
    virtual int Compare(CameraProperty* rhs) const;
    virtual bool CanNudge() const;
};

class Div10Property : public CameraProperty
{
public:
    Div10Property();

    virtual CameraProperty* Clone();
    virtual std::wstring AsString(PropertyValue* in) const;
};

class Div100Property : public CameraProperty
{
public:
    Div100Property();

    virtual CameraProperty* Clone();
    virtual std::wstring AsString(PropertyValue* in) const;
};

class Div1000Property : public CameraProperty
{
public:
    Div1000Property();

    virtual CameraProperty* Clone();
    virtual std::wstring AsString(PropertyValue* in) const;
};

class PercentageProperty : public CameraProperty
{
public:
    PercentageProperty();

    virtual CameraProperty* Clone();
    virtual std::wstring AsString(PropertyValue* in) const;
};

class StringLookupProperty : public CameraProperty
{
public:
    StringLookupProperty();

    virtual CameraProperty* Clone();
    virtual std::wstring AsString(PropertyValue* in) const;

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
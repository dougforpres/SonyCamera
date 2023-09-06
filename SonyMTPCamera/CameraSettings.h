#pragma once
#include "pch.h"
#include <unordered_map>
#include "CameraProperty.h"
#include "MessageReader.h"
#include "PropertyValue.h"

typedef std::unordered_map<Property, CameraProperty*> CAMERAPROP;
typedef std::pair<Property, CameraProperty*> CAMERAPROPERTY;

class CameraSettings : public MessageReader
{
public:
    CameraSettings();
    CameraSettings(Message *message);
    CameraSettings(const CameraSettings& rhs);
    void Copy(const CameraSettings& rhs);
    virtual ~CameraSettings();
    CameraSettings* operator=(CameraSettings* rhs);

    void AddProperty(CameraProperty* property);
    CameraProperty* GetProperty(Property id) const;
    PropertyValue* GetPropertyValue(Property id) const;

    CAMERAPROP::const_iterator begin();
    CAMERAPROP::const_iterator end();
    CAMERAPROP::const_iterator find(Property id) const;

    int size() const;

    //private
    CAMERAPROP m_properties;
};
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
    CameraSettings(Message *message);
    CameraSettings(const CameraSettings& rhs);
    virtual ~CameraSettings();

    void AddProperty(CameraProperty *property);
    std::list<CameraProperty*> GetProperties();
    CameraProperty* GetProperty(Property id);
    PropertyValue* GetPropertyValue(Property id);

    //private
    CAMERAPROP m_properties;
};
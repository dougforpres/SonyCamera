#pragma once
#include "MessageReader.h"
#include "CameraProperty.h"

class CameraSupportedProperties :
    public MessageReader
{
public:
    CameraSupportedProperties(Message* message);

    DWORD Size();
    std::list<Property> Properties();
    std::wstring AsString();

private:
    WORD m_unknown = 0;
    std::list<Property> m_lista;
    std::list<Property> m_listb;
};


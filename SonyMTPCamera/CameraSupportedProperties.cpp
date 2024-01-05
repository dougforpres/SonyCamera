#include "pch.h"
#include "CameraSupportedProperties.h"
#include <list>
#include "ResourceLoader.h"
#include "Logger.h"

CameraSupportedProperties::CameraSupportedProperties()
    : MessageReader(nullptr)
{
#ifdef DEBUG
    LOGTRACE(L"CameraSupportedProperties::CameraSupportedProperties() [this = x%08p]", this);
#endif
}

CameraSupportedProperties::CameraSupportedProperties(Message* message)
    : MessageReader(message)
{
#ifdef DEBUG
    LOGTRACE(L"CameraSupportedProperties::CameraSupportedProperties(Message* = x%08p]) [this = x%08p]", message, this);
#endif
    DWORD offset = 0;

    m_unknown = GetWORD(offset);

#ifdef DEBUG
    LOGTRACE(L"unknown = %d", m_unknown);
#endif
    std::list<WORD> temp;

    temp = GetWORDArray(offset);

    for (auto propId: temp)
    {
        m_lista.push_back((Property)propId);
    }

    temp = GetWORDArray(offset);

    for (auto propId: temp)
    {
        m_listb.push_back((Property)propId);
    }
}

CameraSupportedProperties::CameraSupportedProperties(const CameraSupportedProperties& rhs)
    : MessageReader(nullptr)
{
#ifdef DEBUG
    LOGTRACE(L"CameraSupportedProperties::CameraSupportedProperties(copy = x%08p]) [this = x%08p]", &rhs, this);
#endif
    m_unknown = rhs.m_unknown;
    m_lista = rhs.m_lista;
    m_listb = rhs.m_listb;
}

CameraSupportedProperties::~CameraSupportedProperties()
{
#ifdef DEBUG
    LOGTRACE(L"CameraSupportedProperties::~CameraSupportedProperties() [this = x%08p]", this);
#endif
}

CameraSupportedProperties
CameraSupportedProperties::operator=(const CameraSupportedProperties& rhs)
{
#ifdef DEBUG
    LOGTRACE(L"CameraSupportedProperties::operator=(rhs = x%08p]) [this = x%08p]", &rhs, this);
#endif
    m_unknown = rhs.m_unknown;
    m_lista = rhs.m_lista;
    m_listb = rhs.m_listb;

    return *this;
}

std::wstring
CameraSupportedProperties::AsString()
{
    std::wostringstream builder;

    builder << "lista [" << std::endl;

    std::list<Property>::iterator it;

    for (auto propId: m_lista)
    {
        builder << "    x" << std::hex << std::setw(4) << std::setfill(L'0') << (WORD)propId << std::dec << " (" << ResourceLoader::GetString((DWORD)*it) << ")" << std::endl;
    }

    builder << "]," << std::endl << "listb (buttons?) [" << std::endl;

    for (auto propId: m_listb)
    {
        builder << "    x" << std::hex << std::setw(4) << std::setfill(L'0') << (WORD)propId << std::dec << " (" << ResourceLoader::GetString((DWORD)*it) << ")" << std::endl;
    }

    builder << "]" << std::endl;

    return builder.str();
}

DWORD
CameraSupportedProperties::Size()
{
    return m_lista.size() + m_listb.size();
}

std::list<Property>
CameraSupportedProperties::Properties()
{
    std::list<Property> result = std::list<Property>(m_lista);

    result.merge(m_listb);

    return result;
}
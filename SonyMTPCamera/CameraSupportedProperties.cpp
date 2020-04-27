#include "pch.h"
#include "CameraSupportedProperties.h"
#include <list>
#include "ResourceLoader.h"
#include "Logger.h"

CameraSupportedProperties::CameraSupportedProperties(Message* message)
    : MessageReader(message)
{
    DWORD offset = 0;

    m_unknown = GetWORD(offset);

    LOGTRACE(L"unknown = %d", m_unknown);
    std::list<WORD> temp;

    temp = GetWORDArray(offset);

    for (std::list<WORD>::iterator it = temp.begin(); it != temp.end(); it++)
    {
        m_lista.push_back((Property)*it);
    }

    temp = GetWORDArray(offset);

    for (std::list<WORD>::iterator it = temp.begin(); it != temp.end(); it++)
    {
        m_listb.push_back((Property) * it);
    }
}

std::wstring
CameraSupportedProperties::AsString()
{
    std::wostringstream builder;

    builder << "lista [" << std::endl;

    std::list<Property>::iterator it;

    for (it = m_lista.begin(); it != m_lista.end(); it++)
    {
        builder << "    x" << std::hex << std::setw(4) << std::setfill(L'0') << (WORD)*it << std::dec << " (" << ResourceLoader::GetString((DWORD)*it) << ")" << std::endl;
    }

    builder << "]," << std::endl << "listb [";

    for (it = m_listb.begin(); it != m_listb.end(); it++)
    {
        builder << "    x" << std::hex << std::setw(4) << std::setfill(L'0') << (WORD)*it << std::dec << " (" << ResourceLoader::GetString((DWORD)*it) << ")" << std::endl;
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
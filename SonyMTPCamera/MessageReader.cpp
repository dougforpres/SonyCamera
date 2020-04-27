#include "pch.h"
#include "MessageReader.h"
#include "Logger.h"

MessageReader::MessageReader(Message* message)
{
    if (message)
    {
        m_message = new Message(*message);
    }
}

MessageReader::~MessageReader()
{
    if (m_message)
    {
        delete m_message;
        m_message = nullptr;
    }
}

DWORD
MessageReader::GetDWORD(DWORD &offset)
{
    DWORD result = 0;

    if (m_message)
    {
        if (offset + 3 < m_message->GetDataLen())
        {
            BYTE* data = m_message->GetData();

            result = (DWORD)(data[offset] + ((DWORD)data[offset + 1] << 8) + ((DWORD)data[offset + 2] << 16) + ((DWORD)data[offset + 3] << 24));

            offset += 4;
        }
        else
        {
            LOGERROR(L"Unable to read DWORD value as it would extend beyond end of data");
        }
    }
    else
    {
        LOGERROR(L"Unable to read DWORD as no message object");
    }

    return result;
}

WORD
MessageReader::GetWORD(DWORD &offset)
{
    WORD result = 0;

    if (m_message)
    {
        if (offset + 1 < m_message->GetDataLen())
        {
            BYTE* data = m_message->GetData();

            result = (WORD)(data[offset] + ((WORD)data[offset + 1] << 8));

            offset += 2;
        }
        else
        {
            LOGERROR(L"Unable to read WORD value as it would extend beyond end of data");
        }
    }
    else
    {
        LOGERROR(L"Unable to read WORD as no message object");
    }

    return result;
}

BYTE
MessageReader::GetBYTE(DWORD &offset)
{
    BYTE result = 0;

    if (m_message)
    {
        if (offset < m_message->GetDataLen())
        {
            result = m_message->GetData()[offset++];
        }
        else
        {
            LOGERROR(L"Unable to read BYTE value as it would extend beyond end of data");
        }
    }
    else
    {
        LOGERROR(L"Unable to read BYTE as no message object");
    }

    return result;
}

std::wstring
MessageReader::GetString(DWORD &offset)
{
    std::wstring result;

    if (m_message)
    {
        if (offset < m_message->GetDataLen())
        {
            BYTE* data = m_message->GetData();
            size_t strlen = (size_t)data[offset];

            if (offset + strlen < m_message->GetDataLen())
            {
                offset++;

                if (strlen > 0)
                {
                    bool nullTerminated = (*(WCHAR*)(data + offset + (strlen - 1) * sizeof(WCHAR)) == L'\0');

                    result = std::wstring(((WCHAR*)(data + offset)), strlen - (nullTerminated ? 1 : 0));
                    offset += strlen * sizeof(WCHAR);
                }
            }
            else
            {
                LOGERROR(L"Unable to read string value as it would extend beyond end of data");
            }
        }
    }
    else
    {
        LOGERROR(L"Unable to read String as no message object");
    }

    return result;
}

std::list<BYTE>
MessageReader::GetBYTEArray(DWORD& offset)
{
    std::list<BYTE> result;
    DWORD count = GetDWORD(offset);

    for (DWORD i = 0; i < count; i++)
    {
        result.push_back(GetBYTE(offset));
    }

    return result;
}

std::list<WORD>
MessageReader::GetWORDArray(DWORD& offset)
{
    DWORD count = GetDWORD(offset);
    std::list<WORD> result;

    for (DWORD i = 0; i < count; i++)
    {
        result.push_back(GetWORD(offset));
    }

    return result;
}

std::list<DWORD>
MessageReader::GetDWORDArray(DWORD& offset)
{
    DWORD count = GetDWORD(offset);
    std::list<DWORD> result;

    for (DWORD i = 0; i < count; i++)
    {
        result.push_back(GetDWORD(offset));
    }

    return result;
}

std::list<std::wstring>
MessageReader::GetStringArray(DWORD& offset)
{
    DWORD count = GetDWORD(offset);
    std::list<std::wstring> result;

    for (DWORD i = 0; i < count; i++)
    {
        result.push_back(GetString(offset));
    }

    return result;
}
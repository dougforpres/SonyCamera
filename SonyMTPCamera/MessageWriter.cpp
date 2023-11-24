#include "pch.h"
#include "MessageWriter.h"
#include "Logger.h"

constexpr auto DATALEN_INCREMENT = 128;

MessageWriter::MessageWriter(WORD command)
    : m_command(command)
{

}

MessageWriter::~MessageWriter()
{
    if (m_data)
    {
        delete[] m_data;
        m_data = nullptr;
        m_datalen = 0;
    }
}

Message*
MessageWriter::GetMessageObj() const
{
    Message* result = new Message(m_command);

    // We only want to write data up to the end of the data
    // written, not all that we have (over)-allocated.
    result->SetData(m_data, m_addr);

    return result;
}

void
MessageWriter::Write(const void* data, DWORD datalen)
{
    if (m_addr + datalen > m_datalen)
    {
        DWORD newDatalen = max(m_datalen + datalen, m_datalen + DATALEN_INCREMENT);
        BYTE* newData = (BYTE*)realloc(m_data, newDatalen);

        if (newData)
        {
            m_data = newData;
            m_datalen = newDatalen;
        }
        else
        {
            LOGERROR(L"Unable to allocate memory for message (from %d to %d bytes)", m_datalen, newDatalen);
        }
    }

    if (m_addr + datalen <= m_datalen)
    {
        memcpy(m_data + m_addr, data, datalen);

        m_addr += datalen;
    }
}

void
MessageWriter::WriteBYTE(const BYTE value)
{
    Write(&value, sizeof(BYTE));
}

void
MessageWriter::WriteWORD(const WORD value)
{
    Write(&value, sizeof(WORD));
}

void
MessageWriter::WriteDWORD(const DWORD value)
{
    Write(&value, sizeof(DWORD));
}

void
MessageWriter::WriteString(const std::wstring value)
{
    WriteBYTE((BYTE)value.size());

    if (!value.empty())
    {
        Write(value.c_str(), value.size() * sizeof(WCHAR));
    }
}

void
MessageWriter::WriteBYTEArray(const std::list<BYTE> value)
{
    // Arrays have DWORD header with # of values
    WriteDWORD(value.size());

    for (std::list<BYTE>::const_iterator it = value.begin(); it != value.end(); it++)
    {
        WriteBYTE(*it);
    }
}

void
MessageWriter::WriteWORDArray(const std::list<WORD> value)
{
    // Arrays have DWORD header with # of values
    WriteDWORD(value.size());

    for (std::list<WORD>::const_iterator it = value.begin(); it != value.end(); it++)
    {
        WriteWORD(*it);
    }
}

void
MessageWriter::WriteDWORDArray(const std::list<DWORD> value)
{
    // Arrays have DWORD header with # of values
    WriteDWORD(value.size());

    for (std::list<DWORD>::const_iterator it = value.begin(); it != value.end(); it++)
    {
        WriteDWORD(*it);
    }
}

void
MessageWriter::WriteStringArray(const std::list<std::wstring> value)
{
    // Arrays have DWORD header with # of values
    WriteDWORD(value.size());

    for (std::list<std::wstring>::const_iterator it = value.begin(); it != value.end(); it++)
    {
        WriteString(*it);
    }
}
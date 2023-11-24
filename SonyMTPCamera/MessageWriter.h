#pragma once
#include "pch.h"
#include <list>
#include "Message.h"

// Used by simulated camera to write data as if it had come from USB
class MessageWriter
{
public:
    MessageWriter(WORD command);
    ~MessageWriter();

    void WriteBYTE(const BYTE value);
    void WriteWORD(const WORD value);
    void WriteDWORD(const DWORD value);
    void WriteString(const std::wstring value);

    void WriteBYTEArray(const std::list<BYTE> value);
    void WriteWORDArray(const std::list<WORD> value);
    void WriteDWORDArray(const std::list<DWORD> value);
    void WriteStringArray(const std::list<std::wstring> value);

    void Write(const void* data, DWORD dataSize);

    Message* GetMessageObj() const;

private:
    WORD m_command = 0;
    DWORD m_addr = 0;
    DWORD m_datalen = 0;
    BYTE* m_data = nullptr;
};

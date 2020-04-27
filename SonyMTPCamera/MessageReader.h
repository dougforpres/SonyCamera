#pragma once
#include "pch.h"
#include <set>
#include <list>

#include "Message.h"

class MessageReader
{
public:
    MessageReader(Message *message);
    virtual ~MessageReader();

protected:
    Message* m_message;

    BYTE GetBYTE(DWORD &addr);
    WORD GetWORD(DWORD &addr);
    DWORD GetDWORD(DWORD &addr);
    std::wstring GetString(DWORD &addr);

    std::list<BYTE> GetBYTEArray(DWORD& addr);
    std::list<WORD> GetWORDArray(DWORD& addr);
    std::list<DWORD> GetDWORDArray(DWORD& addr);
    std::list<std::wstring> GetStringArray(DWORD& addr);
};
#pragma once
#include "pch.h"
#include <list>

#include "Property.h"

#define PARAMS_TERMINATOR 0x0000

class Message
{
public:
    /* Constructor */
    Message(WORD command);
    Message(const Message& rhs);

    /* Destructor */
    ~Message();

    std::wstring Dump();

    void Allocate(DWORD size);
    void SetCommand(WORD command);
    WORD GetCommand();
    DWORD GetDataLen();
    BYTE* GetData();

    /* Property Setters */
    bool AddParam(DWORD param);
    bool AddData(BYTE* data, DWORD dataLen);
    bool SetData(BYTE* data, DWORD dataLen);

    std::list<DWORD> GetParams();

    bool IsSuccess();

private:
    WORD m_command = 0;
    std::list<DWORD> m_params;
    BYTE* m_data = nullptr;
    DWORD m_dataLen = 0;
    DWORD m_writeOffset = 0;
};


#pragma once
#include "pch.h"

class Lockable
{
public:
    Lockable();
    ~Lockable();

    bool Lock(DWORD timeout);
    void Unlock();

protected:
    HANDLE m_hLock = INVALID_HANDLE_VALUE;
};


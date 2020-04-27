#pragma once
#include "Lockable.h"

#define DEFAULT_LOCK_WAIT 20000

class Locker
{
public:
    Locker(Lockable* lockable, DWORD timeout = DEFAULT_LOCK_WAIT);
    ~Locker();

private:
    Lockable* m_lockable = nullptr;
};


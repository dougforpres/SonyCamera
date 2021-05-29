#include "pch.h"
#include "Locker.h"

Locker::Locker(Lockable* lockable, DWORD timeout)
    : m_lockable(lockable)
{
    m_lockable->Lock(timeout);
}

Locker::Locker(Lockable& lockable, DWORD timeout)
    : m_lockable(&lockable)
{
    m_lockable->Lock(timeout);
}

Locker::~Locker()
{
    m_lockable->Unlock();
}
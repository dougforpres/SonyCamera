#include "pch.h"
#include "Lockable.h"
#include "Logger.h"
#include "CameraException.h"

Lockable::Lockable()
{
    m_hLock = CreateMutex(nullptr, false, nullptr);
}

Lockable::~Lockable()
{
    CloseHandle(m_hLock);
}

bool
Lockable::Lock(DWORD timeout)
{
    DWORD mutexResult = WaitForSingleObject(m_hLock, timeout);
    bool result = false;

    switch (mutexResult)
    {
    case WAIT_OBJECT_0:
        LOGTRACE(L"(x%08p) Locked", this);
        result = true;
        break;

    case WAIT_TIMEOUT:
        LOGERROR(L"Waiting for Controlled Access Mutex timed-out (%d mS)", timeout);
        throw CameraLockedException(L"Timeout waiting for object to become free");

    case WAIT_ABANDONED:
        LOGERROR(L"Waiting for Controlled Access Mutex abandoned");
        throw CameraException(L"Waiting for object - got WAIT_ABANDONED");

    case WAIT_FAILED:
        LOGERROR(L"Waiting for Controlled Access Mutex failed");
        throw CameraException(L"Waiting for object - got WAIT_FAILED");
    }

    return result;
}

void
Lockable::Unlock()
{
    LOGTRACE(L"(x%08p) Unlocked", this);
    ReleaseMutex(m_hLock);
}
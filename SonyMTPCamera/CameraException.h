#pragma once
#include "pch.h"

class CameraException
{
public:
    CameraException(const std::wstring message);

    const std::wstring GetMessage() const;

protected:
    const std::wstring m_message;
};

class CameraLockedException : public CameraException
{
public:
    CameraLockedException(const std::wstring message);
};

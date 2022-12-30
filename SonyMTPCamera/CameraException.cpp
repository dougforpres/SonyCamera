#include "pch.h"
#include "CameraException.h"
#include "Logger.h"

CameraException::CameraException(const std::wstring message)
    : m_message(message)
{
    LOGWARN(L"new CameraException(%s)", message.c_str());
}

const std::wstring
CameraException::GetMessage() const
{
    return m_message;
}

CameraLockedException::CameraLockedException(const std::wstring message)
    : CameraException(message)
{

}
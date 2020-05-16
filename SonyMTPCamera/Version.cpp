#include "pch.h"
#include "Version.h"

Version::Version(std::wstring productName, std::wstring version)
    : m_productName(productName),
    m_version(version)
{
}

std::wstring
Version::GetProductName()
{
    return m_productName;
}

std::wstring
Version::GetVersion()
{
    return m_version;
}
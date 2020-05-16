#pragma once
#include "pch.h"

class Version
{
public:
    Version(std::wstring productName, std::wstring version);

    std::wstring GetProductName();
    std::wstring GetVersion();

private:
    std::wstring m_productName;
    std::wstring m_version;
};


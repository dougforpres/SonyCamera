#pragma once
#include "pch.h"
#include <unordered_map>
#include "Version.h"

class ResourceLoader
{
public:
    static const std::wstring GetString(DWORD id);
    static Version* GetVersion();

protected:
    static std::unordered_map<DWORD, std::wstring> s_cache;

private:
    static const DWORD s_max_string_resource_length = 128;
};


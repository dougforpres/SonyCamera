#pragma once
#include "pch.h"
#include <unordered_map>

class ResourceLoader
{
public:
    static std::wstring GetString(DWORD id);

protected:
    static std::unordered_map<DWORD, std::wstring> s_cache;

private:
    static const DWORD s_max_string_resource_length = 128;
};


#include "pch.h"
#include "ResourceLoader.h"
#include "Logger.h"

extern HINSTANCE dllInstance;

std::unordered_map<DWORD, std::wstring> ResourceLoader::s_cache;

std::wstring
ResourceLoader::GetString(DWORD id)
{
    std::unordered_map<DWORD, std::wstring>::iterator it = ResourceLoader::s_cache.find(id);
    std::wstring result;

    if (it != ResourceLoader::s_cache.end())
    {
        result = (*it).second;
    }
    else
    {
        // Need to look up
        WCHAR* temp = new WCHAR[ResourceLoader::s_max_string_resource_length];

        int rc = LoadString(dllInstance, id, temp, ResourceLoader::s_max_string_resource_length);

        if (rc == 0)
        {
            // Not Found
            LOGTRACE(L"String resource %d not found :(", id);
        }
        else if (rc >= ResourceLoader::s_max_string_resource_length)
        {
            LOGTRACE(L"String resource %d too long, truncating");
            // String too long to fit
            result = std::wstring(temp, ResourceLoader::s_max_string_resource_length) + L"...";
        }
        else
        {
            result = std::wstring(temp);
        }

        ResourceLoader::s_cache.insert(std::pair<DWORD, std::wstring>(id, result));
        delete[] temp;
    }

    return result;
}

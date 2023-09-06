#include "pch.h"
#include "ResourceLoader.h"
#include "Logger.h"

extern HINSTANCE dllInstance;

std::unordered_map<DWORD, std::wstring> ResourceLoader::s_cache;

const std::wstring
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

Version *
ResourceLoader::GetVersion()
{
    // get the filename of the executable containing the version resource
    TCHAR szFilename[MAX_PATH + 1] = { 0 };

    if (GetModuleFileName(dllInstance, szFilename, MAX_PATH) == 0)
    {
        LOGERROR(L"GetModuleFileName failed with error %d", GetLastError());

        return nullptr;
    }

    // allocate a block of memory for the version info
    DWORD dummy;
    DWORD dwSize = GetFileVersionInfoSize(szFilename, &dummy);

    if (dwSize == 0)
    {
        LOGERROR(L"GetFileVersionInfoSize failed with error %d", GetLastError());

        return nullptr;
    }

    std::vector<BYTE> data(dwSize);

    // load the version info
    if (!GetFileVersionInfo(szFilename, NULL, dwSize, &data[0]))
    {
        LOGERROR(L"GetFileVersionInfo failed with error %d", GetLastError());

        return nullptr;
    }

    // get the name and version strings
    LPVOID pvProductName = NULL;
    unsigned int iProductNameLen = 0;
    LPVOID pvProductVersion = NULL;
    unsigned int iProductVersionLen = 0;

    // replace "040904e4" with the language ID of your resources
    if (!VerQueryValue(&data[0], L"\\StringFileInfo\\040904b0\\ProductName", &pvProductName, &iProductNameLen) ||
        !VerQueryValue(&data[0], L"\\StringFileInfo\\040904b0\\ProductVersion", &pvProductVersion, &iProductVersionLen))
    {
        LOGERROR(L"Can't obtain ProductName and ProductVersion from resources");

        return nullptr;
    }

    return new Version((LPWSTR)pvProductName, (LPWSTR)pvProductVersion);
}
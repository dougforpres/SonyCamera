#include "pch.h"
#include "Registry.h"

Registry::Registry(std::wstring root)
    : m_root(root)
{

}

bool
Registry::Open()
{
    HKEY key;

    m_openCount++;

    if (m_openCount == 1)
    {
        if (RegCreateKeyEx(HKEY_CURRENT_USER, m_root.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &key, NULL) == ERROR_SUCCESS)
        {
            m_hk = key;
        }
        else
        {
            m_openCount--;
        }
    }

    return m_openCount > 0;
}

void
Registry::Close()
{
    if (m_openCount > 0)
    {
        m_openCount--;

        if (m_openCount == 0)
        {
            RegCloseKey(m_hk);
            m_hk = nullptr;
        }
    }
}

std::wstring
Registry::GetString(std::wstring path, std::wstring value, std::wstring def)
{
    std::wstring result;
    DWORD dataSize = 0;

    RegGetValue(m_hk, path.c_str(), value.c_str(), RRF_RT_REG_SZ, nullptr, nullptr, &dataSize);

    if (dataSize)
    {
        LPWSTR str = (LPWSTR)new BYTE[dataSize];
        LSTATUS s = RegGetValue(m_hk, path.c_str(), value.c_str(), RRF_RT_REG_SZ, nullptr, str, &dataSize);

        result = std::wstring(str, dataSize / sizeof(wchar_t) - 1);
        delete[] (BYTE*)str;
    }

    return result.empty() ? def : result;
}

DWORD
Registry::GetDWORD(std::wstring path, std::wstring value, DWORD def)
{
    DWORD result = 0;
    DWORD dataSize = sizeof(DWORD);

    LSTATUS regresult = RegGetValue(m_hk, path.c_str(), value.c_str(), RRF_RT_REG_DWORD, nullptr, &result, &dataSize);

    return regresult == ERROR_SUCCESS ? result : def;
}

double
Registry::GetDouble(std::wstring path, std::wstring value, double def)
{
    double result = def;
    std::wstring strValue = GetString(path, value, L"");

    if (!strValue.empty())
    {
        result = _wtof(strValue.c_str());
    }

    return result;
}

bool
Registry::DoesKeyExist(std::wstring path)
{
    HKEY hk = nullptr;
    LSTATUS hr = RegOpenKeyEx(m_hk, path.c_str(), 0, KEY_READ, &hk);

    if (hr == ERROR_SUCCESS)
    {
        RegCloseKey(hk);

        return true;
    }

    return false;
}

std::list<std::wstring>
Registry::GetChildKeys(std::wstring path)
{
    std::list<std::wstring> results;

    HKEY hk = nullptr;
    LSTATUS hr = RegOpenKeyEx(m_hk, path.c_str(), 0, KEY_READ | KEY_QUERY_VALUE, &hk);

    if (hr == ERROR_SUCCESS)
    {
        DWORD index = 0;
        DWORD size = 0;

        if (RegQueryInfoKey(hk, nullptr, nullptr, nullptr, nullptr, &size, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS)
        {
            size += 1;
            wchar_t* name = new wchar_t[size];

            do
            {
                DWORD sizeIn = size;

                hr = RegEnumKeyEx(hk, index, name, &sizeIn, nullptr, nullptr, nullptr, nullptr);

                if (hr == ERROR_SUCCESS)
                {
                    results.push_back(name);
                    index++;
                }
            } while (hr == ERROR_SUCCESS);

            delete[] name;
        }

        RegCloseKey(hk);
    }

    return results;
}

HKEY
Registry::CreateKey(std::wstring path)
{
    HKEY hk = nullptr;

    RegCreateKeyEx(m_hk, path.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &hk, NULL);

    return hk;
}

void
Registry::CloseKey(HKEY hk)
{
    RegCloseKey(hk);
}

bool
Registry::SetString(HKEY hk, std::wstring value, std::wstring setting)
{
    return RegSetValueEx(hk, value.c_str(), 0, REG_SZ, (BYTE*)setting.c_str(), setting.length() * sizeof(wchar_t)) == ERROR_SUCCESS;
}

bool
Registry::SetString(std::wstring path, std::wstring value, std::wstring setting)
{
    HKEY hk = CreateKey(path);

    SetString(hk, value, setting);

    CloseKey(hk);

    return true;
}

bool
Registry::SetDWORD(HKEY hk, std::wstring value, DWORD setting)
{
    return RegSetValueEx(hk, value.c_str(), 0, REG_DWORD, (BYTE*)&setting, sizeof(DWORD)) == ERROR_SUCCESS;
}

bool
Registry::SetDWORD(std::wstring path, std::wstring value, DWORD setting)
{
    HKEY hk = CreateKey(path);

    SetDWORD(hk, value, setting);

    CloseKey(hk);

    return true;
}

bool
Registry::SetDouble(std::wstring path, std::wstring value, double setting)
{
    std::wostringstream builder;

    builder << setting;

    std::wstring s = builder.str();

    return SetString(path, value, s);
}

bool
Registry::SetStringDefault (std::wstring path, std::wstring value, std::wstring setting)
{
    DWORD dataSize = 0;

    if (RegGetValue(m_hk, path.c_str(), value.c_str(), RRF_RT_REG_SZ, nullptr, nullptr, &dataSize) != ERROR_SUCCESS)
    {
        SetString(path, value, setting);

        return true;
    }

    return false;
}

bool
Registry::SetDWORDDefault(std::wstring path, std::wstring value, DWORD setting)
{
    DWORD dataSize = sizeof(DWORD);

    if (RegGetValue(m_hk, path.c_str(), value.c_str(), RRF_RT_REG_DWORD, nullptr, nullptr, &dataSize) != ERROR_SUCCESS)
    {
        SetDWORD(path, value, setting);

        return true;
    }

    return false;
}

bool
Registry::SetDoubleDefault(std::wstring path, std::wstring value, double setting)
{
    DWORD dataSize = 0;

    if (RegGetValue(m_hk, path.c_str(), value.c_str(), RRF_RT_REG_SZ, nullptr, nullptr, &dataSize) != ERROR_SUCCESS)
    {
        std::wostringstream builder;

        builder << setting;

        SetString(path, value, builder.str());

        return true;
    }

    return false;
}
#pragma once
#include "pch.h"

class Registry
{
public:
    Registry(std::wstring root);

    bool Open();
    void Close();
    std::wstring GetString(std::wstring path, std::wstring value, std::wstring def);
    DWORD GetDWORD(std::wstring path, std::wstring value, DWORD def);
    double GetDouble(std::wstring path, std::wstring value, double def);

    HKEY CreateKey(std::wstring path);
    bool DoesKeyExist(std::wstring path);
    void CloseKey(HKEY hk);
    bool SetString(HKEY hk, std::wstring value, std::wstring setting);
    bool SetString(std::wstring path, std::wstring value, std::wstring setting);
    bool SetDWORD(HKEY hk, std::wstring value, DWORD setting);
    bool SetDWORD(std::wstring path, std::wstring value, DWORD setting);
    bool SetDouble(HKEY hk, std::wstring value, double setting);
    bool SetDouble(std::wstring path, std::wstring value, double setting);

    bool SetStringDefault(std::wstring path, std::wstring value, std::wstring setting);
    bool SetDWORDDefault(std::wstring path, std::wstring value, DWORD setting);
    bool SetDoubleDefault(std::wstring path, std::wstring value, double setting);

private:
    HKEY m_hk = nullptr;
    std::wstring m_root;
    DWORD m_openCount = 0;
};

extern Registry registry;
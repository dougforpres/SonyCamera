#include "pch.h"
#include "Utils.h"

std::wstring
DumpBytes(BYTE* data, DWORD dataLen, DWORD maxLen)
{
    std::wostringstream result;

    result << L"BYTE[len = x" << std::hex << std::setw(2) << std::setfill(L'0') << dataLen << L"] = [";

    DWORD i, j, k;
    DWORD dl = dataLen <= maxLen ? dataLen : maxLen;

    for (i = 0; i < dl; i += 16)
    {
        result << "\n" << std::hex << std::setw(4) << std::setfill(L'0') << i << ": ";

        std::wostringstream chars;

        for (j = 0; j < 16 && i + j < dl; j++)
        {
            BYTE d = data[i + j];

            result << std::hex << std::setw(2) << std::setfill(L'0') << (WORD)d << " ";

            if (d >= 32 && d <= 127)
            {
                chars << (char)d;
            }
            else
            {
                chars << '.';
            }
        }

        for (k = j; k < 16; k++)
        {
            result << "   ";
        }

        result << chars.str();
    }

    if (dl < dataLen)
    {
        result << L"\n...x " << std::hex << std::setw(2) << std::setfill(L'0') << dl << L" / x" << std::hex << std::setw(2) << std::setfill(L'0') << dataLen << L" shown";
    }

    result << "\n]";

    return result.str();
}
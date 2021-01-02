#pragma once
#include <string>
#include "minwindef.h"

std::wstring
DumpBytes(BYTE* data, DWORD dataLen, DWORD maxLen = 0x200);
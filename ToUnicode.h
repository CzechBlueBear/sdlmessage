#pragma once
#include <vector>
#include <wchar.h>

bool StringToUnicode(const char* text, std::vector<wchar_t> &result);

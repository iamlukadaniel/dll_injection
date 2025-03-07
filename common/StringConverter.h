#ifndef STRINGCONVERTER_H
#define STRINGCONVERTER_H

#include <string>
#include <codecvt>

std::wstring s2ws(const std::string& str);

std::string ws2s(const std::wstring& wstr);

#endif
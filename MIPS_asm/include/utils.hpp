#pragma once
#include <format>
#include <string>
#include <string_view>

void debugInfo(const char* pattern, ...);
void debugError(const char* pattern, ...);

template <typename... Args>
std::string dynFormat(std::string_view fmt, Args&&... args) {
    return std::vformat(fmt, std::make_format_args(args...));
}

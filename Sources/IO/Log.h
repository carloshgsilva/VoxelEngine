#pragma once

#define FMT_USE_WINDOWS_H 0
#include <fmt/core.h>
#include <fmt/format.h>

#define TC_red    "\033[0;31m"
#define TC_RED    "\033[1;31m"
#define TC_blue   "\033[0;34m"
#define TC_BLUE   "\033[1;34m"
#define TC_cyan   "\033[0;36m"
#define TC_CYAN   "\033[1;36m"
#define TC_green  "\033[0;32m"
#define TC_GREEN  "\033[1;32m"
#define TC_yellow "\033[0;33m"
#define TC_YELLOW "\033[1;33m"
#define TC_NC     "\033[0m"

class Log {

public:
    template<typename... T>
    static inline void info(fmt::format_string<T...> fmt, T&&...args) {
        printf("[" TC_BLUE "info" TC_NC "] %s\n", fmt::vformat(fmt, fmt::make_format_args(args...)).c_str());
    }

    template<typename... T>
    static inline void warn(fmt::format_string<T...> fmt, T&&...args) {
        printf("[" TC_YELLOW "warn" TC_NC "] %s\n", fmt::vformat(fmt, fmt::make_format_args(args...)).c_str());
    }

    template<typename... T>
    static inline void error(fmt::format_string<T...> fmt, T&&... args) {
        printf("[" TC_RED "error" TC_NC "] %s\n", fmt::vformat(fmt, fmt::make_format_args(args...)).c_str());
    }
};
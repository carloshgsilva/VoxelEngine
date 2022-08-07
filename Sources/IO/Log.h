#pragma once

#ifndef NOMINMAX
    #define NOMINMAX // prevent windows redefining min/max
#endif

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#undef LoadImage
#undef near
#undef far

#include <fmt/core.h>
#include <fmt/format.h>
#include <iostream>

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

    template<typename FormatString, typename... Args>
    static inline void info(const FormatString& fmt, Args&&...args) {
        std::cout << "[" TC_BLUE "info" TC_NC "] " << fmt::format(fmt, std::forward<Args>(args)...) << std::endl;
    }

    template<typename FormatString, typename... Args>
    static inline void warn(const FormatString& fmt, Args&&...args) {
        std::cout << "[" TC_YELLOW "warn" TC_NC "] " << fmt::format(fmt, std::forward<Args>(args)...) << std::endl;
    }

    template<typename FormatString, typename... Args>
    static inline void error(const FormatString& fmt, Args&&...args) {
        std::cout << "[" TC_RED "error" TC_NC "] " << fmt::format(fmt, std::forward<Args>(args)...) << std::endl;
    }

    template<typename T> static inline void info(const T& msg) { info("{}", msg); }
    template<typename T> static inline void warn(const T& msg) { warn("{}", msg); }
    template<typename T> static inline void error(const T& msg) { error("{}", msg); }
};
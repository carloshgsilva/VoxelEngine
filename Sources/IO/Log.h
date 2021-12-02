#pragma once

#include "spdlog/spdlog.h"

namespace Log2 = spdlog;


class Log {

	static spdlog::logger* I() {
        return spdlog::default_logger_raw();
	}
public:

    static const spdlog::level::level_enum L0_Trace = spdlog::level::level_enum::trace;
    static const spdlog::level::level_enum L1_Debug = spdlog::level::level_enum::debug;
    static const spdlog::level::level_enum L2_Info = spdlog::level::level_enum::info;
    static const spdlog::level::level_enum L3_Warn = spdlog::level::level_enum::warn;
    static const spdlog::level::level_enum L4_Error = spdlog::level::level_enum::err;
    static const spdlog::level::level_enum L5_Critical = spdlog::level::level_enum::critical;

    static void level(spdlog::level::level_enum l) {
        I()->set_level(spdlog::level::level_enum::trace);
    }

    //Raw
    template<typename T>
    static inline void trace(const T& msg)
    {
        I()->trace(msg);
    }

    template<typename T>
    static inline void debug(const T& msg)
    {
        I()->debug(msg);
    }

    template<typename T>
    static inline void info(const T& msg)
    {
        I()->info(msg);
    }

    template<typename T>
    static inline void warn(const T& msg)
    {
        I()->warn(msg);
    }

    template<typename T>
    static inline void error(const T& msg)
    {
        I()->error(msg);
    }

    template<typename T>
    static inline void critical(const T& msg)
    {
        I()->critical(msg);
    }

    //Formatted
    template<typename FormatString, typename... Args>
    static inline void trace(const FormatString& fmt, Args&&...args)
    {
        I()->trace(fmt, std::forward<Args>(args)...);
    }

    template<typename FormatString, typename... Args>
    static inline void debug(const FormatString& fmt, Args&&...args)
    {
        I()->debug(fmt, std::forward<Args>(args)...);
    }

    template<typename FormatString, typename... Args>
    static inline void info(const FormatString& fmt, Args&&...args)
    {
        I()->info(fmt, std::forward<Args>(args)...);
    }

    template<typename FormatString, typename... Args>
    static inline void warn(const FormatString& fmt, Args&&...args)
    {
        I()->warn(fmt, std::forward<Args>(args)...);
    }

    template<typename FormatString, typename... Args>
    static inline void error(const FormatString& fmt, Args&&...args)
    {
        I()->error(fmt, std::forward<Args>(args)...);
    }

    template<typename FormatString, typename... Args>
    static inline void critical(const FormatString& fmt, Args&&...args)
    {
        I()->critical(fmt, std::forward<Args>(args)...);
    }


};
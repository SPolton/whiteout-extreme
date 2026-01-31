#pragma once

//------------------------------------------------------------------------------
// Some simple logging helpers.
//
// Uses the excellent https://fmt.dev/ library to allow for simple,
// python like formatting.
//
// Example: logger::debug("Elapsed time: {0:.2f} seconds", 1.23);
//		    logger::warning("Elapsed time: {0:.2f} seconds", 1.23);
//		    logger::error("Elapsed time: {0:.2f} seconds", 1.23);
//
// This code isn't intented for your review. Of course, if you feel like it, dive
// right in.
//------------------------------------------------------------------------------

#include <fmt/format.h>
#include <vivid/vivid.h>

/// Options:
/// - trace (grey)
/// - debug (blue)
/// - info (green)
/// - warning (yellow)
/// - error (red)
/// - fatal (light red)
///
/// Usage example:
/// - logger::debug("Elapsed time: {0:.2f}", 1.23);
namespace logger {
    namespace ansi = vivid::ansi;

    /// Logging levels
    enum class Level {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        OFF
    };

#ifndef NDEBUG
    static inline Level current_level = Level::DEBUG;
#else
    static inline Level current_level = Level::INFO;
#endif

    inline bool is_level_enabled(Level level) {
        return level >= current_level;
    }

    template <typename S1, typename S2, typename S3, typename... Args>
    void _log(const S1& prefix, const S2& c, const S3& format_str, Args&&... args) {
        fmt::print(
            "{}[{}]{}: {}\n",
            c,
            prefix,
            ansi::reset,
            fmt::vformat(format_str, fmt::make_format_args(std::forward<Args>(args)...))
        );
    }

    template <typename S, typename... Args>
    void trace(const S& format_str, Args&&... args) {
        if (is_level_enabled(Level::TRACE)) {
            _log("TRACE", ansi::subtleText, format_str, args...);
        }
    }

    template <typename S, typename... Args>
    void debug(const S& format_str, Args&&... args) {
        if (is_level_enabled(Level::DEBUG)) {
            _log("DEBUG", ansi::blue, format_str, args...);
        }
    }

    template <typename S, typename... Args>
    void info(const S& format_str, Args&&... args) {
        if (is_level_enabled(Level::INFO)) {
            _log("INFO ", ansi::green, format_str, args...);
        }
    }

    template <typename S, typename... Args>
    void warning(const S& format_str, Args&&... args) {
        if (is_level_enabled(Level::WARN)) {
            _log("WARN ", ansi::yellow, format_str, args...);
        }
    }
    template <typename S, typename... Args>
    void warn(const S& format_str, Args&&... args) {
        if (is_level_enabled(Level::WARN)) {
            _log("WARN ", ansi::yellow, format_str, args...);
        }
    }

    template <typename S, typename... Args>
    void error(const S& format_str, Args&&... args) {
        if (is_level_enabled(Level::ERROR)) {
            _log("ERROR", ansi::red, format_str, args...);
        }
    }

    template <typename S, typename... Args>
    void fatal(const S& format_str, Args&&... args) {
        if (is_level_enabled(Level::FATAL)) {
            _log("FATAL", ansi::lightRed, format_str, args...);
        }
    }

    inline void test_logger_messages() {
        trace("This is {:.2f} trace message", 1.0);
        debug("This is {:.2f} debug message", 2.0);
        info("This is {:.2f} info message", 3.0);
        warning("This is {:.2f} warning message", 4.0);
        error("This is {:.2f} error message", 5.0);
        fatal("This is {:.2f} fatal message", 6.0);
    }

} // namespace logger

/*  =========================================================================
    fty_log - Log management

    Copyright (C) 2014 - 2020 Eaton

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
    =========================================================================
 */

#ifndef FTY_LOG_H_INCLUDED
#define FTY_LOG_H_INCLUDED

#include <string.h>

// Trick to avoid conflict with CXXTOOLS logger, currently the BIOS code
// prefers OUR logger macros
#if defined(LOG_CXXTOOLS_H) || defined(CXXTOOLS_LOG_CXXTOOLS_H)
#undef log_trace
#undef log_error
#undef log_debug
#undef log_info
#undef log_fatal
#undef log_warn
#undef log_define
#define log_define(category)
#else
#define LOG_CXXTOOLS_H
#define CXXTOOLS_LOG_CXXTOOLS_H
#endif

#ifdef __cplusplus
#include <log4cplus/configurator.h>
#endif
// Macro for logging

#ifdef __cplusplus

#define log_macro(level, ftylogger, ...)                                                                               \
    do {                                                                                                               \
        ftylogger->insertLog((level), __FILE__, __LINE__, __func__, __VA_ARGS__);                                      \
    } while (0)
#else
#define log_macro(level, ftylogger, ...)                                                                               \
    do {                                                                                                               \
        ftylog_insertLog(ftylogger, (level), __FILE__, __LINE__, __func__, __VA_ARGS__);                               \
    } while (0)
#endif

// Logging with explicit logger
/* Prints message with TRACE level. 0 <=> log4cplus::TRACE_LOG_LEVEL */
#define log_trace_log(ftylogger, ...) log_macro(0, ftylogger, __VA_ARGS__)

/* Prints message with DEBUG level. 10000 <=> log4cplus::DEBUG_LOG_LEVEL */
#define log_debug_log(ftylogger, ...) log_macro(10000, ftylogger, __VA_ARGS__)

/* Prints message with INFO level. 20000 <=> log4cplus::INFO_LOG_LEVEL */
#define log_info_log(ftylogger, ...) log_macro(20000, ftylogger, __VA_ARGS__)

/* Prints message with WARNING level 30000 <=> log4cplus::WARN_LOG_LEVEL*/
#define log_warning_log(ftylogger, ...) log_macro(30000, ftylogger, __VA_ARGS__)

/* Prints message with ERROR level 40000 <=> log4cplus::ERROR_LOG_LEVEL*/
#define log_error_log(ftylogger, ...) log_macro(40000, ftylogger, __VA_ARGS__)

/* Prints message with FATAL level. 50000 <=> log4cplus::FATAL_LOG_LEVEL*/
#define log_fatal_log(ftylogger, ...) log_macro(50000, ftylogger, __VA_ARGS__)

// Logging with default logger
/* Prints message with TRACE level. 0 <=> log4cplus::TRACE_LOG_LEVEL */
#define log_trace(...) log_macro(0, ftylog_getInstance(), __VA_ARGS__)

/* Prints message with DEBUG level. 10000 <=> log4cplus::DEBUG_LOG_LEVEL */
#define log_debug(...) log_macro(10000, ftylog_getInstance(), __VA_ARGS__)

/* Prints message with INFO level. 20000 <=> log4cplus::INFO_LOG_LEVEL */
#define log_info(...) log_macro(20000, ftylog_getInstance(), __VA_ARGS__)

/* Prints message with WARNING level 30000 <=> log4cplus::WARN_LOG_LEVEL*/
#define log_warning(...) log_macro(30000, ftylog_getInstance(), __VA_ARGS__)

/* Prints message with ERROR level 40000 <=> log4cplus::ERROR_LOG_LEVEL*/
#define log_error(...) log_macro(40000, ftylog_getInstance(), __VA_ARGS__)

/* Prints message with FATAL level. 50000 <=> log4cplus::FATAL_LOG_LEVEL*/
#define log_fatal(...) log_macro(50000, ftylog_getInstance(), __VA_ARGS__)

#define LOG_START log_debug("start")

#define LOG_END log_debug("end::normal")

#define LOG_END_ABNORMAL(exp) log_error("end::abnormal with %s", (exp).what())

// Default layout pattern
#define LOGPATTERN "%c [%t] -%-5p- %M (%l) %m%n"

//  @interface
#ifdef __cplusplus
#include <fmt/format.h>
// Log class

#define logError(...)\
    fmtlog(log4cplus::ERROR_LOG_LEVEL, __VA_ARGS__)

#define logDebug(...)\
    fmtlog(log4cplus::DEBUG_LOG_LEVEL, __VA_ARGS__)

#define logInfo(...)\
    fmtlog(log4cplus::INFO_LOG_LEVEL, __VA_ARGS__)

#define logWarn(...)\
    fmtlog(log4cplus::WARN_LOG_LEVEL, __VA_ARGS__)

#define logFatal(...)\
    fmtlog(log4cplus::FATAL_LOG_LEVEL, __VA_ARGS__)

#define logTrace(...)\
    fmtlog(log4cplus::TRACE_LOG_LEVEL, __VA_ARGS__)

#define fmtlog(level, ...) \
    ftylog_getInstance()->insertLog(level, __FILE__, __LINE__, __func__, fty::logger::format(__VA_ARGS__).c_str())

namespace fty::logger {

inline std::string format(const std::string& str)
{
    return str;
}

template<typename... Args>
inline std::string format(const std::string& str, const Args&... args)
{
    return fmt::format(str, args...);
}

}

class Ftylog
{
private:
    // Name of the agent/component
    std::string _agentName;
    // Path to the log configuration file if any
    std::string _configFile;
    // Layout pattern for logs
    std::string _layoutPattern;
    // log4cplus object to print logs
    log4cplus::Logger _logger;
    // Thread for watching modification of the log configuration file if any
    log4cplus::ConfigureAndWatchThread* _watchConfigFile;

    // Initialize the Ftylog object
    void init(std::string _component, std::string logConfigFile = "");

    // Return true if level is included in the logger level
    bool isLogLevel(log4cplus::LogLevel level);

    // Set the console appender
    void setConsoleAppender();

    // Remove instances of log4cplus::ConsoleAppender from a given logger
    static void removeConsoleAppenders(log4cplus::Logger logger);

    // Set log level with level from syslog.h
    // for debug, info, warning, error, fatal or off
    bool setLogLevelFromEnvDefinite(const std::string& level);

    // Set logging subsystem initialization messages' log level with a valid
    // string; log level set to OFF level otherwise
    // used only in loadAppenders() to control its spamminess
    void setLogInitLevelFromEnv(const std::string& level);

    // Set default log level from envvar with a valid string;
    // log level set to trace level otherwise
    void setLogLevelFromEnv(const std::string& level);

    // Set needed variables from env
    void setLogLevelFromEnv();
    void setPatternFromEnv();

    // Load appenders from the config file
    // or set the default console appender if no can't load from the config file
    void loadAppenders();

public:
    // Constructor/destructor
    Ftylog(std::string _component, std::string logConfigFile = "");
    Ftylog();
    ~Ftylog();

    // getter
    std::string getAgentName();

    // setter
    // Set the path to the log config file
    // And try to load it
    void setConfigFile(std::string file);

    // Change properties of the Ftylog object
    void change(std::string name, std::string configFile);

    // Set the logger to a specific log level
    void setLogLevelTrace();
    void setLogLevelDebug();
    void setLogLevelInfo();
    void setLogLevelWarning();
    void setLogLevelError();
    void setLogLevelFatal();
    void setLogLevelOff();

    // Check the log level
    bool isLogTrace();
    bool isLogDebug();
    bool isLogInfo();
    bool isLogWarning();
    bool isLogError();
    bool isLogFatal();
    bool isLogOff();

    /*! \brief insertLog
      An internal logging function, use specific log_error, log_debug  macros!
      \param level - level for message, see \ref log4cplus::logLevel
      \param file - name of file issued print, usually content of __FILE__ macro
      \param line - number of line, usually content of __LINE__ macro
      \param func - name of function issued log, usually content of __func__ macro
      \param format - printf-like format string
     */
    void insertLog(log4cplus::LogLevel level, const char* file, int line, const char* func, const char* format, ...);

    void insertLog(
        log4cplus::LogLevel level, const char* file, int line, const char* func, const char* format, va_list args);

    // Load a specific appender if verbose mode is set to true :
    // -Save the logger logging level and set it to TRACE logging level
    // -Remove an already existing ConsoleAppender
    // -For the other appender, set the threshold parameter to the old logger log level
    //    if no loglevel defined for this appender
    // -Add a new console appender
    void setVerboseMode();
    void setVeboseMode() { setVerboseMode(); } // legacy misnomer

    /**
     * Set a context for a mapped diagnostic context (MDC)
     * @param contextParam The context params mapped.
     */
    static void setContext(const std::map<std::string, std::string>& contextParam);

    /**
     * Clear the mapped diagnostic context.
     */
    static void clearContext();
};

// singleton for logger managment
class ManageFtyLog
{
private:
    // Avoid use of the following procedures/functions
    ManageFtyLog(){}
    ~ManageFtyLog(){}
    ManageFtyLog(const ManageFtyLog&) = delete;
    ManageFtyLog& operator=(const ManageFtyLog&) = delete;
    static Ftylog _ftylogdefault;

public:
    // Return the Ftylog obect from the instance
    static Ftylog* getInstanceFtylog();
    // Create or replace the Ftylog object in the instance using a new Ftylog object
    static void setInstanceFtylog(std::string componentName, std::string logConfigFile = "");
};

#else
typedef struct Ftylog Ftylog;
#endif

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////
// wrapper to use for C code only
///////////////////////////////

// Default layout configuration file.
#define FTY_COMMON_LOGGING_DEFAULT_CFG "/etc/fty/ftylog.cfg"

// Constructor
Ftylog* ftylog_new(const char* component, const char* logConfigFile);
// destructor
void ftylog_delete(Ftylog* log);

// setter
void ftylog_setConfigFile(Ftylog* log, const char* file);

// Set the logger to a specific log level
void ftylog_setLogLevelTrace(Ftylog* log);
void ftylog_setLogLevelDebug(Ftylog* log);
void ftylog_setLogLevelInfo(Ftylog* log);
void ftylog_setLogLevelWarning(Ftylog* log);
void ftylog_setLogLevelError(Ftylog* log);
void ftylog_setLogLevelFatal(Ftylog* log);

// Check the log level
bool ftylog_isLogTrace(Ftylog* log);
bool ftylog_isLogDebug(Ftylog* log);
bool ftylog_isLogInfo(Ftylog* log);
bool ftylog_isLogWarning(Ftylog* log);
bool ftylog_isLogError(Ftylog* log);
bool ftylog_isLogFatal(Ftylog* log);

// Procedure to print the log in the appenders
void ftylog_insertLog(Ftylog* log, int level, const char* file, int line, const char* func, const char* format, ...);

// Load a specific appender if verbose mode is set to true :
// -Save the logger logging level and set it to TRACE logging level
// -Remove an already existing ConsoleAppender
// -For the other appender, set the threshold parameter to the old logger log level
//    if no loglevel defined for this appender
// -Add a new console appender
void ftylog_setVerboseMode(Ftylog* log);
void ftylog_setVeboseMode(Ftylog* log); // legacy misnomer

// Return the Ftylog obect from the instance (C code)
Ftylog* ftylog_getInstance();
// Initialize the Ftylog object in the instance
void ftylog_setInstance(const char* component, const char* configFile);

#ifdef __cplusplus
}
#endif

//  Self test of this class
void fty_common_log_fty_log_test(bool verbose);

//  @end
#endif

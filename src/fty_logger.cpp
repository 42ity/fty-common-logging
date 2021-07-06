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

/*
@header
    fty_log - Log management
@discuss
@end
 */
#include "fty-log/fty_logger.h"
#include <fstream>
#include <log4cplus/configurator.h>
#include <log4cplus/consoleappender.h>
#include <log4cplus/hierarchy.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/loglevel.h>
#include <log4cplus/mdc.h>
#include <sstream>
#include <stdarg.h>
#include <stdio.h>
#include <thread>
#include <typeinfo>
#include <unistd.h>

using namespace log4cplus::helpers;

////////////////////////
// Ftylog section
////////////////////////

Ftylog::Ftylog(std::string component, std::string configFile)
{
    _watchConfigFile = nullptr;
    init(component, configFile);
}

Ftylog::Ftylog()
{
    std::ostringstream threadId;
    threadId << std::this_thread::get_id();
    std::string name = "log-default-" + threadId.str();

    _watchConfigFile = nullptr;
    init(name);
}

void Ftylog::init(std::string component, std::string configFile)
{
    if (nullptr != _watchConfigFile) {
        delete _watchConfigFile;
        _watchConfigFile = nullptr;
    }
    _logger.shutdown();
    _agentName     = component;
    _configFile    = configFile;
    _layoutPattern = LOGPATTERN;

    // initialize log4cplus
    log4cplus::initialize();

    // Create logger
    auto log = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT(component));
    _logger  = log;

    // Get log level from bios and set to the logger
    // even if there is a log configuration file
    setLogLevelFromEnv();

    // Get pattern layout from env
    setPatternFromEnv();

    // load appenders
    loadAppenders();
}

// Clean objects in destructor
Ftylog::~Ftylog()
{
    if (nullptr != _watchConfigFile) {
        delete _watchConfigFile;
        _watchConfigFile = nullptr;
    }
    _logger.shutdown();
}

// getter
std::string Ftylog::getAgentName()
{
    return _agentName;
}

// setter
void Ftylog::setConfigFile(std::string file)
{
    _configFile = file;
    loadAppenders();
}

void Ftylog::change(std::string name, std::string configFile)
{
    init(name, configFile);
}

// Initialize from environment variables
void Ftylog::setLogLevelFromEnv()
{
    // get BIOS_LOG_LEVEL value and set correction logging level
    const char* varEnv = getenv("BIOS_LOG_LEVEL");
    setLogLevelFromEnv(varEnv ? varEnv : "");
}
void Ftylog::setPatternFromEnv()
{
    // Get BIOS_LOG_PATTERN for a default patternlayout
    const char* varEnv = getenv("BIOS_LOG_PATTERN");
    if (varEnv && !std::string(varEnv).empty()) {
        _layoutPattern = varEnv;
    }
}

// Add a simple ConsoleAppender to the logger
void Ftylog::setConsoleAppender()
{
    _logger.removeAllAppenders();

    // create appender
    // Note: the first bool argument controls logging to stderr(true) as output stream
    SharedObjectPtr<log4cplus::Appender> append(new log4cplus::ConsoleAppender(true, true));

    // Create and affect layout
    append->setLayout(std::unique_ptr<log4cplus::Layout>(new log4cplus::PatternLayout(_layoutPattern)));
    append.get()->setName(LOG4CPLUS_TEXT("Console" + this->_agentName));

    // Add appender to logger
    _logger.addAppender(append);
}

void Ftylog::removeConsoleAppenders(log4cplus::Logger logger)
{
    for (log4cplus::SharedAppenderPtr& appenderPtr : logger.getAllAppenders())
    {
        log4cplus::Appender& app = *appenderPtr;

        if (typeid(app) == typeid(log4cplus::ConsoleAppender)) {
            // If any, remove it
            logger.removeAppender(appenderPtr);
            break;
        }
    }
}

// Switch the logging system to verbose
void Ftylog::setVerboseMode()
{
    // Save the loglevel of the logger
    log4cplus::LogLevel oldLevel = _logger.getLogLevel();

    // set log level of the logger to TRACE
    setLogLevelTrace();

    // Search if a console appender already exist in our logger instance or in
    // the root logger (we assume a flat hierarchy with the root logger and
    // specialized instances directly below the root logger)
    removeConsoleAppenders(_logger);
    removeConsoleAppenders(log4cplus::getDefaultHierarchy().getRoot());

    // Set all remaining appenders with the old log level as threshold if not defined
    for (log4cplus::SharedAppenderPtr& appenderPtr : _logger.getAllAppenders()) {
        log4cplus::Appender& app = *appenderPtr;
        if (app.getThreshold() == log4cplus::NOT_SET_LOG_LEVEL) {
            app.setThreshold(oldLevel);
        }
    }

    // create and add the appender
    SharedObjectPtr<log4cplus::Appender> append(new log4cplus::ConsoleAppender(false, true));
    // Create and affect layout
    append->setLayout(std::unique_ptr<log4cplus::Layout>(new log4cplus::PatternLayout(_layoutPattern)));
    append.get()->setName(LOG4CPLUS_TEXT("Verbose-" + this->_agentName));

    // Add verbose appender to logger
    _logger.addAppender(append);
}

void Ftylog::setContext(const std::map<std::string, std::string>& contextParam)
{
    log4cplus::getMDC().clear();
    for (auto const& entry : contextParam) {
        log4cplus::getMDC().put(entry.first, entry.second);
    }
}

void Ftylog::clearContext()
{
    log4cplus::getMDC().clear();
}

// Set appenders from log config file if exist
// or set a basic ConsoleAppender
void Ftylog::loadAppenders()
{
    // Get BIOS_LOG_INIT_LEVEL value and set correction logging level
    // before we start processing the rest - perhaps the user does not
    // want to see reports about early logging initialization itself.
    // When we do have some configuration loaded or defaulted, it will
    // later override this setting.
    // Note that each call to loadAppenders() would process this envvar
    // again - this is because the de-initialization below can make
    // some noise, and/or config can be reloaded at run-time.
    const char*         varEnvInit = getenv("BIOS_LOG_INIT_LEVEL");
    log4cplus::LogLevel oldLevel   = log4cplus::NOT_SET_LOG_LEVEL;
    if (nullptr != varEnvInit) {
        // If the caller provided a BIOS_LOG_INIT_LEVEL setting,
        // honor it until we load a config file or have none -
        // and at that point revert to current logging level first.
        // This should allow for quiet tool startups when explicitly
        // desired.

        // Save the loglevel of the logger - e.g. a value set
        // by common BIOS_LOG_LEVEL earlier
        oldLevel = _logger.getLogLevel();
        setLogInitLevelFromEnv(varEnvInit);
    }

    // If true, load file
    bool loadFile = false;

    // Stop the watch confile file thread if any
    if (nullptr != _watchConfigFile) {
        delete _watchConfigFile;
        _watchConfigFile = nullptr;
    }

    // if path to log config file
    if (!_configFile.empty()) {
        // file can be accessed with read rights
        if (FILE* file = fopen(_configFile.c_str(), "r")) {
            fclose(file);
            loadFile = true;
        } else {
            log_error_log(this,
                "File %s can't be accessed with read rights; this process will "
                "not monitor whether it becomes available later",
                _configFile.c_str());
            _configFile = "";
        }
    }
    else {
        if (nullptr != varEnvInit)
            log_warning_log(this, "No log configuration file defined");
    }

    // if no file or file not valid, set default ConsoleAppender
    if (loadFile) {
        if (nullptr != varEnvInit)
            log_info_log(this, "Load Config file %s ", _configFile.c_str());

        // Remove previous appender
        _logger.removeAllAppenders();

        if (log4cplus::NOT_SET_LOG_LEVEL != oldLevel) {
            _logger.setLogLevel(oldLevel);
        }

        // Load the file
        log4cplus::PropertyConfigurator::doConfigure(LOG4CPLUS_TEXT(_configFile));

        // Start the thread watching the modification of the log config file
        _watchConfigFile = new log4cplus::ConfigureAndWatchThread(_configFile.c_str(), 60000);
    }
    else {
        // by default, set console appenders
        setConsoleAppender();

        if (nullptr != varEnvInit)
            log_info_log(this,
                "No log configuration file was loaded, will "
                "log to stderr by default");
        if (log4cplus::NOT_SET_LOG_LEVEL != oldLevel) {
            _logger.setLogLevel(oldLevel);
        }
    }
}

// Set the logging level corresponding to the BIOS_LOG_LEVEL value
bool Ftylog::setLogLevelFromEnvDefinite(const std::string& level)
{
    if (level.empty()) {
        // If empty string, set nothing and return false
        return false;
    } else if (level == "LOG_TRACE") {
        setLogLevelTrace();
    } else if (level == "LOG_DEBUG") {
        setLogLevelDebug();
    } else if (level == "LOG_INFO") {
        setLogLevelInfo();
    } else if (level == "LOG_WARNING") {
        setLogLevelWarning();
    } else if (level == "LOG_ERR") {
        setLogLevelError();
    } else if (level == "LOG_CRIT") {
        setLogLevelFatal();
    } else if (level == "LOG_OFF") {
        setLogLevelOff();
    } else {
        // If unknown string, set nothing and return false
        return false;
    }
    return true;
}

void Ftylog::setLogInitLevelFromEnv(const std::string& level)
{
    // If empty or unknown string, set logging subsystem initialization level to OFF
    if (!setLogLevelFromEnvDefinite(level)) {
        setLogLevelOff();
    }
}

void Ftylog::setLogLevelFromEnv(const std::string& level)
{
    // If empty or unknown string, set log level to TRACE by default
    if (!setLogLevelFromEnvDefinite(level)) {
        setLogLevelTrace();
    }
}

// Set logger to a specific logging level
void Ftylog::setLogLevelTrace()   { _logger.setLogLevel(log4cplus::TRACE_LOG_LEVEL); }
void Ftylog::setLogLevelDebug()   { _logger.setLogLevel(log4cplus::DEBUG_LOG_LEVEL); }
void Ftylog::setLogLevelInfo()    { _logger.setLogLevel(log4cplus::INFO_LOG_LEVEL); }
void Ftylog::setLogLevelWarning() { _logger.setLogLevel(log4cplus::WARN_LOG_LEVEL); }
void Ftylog::setLogLevelError()   { _logger.setLogLevel(log4cplus::ERROR_LOG_LEVEL); }
void Ftylog::setLogLevelFatal()   { _logger.setLogLevel(log4cplus::FATAL_LOG_LEVEL); }
void Ftylog::setLogLevelOff()     { _logger.setLogLevel(log4cplus::OFF_LOG_LEVEL); }

// Return true if the logging level is included in the logger log level
bool Ftylog::isLogLevel(log4cplus::LogLevel level)
{
    return _logger.getLogLevel() <= level;
}

bool Ftylog::isLogTrace()   { return isLogLevel(log4cplus::TRACE_LOG_LEVEL); }
bool Ftylog::isLogDebug()   { return isLogLevel(log4cplus::DEBUG_LOG_LEVEL); }
bool Ftylog::isLogInfo()    { return isLogLevel(log4cplus::INFO_LOG_LEVEL); }
bool Ftylog::isLogWarning() { return isLogLevel(log4cplus::WARN_LOG_LEVEL); }
bool Ftylog::isLogError()   { return isLogLevel(log4cplus::ERROR_LOG_LEVEL); }
bool Ftylog::isLogFatal()   { return isLogLevel(log4cplus::FATAL_LOG_LEVEL); }
bool Ftylog::isLogOff()     { return _logger.getLogLevel() == log4cplus::OFF_LOG_LEVEL; }

// Call log4cplus system to print logs in logger appenders
void Ftylog::insertLog(
    log4cplus::LogLevel level, const char* file, int line, const char* func, const char* format, va_list args)
{
    char* buffer = nullptr;
    int   r;

    // Check if the level of this log is included in the log level
    if (!isLogLevel(level)) {
        return;
    }
    // Construct the main log message
    r = vasprintf(&buffer, format, args);
    if (r == -1) {
        fprintf(stderr,
            "[ERROR]: %s:%d (%s) can't allocate enough memory for message "
            "string: buffer",
            __FILE__, __LINE__, __func__);
        if (buffer) {
            free(buffer);
        }
        return;
    }

    if (buffer == nullptr) {
        log_error_log(this, "Buffer == NULL");
        return;
    }

    // Give the printing job to log4cplus
    log4cplus::detail::macro_forced_log(_logger, level, LOG4CPLUS_TEXT(buffer), file, line, func);
    free(buffer);
}

void Ftylog::insertLog(log4cplus::LogLevel level, const char* file, int line, const char* func, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    insertLog(level, file, line, func, format, args);
    va_end(args);
}

////////////////////////
// ManageFtyLog section
////////////////////////

Ftylog ManageFtyLog::_ftylogdefault = Ftylog("ftylog", FTY_COMMON_LOGGING_DEFAULT_CFG);

Ftylog* ManageFtyLog::getInstanceFtylog()
{
    return &_ftylogdefault;
}

void ManageFtyLog::setInstanceFtylog(std::string componentName, std::string logConfigFile)
{
    _ftylogdefault.change(componentName, logConfigFile);
}

////////////////////////
// Wrapper for C code use
////////////////////////

// construtor
Ftylog* ftylog_new(const char* component, const char* logConfigFile)
{
    return new Ftylog(std::string(component), std::string(logConfigFile));
}

// destructor
void ftylog_delete(Ftylog* log)
{
    if (log) delete log;
}

// setter
void ftylog_setConfigFile(Ftylog* log, const char* file)
{
    if (log) log->setConfigFile(std::string(file));
}

// Set the logger to a specific log level
void ftylog_setLogLevelTrace(Ftylog* log)
{
    if (log) log->setLogLevelTrace();
}

void ftylog_setLogLevelDebug(Ftylog* log)
{
    if (log) log->setLogLevelDebug();
}

void ftylog_setLogLevelInfo(Ftylog* log)
{
    if (log) log->setLogLevelInfo();
}

void ftylog_setLogLevelWarning(Ftylog* log)
{
    if (log) log->setLogLevelWarning();
}

void ftylog_setLogLevelError(Ftylog* log)
{
    if (log) log->setLogLevelError();
}

void ftylog_setLogLevelFatal(Ftylog* log)
{
    if (log) log->setLogLevelFatal();
}

// Check the log level
bool ftylog_isLogTrace(Ftylog* log)
{
    return log ? log->isLogTrace() : false;
}

bool ftylog_isLogDebug(Ftylog* log)
{
    return log ? log->isLogDebug() : false;
}

bool ftylog_isLogInfo(Ftylog* log)
{
    return log ? log->isLogInfo() : false;
}

bool ftylog_isLogWarning(Ftylog* log)
{
    return log ? log->isLogWarning() : false;
}

bool ftylog_isLogError(Ftylog* log)
{
    return log ? log->isLogError() : false;
}

bool ftylog_isLogFatal(Ftylog* log)
{
    return log ? log->isLogTrace() : false;
}

// Print log in logger appenders
void ftylog_insertLog(Ftylog* log, int level, const char* file, int line, const char* func, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    if (log) log->insertLog(level, file, line, func, format, args);
    va_end(args);
}

// Switch to verbose mode
void ftylog_setVeboseMode(Ftylog* log) // legacy misnomer
{
    ftylog_setVerboseMode(log);
}

void ftylog_setVerboseMode(Ftylog* log)
{
    if (log) log->setVerboseMode();
}

Ftylog* ftylog_getInstance()
{
    return ManageFtyLog::getInstanceFtylog();
}

void ftylog_setInstance(const char* component, const char* configFile)
{
    ManageFtyLog::setInstanceFtylog(std::string(component), std::string(configFile));
}

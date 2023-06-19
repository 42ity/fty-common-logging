#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_DISABLE_EXCEPTIONS
#include <catch2/catch.hpp>

#include "fty_log.h"

TEST_CASE("Main")
{
    INFO(" * fty_log");
    INFO(" * Check default log");
    ManageFtyLog::getInstanceFtylog()->setLogLevelTrace();
    log_trace("This is a simple %s log with default logger", "trace");
    log_debug("This is a simple %s log with default logger", "debug");
    log_info("This is a simple %s log with default logger", "info");
    log_warning("This is a simple %s log with default logger", "warning");
    log_error("This is a simple %s log with default logger", "error");
    log_fatal("This is a simple %s log with default logger", "fatal");
    INFO(" * Check default log : OK \n");


    INFO(" * Check level test \n");
    ManageFtyLog::setInstanceFtylog("fty-log-agent");
    Ftylog* test = ManageFtyLog::getInstanceFtylog();

    test->setLogLevelTrace();
    log_trace_log(test, "This is a simple trace log");
    log_trace_log(test, "This is a %s log test number %d", "trace", 1);
    CHECK(test->isLogTrace());
    CHECK(test->isLogDebug());
    CHECK(test->isLogInfo());
    CHECK(test->isLogWarning());
    CHECK(test->isLogError());
    CHECK(test->isLogFatal());

    test->setLogLevelDebug();
    log_debug_log(test, "This is a simple debug log");
    log_debug_log(test, "This is a %s log test number %d", "debug", 1);
    CHECK(!test->isLogTrace());
    CHECK(test->isLogDebug());
    CHECK(test->isLogInfo());
    CHECK(test->isLogWarning());
    CHECK(test->isLogError());
    CHECK(test->isLogFatal());

    test->setLogLevelInfo();
    log_info_log(test, "This is a simple info log");
    log_info_log(test, "This is a %s log test number %d", "info", 1);
    CHECK(!test->isLogTrace());
    CHECK(!test->isLogDebug());
    CHECK(test->isLogInfo());
    CHECK(test->isLogWarning());
    CHECK(test->isLogError());
    CHECK(test->isLogFatal());

    test->setLogLevelWarning();
    log_warning_log(test, "This is a simple warning log");
    log_warning_log(test, "This is a %s log test number %d", "warning", 1);
    CHECK(!test->isLogTrace());
    CHECK(!test->isLogDebug());
    CHECK(!test->isLogInfo());
    CHECK(test->isLogWarning());
    CHECK(test->isLogError());
    CHECK(test->isLogFatal());

    test->setLogLevelError();
    log_error_log(test, "This is a simple error log");
    log_error_log(test, "This is a %s log test number %d", "error", 1);
    CHECK(!test->isLogTrace());
    CHECK(!test->isLogDebug());
    CHECK(!test->isLogInfo());
    CHECK(!test->isLogWarning());
    CHECK(test->isLogError());
    CHECK(test->isLogFatal());

    test->setLogLevelFatal();
    log_fatal_log(test, "This is a simple fatal log");
    log_fatal_log(test, "This is a %s log test number %d", "fatal", 1);
    CHECK(!test->isLogTrace());
    CHECK(!test->isLogDebug());
    CHECK(!test->isLogInfo());
    CHECK(!test->isLogWarning());
    CHECK(!test->isLogError());
    CHECK(test->isLogFatal());

    test->setLogLevelTrace();
    INFO(" * Check level test : OK \n");
    INFO(" * Check log config file test\n");
    test->setConfigFile("test/conf/not-valid-test-config.conf");
    test->setConfigFile("test/conf/test-config.conf");

    log_info_log(test, "This is a simple info log");
    log_info_log(test, "This is a %s log test number %d", "info", 1);

    // file is created
    CHECK(access("/tmp/logging.txt", F_OK) != -1);

    // this size is not 0
    std::ifstream file("/tmp/logging.txt", std::ifstream::in | std::ifstream::binary);
    file.seekg(0, std::ios::end);
    auto fileSize = file.tellg();

    CHECK(0 != fileSize);
    INFO(" * Check log config file test : OK");

    INFO(" * Check verbose");
    test->setVerboseMode();
    log_trace_log(test, "This is a verbose trace log");
    INFO(" * Check verbose : OK");

    INFO(" * Check legacy misnamed vebose");
    test->setVeboseMode();
    log_trace_log(test, "This is a legacy misnamed vebose trace log");
    INFO(" * Check legacy misnamed vebose : OK");

    // delete the log file test
    remove("./src/selftest-rw/logfile.log");

    //  @selftest
    printf("OK\n");
}

TEST_CASE("IPMPROG-6348 C++ API SegFault")
{
    logInfo("%d%s"); // ***Exception: SegFault
    logInfo("{}", "%s%s"); // ***Exception: SegFault

    logTrace("%s%s {}", "trace");
    logDebug("%i%s {}", "debug");
    logInfo("%u%s {}", "info");
    logWarn("%x%s {}", "warn");
    logError("%lf%s {}", "error");

    logInfo("%d%s {} %d%s", "%d%s");
    logInfo("%s%s {} %s%s", "%s%s");
}

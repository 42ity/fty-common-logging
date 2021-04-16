cmake_minimum_required(VERSION 3.13)
cmake_policy(VERSION 3.13)

########################################################################################################################

project(fty_common_logging
    VERSION 1.0.0
    DESCRIPTION "Provides common logs"
)

########################################################################################################################
find_package(fty-cmake PATHS ${CMAKE_BINARY_DIR}/fty-cmake)
########################################################################################################################

etn_target(shared ${PROJECT_NAME}
    PUBLIC_INCLUDE_DIR include
    PUBLIC
        fty_log.h
        fty-log/fty_logger.h
    SOURCES
        src/fty_logger.cpp
        fty_common_logging.pc.in
    FLAGS -Wno-format-nonliteral
    USES
        log4cplus
)

########################################################################################################################

etn_test_target(${PROJECT_NAME}
    CONFIGS
        test/conf/test-config.conf
    SOURCES
        test/main.cpp
    FLAGS
        -Wno-extra-semi-stmt
    SUBDIR
        test
)

########################################################################################################################


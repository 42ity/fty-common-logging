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
    USES_PUBLIC
        log4cplus::log4cplus
)

########################################################################################################################

if (BUILD_TESTING)
    etn_test(${PROJECT_NAME}_test
        CONFIGS
            test/conf/test-config.conf
        SOURCES
            test/main.cpp
        USES
            ${PROJECT_NAME}
            Catch2::Catch2
    )

    etn_coverage(${PROJECT_NAME}_test)
endif()

########################################################################################################################


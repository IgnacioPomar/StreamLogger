cmake_minimum_required(VERSION 3.23)

project(StreamLogger VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(STREAMLOGGER_BUILD_TESTS "Build the unit tests" OFF)
option(STREAMLOGGER_BUILD_EXAMPLES "Build the examples" OFF)

find_package(Threads REQUIRED)

set(STREAMLOGGER_SOURCES
    src/LoggerConsoleUtils.cpp
    src/StackLogger.cpp
    src/StackLoggerConfig.cpp
    src/StackLoggerSingleton.cpp
    src/StreamLogger.cpp
    src/StreamLoggerConsts.cpp
)

add_library(StreamLogger ${STREAMLOGGER_SOURCES})
add_library(StreamLogger::StreamLogger ALIAS StreamLogger)

target_include_directories(StreamLogger PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(StreamLogger PUBLIC Threads::Threads)

# Only the symbols marked with LGGR_API are exported from the shared library
set_target_properties(StreamLogger PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)

# Generated LGGR_API / LGGR_LOCAL / LGGR_DEPRECATED: empty for a static library, export/import for a shared one
# (StreamLogger_EXPORTS is defined by CMake when building the shared library)
include(GenerateExportHeader)
generate_export_header(StreamLogger
    BASE_NAME LGGR
    EXPORT_MACRO_NAME LGGR_API
    NO_EXPORT_MACRO_NAME LGGR_LOCAL
    EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/include/StreamLogger/lggrExportCfg.h
)

install(TARGETS StreamLogger)
install(DIRECTORY include/ ${CMAKE_CURRENT_BINARY_DIR}/include/ DESTINATION include)

if(STREAMLOGGER_BUILD_EXAMPLES)
    add_executable(StreamLoggerExample examples/example.cpp)
    target_link_libraries(StreamLoggerExample PRIVATE StreamLogger)
endif()

if(STREAMLOGGER_BUILD_TESTS)
    enable_testing()

    find_package(Catch2 3 REQUIRED)
    find_package(trompeloeil REQUIRED)

    # The tests use the internal classes (isolated StackLogger instances), which the shared library does not export:
    # the tester is compiled with the sources instead of linking the library (LGGR_STATIC_DEFINE: empty LGGR_API)
    add_executable(StreamLoggerTester
        ${STREAMLOGGER_SOURCES}
        test/001_Levels.cpp
        test/002_Subscribers.cpp
        test/003_TimedEvents.cpp
        test/004_LogFile.cpp
        test/005_Console.cpp
        test/006_MultiThread.cpp
        test/007_Hardening.cpp
    )
    target_include_directories(StreamLoggerTester PRIVATE
        include
        ${CMAKE_CURRENT_BINARY_DIR}/include
        src
    )
    target_compile_definitions(StreamLoggerTester PRIVATE LGGR_STATIC_DEFINE)
    target_link_libraries(StreamLoggerTester PRIVATE Threads::Threads Catch2::Catch2WithMain trompeloeil::trompeloeil)

    include(Catch)
    catch_discover_tests(StreamLoggerTester PROPERTIES TIMEOUT 60)
endif()

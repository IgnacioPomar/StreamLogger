cmake_minimum_required(VERSION 3.23)

project(StreamLogger VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(STREAMLOGGER_BUILD_TESTS "Build the unit tests" OFF)
option(STREAMLOGGER_BUILD_EXAMPLES "Build the examples" OFF)

find_package(Threads REQUIRED)

add_library(StreamLogger
    src/LoggerConsoleUtils.cpp
    src/StackLogger.cpp
    src/StackLoggerConfig.cpp
    src/StackLoggerSingleton.cpp
    src/StreamLogger.cpp
    src/StreamLoggerConsts.cpp
)
add_library(StreamLogger::StreamLogger ALIAS StreamLogger)

target_include_directories(StreamLogger PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(StreamLogger PUBLIC Threads::Threads)

if(BUILD_SHARED_LIBS)
    target_compile_definitions(StreamLogger PUBLIC LGGR_DLL PRIVATE STREAMLOGGER_EXPORTS)
endif()

install(TARGETS StreamLogger)
install(DIRECTORY include/ DESTINATION include)

if(STREAMLOGGER_BUILD_EXAMPLES)
    add_executable(StreamLoggerExample examples/example.cpp)
    target_link_libraries(StreamLoggerExample PRIVATE StreamLogger)
endif()

if(STREAMLOGGER_BUILD_TESTS)
    enable_testing()

    find_package(Catch2 3 REQUIRED)
    find_package(trompeloeil REQUIRED)

    add_executable(StreamLoggerTester
        test/001_Levels.cpp
        test/002_Subscribers.cpp
        test/003_TimedEvents.cpp
        test/004_LogFile.cpp
        test/005_Console.cpp
        test/006_MultiThread.cpp
        test/007_Hardening.cpp
    )
    # The tests also use the internal headers, to test isolated StackLogger instances
    target_include_directories(StreamLoggerTester PRIVATE src)
    target_link_libraries(StreamLoggerTester PRIVATE StreamLogger Catch2::Catch2WithMain trompeloeil::trompeloeil)

    include(Catch)
    catch_discover_tests(StreamLoggerTester PROPERTIES TIMEOUT 60)
endif()

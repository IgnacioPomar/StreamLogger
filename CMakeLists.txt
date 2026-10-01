cmake_minimum_required(VERSION 3.23)

# Single source of truth for the version: the VERSION file (MAJOR.MINOR.PATCH), also read by Conan and the CI
file(STRINGS ${CMAKE_CURRENT_SOURCE_DIR}/VERSION STREAMLOGGER_VERSION LIMIT_COUNT 1)
if(NOT STREAMLOGGER_VERSION MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+$")
    message(FATAL_ERROR "VERSION must be MAJOR.MINOR.PATCH, not '${STREAMLOGGER_VERSION}'")
endif()
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS VERSION)

project(StreamLogger
    VERSION ${STREAMLOGGER_VERSION}
    DESCRIPTION "Modern C++ logger library, with event retrieval and color support"
    LANGUAGES CXX
)

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
    src/LibraryInfo.cpp
)

add_library(StreamLogger ${STREAMLOGGER_SOURCES})

# The build date (getLibraryInfo) is taken when LibraryInfo.cpp is compiled: recompile it on any change
file(GLOB STREAMLOGGER_HEADERS CONFIGURE_DEPENDS include/StreamLogger/*.h src/*.h)
set(STREAMLOGGER_OTHER_SOURCES ${STREAMLOGGER_SOURCES})
list(REMOVE_ITEM STREAMLOGGER_OTHER_SOURCES src/LibraryInfo.cpp)
list(TRANSFORM STREAMLOGGER_OTHER_SOURCES PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/)
set_source_files_properties(src/LibraryInfo.cpp PROPERTIES
    OBJECT_DEPENDS "${STREAMLOGGER_OTHER_SOURCES};${STREAMLOGGER_HEADERS}"
)
add_library(StreamLogger::StreamLogger ALIAS StreamLogger)

target_include_directories(StreamLogger PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(StreamLogger PUBLIC Threads::Threads)

# Before 1.0 a minor version may break the ABI: libStreamLogger.so.0.1 -> libStreamLogger.so.0.1.0
if(PROJECT_VERSION_MAJOR EQUAL 0)
    set(STREAMLOGGER_SOVERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})
else()
    set(STREAMLOGGER_SOVERSION ${PROJECT_VERSION_MAJOR})
endif()

# The shared library exports all its symbols (default visibility)
set_target_properties(StreamLogger PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${STREAMLOGGER_SOVERSION}
)

# Name, description and version of project(), as macros (STREAMLOGGER_VERSION...)
configure_file(cmake/StreamLoggerVersion.h.in ${CMAKE_CURRENT_BINARY_DIR}/include/StreamLogger/StreamLoggerVersion.h @ONLY)

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

    # The tests also use the internal classes (isolated StackLogger instances): src/ is in the include path
    add_executable(StreamLoggerTester
        test/001_Levels.cpp
        test/002_Subscribers.cpp
        test/003_TimedEvents.cpp
        test/004_LogFile.cpp
        test/005_Console.cpp
        test/006_MultiThread.cpp
        test/007_Hardening.cpp
        test/008_LibraryInfo.cpp
    )
    target_include_directories(StreamLoggerTester PRIVATE src)
    target_link_libraries(StreamLoggerTester PRIVATE StreamLogger Catch2::Catch2WithMain trompeloeil::trompeloeil)

    include(Catch)
    catch_discover_tests(StreamLoggerTester PROPERTIES TIMEOUT 60)
endif()

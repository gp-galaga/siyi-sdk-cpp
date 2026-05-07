cmake_minimum_required(VERSION 3.15)
project(SIYI_SDK_CPP CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Python3 COMPONENTS Interpreter QUIET)

if(Python3_Interpreter_FOUND)
    add_custom_target(generate_protocol_headers
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate_protocol_headers.py --root ${CMAKE_CURRENT_SOURCE_DIR}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Generating protocol headers from protocol/tc/definitions.yaml and protocol/tm/definitions.yaml")
else()
    message(WARNING "Python3 not found: 'generate_protocol_headers' target is unavailable.")
endif()

# Main library sources
set(SIYI_SOURCES
    src/camera/base_camera.cpp
    src/camera/zr30_camera.cpp
    src/helper/log_manager.cpp
)

# Create library
add_library(siyi_sdk ${SIYI_SOURCES})
target_include_directories(siyi_sdk PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(siyi_demo apps/main.cpp)
target_link_libraries(siyi_demo PRIVATE siyi_sdk)

# Tests
include(CTest)

if(DEFINED DOCTEST_HEADER_PATH)
    message(STATUS "Using doctest from: ${DOCTEST_HEADER_PATH}")
else()
    message(STATUS "Doctest not specified. Download doctest.h to tests/ directory")
    message(STATUS "  wget -O tests/doctest.h https://raw.githubusercontent.com/doctest/doctest/master/doctest/doctest.h")
    set(DOCTEST_HEADER_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tests")
endif()

if(EXISTS "${DOCTEST_HEADER_PATH}/doctest.h")
    add_executable(test_siyi_commands tests/test_siyi_commands.cpp)
    target_include_directories(test_siyi_commands PRIVATE ${DOCTEST_HEADER_PATH})
    target_link_libraries(test_siyi_commands siyi_sdk)
    
    add_test(NAME SIYICommands COMMAND test_siyi_commands)
    message(STATUS "Tests enabled")
else()
    message(WARNING "doctest.h not found. Tests disabled.")
    message(WARNING "Download: wget -O tests/doctest.h https://raw.githubusercontent.com/doctest/doctest/master/doctest/doctest.h")
endif()

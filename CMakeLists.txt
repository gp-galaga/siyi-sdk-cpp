cmake_minimum_required(VERSION 3.15)
project(SIYI_SDK_CPP VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

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
    src/camera/shared_camera_model.cpp
    src/camera/optical_zoom_camera_model.cpp
    src/camera/zr30_camera_model.cpp
    src/transport/udp_transport.cpp
    src/transport/tcp_transport.cpp
    src/helper/log_manager.cpp
)

# Create library
add_library(siyi_sdk ${SIYI_SOURCES})
add_library(siyi_sdk::siyi_sdk ALIAS siyi_sdk)

target_include_directories(siyi_sdk
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

set_target_properties(siyi_sdk PROPERTIES EXPORT_NAME siyi_sdk)

set(SIYI_SDK_CMAKE_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/siyi_sdk")

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/siyi_sdkConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/siyi_sdkConfig.cmake
    INSTALL_DESTINATION ${SIYI_SDK_CMAKE_INSTALL_DIR})

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/siyi_sdkConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

install(TARGETS siyi_sdk
    EXPORT siyi_sdkTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT siyi_sdkTargets
    FILE siyi_sdkTargets.cmake
    NAMESPACE siyi_sdk::
    DESTINATION ${SIYI_SDK_CMAKE_INSTALL_DIR})

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/siyi_sdkConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/siyi_sdkConfigVersion.cmake
    DESTINATION ${SIYI_SDK_CMAKE_INSTALL_DIR})

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
    add_executable(test_siyi_commands
        tests/test_shared_camera_model.cpp
        tests/test_optical_zoom_camera_model.cpp
        tests/test_zr30_camera_model.cpp)
    target_include_directories(test_siyi_commands PRIVATE ${DOCTEST_HEADER_PATH})
    target_link_libraries(test_siyi_commands siyi_sdk)
    
    add_test(NAME SIYICommands COMMAND test_siyi_commands)
    message(STATUS "Tests enabled")
else()
    message(WARNING "doctest.h not found. Tests disabled.")
    message(WARNING "Download: wget -O tests/doctest.h https://raw.githubusercontent.com/doctest/doctest/master/doctest/doctest.h")
endif()

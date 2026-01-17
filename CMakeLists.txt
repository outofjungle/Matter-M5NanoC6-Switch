# M5NanoC6 Matter Switch - CMakeLists.txt
# ESP32-C6 Matter device using esp-matter SDK

cmake_minimum_required(VERSION 3.10)

if(NOT DEFINED ENV{ESP_MATTER_PATH})
    message(FATAL_ERROR "Please set ESP_MATTER_PATH to the path of esp-matter repo")
endif()

set(PROJECT_VER "1.0")
set(PROJECT_VER_NUMBER 1)

set(ESP_MATTER_PATH $ENV{ESP_MATTER_PATH})
set(MATTER_SDK_PATH ${ESP_MATTER_PATH}/connectedhomeip/connectedhomeip)

# This should be done before using the IDF_TARGET variable.
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
include(${ESP_MATTER_PATH}/examples/common/cmake_common/components_include.cmake)

# Exclude app_reset from examples/common - we have a local version with updated API
set(EXTRA_COMPONENT_DIRS
    "${MATTER_SDK_PATH}/config/esp32/components"
    "${ESP_MATTER_PATH}/components"
    "${ESP_MATTER_PATH}/examples/common/utils"
    "${ESP_MATTER_PATH}/examples/common/app_bridge"
    ${extra_components_dirs_append})

project(M5NanoC6-Switch)

idf_build_set_property(CXX_COMPILE_OPTIONS "-std=gnu++17;-Os;-DCHIP_HAVE_CONFIG_H;-Wno-overloaded-virtual" APPEND)
idf_build_set_property(C_COMPILE_OPTIONS "-Os" APPEND)
# Note: Removed -Wno-format-nonliteral and -Wno-format-security suppressions
# These security warnings help catch format string vulnerabilities and should not be globally disabled

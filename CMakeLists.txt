cmake_minimum_required(VERSION 3.1.0)

set(PROJECT_NAME "nmea_parcer_c_code")

project(${PROJECT_NAME} VERSION 0.0.0)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

include_directories(
    ${CMAKE_SOURCE_DIR}/source
)

add_executable(${PROJECT_NAME}
    ${CMAKE_SOURCE_DIR}/source/test_c_code.c
)

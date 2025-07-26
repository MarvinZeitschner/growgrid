cmake_minimum_required(VERSION 3.16)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(EXTRA_COMPONENT_DIRS "${CMAKE_SOURCE_DIR}/main/components" "${CMAKE_SOURCE_DIR}/main/components/drivers")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(growgrid)

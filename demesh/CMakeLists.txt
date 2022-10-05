# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)
include($ENV{MDF_PATH}/project.cmake)
project(demesh)


# TMoor: somehow includes are broken when building the app (not the components)
idf_build_set_property(COMPILE_OPTIONS "-I${CMAKE_CURRENT_LIST_DIR}/components/m5stickc/include" APPEND)
idf_build_set_property(COMPILE_OPTIONS "-I${IDF_PATH}/components/mqtt/esp-mqtt/include" APPEND)

# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)
include($ENV{MDF_PATH}/project.cmake)
project(demesh)



# TMoor: includes are broken when building the app (not the components)
idf_build_set_property(COMPILE_OPTIONS "-I${CMAKE_CURRENT_LIST_DIR}/components/m5stickc/include" APPEND)
idf_build_set_property(COMPILE_OPTIONS "-I${IDF_PATH}/components/mqtt/esp-mqtt/include" APPEND)


# TMoor: attempt to debug/investigate
idf_component_get_property(dirs m5stickc INCLUDE_DIRS)
message(STATUS "The 'm5stickc' include directories are: ${dirs}")
idf_component_get_property(dirs mqtt INCLUDE_DIRS)
message(STATUS "The 'mqtt' include directories are: ${dirs}")
idf_build_get_property(comps BUILD_COMPONENT_ALIASES)
message(STATUS "The project components are  ${comps}")


idf_build_get_property(dir INCLUDE_DIRECTORIES)
message(STATUS "The projects include directories are: ${dir}")
idf_build_get_property(opt COMPILE_OPTIONS)
message(STATUS "The projects options are: ${opt}")
idf_build_get_property(opt BUILD_COMPONENTS)
message(STATUS "The projects components are: ${opt}")

cmake_minimum_required(VERSION 3.15)
project(snapsize VERSION 0.0.0)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
include_directories(snapsize PUBLIC inc)
add_compile_options(-Wall -Wextra -pedantic -Werror)
# add_compile_definitions(SNAPSIZE_VERSION=)

set(CMAKE_DEBUG_POSTFIX _debug)
# Not a real library, just used to set common compiler flags
add_library(snapsize_compiler_flags INTERFACE)

list(APPEND common_sources
    src/Extents.cc
    src/Extents_ioctl.cc
    src/HumanSize.cc
    src/UniqueFileDescriptor.cc
)

add_executable(de src/de.cc ${common_sources})
set_target_properties(de PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
target_link_libraries(de PUBLIC snapsize_compiler_flags)

set(CMAKE_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
install(TARGETS de)
install(FILES LICENSE DESTINATION "${CMAKE_INSTALL_DATADIR}/licenses/snapsize")

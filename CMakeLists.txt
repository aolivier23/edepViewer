cmake_minimum_required( VERSION 3.5.1 )
project( evd )

#Compiler flags
set( GCC_Flags_For_CXX "-std=c++14 -Wall" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_Flags_For_CXX}" )
set( CMAKE_CXX_FLAGS_DEBUG "-ggdb" )
set( CMAKE_CXX_FLAGS_RELEASE "-O2" )

#gtkmm stuff
find_package(PkgConfig)
pkg_check_modules(GTKMM gtkmm-3.0)

#Support for glad
add_library(glad glad/src/glad.c)
target_include_directories("glad" PRIVATE "glad/include")
target_link_libraries("glad" ${CMAKE_DL_LIBS})

add_subdirectory(gl)
add_subdirectory(gtk)

cmake_minimum_required( VERSION 3.5.1 )
project( evd )

#Compiler flags
set( CMAKE_CXX_STANDARD 14 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS ON )

set( GCC_Flags_For_CXX "-std=c++14 -Wall -fPIC" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_Flags_For_CXX}" )
set( CMAKE_CXX_FLAGS_DEBUG "-ggdb" )
set( CMAKE_CXX_FLAGS_RELEASE "-O2" )

#gtkmm stuff
find_package(PkgConfig)
pkg_check_modules(GTKMM gtkmm-3.0)

#gtkmm stuff from https://wiki.gnome.org/Projects/gtkmm/UsingCMake
link_directories( ${GTKMM_LIBRARY_DIRS} )
include_directories( ${GTKMM_INCLUDE_DIRS} )

#XML parser library for configuration
find_package( tinyxml2 REQUIRED )

#Support for glad
add_library(glad SHARED glad/src/glad.c)
target_include_directories("glad" PRIVATE "glad/include")
target_link_libraries("glad" ${CMAKE_DL_LIBS})

#Stuff for Opengl
find_package( OpenGL REQUIRED )

#Set up to build against ROOT
#I learned how to do this here: https://root.cern.ch/how/integrate-root-my-project-cmake
list( APPEND CMAKE_PREFIX_PATH $ENV{ROOT_SYS})
find_package(ROOT REQUIRED COMPONENTS Core Geom Tree RIO TreePlayer EG)
include(${ROOT_USE_FILE})

#Set output directory for executables
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../bin )

#local stuff that GLDrawArea needs to know about
include_directories( "${PROJECT_SOURCE_DIR}" )

#Add plugins that depend on external libraries to these variables
set(EXTERNAL_SOURCE "")
set(EXTERNAL_LIBS "")

if(DEFINED EDEPNEUTRONS_DIR)
  list(APPEND EXTERNAL_SOURCE MCHitDrawer.cpp)
  list(APPEND EXTERNAL_LIBS persistency)

  #Find EDepNeutrons directory
  message("Building external plugins for EDepNeutrons with installation at path ${EDEPNEUTRONS_DIR}")
  include_directories( "${EDEPNEUTRONS_DIR}/src" )
  link_directories( "${EDEPNEUTRONS_DIR}/lib" )
endif()

add_subdirectory(external)
add_subdirectory(util)
add_subdirectory(gl)
add_subdirectory(gtk)
add_subdirectory(plugins)

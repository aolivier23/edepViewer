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

message( "CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}" )

#Force CMake to link paths "correctly".  See the discussion at https://cmake.org/pipermail/cmake/2011-June/044807.html
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

#Let source code know where this project is installed.  Needed for finding shader sources.  
#TODO: Get shader sources from an environment variable instead.
add_definitions(-DINSTALL_GLSL_DIR="${CMAKE_INSTALL_PREFIX}/conf/glsl")
add_definitions(-DINSTALL_XML_DIR="${CMAKE_INSTALL_PREFIX}/conf/xml")

#XML parser library for configuration
find_package( tinyxml2 REQUIRED )

#Support for glad
add_library(glad SHARED glad/src/glad.c)
target_include_directories("glad" PRIVATE "glad/include")
target_link_libraries("glad" ${CMAKE_DL_LIBS})
install(TARGETS glad DESTINATION lib)
file(GLOB GLAD_HEADERS ${PROJECT_SOURCE_DIR}/glad/include/*/*.h)
install(FILES ${GLAD_HEADERS} DESTINATION include)

add_subdirectory(imgui)

#Stuff for Opengl
find_package( OpenGL REQUIRED )

#Set up to build against ROOT
#I learned how to do this here: https://root.cern.ch/how/integrate-root-my-project-cmake
list( APPEND CMAKE_PREFIX_PATH $ENV{ROOT_SYS})
find_package(ROOT REQUIRED COMPONENTS Core Geom Tree RIO TreePlayer EG)
include(${ROOT_USE_FILE})

#local stuff that GLDrawArea needs to know about
include_directories( "${PROJECT_SOURCE_DIR}" )

#Add plugins that depend on external libraries to these variables
set(EXTERNAL_SOURCE "")
set(EXTERNAL_HEADERS "")
set(EXTERNAL_LIBS "")

#Adding support for external plugins that need my EdepNeutrons package.
if(DEFINED EDEPNEUTRONS_DIR)
  list(APPEND EXTERNAL_SOURCE MCHitDrawer.cpp MCClusterDrawer.cpp)
  list(APPEND EXTERNAL_HEADERS MCHitDrawer.h MCClusterDrawer.h)
  list(APPEND EXTERNAL_LIBS persistency)

  #Find EDepNeutrons directory
  message("Building external plugins for EDepNeutrons with installation at path ${EDEPNEUTRONS_DIR}")
  include_directories( "${EDEPNEUTRONS_DIR}/src" )
  link_directories( "${EDEPNEUTRONS_DIR}/lib" )
endif()

add_subdirectory(config)
add_subdirectory(external)
add_subdirectory(util)
add_subdirectory(gl)
add_subdirectory(app)
add_subdirectory(plugins)

#Make the results of this build into a package.  Designed to be distributed as a .tar.gz
#Learned to do this from http://agateau.com/2009/cmake-and-make-dist/
set( CPACK_PACKAGE_VERSION_MAJOR "1" )
set( CPACK_PACKAGE_VERSION_MINOR "3" )
set( CPACK_PACKAGE_VERSION_PATCH "0" )
set( CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}" )
set( CPACK_SOURCE_GENERATOR "TGZ" )
#set( CPACK_SOURCE_IGNORE_FILES 

include( CPack )
add_custom_target( dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source )

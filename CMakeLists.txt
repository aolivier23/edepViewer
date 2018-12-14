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
add_definitions(-DINSTALL_YAML_DIR="${CMAKE_INSTALL_PREFIX}/conf/yaml")

#Find the EDepSim library that this package is designed to work with.  I will provide for 3 different strategies:
#1.) Look for the currently setup edep-sim.  This is not CMake's usual/default behavior as far as I can tell.  
#2.) Look for edep-sim that is "registered" via CMake.  I think this is how CMake usually looks for packages.  
#3.) Look for edep-sim without a CMake setup system.  v2.0.1 and older didn't have a Find*.cmake file.  

#First, look for currently setup edep-sim.  This should override all other cases/
if(DEFINED ENV{"$ENV{EDEP_ROOT}/$ENV{EDEP_TARGET}/lib"})
  find_package(EDepSim HINTS "$ENV{EDEP_ROOT}/$ENV{EDEP_TARGET}/lib" NO_DEFAULT_PATH)
else() 
  find_package(EDepSim) #If edep-sim is not set up, try to find it "the CMake way" as documented at 
                        #https://cmake.org/cmake/help/v3.0/command/find_package.html
endif()

if(EDepSim_FOUND) #If edep-sim is not new enough to have CMake support, set it up using EDEP_DIR
  add_definitions(-DEDEPSIM_FORCE_PRIVATE_FIELDS=true) #Use accessor functions instead of public fields so that Clark can change the format of 
                                                       #the io classes without breaking things like edepViewer
  set(EDepSimIO EDepSim::edepsim_io) 
  include_directories("${EDepSim_DIR}/include/EDepSim") 
  message("EDepSim_DIR is " ${EDepSim_DIR} )
else() #If I couldn't find edep-sim using either its' environment variables or CMake's default behavior, try the old edep-sim setup method
  #add_definitions(-DEDEPSIM_FORCE_PRIVATE_FIELDS=false) #This option wasn't available until after v2.0.1
  set(EDepSimIO edepsim)
  link_directories("$ENV{EDEP_ROOT}/$ENV{EDEP_TARGET}/lib")
  include_directories("$ENV{EDEP_ROOT}/$ENV{EDEP_TARGET}/include/edep-sim") 
  message("EDepSim_DIR is " $ENV{EDEP_ROOT}/$ENV{EDEP_TARGET} )
endif()

#YAML parser library for configuration
find_package( yaml-cpp REQUIRED )

#Support for glad
add_library(glad SHARED glad/src/glad.c)
target_include_directories("glad" PRIVATE "glad/include")
target_link_libraries("glad" ${CMAKE_DL_LIBS})
install(TARGETS glad DESTINATION lib)
file(GLOB GLAD_HEADERS ${PROJECT_SOURCE_DIR}/glad/include/*/*.h)
install(FILES ${GLAD_HEADERS} DESTINATION include)

add_subdirectory(imgui)

#Include imgui
include_directories( "${IMGUI_DIR}" )
link_directories( "${IMGUI_DIR}" )

#Stuff for Opengl
find_package( OpenGL REQUIRED )

#Set up to build against ROOT
#I learned how to do this here: https://root.cern.ch/how/integrate-root-my-project-cmake
list( APPEND CMAKE_PREFIX_PATH $ENV{ROOT_SYS})
find_package(ROOT REQUIRED COMPONENTS Core Geom Tree RIO TreePlayer EG ASImage) #ASImage to write png files
include(${ROOT_USE_FILE})

#local stuff that GLDrawArea needs to know about
include_directories( "${PROJECT_SOURCE_DIR}" )

#Add plugins that depend on external libraries to these variables
set(EXTERNAL_SOURCE "")
set(EXTERNAL_HEADERS "")
set(EXTERNAL_LIBS "")

#Adding support for external plugins that need my EdepNeutrons package.
if(DEFINED 3DSTNEUTRONS_DIR)
  list(APPEND EXTERNAL_SOURCE MCHitDrawer.cpp MCHitContrib.cpp MCClusterDrawer.cpp MCHitCamera.cpp ClustersByCand.cpp)
  list(APPEND EXTERNAL_HEADERS MCHitDrawer.h MCHitContrib.h MCClusterDrawer.h MCHitCamera.h ClustersByCand.h)
  list(APPEND EXTERNAL_LIBS persistency)

  #Find 3DSTNeutrons directory
  message("Building external plugins for 3DSTNeutrons with installation at path ${3DSTNEUTRONS_DIR}")
  include_directories( "${3DSTNEUTRONS_DIR}/include" )
  link_directories( "${3DSTNEUTRONS_DIR}/lib" )
endif()

add_subdirectory(config)
#add_subdirectory(external)
add_subdirectory(util)
add_subdirectory(gl)
add_subdirectory(app)
add_subdirectory(plugins)

#Make the results of this build into a package.  Designed to be distributed as a .tar.gz
#Learned to do this from http://agateau.com/2009/cmake-and-make-dist/
set( CPACK_PACKAGE_VERSION_MAJOR "2" )
set( CPACK_PACKAGE_VERSION_MINOR "0" )
set( CPACK_PACKAGE_VERSION_PATCH "0" )
set( CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}" )
set( CPACK_SOURCE_GENERATOR "TGZ" )
#set( CPACK_SOURCE_IGNORE_FILES 

include( CPack )
add_custom_target( dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source )

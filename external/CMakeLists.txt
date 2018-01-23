#gtkmm stuff
find_package(PkgConfig)
pkg_check_modules(GTKMM gtkmm-3.0)

#gtkmm stuff from https://wiki.gnome.org/Projects/gtkmm/UsingCMake
link_directories( ${GTKMM_LIBRARY_DIRS} )
include_directories( ${GTKMM_INCLUDE_DIRS} )

#XML parser library for configuration
find_package( tinyxml2 REQUIRED )

#Stuff for Opengl
find_package( OpenGL REQUIRED )

#Set up to build against ROOT
#I learned how to do this here: https://root.cern.ch/how/integrate-root-my-project-cmake
list( APPEND CMAKE_PREFIX_PATH $ENV{ROOT_SYS})
find_package(ROOT REQUIRED COMPONENTS Core Geom Tree RIO TreePlayer EG)
include(${ROOT_USE_FILE})

#Make sure system libraries can be found
link_directories( /usr/local/lib )

#Set ouput directory for executables
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../bin )

#Set up edep-sim
set(EDEP_DIR $ENV{EDEP_ROOT}/$ENV{EDEP_TARGET})
include_directories( "${EDEP_DIR}/include/edep-sim" )
link_directories( "${EDEP_DIR}/lib" )

#local stuff that GLDrawArea needs to know about
include_directories( "${PROJECT_SOURCE_DIR}" )

if(DEFINED EDEPNEUTRONS_DIR)
  #list(APPEND EXTERNAL_SOURCE MCHitDrawer.cpp)
  #list(APPEND EXTERNAL_LIBS persistency)
  
  #Add libraries of plugins
 # add_library( NeutronDrawers SHARED MCHitDrawer.cpp )
 # target_link_libraries( NeutronDrawers ${GTKMM_LIBRARIES} Scene ${ROOT_LIBRARIES} Color Drawable PolyMesh Point Path Grid Viewer 
 #                        Factory tinyxml2 Geometry Drawers )
 # install( TARGETS NeutronDrawers DESTINATION lib )
endif()

#Install all of the external Drawers as one target
add_library( external SHARED ${EXTERNAL_SOURCE} )
target_link_libraries( external ${GTKMM_LIBRARIES} Scene ${ROOT_LIBRARIES} Color Drawable PolyMesh Point Path Grid Viewer 
                       Factory tinyxml2 Geometry Drawers ${EXTERNAL_LIBS} )
install( TARGETS external DESTINATION lib )

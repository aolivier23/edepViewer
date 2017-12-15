#Make sure GLFW and friends can be found
link_directories( /usr/local/lib )

#Set ouput directory for executables
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../bin )

#Stuff for Opengl and GLEW
find_package( OpenGL REQUIRED )

#Set up to build against ROOT
#I learned how to do this here: https://root.cern.ch/how/integrate-root-my-project-cmake
list( APPEND CMAKE_PREFIX_PATH $ENV{ROOT_SYS})
find_package(ROOT REQUIRED COMPONENTS Core Geom RIO EG)
include(${ROOT_USE_FILE})

#gtkmm stuff from https://wiki.gnome.org/Projects/gtkmm/UsingCMake
link_directories( ${GTKMM_LIBRARY_DIRS} )
include_directories( ${GTKMM_INCLUDE_DIRS} )

#local stuff that GLDrawArea needs to know about
include_directories( "${PROJECT_SOURCE_DIR}" )

#Add libraries of plugins
add_library( Drawer SHARED Drawer.cpp )
target_link_libraries( Drawer ${GTKMM_LIBRARIES} Color Scene )
install( TARGETS Drawer DESTINATION lib )

add_library( Geo SHARED GeoDrawer.cpp )
target_link_libraries( Geo Drawer ${ROOT_LIBRARIES} Color Drawable PolyMesh Viewer Factory)
install( TARGETS Geo DESTINATION lib )

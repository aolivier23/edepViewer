#Make sure GLFW and friends can be found
link_directories( /usr/local/lib )

#Set ouput directory for executables
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../bin )

#Set up edep-sim
set(EDEP_DIR $ENV{EDEP_ROOT}/$ENV{EDEP_TARGET})
include_directories( "${EDEP_DIR}/include/edep-sim" )
link_directories( "${EDEP_DIR}/lib" )

message( "Getting edepsim from ${EDEP_DIR}" )

#gtkmm stuff from https://wiki.gnome.org/Projects/gtkmm/UsingCMake
link_directories( ${GTKMM_LIBRARY_DIRS} )
include_directories( ${GTKMM_INCLUDE_DIRS} )

#local stuff that GLDrawArea needs to know about
include_directories( "${PROJECT_SOURCE_DIR}" )

#Add libraries of plugins
add_library( Drawers SHARED GeoDrawer.cpp DefaultGeo.cpp Services.cpp EventDrawer.cpp
                            LinearTraj.cpp EDepDEdx.cpp EDepContributor.cpp Guides.cpp 
                            ExternalDrawer.cpp )
target_link_libraries( Drawers ${GTKMM_LIBRARIES} Scene ${ROOT_LIBRARIES} Color Drawable PolyMesh Point Path Grid Viewer 
                       Factory tinyxml2 Geometry edepsim )
install( TARGETS Drawers DESTINATION lib )

#Make sure GLFW and friends can be found
link_directories( /usr/local/lib )

#Set up edep-sim
#set(EDEP_DIR $ENV{EDEP_ROOT}/$ENV{EDEP_TARGET})
#include_directories( "${EDEP_DIR}/include/edep-sim" )
#link_directories( "${EDEP_DIR}/lib" )
#
#message( "Getting edepsim from ${EDEP_DIR}" )

#local stuff that GLDrawArea needs to know about
include_directories( "${PROJECT_SOURCE_DIR}" )

#Add libraries of plugins
add_library( GeoDrawers SHARED GeoController.cpp DefaultGeo.cpp Grids.cpp )
target_link_libraries( GeoDrawers Controller Scene ${ROOT_LIBRARIES} Services Drawable PolyMesh Grid Viewer 
                       Factory Geometry Row Tree )
install( TARGETS GeoDrawers DESTINATION lib )

#install headers
install( FILES GeoController.cpp DefaultGeo.h Grids.h DESTINATION include/plugins/drawing/geometry )

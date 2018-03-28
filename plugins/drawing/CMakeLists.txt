#Make sure GLFW and friends can be found
link_directories( /usr/local/lib )

#Set up edep-sim
set(EDEP_DIR $ENV{EDEP_ROOT}/$ENV{EDEP_TARGET})
include_directories( "${EDEP_DIR}/include/edep-sim" )
link_directories( "${EDEP_DIR}/lib" )

message( "Getting edepsim from ${EDEP_DIR}" )

#local stuff that GLDrawArea needs to know about
include_directories( "${PROJECT_SOURCE_DIR}" )

#Add libraries of plugins
add_library( Drawers SHARED GeoDrawer.cpp DefaultGeo.cpp Services.cpp EventDrawer.cpp
                            LinearTraj.cpp EDepDEdx.cpp EDepContributor.cpp Guides.cpp )
target_link_libraries( Drawers Scene ${ROOT_LIBRARIES} Color Drawable PolyMesh Point Path Grid Viewer 
                       Factory tinyxml2 Geometry edepsim imgui )
install( TARGETS Drawers DESTINATION lib )

#install headers
install( FILES DefaultGeo.h LinearTraj.h EDepDEdx.h EDepContributor.h Guides.h 
         DESTINATION include/plugins/drawing )

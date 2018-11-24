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
add_library( CameraConfig SHARED CameraConfig.cpp VertexCamera.cpp )
target_link_libraries( CameraConfig Services Scene ${ROOT_LIBRARIES} Viewer 
                       Factory Geometry ${EDepSimIO} )
install( TARGETS CameraConfig DESTINATION lib )

#install headers
install( FILES CameraConfig.cpp VertexCamera.h DESTINATION include/plugins/drawing/camera )

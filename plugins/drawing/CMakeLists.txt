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
add_library( Services SHARED Services.cpp )
target_link_libraries( Services ${ROOT_LIBRARIES} Color imgui )
install( TARGETS Services DESTINATION lib )

add_library( Controller SHARED Controller.cpp )
target_link_libraries( Controller )
install( TARGETS Controller DESTINATION lib)

#install headers
install( FILES Services.cpp Controller.cpp DESTINATION include/plugins/drawing )

add_subdirectory(geometry)
#add_subdirectory(event)
#add_subdirectory(camera)

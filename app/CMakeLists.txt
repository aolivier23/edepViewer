#Make sure GLFW and friends can be found
link_directories( /usr/local/lib )

#Include imgui
include_directories( "${IMGUI_DIR}" )
link_directories( "${IMGUI_DIR}" )

add_definitions(-DFONT_DIR="${IMGUI_DIR}/misc/fonts")

#local stuff that GLDrawArea needs to know about
include_directories( "${PROJECT_SOURCE_DIR}" )

#Libraries for final EDepSim visualization window
add_library( Source SHARED Source.cpp )
target_link_libraries( Source ${ROOT_LIBRARIES} ${EDepSimIO} )
install( TARGETS Source DESTINATION lib )

add_library( Window SHARED Window.cpp )
target_link_libraries( Window PUBLIC ${ROOT_LIBRARIES} ${OPENGL_LIBRARIES} ${EDepSimIO} yaml-cpp Geometry 
                       Source Node ${EXTERNAL_LIBS} Viewer EventDrawers GeoDrawers CameraConfig ) #external)
install( TARGETS Window DESTINATION lib )

add_subdirectory( states )

add_library( EvdController SHARED Controller.cpp )
target_link_libraries( EvdController PUBLIC States Window )
install( TARGETS EvdController DESTINATION lib )

add_executable( GLFWApp GLFWApp.cpp )
target_link_libraries( GLFWApp ${ROOT_LIBRARIES} EvdController States glfw imgui imgui_glfw ${EDepSimIO} )
install( TARGETS GLFWApp DESTINATION bin )

#install headers
install( FILES Source.h Window.h Controller.h DESTINATION include/app )

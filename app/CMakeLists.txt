#Make sure GLFW and friends can be found
link_directories( /usr/local/lib )

#Include imgui
include_directories( "${IMGUI_DIR}" )
link_directories( "${IMGUI_DIR}" )

add_definitions(-DFONT_DIR="${IMGUI_DIR}/misc/fonts")

#local stuff that GLDrawArea needs to know about
include_directories( "${PROJECT_SOURCE_DIR}" )

#Libraries for final EDepSim visualization window
add_library( GUI SHARED FileChoose.cpp )
target_link_libraries( GUI imgui )
install( TARGETS GUI DESTINATION lib )

add_library( Source SHARED Source.cpp )
target_link_libraries( Source ${ROOT_LIBRARIES} ${EDepSimIO} )
install( TARGETS Source DESTINATION lib )

add_library( EvdWindow SHARED EvdWindow.cpp )
target_link_libraries( EvdWindow PUBLIC ${ROOT_LIBRARIES} ${OPENGL_LIBRARIES} ${EDepSimIO} yaml-cpp GUI Geometry 
                       Source Node ${EXTERNAL_LIBS} Viewer EventDrawers GeoDrawers ) #external)
install( TARGETS EvdWindow DESTINATION lib )

add_library( CmdLine SHARED CmdLine.cpp )
target_link_libraries( CmdLine yaml-cpp Source )
install( TARGETS CmdLine DESTINATION lib )

add_executable( GLFWApp GLFWApp.cpp )
target_link_libraries( GLFWApp ${ROOT_LIBRARIES} EvdWindow glfw imgui imgui_glfw CmdLine ${EDepSimIO})

install( TARGETS GLFWApp DESTINATION bin )

#install headers
install( FILES Source.h EvdWindow.h FileChoose.h DESTINATION include/app )

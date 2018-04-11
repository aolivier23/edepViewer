#Make sure GLFW and friends can be found
link_directories( /usr/local/lib )

#Include imgui
include_directories( "${IMGUI_DIR}" )
link_directories( "${IMGUI_DIR}" )

add_definitions(-DFONT_DIR="${IMGUI_DIR}/misc/fonts")

#local stuff that GLDrawArea needs to know about
include_directories( "${PROJECT_SOURCE_DIR}" )

#Libraries for final EDepSim visualization window
add_library( GUI FileChoose.cpp )
target_link_libraries( GUI imgui )
install( TARGETS GUI DESTINATION lib )

add_library( Source Source.cpp )
target_link_libraries( Source LINK_PUBLIC ${ROOT_LIBRARIES} ${EDepSimIO} )
install( TARGETS Source DESTINATION lib )

add_library( EvdWindow EvdWindow.cpp )
target_link_libraries( EvdWindow LINK_PUBLIC ${ROOT_LIBRARIES} ${OPENGL_LIBRARIES} Viewer PolyMesh Path Grid Point
                                             ${EDepSimIO} Factory Drawers tinyxml2 GUI Geometry Source TreeModel ${EXTERNAL_LIBS} external)
install( TARGETS EvdWindow DESTINATION lib )

add_library( CmdLine CmdLine.cpp )
target_link_libraries( CmdLine LINK_PUBLIC tinyxml2 Source )
install( TARGETS CmdLine DESTINATION lib )

add_executable( GLFWApp GLFWApp.cpp )
target_link_libraries( GLFWApp LINK_PUBLIC ${ROOT_LIBRARIES} EvdWindow glfw imgui imgui_glfw TreeModel CmdLine ${EDepSimIO})

install( TARGETS GLFWApp DESTINATION bin )

#install headers
install( FILES Source.h EvdWindow.h FileChoose.h DESTINATION include/app )

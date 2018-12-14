#Include imgui
include_directories( "${IMGUI_DIR}" )
link_directories( "${IMGUI_DIR}" )

#Make sure GLFW and friends can be found
link_directories( /usr/local/lib )

#Build my own libraries for Viewer system
add_library( Selection VisID.cpp UserCut.cpp )
target_link_libraries( Selection exception Row)
install( TARGETS Selection DESTINATION lib )

install( FILES VisID.h UserCut.h DESTINATION include/gl/selection )

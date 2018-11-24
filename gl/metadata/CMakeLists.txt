#Include imgui
include_directories( "${IMGUI_DIR}" )
link_directories( "${IMGUI_DIR}" )

#Make sure GLFW and friends can be found
link_directories( /usr/local/lib )

#Build my own libraries for Viewer system
add_library( Row Data.cpp Column.cpp Row.cpp )
target_link_libraries( Row )
install( TARGETS Row DESTINATION lib )

add_library(Tree TreeNode.cpp)
target_link_libraries(Tree Row)
install(TARGETS Tree DESTINATION lib)

install( FILES Data.cpp Column.cpp Row.h TreeNode.cpp DESTINATION include/gl/tree )

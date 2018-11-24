#Include imgui
include_directories( "${IMGUI_DIR}" )
link_directories( "${IMGUI_DIR}" )

#Make sure GLFW and friends can be found
link_directories( /usr/local/lib )

add_library( Scene SceneConfig.cpp SceneModel.cpp SceneController.cpp )
target_link_libraries( Scene glad exception Row Node GLObjects Drawable Selection)
install( TARGETS Scene DESTINATION lib )

install( FILES SceneConfig.cpp SceneModel.cpp SceneController.h DESTINATION include/gl/scene )

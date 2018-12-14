#Include imgui
include_directories( "${IMGUI_DIR}" )
link_directories( "${IMGUI_DIR}" )

add_subdirectory(objects)
add_subdirectory(model)
add_subdirectory(camera)
add_subdirectory(shaders)
add_subdirectory(metadata)
add_subdirectory(selection)
add_subdirectory(scene)

#Make sure GLFW and friends can be found
link_directories( /usr/local/lib )

#Build my own libraries for Viewer system
add_library( Viewer Viewer.cpp )
target_link_libraries( Viewer Camera Scene ${OPENGL_LIBRARIES})
install( TARGETS Viewer DESTINATION lib )

install( FILES Viewer.h DESTINATION include/gl )

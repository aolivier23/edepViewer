#Include imgui
include_directories( "${IMGUI_DIR}" )
link_directories( "${IMGUI_DIR}" )

add_subdirectory(model)
add_subdirectory(camera)
add_subdirectory(shaders)
add_subdirectory(objects)

#Make sure GLFW and friends can be found
link_directories( /usr/local/lib )

#Build my own libraries for Viewer system
add_library( TreeModel TreeModel.cpp )
target_link_libraries( TreeModel )
install( TARGETS TreeModel DESTINATION lib )

add_library( Scene VisID.cpp ColRecord.cpp UserCut.cpp Scene.cpp )
target_link_libraries( Scene glad Drawable exception GLObjects TreeModel VAO)
install( TARGETS Scene DESTINATION lib )

add_library( Viewer Viewer.cpp )
target_link_libraries( Viewer Camera Scene ${OPENGL_LIBRARIES})
install( TARGETS Viewer DESTINATION lib )

install( FILES TreeModel.h Viewer.h VisID.h UserCut.h Scene.h DESTINATION include/gl )

add_subdirectory(model)
add_subdirectory(camera)

#Make sure GLFW and friends can be found
link_directories( /usr/local/lib )

#Build my own libraries for Viewer system
add_library( Scene VisID.cpp ColRecord.cpp UserCut.cpp Scene.cpp )
target_link_libraries( Scene glad Drawable exception Shader )
install( TARGETS Scene DESTINATION lib )

add_library( Viewer Viewer.cpp )
target_link_libraries( Viewer Camera Scene ${OPENGL_LIBRARIES})
install( TARGETS Viewer DESTINATION lib )

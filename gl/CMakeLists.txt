add_subdirectory(model)
add_subdirectory(camera)
add_subdirectory(shaders)

#Make sure GLFW and friends can be found
link_directories( /usr/local/lib )

#Build my own libraries for Viewer system
add_library( Scene VisID.cpp ColRecord.cpp UserCut.cpp Scene.cpp )
target_link_libraries( Scene glad Drawable exception Shader )
install( TARGETS Scene DESTINATION lib )

add_library( Viewer PuglArea.cpp Viewer.cpp )

#Attempt to build pugl from source tree
add_dependencies( Viewer pugl )

#message("Linking against the following pugl libraries: ${PUGL_LIBRARIES}")
message("Including pugl headers from ${PUGL_INCLUDE_DIR}")
target_link_libraries( Viewer Camera Scene ${OPENGL_LIBRARIES} ${PUGL_LIBRARIES})
install( TARGETS Viewer DESTINATION lib )

install( FILES PuglArea.h Viewer.h VisID.h UserCut.h Scene.h DESTINATION include/gl )

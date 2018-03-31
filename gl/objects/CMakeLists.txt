#Make sure GLFW and friends can be found
link_directories( /usr/local/lib )

#Build my own libraries for interacting with opengl
add_library( GLObjects Texture2D.cpp ShaderProg.cpp Framebuffer.cpp )
target_link_libraries( GLObjects ${OPENGL_LIBRARIES} )
install( TARGETS GLObjects DESTINATION lib )

install( FILES Texture2D.cpp enumToType.h ShaderProg.h Framebuffer.h DESTINATION include/gl/objects )

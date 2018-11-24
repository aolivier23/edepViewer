#Make sure GLFW and friends can be found
link_directories( /usr/local/lib )

#Build my own libraries for interacting with opengl
#add_library( GLObjects Texture2D.cpp ShaderProg.cpp Framebuffer.cpp )
add_library( GLObjects ShaderProg.cpp Framebuffer.cpp VAO.cpp ) #TODO: Restore Texture2D if/when I need it
target_link_libraries( GLObjects ${OPENGL_LIBRARIES} )
install( TARGETS GLObjects DESTINATION lib )

#TODO: Figure out how to structure enumToType
#install( FILES Texture2D.cpp enumToType.h ShaderProg.h Framebuffer.h DESTINATION include/gl/objects )
install( FILES ShaderProg.h Framebuffer.h VAO.h DESTINATION include/gl/objects )

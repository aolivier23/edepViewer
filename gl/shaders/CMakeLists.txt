#Install shaders in the configuration directory
FILE( GLOB Shaders *.frag *.vert *.geom )
install( FILES ${Shaders} DESTINATION conf/glsl )

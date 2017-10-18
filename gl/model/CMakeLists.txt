#Make sure GLFW and friends can be found
link_directories( /usr/local/lib )

#Stuff for Opengl and GLEW
find_package( OpenGL REQUIRED )
find_package( GLEW REQUIRED )

#gtkmm stuff from https://wiki.gnome.org/Projects/gtkmm/UsingCMake
link_directories( ${GTKMM_LIBRARY_DIRS} )
include_directories( ${GTKMM_INCLUDE_DIRS} )


#find_package( OpenGL REQUIRED )
#find_package( GLEW REQUIRED )

#include util directory for headers
#include_directories( "${PROJECT_SOURCE_DIR}/GtkGL" )

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../bin ) 

add_library( exception GenException.cpp )
add_library( Shader    ShaderProg.cpp )
add_library( Drawable  Drawable.cpp )
add_library( PolyMesh  PolyMesh.cpp )

target_link_libraries( exception )
target_link_libraries( Shader exception ${GTKMM_LIBRARIES} )
target_link_libraries( Drawable Shader )
target_link_libraries( PolyMesh Drawable )

#Make sure GLFW and friends can be found
link_directories( /usr/local/lib )

#Stuff for Opengl and GLEW
find_package( OpenGL REQUIRED )
find_package( GLEW REQUIRED )

#gtkmm stuff from https://wiki.gnome.org/Projects/gtkmm/UsingCMake
link_directories( ${GTKMM_LIBRARY_DIRS} )
include_directories( ${GTKMM_INCLUDE_DIRS} )

#Set up to build against ROOT
#I learned how to do this here: https://root.cern.ch/how/integrate-root-my-project-cmake
list( APPEND CMAKE_PREFIX_PATH $ENV{ROOT_SYS})
find_package(ROOT REQUIRED COMPONENTS Core Geom)
include(${ROOT_USE_FILE})

include_directories( "${PROJECT_SOURCE_DIR}" )

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../bin ) 

add_library( exception GenException.cpp )
add_library( Shader    ShaderProg.cpp )
add_library( Drawable  Drawable.cpp )
add_library( PolyMesh  PolyMesh.cpp )
add_library( Path      Path.cpp )
add_library( Grid      Grid.cpp )
add_library( Traits    NoView.cpp )

target_link_libraries( exception )
target_link_libraries( Shader exception ${GTKMM_LIBRARIES} ${GLEW_LIBRARIES} )
target_link_libraries( Drawable Shader ${GTKMM_LIBRARIES} )
target_link_libraries( PolyMesh Drawable ${ROOT_LIBRARIES})
target_link_libraries( Path Drawable )
target_link_libraries( Grid Drawable )
target_link_libraries( Traits Shader )

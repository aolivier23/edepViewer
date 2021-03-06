#Make sure GLFW and friends can be found
link_directories( /usr/local/lib )

#Stuff for Opengl and GLEW
find_package( OpenGL REQUIRED )

#Set up to build against ROOT
#I learned how to do this here: https://root.cern.ch/how/integrate-root-my-project-cmake
list( APPEND CMAKE_PREFIX_PATH $ENV{ROOT_SYS})
find_package(ROOT REQUIRED COMPONENTS Core Geom)
include(${ROOT_USE_FILE})

include_directories( "${PROJECT_SOURCE_DIR}" )

add_library( Drawable  Drawable.cpp )
add_library( PolyMesh  PolyMesh.cpp )
add_library( Path      Path.cpp )
add_library( Grid      Grid.cpp )
add_library( Point     Point.cpp )
add_library( Noop      Noop.cpp )

target_link_libraries( Drawable GLObjects )
target_link_libraries( PolyMesh Drawable ${ROOT_LIBRARIES})
target_link_libraries( Path Drawable )
target_link_libraries( Grid Drawable )
target_link_libraries( Point Drawable )
target_link_libraries( Noop Drawable )

install(TARGETS Drawable DESTINATION lib)
install(TARGETS PolyMesh DESTINATION lib)
install(TARGETS Path DESTINATION lib)
install(TARGETS Grid DESTINATION lib)
install(TARGETS Point DESTINATION lib)
install(TARGETS Noop DESTINATION lib)

install(FILES Drawable.h PolyMesh.h Path.h Grid.h Point.h Noop.h DESTINATION include/gl/model )

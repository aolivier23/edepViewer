#Make sure GLFW and friends can be found
link_directories( /usr/local/lib )

#Set up to build against ROOT
#I learned how to do this here: https://root.cern.ch/how/integrate-root-my-project-cmake
list( APPEND CMAKE_PREFIX_PATH $ENV{ROOT_SYS})
find_package(ROOT REQUIRED COMPONENTS Core Geom Tree RIO TreePlayer EG)
include(${ROOT_USE_FILE})

#local stuff that GLDrawArea needs to know about
include_directories( "${PROJECT_SOURCE_DIR}" )

add_library( exception GenException.cpp )
target_link_libraries( exception )
install( TARGETS exception DESTINATION lib )

add_library( Color ColorIter.cxx Palette.cpp PDGToColor.cpp )
target_link_libraries( Color ${ROOT_LIBRARIES} )
install( TARGETS Color DESTINATION lib )

add_library( Geometry Geometry.cpp )
target_link_libraries( Geometry  ${ROOT_LIBRARIES} )
install( TARGETS Geometry DESTINATION lib )

#install header(s)
install( FILES GenException.h PDGToColor.h DESTINATION include/util )

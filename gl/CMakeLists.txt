add_subdirectory(model)
add_subdirectory(camera)

#Make sure GLFW and friends can be found
link_directories( /usr/local/lib )

#Stuff for Opengl and GLEW
find_package( OpenGL REQUIRED )
find_package( GLEW REQUIRED )

#gtkmm stuff from https://wiki.gnome.org/Projects/gtkmm/UsingCMake
link_directories( ${GTKMM_LIBRARY_DIRS} )
include_directories( ${GTKMM_INCLUDE_DIRS} )

include_directories( "${PROJECT_SOURCE_DIR}" )

#Build my own libraries for Viewer system
add_library( Scene VisID.cpp ColRecord.cpp Scene.cpp )
target_link_libraries( Scene ${GLEW_LIBRARIES} Drawable exception Shader )
install( TARGETS Scene DESTINATION lib )

add_library( Viewer Viewer.cpp )
target_link_libraries( Viewer Camera Scene ${OPENGL_LIBRARIES})
install( TARGETS Viewer DESTINATION lib )

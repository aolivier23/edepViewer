#Make sure GLFW and friends can be found
link_directories( /usr/local/lib )

#Stuff for Opengl and GLEW
find_package( OpenGL REQUIRED )
find_package( GLEW 1.13.0 EXACT REQUIRED )

#gtkmm stuff from https://wiki.gnome.org/Projects/gtkmm/UsingCMake
link_directories( ${GTKMM_LIBRARY_DIRS} )
include_directories( ${GTKMM_INCLUDE_DIRS} )

#local stuff that GLDrawArea needs to know about
include_directories( "${PROJECT_SOURCE_DIR}" )


#Build my own libraries for old camera system
add_library( Camera GLCameraArea.cpp Vec3Entry.cpp Camera.cpp PerspCamera.cpp OrthoCamera.cpp FPSCam.cpp PlaneCam.cpp )
target_link_libraries( Camera ${GLEW_LIBRARIES} )
install( TARGETS Camera DESTINATION lib )

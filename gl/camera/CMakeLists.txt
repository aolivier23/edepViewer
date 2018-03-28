#Make sure GLFW and friends can be found
link_directories( /usr/local/lib )

#Include imgui
include_directories( "${IMGUI_DIR}" )
link_directories( "${IMGUI_DIR}" )

#Stuff for Opengl and GLEW
find_package( OpenGL REQUIRED )

#local stuff that GLDrawArea needs to know about
include_directories( "${PROJECT_SOURCE_DIR}" )

#Build my own libraries for old camera system
add_library( Camera Camera.cpp PerspCamera.cpp OrthoCamera.cpp FPSCam.cpp PlaneCam.cpp )
target_link_libraries( Camera glad )
install( TARGETS Camera DESTINATION lib )

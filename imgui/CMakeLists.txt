#Make sure GLFW and friends can be found
link_directories( /usr/local/lib )

#Include imgui
include_directories( "${IMGUI_DIR}" )
link_directories( "${IMGUI_DIR}" )

#Grab imgui from wherever it is and build it here.  Distributes a version of imgui with this project
add_library(imgui SHARED ${IMGUI_DIR}/imgui.cpp ${IMGUI_DIR}/imgui_draw.cpp)
target_link_libraries(imgui LINK_PUBLIC)
install( TARGETS imgui DESTINATION lib )

#local stuff that GLDrawArea needs to know about
include_directories( "${PROJECT_SOURCE_DIR}" )

add_library(imgui_glfw SHARED imgui_impl_glfw_gl3.cpp)
target_link_libraries(imgui_glfw imgui glfw)
install( TARGETS imgui_glfw DESTINATION lib )

#message("Linking RunEvdApp against external libraries ${EXTERNAL_LIBS}")
#add_executable( RunEvdApp RunEvdApp.cpp )
#target_link_libraries( RunEvdApp EvdApp ${EXTERNAL_LIBS})
#install( TARGETS RunEvdApp DESTINATION bin )

#install headers
install( FILES imgui_impl_glfw_gl3.h DESTINATION include )

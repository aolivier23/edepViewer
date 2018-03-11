#Make sure GLFW and friends can be found
link_directories( /usr/local/lib )

#Include imgui
include_directories( "${IMGUI_DIR}" )
link_directories( "${IMGUI_DIR}" )

#Set up edep-sim
set(EDEP_DIR $ENV{EDEP_ROOT}/$ENV{EDEP_TARGET})
include_directories( "${EDEP_DIR}/include/edep-sim" )
link_directories( "${EDEP_DIR}/lib" )

#Grab imgui from wherever it is and build it here.  Distributes a version of imgui with this project
#add_library(imgui ${IMGUI_DIR}/imgui.cpp)
#target_link_libraries(imgui LINK_PUBLIC)

#add_library(imgui_glfw ${IMGUI_DIR}/examples/opengl3_example/imgui_impl_glfw_gl3.cpp)
#target_link_libraries(imgui_glfw imgui glfw)

#Libraries for small GUIS
add_library( GUI LegendView.cpp )
target_link_libraries( GUI LINK_PUBLIC ${GTKMM_LIBRARIES} )

#local stuff that GLDrawArea needs to know about
include_directories( "${PROJECT_SOURCE_DIR}" )

#Let source code know where this project is installed.  Needed for finding shader sources.  
#TODO: Get shader sources from an environment variable instead.
add_definitions(-DINSTALL_GLSL_DIR="${CMAKE_INSTALL_PREFIX}/conf/glsl")
add_definitions(-DINSTALL_XML_DIR="${CMAKE_INSTALL_PREFIX}/conf/xml")

#Libraries for final EDepSim visualization window
add_library( Source Source.cpp )
target_link_libraries( Source LINK_PUBLIC ${ROOT_LIBRARIES} edepsim )
install( TARGETS Source DESTINATION lib )

add_library( EvdWindow EvdWindow.cpp )
target_link_libraries( EvdWindow LINK_PUBLIC ${GTKMM_LIBRARIES} ${ROOT_LIBRARIES} ${OPENGL_LIBRARIES} Viewer PolyMesh Path Grid Point
                                             edepsim Factory Drawers external tinyxml2 GUI Geometry Source )
install( TARGETS EvdWindow DESTINATION lib )

#add_library( EvdApp EvdApp.cpp )
#target_link_libraries( EvdApp LINK_PUBLIC ${GTKMM_LIBRARIES} ${ROOT_LIBRARIES} EvdWindow )
#install( TARGETS EvdApp DESTINATION lib )

add_executable( GLFWApp GLFWApp.cpp )
target_link_libraries( GLFWApp LINK_PUBLIC ${ROOT_LIBRARIES} EvdWindow ${EXTERNAL_LIBS} glfw imgui imgui_glfw)
install( TARGETS GLFWApp DESTINATION bin )

#message("Linking RunEvdApp against external libraries ${EXTERNAL_LIBS}")
#add_executable( RunEvdApp RunEvdApp.cpp )
#target_link_libraries( RunEvdApp EvdApp ${EXTERNAL_LIBS})
#install( TARGETS RunEvdApp DESTINATION bin )

#install headers
install( FILES Source.h EvdWindow.h EvdApp.h DESTINATION include/gtk )

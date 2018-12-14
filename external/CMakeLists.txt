#XML parser library for configuration
find_package( tinyxml2 REQUIRED )

#Stuff for Opengl
find_package( OpenGL REQUIRED )

#Find IMGUI
include_directories( "${IMGUI_DIR}" )
link_directories( "${IMGUI_DIR}" )

#Set up to build against ROOT
#I learned how to do this here: https://root.cern.ch/how/integrate-root-my-project-cmake
list( APPEND CMAKE_PREFIX_PATH $ENV{ROOT_SYS})
find_package(ROOT REQUIRED COMPONENTS Core Geom Tree RIO TreePlayer EG)
include(${ROOT_USE_FILE})

#Make sure system libraries can be found
link_directories( /usr/local/lib )

#local stuff that GLDrawArea needs to know about
include_directories( "${PROJECT_SOURCE_DIR}" )

#Install all of the external Drawers as one target
message( "From the external directory, EXTERNAL_LIBS is " ${EXTERNAL_LIBS} )
add_library( external SHARED ExternalDrawer.cpp ${EXTERNAL_SOURCE} )
target_link_libraries( external Scene ${ROOT_LIBRARIES} Color Drawable PolyMesh Point Path Grid Viewer 
                       Factory tinyxml2 Geometry Drawers ${EXTERNAL_LIBS} imgui ${EDepSimIO})
install( TARGETS external DESTINATION lib )

#install headers
install( FILES ${EXTERNAL_HEADERS} DESTINATION include/external )

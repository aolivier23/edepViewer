#Make sure GLFW and friends can be found
link_directories( /usr/local/lib )

#Set ouput directory for executables
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../bin )

#Stuff for Opengl and GLEW
find_package( OpenGL REQUIRED )
find_package( GLEW REQUIRED )

#Set up to build against ROOT
#I learned how to do this here: https://root.cern.ch/how/integrate-root-my-project-cmake
list( APPEND CMAKE_PREFIX_PATH $ENV{ROOT_SYS})
find_package(ROOT REQUIRED COMPONENTS Core Geom)
include(${ROOT_USE_FILE})

#gtkmm stuff from https://wiki.gnome.org/Projects/gtkmm/UsingCMake
link_directories( ${GTKMM_LIBRARY_DIRS} )
include_directories( ${GTKMM_INCLUDE_DIRS} )

#local stuff that GLDrawArea needs to know about
include_directories( "${PROJECT_SOURCE_DIR}" )

#Add libraries to test visualization with geometry
add_library( GeoDispWindow GeoDispWindow.cpp )
target_link_libraries( GeoDispWindow ${GTKMM_LIBRARIES} ${ROOT_LIBRARIES} )
install( TARGETS GeoDispWindow DESTINATION lib )

add_executable( GeoDisp GeoDisp.cpp )
target_link_libraries( GeoDisp GeoDispWindow )
install( TARGETS GeoDisp DESTINATION bin )

add_library( ColorIter ColorIter.cxx )
target_link_libraries( ColorIter ${ROOT_LIBRARIES} )
install( TARGETS ColorIter DESTINATION lib )

#Add libraries for drawing geometry based on list tree
add_library( GeoDrawWindow GeoDrawWindow.cpp )
target_link_libraries( GeoDrawWindow ${GTKMM_LIBRARIES} ${ROOT_LIBRARIES} ${OPENGL_LIBRARIES} Camera Drawable PolyMesh ColorIter )
install( TARGETS GeoDrawWindow DESTINATION lib )

add_executable( GeoDraw GeoDraw.cpp )
target_link_libraries( GeoDraw GeoDrawWindow )
install( TARGETS GeoDraw DESTINATION bin )

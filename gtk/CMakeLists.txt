#Make sure GLFW and friends can be found
link_directories( /usr/local/lib )

#Set ouput directory for executables
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../bin )

#Stuff for Opengl and GLEW
find_package( OpenGL REQUIRED )

#Set up to build against ROOT
#I learned how to do this here: https://root.cern.ch/how/integrate-root-my-project-cmake
list( APPEND CMAKE_PREFIX_PATH $ENV{ROOT_SYS})
find_package(ROOT REQUIRED COMPONENTS Core Geom Tree RIO TreePlayer EG)
include(${ROOT_USE_FILE})

#gtkmm stuff from https://wiki.gnome.org/Projects/gtkmm/UsingCMake
link_directories( ${GTKMM_LIBRARY_DIRS} )
include_directories( ${GTKMM_INCLUDE_DIRS} )

#Set up edep-sim
set(EDEP_DIR $ENV{EDEP_ROOT}/$ENV{EDEP_TARGET})
include_directories( "${EDEP_DIR}/include/edep-sim" )
link_directories( "${EDEP_DIR}/lib" )

#local stuff that GLDrawArea needs to know about
include_directories( "${PROJECT_SOURCE_DIR}" )

add_library( ColorIter ColorIter.cxx )
target_link_libraries( ColorIter ${ROOT_LIBRARIES} )
install( TARGETS ColorIter DESTINATION lib )

add_library( Palette Palette.cpp )
target_link_libraries( Palette )
install( TARGETS Palette DESTINATION lib )

#Let source code know where this project is installed.  Needed for finding shader sources.  
#TODO: Get shader sources from an environment variable instead.
add_definitions(-DINSTALL_GLSL_DIR="${PROJECT_SOURCE_DIR}/gl/shaders/")

#Libraries for final EDepSim visualization window
add_library( EvdWindow EvdWindow.cpp )
target_link_libraries( EvdWindow LINK_PUBLIC ${GTKMM_LIBRARIES} ${ROOT_LIBRARIES} ${OPENGL_LIBRARIES} Viewer PolyMesh Path Grid Point
                                             ColorIter Palette edepsim)
install( TARGETS EvdWindow DESTINATION lib )

add_executable( EvdDraw EvdDraw.cpp )
target_link_libraries( EvdDraw EvdWindow )
install( TARGETS EvdDraw DESTINATION bin )

#Make sure GLFW and friends can be found
link_directories( /usr/local/lib )

#Include imgui
include_directories( "${IMGUI_DIR}" )
link_directories( "${IMGUI_DIR}" )

add_definitions(-DFONT_DIR="${IMGUI_DIR}/misc/fonts")

#local stuff that GLDrawArea needs to know about
include_directories( "${PROJECT_SOURCE_DIR}" )

#Libraries for final EDepSim visualization window
add_library( States SHARED State.cpp ChooseFile.cpp FirstEvent.cpp Goto.cpp NewFile.cpp 
                           NonSequential.cpp Running.cpp TryLoadNextEvent.cpp )
target_link_libraries( States Window imgui )
install( TARGETS States DESTINATION lib )
install( FILES State.h ChooseFile.cpp Goto.h FirstEvent.h NewFile.h 
               NonSequential.h Running.h TryLoadNextEvent.h DESTINATION include/app/states )

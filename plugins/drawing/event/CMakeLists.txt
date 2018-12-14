#Make sure GLFW and friends can be found
link_directories( /usr/local/lib )

#local stuff that GLDrawArea needs to know about
include_directories( "${PROJECT_SOURCE_DIR}" )

#Add libraries of plugins
add_library( EventDrawers SHARED EventController.cpp LinearTraj.cpp EDepDEdx.cpp EDepContributor.cpp TrajPts.cpp )
target_link_libraries( EventDrawers Controller Services Scene ${ROOT_LIBRARIES} Color Drawable PolyMesh Point Path Grid Viewer Noop
                       Factory ${EDepSimIO} )
install( TARGETS EventDrawers DESTINATION lib )

#install headers
install( FILES EventController.cpp LinearTraj.h EDepDEdx.h EDepContributor.h TrajPts.h DESTINATION include/plugins/drawing/event )

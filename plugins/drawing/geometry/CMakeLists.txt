#Make sure GLFW and friends can be found
link_directories( /usr/local/lib )

#local stuff that GLDrawArea needs to know about
include_directories( "${PROJECT_SOURCE_DIR}" )

#Add libraries of plugins
add_library( GeoDrawers SHARED GeoController.cpp DefaultGeo.cpp Grids.cpp )
target_link_libraries( GeoDrawers Controller Scene ${ROOT_LIBRARIES} Services Drawable PolyMesh Grid Viewer 
                       Factory Geometry Row Tree )
install( TARGETS GeoDrawers DESTINATION lib )

#install headers
install( FILES GeoController.cpp DefaultGeo.h Grids.h DESTINATION include/plugins/drawing/geometry )

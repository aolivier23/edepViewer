include_directories( "${PROJECT_SOURCE_DIR}" )

add_library(Factory SHARED Factory.cpp)
#TODO: Link against configuration system
#target_link_libraries(Factory )
install(TARGETS Factory DESTINATION lib)

add_subdirectory(drawing)

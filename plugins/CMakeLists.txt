include_directories( "${PROJECT_SOURCE_DIR}" )

add_library(Factory SHARED Factory.cpp)
target_link_libraries(Factory tinyxml2)
install(TARGETS Factory DESTINATION lib)

add_subdirectory(drawing)

#Install pugl using waf (unfortunately) since I really don't want to deal with objective c by hand. 
file(GLOB PUGL_SRC pugl/*.c pugl/*.m)

set(PUGL_LIBRARIES ${CMAKE_BINARY_DIR}/lib/libpugl-0.so
                   ${CMAKE_BINARY_DIR}/lib/libpugl-0.so.0
                   ${CMAKE_BINARY_DIR}/lib/libpugl-0.so.0.2.0)

add_custom_command(OUTPUT ${PUGL_LIBRARIES}
                   WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/pugl"
                   COMMAND ${PROJECT_SOURCE_DIR}/pugl/waf configure --prefix ${CMAKE_INSTALL_PREFIX} -o ${CMAKE_BINARY_DIR} --no-cairo
                   COMMAND ${PROJECT_SOURCE_DIR}/pugl/waf
                   COMMAND ${PROJECT_SOURCE_DIR}/pugl/waf install
                   DEPENDS "${PUGL_SRC}"
                   COMMENT "Building pugl with waf\n"
                   VERBATIM)

add_custom_target(pugl DEPENDS ${PUGL_LIBRARIES})

#waf does this for me?
#install(TARGETS pugl DESTINATION lib)
#install(FILES ${PUGL_LIBRARIES} DESTINATION lib)
#install(FILES ${PUGL_HEADERS} DESTINATION include/pugl)

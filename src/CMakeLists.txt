# TODO
add_subdirectory(${PROJECT_NAME})

add_executable(${PROJECT_NAME}-print-version
  print-version.cpp
)

target_link_libraries(${PROJECT_NAME}-print-version
  PRIVATE ${PROJECT_NAME}::${PROJECT_NAME}
)

install(TARGETS ${PROJECT_NAME}-print-version
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

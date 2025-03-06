# TODO
add_subdirectory(${PROJECT_NAME})

add_executable(${PROJECT_NAME}-print-config
  print-config.cpp
)

target_link_libraries(${PROJECT_NAME}-print-config
  PRIVATE ${PROJECT_NAME}::${PROJECT_NAME}
)

install(TARGETS ${PROJECT_NAME}-print-config
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

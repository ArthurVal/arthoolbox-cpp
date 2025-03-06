add_library(${PROJECT_NAME} INTERFACE)

add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
  # All symbols are NO_EXPORT by default
  CXX_VISIBILITY_PRESET hidden

  # Add the '-debug' when compiled in CMAKE_BUILD_TYPE=DEBUG
  DEBUG_POSTFIX "-debug"

  # Name used when exporting (otherwise would be named ${PROJECT_NAME}-toto)
  # EXPORT_NAME "${PROJECT_NAME}"

  # Version stuff for the export lib names, that handles symlinks
  # like:
  # libtoto.so -> libtoto.1.so -> libtoto.1.0.2.so
  # VERSION ${PROJECT_VERSION}
  # SOVERSION ${PROJECT_VERSION_MAJOR}

  INTERFACE_${PROJECT_NAME}_VERSION ${PROJECT_VERSION}
  COMPATIBLE_INTERFACE_STRING ${PROJECT_VERSION_MAJOR}
)

# Automatically create the export rules
# generate_export_header(${PROJECT_NAME}
#   EXPORT_FILE_NAME "${CMAKE_BINARY_DIR}/include/${PROJECT_NAME}/export.h"
# )

# target_compile_definitions(${PROJECT_NAME}
#   PRIVATE
#   $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,STATIC_LIBRARY>:-DATB_CPP_STATIC_DEFINE>
# )

target_include_directories(${PROJECT_NAME}
  INTERFACE
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>

  # PRIVATE
  # $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  # $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

# target_link_libraries(${PROJECT_NAME}

# )

target_compile_features(${PROJECT_NAME}
  INTERFACE cxx_std_17
)

# target_compile_options(${PROJECT_NAME}
#   PRIVATE
#   -Wall
#   -Wextra
#   -Wpedantic
#   -Wshadow
#   -Wdouble-promotion
#   -Wundef
#   -Wconversion
#   # -Werror
# )

install(TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}-Core
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT ${PROJECT_NAME}-Core
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

# .rst:
# ----- utils.cmake -----
# This module provide global utils functionallity to the CMake project. All the
# following elements provided are prefixed by 'utils_'.
#
# ~~~
# Variables:
#   * n/a
# Functions:
#   * disable_in_source_builds([ONLY_LOG <WARNING;STATUS>])
# Macros:
#   * n/a
# ~~~

# ~~~
# Disables in-source builds (i.e. when SOURCE_DIR == BINARY_DIR when doing, for
# example, 'cmake .' or 'cmake -S . -B .'.
#
# Arguments:
#   * ONLY_LOG <WARNING, STATUS> (optional - in):
#     Use message(${ONLY_LOG} ...) instead of FATAL_ERROR.
# ~~~
function(utils_disable_in_source_builds)
  cmake_parse_arguments(
    args
    ""         # <- Flags
    "ONLY_LOG" # <- One value
    ""         # <- Multi values
    ${ARGN}
  )

  list(APPEND ALLOWED_LOG WARNING STATUS)
  if(args_ONLY_LOG AND NOT args_ONLY_LOG IN_LIST ALLOWED_LOG)
    list(JOIN ALLOWED_LOG " \n - " error_msg)

    string(PREPEND error_msg
      " Unknown ONLY_LOG argument \"${args_ONLY_LOG}\".\n"
      " Expecting one of:\n"
      " - "
    )

    message(FATAL_ERROR ${error_msg})
  endif()

  # make sure the user doesn't play dirty with simlinks
  get_filename_component(SRC_DIR "${CMAKE_SOURCE_DIR}" REALPATH)
  get_filename_component(BIN_DIR "${CMAKE_BINARY_DIR}" REALPATH)

  if("${SRC_DIR}" STREQUAL "${BIN_DIR}")
    if(args_ONLY_LOG)
      message(
        ${args_ONLY_LOG}
        " In-source build detected.\n"
        " This will pollute your sources with build artifacts.\n"
        " We highly recommend making an independant build/ directory using:\n"
        " cmake -B <SOURCE_DIR>/build/ -S <SOURCE_DIR>\n")
    else()
      message(
        FATAL_ERROR
          " In-source build is not allowed.\n"
          " Please make an independant build directory using:\n"
          " cmake -B <BUILD_DIR> -S <SOURCE_DIR>\n"
          " \n"
          " Feel free to:\n"
          " rm ${SRC_DIR}/CMakeCache.txt && rm -rf ${SRC_DIR}/CMakeFiles/*\n")
    endif()
  endif()
endfunction()

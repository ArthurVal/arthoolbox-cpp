###############################################################################
#                                TESTS OPTIONS                                #
###############################################################################
set(${PROJECT_NAME}_TESTING_GTEST_URL
  "https://github.com/google/googletest/archive/v1.14.0.zip"
  CACHE STRING
  "Points towards the googletest.zip URL that will be fetch, if GTest is not installed on the system."
  )
cmake_print_variables(${PROJECT_NAME}_TESTING_GTEST_URL)

find_package(GTest)
if(NOT GTest_FOUND)
  message(STATUS "Trying to fetch it from URL \"${${PROJECT_NAME}_TESTING_GTEST_URL}\":...")
  set(INSTALL_GTEST OFF)

  include(FetchContent)
  FetchContent_Declare(googletest
    URL ${${PROJECT_NAME}_TESTING_GTEST_URL}
    DOWNLOAD_EXTRACT_TIMESTAMP
    # FIND_PACKAGE_ARGS NAMES GTest
  )
  FetchContent_MakeAvailable(googletest)
  message(STATUS "Trying to fetch it from URL \"${${PROJECT_NAME}_TESTING_GTEST_URL}\": DONE")
endif()

include(GoogleTest)             # Add gtest_discover_tests()
add_subdirectory(utils)
add_subdirectory(${PROJECT_NAME})

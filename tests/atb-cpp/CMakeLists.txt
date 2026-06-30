add_executable(tests-${PROJECT_NAME}
  test_integer_sequence.cpp
  test_tuple.cpp
  test_matchers.cpp
  test_string.cpp
)

target_link_libraries(tests-${PROJECT_NAME}
  PRIVATE ${PROJECT_NAME}::${PROJECT_NAME}
  PRIVATE ${PROJECT_NAME}-tests-utils
  PRIVATE GTest::gtest_main
  PRIVATE GTest::gmock_main
)

utils_append_default_warnings_to(tests-${PROJECT_NAME}-WARNINGS)

target_compile_options(tests-${PROJECT_NAME}
  PRIVATE
  ${tests-${PROJECT_NAME}-WARNINGS}
)

gtest_discover_tests(tests-${PROJECT_NAME})

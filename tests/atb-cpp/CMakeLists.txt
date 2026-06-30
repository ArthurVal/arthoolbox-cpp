add_executable(tests-${PROJECT_NAME}
  test_integer_sequence.cpp
  test_tuple.cpp
  test_matchers.cpp
)

target_link_libraries(tests-${PROJECT_NAME}
  PRIVATE ${PROJECT_NAME}::${PROJECT_NAME}
  PRIVATE ${PROJECT_NAME}-tests-utils
  PRIVATE GTest::gtest_main
  PRIVATE GTest::gmock_main
)

target_compile_options(tests-${PROJECT_NAME}
  PRIVATE
  -Wall
  -Wextra
  -Wpedantic
  -Wshadow
  -Wdouble-promotion
  -Wundef
  -Wconversion
  # -Werror
)

gtest_discover_tests(tests-${PROJECT_NAME})

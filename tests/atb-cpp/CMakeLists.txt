add_executable(tests-${PROJECT_NAME}
  test_integer_sequence.cpp
)

target_link_libraries(tests-${PROJECT_NAME}
  PRIVATE ${PROJECT_NAME}::${PROJECT_NAME}
  PRIVATE ${PROJECT_NAME}-tests-utils
  PRIVATE GTest::gtest_main
)

gtest_discover_tests(tests-${PROJECT_NAME})

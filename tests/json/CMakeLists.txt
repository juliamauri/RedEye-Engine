add_executable(
  json_test
  json_test.cpp
)

target_link_libraries(json_test PRIVATE
  RedEye_lib
  GTest::gtest
  GTest::gtest_main
  GTest::gmock
  GTest::gmock_main
)

add_test(json json_test)
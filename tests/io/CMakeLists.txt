add_executable(test_io test_runner.c test_image_loader.c)
target_link_libraries(test_io printAsciiLib test_helpers m)
target_compile_definitions(test_io PRIVATE TEST_IMAGES_DIR="${CMAKE_SOURCE_DIR}/tests/images")
add_test(NAME io_tests COMMAND test_io)
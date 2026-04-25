add_executable(test_video_cli test_video_cli.c)
target_compile_definitions(test_video_cli PRIVATE
	PRINTASCII_BIN_PATH="${CMAKE_BINARY_DIR}/src/printAscii"
)
add_test(NAME integration_video_cli COMMAND test_video_cli)

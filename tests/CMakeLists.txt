# Create test_helpers library
add_library(test_helpers STATIC test_helpers.c)
target_include_directories(test_helpers PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(test_helpers PUBLIC printAsciiLib)

# Copy image files to build directory
file(COPY ${CMAKE_SOURCE_DIR}/tests/images DESTINATION ${CMAKE_BINARY_DIR}/tests)

# Add test subsystems
add_subdirectory(core)
add_subdirectory(io)
add_subdirectory(integration)
add_subdirectory(conversion)
add_subdirectory(preprocessing)
add_subdirectory(utilities)
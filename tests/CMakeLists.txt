# Note: the test targets deliberately do NOT link tiny-stl-warnings. The header
# under test is compiled as part of every test TU, so -Werror there turns any
# warning in the implementation into a build failure and hides the test results.

add_executable(test_unique_ptr test_unique_ptr.cpp)
target_link_libraries(test_unique_ptr PRIVATE tiny-stl::tiny-stl tiny-stl-sanitizers)
target_include_directories(test_unique_ptr PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME unique_ptr COMMAND test_unique_ptr)

add_executable(test_split ext/test_split.cpp)
target_link_libraries(test_split PRIVATE tiny-stl::tiny-stl tiny-stl-sanitizers)
target_include_directories(test_split PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME split COMMAND test_split)

# The custom-deleter suite is excluded from the default build so that a
# compilation failure in that half does not stop the other suite from running.
# ctest builds it on demand, and reports a compile error as a failing test.
add_executable(test_unique_ptr_deleter EXCLUDE_FROM_ALL test_unique_ptr_deleter.cpp)
target_link_libraries(test_unique_ptr_deleter PRIVATE tiny-stl::tiny-stl tiny-stl-sanitizers)
target_include_directories(test_unique_ptr_deleter PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_test(NAME unique_ptr_custom_deleter_build
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}
            --target test_unique_ptr_deleter --config $<CONFIG>
)
add_test(NAME unique_ptr_custom_deleter COMMAND test_unique_ptr_deleter)
set_tests_properties(unique_ptr_custom_deleter PROPERTIES DEPENDS unique_ptr_custom_deleter_build)

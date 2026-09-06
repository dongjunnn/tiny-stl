cmake_minimum_required(VERSION 3.14)

project(tiny-stl
    VERSION 0.1.0
    LANGUAGES CXX
)

add_library(tiny-stl INTERFACE)
add_library(tiny-stl::tiny-stl ALIAS tiny-stl)

target_include_directories(tiny-stl INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(tiny-stl INTERFACE cxx_std_20)

# ---------------------------------------------------------------------------
# Project-only options: warnings-as-errors and sanitizers.
# These are intentionally NOT attached to the tiny-stl interface target so
# that consumers of the library aren't forced to build with them.
# ---------------------------------------------------------------------------

add_library(tiny-stl-warnings INTERFACE)

if(MSVC)
    target_compile_options(tiny-stl-warnings INTERFACE /W4 /WX)
else()
    target_compile_options(tiny-stl-warnings INTERFACE -Wall -Wextra -Wpedantic -Werror)
endif()

add_library(tiny-stl-sanitizers INTERFACE)

option(TINY_STL_ENABLE_SANITIZERS "Build with address/undefined/leak sanitizers" ${PROJECT_IS_TOP_LEVEL})

if(TINY_STL_ENABLE_SANITIZERS AND NOT MSVC)
    target_compile_options(tiny-stl-sanitizers INTERFACE
        -fsanitize=address,undefined,leak
        -fno-omit-frame-pointer
    )
    target_link_options(tiny-stl-sanitizers INTERFACE
        -fsanitize=address,undefined,leak
    )
endif()

option(TINY_STL_BUILD_TESTS "Build tiny-stl tests" ${PROJECT_IS_TOP_LEVEL})

if(TINY_STL_BUILD_TESTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
    enable_testing()
    add_subdirectory(tests)
endif()

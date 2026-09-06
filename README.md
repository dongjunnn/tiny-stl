# tiny-stl

Re-implementations of C++ standard library containers and smart pointers, written from
scratch to understand how they work. Header-only, C++20.

Each type is built alongside a test suite that pins down its observable behaviour —
ownership transfer, exception safety, when destructors actually run — so the tests double
as an executable spec.

## Contents

| Header                 | Status      |
| ---------------------- | ----------- |
| `tiny/unique_ptr.h`    | in progress |

## Build & test

Requires CMake 3.14+ and a C++20 compiler.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

Building this repo directly turns on the address and undefined-behaviour sanitizers
(`-DTINY_STL_ENABLE_SANITIZERS=OFF` to opt out). They stay off when tiny-stl is consumed as
a subproject.

`ctest` runs every suite. Individual suites can be run directly for their numbered
pass/fail list:

```bash
./build/tests/test_unique_ptr          # unique_ptr with the default deleter

# The custom-deleter suite is excluded from the default build, so that a compile
# error there cannot stop the other suites from running. ctest builds it on demand;
# to run it on its own, build the target first:
cmake --build build --target test_unique_ptr_deleter
./build/tests/test_unique_ptr_deleter
```

## Using it

The library is header-only, so consuming it is just an include path:

```cmake
add_subdirectory(tiny-stl)
target_link_libraries(your_target PRIVATE tiny-stl::tiny-stl)
```

## Tests

Tests use a small in-repo harness (`tests/test_harness.h`) rather than a framework, to keep
the dependency list at zero. Each case is registered with a description and runs in its own
forked process, so a segfault or sanitizer abort is reported as a single failed case instead
of ending the run:

```cpp
TINY_TEST("release(): returns the pointer and gives up ownership") {
    Tracker* raw = new Tracker(8);
    unique_ptr<Tracker> p(raw);
    TINY_CHECK_EQ(p.release(), raw);
    TINY_CHECK_EQ(p.get(), nullptr);
    delete raw;
}
```

To add a suite, drop a `.cpp` file in `tests/`, end it with
`int main() { return tinytest::run_all("suite name"); }`, and register it in
`tests/CMakeLists.txt`.

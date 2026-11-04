# tiny-stl

Header-only C++20 re-implementations of standard library pieces, written from scratch to
understand how they work. Each one has a test suite that doubles as an executable spec.

| Header                     | Status      |
| -------------------------- | ----------- |
| `tiny/unique_ptr.h`        | in progress |
| `tiny/ext/string_parser.h` | in progress |

## Build and test

Requires CMake 3.14+ and a C++20 compiler.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

Address and undefined-behaviour sanitizers are on by default
(`-DTINY_STL_ENABLE_SANITIZERS=OFF` to disable).

## Running one suite

```bash
ctest --test-dir build -R split -V     # via ctest; -V shows each case
./build/tests/test_unique_ptr          # or run the binary directly
./build/tests/test_split
```

ctest reports one result per suite, not per case. The custom-deleter suite is excluded
from the default build so a compile error there can't block the others. ctest builds it
on demand; to run it by hand:

```bash
cmake --build build --target test_unique_ptr_deleter
./build/tests/test_unique_ptr_deleter
```

## Using it

```cmake
add_subdirectory(tiny-stl)
target_link_libraries(your_target PRIVATE tiny-stl::tiny-stl)
```

## Writing tests

Tests use a small in-repo harness (`tests/test_harness.h`), no external framework. Each
case runs in its own forked process, so a crash is reported as one failed case instead of
ending the run.

```cpp
TINY_TEST("release(): returns the pointer and gives up ownership") {
    Tracker* raw = new Tracker(8);
    unique_ptr<Tracker> p(raw);
    TINY_CHECK_EQ(p.release(), raw);
    TINY_CHECK_EQ(p.get(), nullptr);
    delete raw;
}
```

To add a suite: create a `.cpp` in `tests/`, end it with
`int main() { return tinytest::run_all("suite name"); }`, and register it in
`tests/CMakeLists.txt`.

#pragma once

// A tiny "submission platform" style test harness: every registered case gets a
// number, runs independently, and prints PASS/FAIL with the failing expectation.
//
// On POSIX each case runs in a forked child, so a case that crashes (segfault,
// double free, sanitizer abort) is reported as a failure instead of taking the
// whole run down with it.

#include <cstdio>
#include <cstring>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#if defined(__unix__) || defined(__APPLE__)
#define TINY_TEST_FORK 1
#include <sys/wait.h>
#include <unistd.h>
#else
#define TINY_TEST_FORK 0
#endif

// Leaks are asserted explicitly by the test cases themselves (via destruction
// counters), so LeakSanitizer's process-exit report is turned off to keep the
// result list readable.
extern "C" inline const char* __lsan_default_options() { return "detect_leaks=0"; }

namespace tinytest {

struct Failure {
    const char* file;
    int line;
    std::string message;
};

struct TestCase {
    const char* name;
    void (*fn)();
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> cases;
    return cases;
}

inline std::vector<Failure>& failures() {
    static std::vector<Failure> f;
    return f;
}

struct Registrar {
    Registrar(const char* name, void (*fn)()) { registry().push_back(TestCase{name, fn}); }
};

inline void record(const char* file, int line, std::string message) {
    failures().push_back(Failure{file, line, std::move(message)});
}

template <typename T>
std::string show(const T& value) {
    std::ostringstream os;
    os << value;
    return os.str();
}

inline std::string show(bool value) { return value ? "true" : "false"; }
inline std::string show(std::nullptr_t) { return "nullptr"; }

inline const char* basename_of(const char* path) {
    const char* last = path;
    for (const char* p = path; *p != '\0'; ++p) {
        if (*p == '/' || *p == '\\') last = p + 1;
    }
    return last;
}

inline std::string collect_details() {
    std::string out;
    for (const Failure& f : failures()) {
        out += basename_of(f.file);
        out += ':';
        out += std::to_string(f.line);
        out += ": ";
        out += f.message;
        out += '\n';
    }
    return out;
}

// Indents every line of captured output so it sits under its test case.
inline void print_details(const std::string& text) {
    std::size_t pos = 0;
    while (pos < text.size()) {
        std::size_t end = text.find('\n', pos);
        if (end == std::string::npos) end = text.size();
        if (end > pos) {
            std::printf("             %.*s\n", static_cast<int>(end - pos), text.data() + pos);
        }
        pos = end + 1;
    }
}

// Runs one case; returns true on pass and fills `details` with failure text.
inline bool run_case(const TestCase& test, std::string& details) {
#if TINY_TEST_FORK
    int fds[2];
    if (::pipe(fds) != 0) {
        details = "harness: pipe() failed\n";
        return false;
    }

    std::fflush(stdout);
    const pid_t pid = ::fork();
    if (pid == 0) {
        ::close(fds[0]);
        // Fold the child's stderr into the pipe so sanitizer diagnostics are
        // reported under the test case that triggered them.
        ::dup2(fds[1], STDERR_FILENO);
        failures().clear();
        test.fn();
        const std::string out = collect_details();
        if (!out.empty()) {
            const ssize_t ignored = ::write(fds[1], out.data(), out.size());
            (void)ignored;
        }
        ::close(fds[1]);
        ::_exit(failures().empty() ? 0 : 1);
    }

    ::close(fds[1]);
    details.clear();
    char buffer[512];
    for (;;) {
        const ssize_t n = ::read(fds[0], buffer, sizeof(buffer));
        if (n <= 0) break;
        details.append(buffer, static_cast<std::size_t>(n));
    }
    ::close(fds[0]);

    int status = 0;
    ::waitpid(pid, &status, 0);

    if (WIFSIGNALED(status)) {
        details += "harness: case crashed (signal ";
        details += std::to_string(WTERMSIG(status));
        details += " - ";
        details += ::strsignal(WTERMSIG(status));
        details += ")\n";
        return false;
    }
    if (!WIFEXITED(status)) {
        details += "harness: case terminated abnormally\n";
        return false;
    }
    const int code = WEXITSTATUS(status);
    if (code == 0) return true;
    if (code != 1) {
        details += "harness: case exited with code " + std::to_string(code) + "\n";
    } else if (details.empty()) {
        details += "harness: case aborted before it could report a result\n";
    }
    return false;
#else
    failures().clear();
    test.fn();
    details = collect_details();
    return failures().empty();
#endif
}

inline int run_all(const char* suite) {
    const std::vector<TestCase>& cases = registry();
    std::printf("=== %s : %zu test cases ===\n\n", suite, cases.size());

    std::vector<std::size_t> failed;
    for (std::size_t i = 0; i < cases.size(); ++i) {
        std::string details;
        const bool ok = run_case(cases[i], details);
        std::printf("Test %2zu ... %s  %s\n", i + 1, ok ? "PASS" : "FAIL", cases[i].name);
        if (!ok) {
            failed.push_back(i + 1);
            print_details(details);
        }
    }

    std::printf("\n----------------------------------------------------------\n");
    std::printf("Passed: %zu/%zu   Failed: %zu\n",
                cases.size() - failed.size(), cases.size(), failed.size());
    if (!failed.empty()) {
        std::printf("Failed cases:");
        for (std::size_t n : failed) std::printf(" %zu", n);
        std::printf("\n");
    }
    std::fflush(stdout);
    return failed.empty() ? 0 : 1;
}

}  // namespace tinytest

#define TINY_CAT_(a, b) a##b
#define TINY_CAT(a, b) TINY_CAT_(a, b)

#define TINY_TEST(description)                                                    \
    static void TINY_CAT(tiny_test_fn_, __LINE__)();                              \
    static const ::tinytest::Registrar TINY_CAT(tiny_test_reg_, __LINE__)(        \
        description, &TINY_CAT(tiny_test_fn_, __LINE__));                         \
    static void TINY_CAT(tiny_test_fn_, __LINE__)()

#define TINY_CHECK(cond)                                                          \
    do {                                                                          \
        if (!(cond)) {                                                            \
            ::tinytest::record(__FILE__, __LINE__,                                \
                               std::string("expected: ") + #cond);                \
        }                                                                         \
    } while (false)

#define TINY_CHECK_EQ(actual, expected)                                           \
    do {                                                                          \
        auto&& tiny_a_ = (actual);                                                \
        auto&& tiny_e_ = (expected);                                              \
        if (!(tiny_a_ == tiny_e_)) {                                              \
            ::tinytest::record(__FILE__, __LINE__,                                \
                               std::string(#actual) + " == " + #expected +        \
                                   "  (actual: " + ::tinytest::show(tiny_a_) +    \
                                   ", expected: " + ::tinytest::show(tiny_e_) + ")"); \
        }                                                                         \
    } while (false)

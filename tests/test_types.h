#pragma once

#include <utility>

// Shared instrumentation for the unique_ptr suites.

namespace tinytest {

// A type that counts its own construction/destruction so the tests can assert
// exactly when (and how often) the smart pointer frees what it owns.
struct Tracker {
    static int constructed;
    static int destroyed;

    static void reset_counts() {
        constructed = 0;
        destroyed = 0;
    }

    static int alive() { return constructed - destroyed; }

    int value;

    explicit Tracker(int v = 0) : value(v) { ++constructed; }
    ~Tracker() { ++destroyed; }

    Tracker(const Tracker&) = delete;
    Tracker& operator=(const Tracker&) = delete;

    int twice() const { return value * 2; }
};

inline int Tracker::constructed = 0;
inline int Tracker::destroyed = 0;

// A deleter that is identifiable (`id`), counts its invocations, and records
// whether it was moved from.
struct TaggedDeleter {
    int id = 0;
    int* calls = nullptr;
    bool moved_from = false;

    TaggedDeleter() = default;
    TaggedDeleter(int i, int* c) : id(i), calls(c) {}

    TaggedDeleter(const TaggedDeleter& other) : id(other.id), calls(other.calls) {}
    TaggedDeleter(TaggedDeleter&& other) noexcept : id(other.id), calls(other.calls) {
        other.moved_from = true;
    }

    TaggedDeleter& operator=(const TaggedDeleter& other) {
        id = other.id;
        calls = other.calls;
        return *this;
    }

    TaggedDeleter& operator=(TaggedDeleter&& other) noexcept {
        id = other.id;
        calls = other.calls;
        other.moved_from = true;
        return *this;
    }

    void operator()(Tracker* p) const {
        if (calls != nullptr) ++*calls;
        delete p;
    }
};

// Routing a self-assignment through a function keeps -Wself-move quiet while
// still performing the exact `p = std::move(p)` the test cares about.
template <typename T>
void move_assign(T& target, T& source) {
    target = std::move(source);
}

}  // namespace tinytest

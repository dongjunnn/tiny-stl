// unique_ptr behaviour that depends on the second template parameter D:
// the deleter constructors, get_deleter(), and deleter propagation on move.

#include "tiny/unique_ptr.h"

#include <type_traits>
#include <utility>

#include "test_harness.h"
#include "test_types.h"

namespace {

using tinytest::TaggedDeleter;
using tinytest::Tracker;

using TaggedOwner = unique_ptr<Tracker, TaggedDeleter>;

// ---------------------------------------------------------------------------
// Deleter constructors & destructor
// ---------------------------------------------------------------------------

TINY_TEST("pointer + lvalue deleter constructor: stores the pointer and copies the deleter") {
    int calls = 0;
    TaggedDeleter d(11, &calls);
    Tracker* raw = new Tracker(1);
    {
        TaggedOwner p(raw, d);
        TINY_CHECK_EQ(p.get(), raw);
        TINY_CHECK_EQ(p.get_deleter().id, 11);
        TINY_CHECK_EQ(d.moved_from, false);  // an lvalue deleter must be copied
    }
    TINY_CHECK_EQ(calls, 1);
}

TINY_TEST("pointer + rvalue deleter constructor: moves the deleter") {
    int calls = 0;
    TaggedDeleter d(22, &calls);
    {
        TaggedOwner p(new Tracker(1), std::move(d));
        TINY_CHECK_EQ(p.get_deleter().id, 22);
        TINY_CHECK_EQ(d.moved_from, true);  // an rvalue deleter must be moved
    }
    TINY_CHECK_EQ(calls, 1);
}

TINY_TEST("destructor: invokes the custom deleter exactly once") {
    int calls = 0;
    Tracker::reset_counts();
    {
        TaggedOwner p(new Tracker(3), TaggedDeleter(1, &calls));
    }
    TINY_CHECK_EQ(calls, 1);
    TINY_CHECK_EQ(Tracker::destroyed, 1);
}

TINY_TEST("destructor: does not invoke the deleter when the pointer is null") {
    int calls = 0;
    {
        TaggedOwner p(nullptr, TaggedDeleter(1, &calls));
    }
    TINY_CHECK_EQ(calls, 0);
}

// ---------------------------------------------------------------------------
// Deleter propagation through moves
// ---------------------------------------------------------------------------

TINY_TEST("move constructor: transfers the deleter along with the pointer") {
    int calls = 0;
    {
        TaggedOwner src(new Tracker(5), TaggedDeleter(33, &calls));
        TaggedOwner dst(std::move(src));
        TINY_CHECK_EQ(dst.get_deleter().id, 33);
        TINY_CHECK_EQ(dst.get_deleter().calls, &calls);
    }
    TINY_CHECK_EQ(calls, 1);
}

TINY_TEST("move assignment: frees the old object with the old deleter, then adopts the new one") {
    int src_calls = 0;
    int dst_calls = 0;
    {
        TaggedOwner dst(new Tracker(1), TaggedDeleter(44, &dst_calls));
        TaggedOwner src(new Tracker(2), TaggedDeleter(55, &src_calls));
        dst = std::move(src);
        TINY_CHECK_EQ(dst_calls, 1);
        TINY_CHECK_EQ(dst.get_deleter().id, 55);
    }
    TINY_CHECK_EQ(src_calls, 1);
    TINY_CHECK_EQ(dst_calls, 1);
}

// ---------------------------------------------------------------------------
// get_deleter()
// ---------------------------------------------------------------------------

TINY_TEST("get_deleter(): non-const overload returns a mutable D&") {
    int calls = 0;
    TaggedOwner p(new Tracker(1), TaggedDeleter(60, &calls));
    p.get_deleter().id = 61;
    TINY_CHECK_EQ(p.get_deleter().id, 61);
    TINY_CHECK_EQ((std::is_same<decltype(p.get_deleter()), TaggedDeleter&>::value), true);
}

TINY_TEST("get_deleter(): const overload returns a const D&") {
    int calls = 0;
    const TaggedOwner p(new Tracker(1), TaggedDeleter(70, &calls));
    TINY_CHECK_EQ(p.get_deleter().id, 70);
    TINY_CHECK_EQ((std::is_same<decltype(p.get_deleter()), const TaggedDeleter&>::value), true);
}

// ---------------------------------------------------------------------------
// Modifiers with a custom deleter
// ---------------------------------------------------------------------------

TINY_TEST("release(): does not run the custom deleter") {
    int calls = 0;
    Tracker* raw = nullptr;
    {
        TaggedOwner p(new Tracker(1), TaggedDeleter(80, &calls));
        raw = p.release();
    }
    TINY_CHECK_EQ(calls, 0);  // ownership left the pointer before ~unique_ptr ran
    delete raw;
}

TINY_TEST("reset(ptr): frees the old object with the custom deleter and keeps it") {
    int calls = 0;
    {
        TaggedOwner p(new Tracker(1), TaggedDeleter(90, &calls));
        p.reset(new Tracker(2));
        TINY_CHECK_EQ(calls, 1);
        TINY_CHECK_EQ(p.get_deleter().id, 90);  // reset must not replace the deleter
    }
    TINY_CHECK_EQ(calls, 2);
}

TINY_TEST("reset(): on an empty unique_ptr does not invoke the deleter") {
    int calls = 0;
    {
        TaggedOwner p(nullptr, TaggedDeleter(95, &calls));
        p.reset();
        TINY_CHECK_EQ(calls, 0);
    }
    TINY_CHECK_EQ(calls, 0);
}

}  // namespace

int main() { return tinytest::run_all("unique_ptr - custom deleter"); }

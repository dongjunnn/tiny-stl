// Core unique_ptr behaviour, exercised with the default deleter.

#include "tiny/unique_ptr.h"

#include <type_traits>
#include <utility>

#include "test_harness.h"
#include "test_types.h"

namespace {

using tinytest::move_assign;
using tinytest::Tracker;

using Owner = unique_ptr<Tracker>;

// ---------------------------------------------------------------------------
// Constructors & destructor
// ---------------------------------------------------------------------------

TINY_TEST("default constructor: get() returns nullptr") {
    Owner p;
    TINY_CHECK_EQ(p.get(), nullptr);
}

TINY_TEST("default constructor: converts to false") {
    Owner p;
    TINY_CHECK_EQ(static_cast<bool>(p), false);
}

TINY_TEST("pointer constructor: takes ownership of the raw pointer") {
    Tracker* raw = new Tracker(42);
    Owner p(raw);
    TINY_CHECK_EQ(p.get(), raw);
    TINY_CHECK_EQ(static_cast<bool>(p), true);
}

TINY_TEST("pointer constructor: does not copy or destroy the pointee") {
    Tracker::reset_counts();
    Owner p(new Tracker(7));
    TINY_CHECK_EQ(Tracker::constructed, 1);
    TINY_CHECK_EQ(Tracker::destroyed, 0);
}

TINY_TEST("pointer constructor is explicit (no implicit T* -> unique_ptr)") {
    TINY_CHECK_EQ((std::is_constructible<Owner, Tracker*>::value), true);
    TINY_CHECK_EQ((std::is_convertible<Tracker*, Owner>::value), false);
}

TINY_TEST("destructor: destroys the owned object exactly once") {
    Tracker::reset_counts();
    {
        Owner p(new Tracker(3));
    }
    TINY_CHECK_EQ(Tracker::destroyed, 1);
    TINY_CHECK_EQ(Tracker::alive(), 0);
}

TINY_TEST("destructor: an empty unique_ptr destroys nothing") {
    Tracker::reset_counts();
    {
        Owner p;
    }
    TINY_CHECK_EQ(Tracker::destroyed, 0);
}

// ---------------------------------------------------------------------------
// Rule of five
// ---------------------------------------------------------------------------

TINY_TEST("copy constructor is deleted") {
    TINY_CHECK_EQ(std::is_copy_constructible<Owner>::value, false);
}

TINY_TEST("copy assignment is deleted") {
    TINY_CHECK_EQ(std::is_copy_assignable<Owner>::value, false);
}

TINY_TEST("type is move constructible and move assignable") {
    TINY_CHECK_EQ(std::is_move_constructible<Owner>::value, true);
    TINY_CHECK_EQ(std::is_move_assignable<Owner>::value, true);
}

TINY_TEST("move constructor: transfers the pointer and empties the source") {
    Tracker* raw = new Tracker(5);
    Owner src(raw);
    Owner dst(std::move(src));
    TINY_CHECK_EQ(dst.get(), raw);
    TINY_CHECK_EQ(src.get(), nullptr);
}

TINY_TEST("move constructor: does not destroy the owned object") {
    Tracker::reset_counts();
    {
        Owner src(new Tracker(5));
        Owner dst(std::move(src));
        TINY_CHECK_EQ(Tracker::destroyed, 0);
    }
    TINY_CHECK_EQ(Tracker::destroyed, 1);
}

TINY_TEST("move assignment: transfers the pointer and empties the source") {
    Tracker* raw = new Tracker(9);
    Owner src(raw);
    Owner dst;
    dst = std::move(src);
    TINY_CHECK_EQ(dst.get(), raw);
    TINY_CHECK_EQ(src.get(), nullptr);
}

TINY_TEST("move assignment: destroys the object the target already owned") {
    Tracker::reset_counts();
    {
        Owner dst(new Tracker(1));
        Owner src(new Tracker(2));
        dst = std::move(src);
        TINY_CHECK_EQ(Tracker::destroyed, 1);  // the old Tracker(1) is gone
        TINY_CHECK_EQ(dst->value, 2);
    }
    TINY_CHECK_EQ(Tracker::destroyed, 2);
}

TINY_TEST("move assignment: returns a reference to *this") {
    Owner a(new Tracker(1));
    Owner b;
    Owner& result = (b = std::move(a));
    TINY_CHECK_EQ(&result, &b);
    TINY_CHECK_EQ(result.get(), b.get());
}

TINY_TEST("move assignment: self-assignment keeps the object alive") {
    Tracker::reset_counts();
    {
        Owner p(new Tracker(4));
        Tracker* raw = p.get();
        move_assign(p, p);
        TINY_CHECK_EQ(Tracker::destroyed, 0);
        TINY_CHECK_EQ(p.get(), raw);
    }
    TINY_CHECK_EQ(Tracker::destroyed, 1);
}

// ---------------------------------------------------------------------------
// Observers
// ---------------------------------------------------------------------------

TINY_TEST("operator*: reads the pointee") {
    Owner p(new Tracker(17));
    TINY_CHECK_EQ((*p).value, 17);
}

TINY_TEST("operator*: yields a mutable reference through a const unique_ptr") {
    const Owner p(new Tracker(1));
    (*p).value = 99;
    TINY_CHECK_EQ((*p).value, 99);
    TINY_CHECK_EQ((std::is_same<decltype(*p), Tracker&>::value), true);
}

TINY_TEST("operator->: reaches members of the pointee") {
    Owner p(new Tracker(21));
    TINY_CHECK_EQ(p->value, 21);
    TINY_CHECK_EQ(p->twice(), 42);
}

TINY_TEST("get(): returns the raw pointer without releasing ownership") {
    Tracker::reset_counts();
    Tracker* raw = new Tracker(2);
    {
        Owner p(raw);
        TINY_CHECK_EQ(p.get(), raw);
        TINY_CHECK_EQ(Tracker::destroyed, 0);
    }
    TINY_CHECK_EQ(Tracker::destroyed, 1);  // still owned, so still destroyed
}

TINY_TEST("operator bool: true while owning, false once empty") {
    Owner p(new Tracker(1));
    TINY_CHECK_EQ(static_cast<bool>(p), true);
    delete p.release();
    TINY_CHECK_EQ(static_cast<bool>(p), false);
}

TINY_TEST("operator bool is explicit (no implicit conversion to int)") {
    TINY_CHECK_EQ((std::is_constructible<bool, Owner>::value), true);
    TINY_CHECK_EQ((std::is_convertible<Owner, bool>::value), false);
    TINY_CHECK_EQ((std::is_convertible<Owner, int>::value), false);
}

// ---------------------------------------------------------------------------
// Modifiers
// ---------------------------------------------------------------------------

TINY_TEST("release(): returns the pointer and gives up ownership") {
    Tracker* raw = new Tracker(8);
    Owner p(raw);
    Tracker* got = p.release();
    TINY_CHECK_EQ(got, raw);
    TINY_CHECK_EQ(p.get(), nullptr);
    delete got;
}

TINY_TEST("release(): returns nullptr when empty") {
    Owner p;
    TINY_CHECK_EQ(p.release(), nullptr);
}

TINY_TEST("release(): the object outlives the unique_ptr and is freed once") {
    Tracker::reset_counts();
    Tracker* raw = nullptr;
    {
        Owner p(new Tracker(1));
        raw = p.release();
    }
    TINY_CHECK_EQ(Tracker::destroyed, 0);
    delete raw;
    TINY_CHECK_EQ(Tracker::destroyed, 1);
}

TINY_TEST("reset(ptr): destroys the previously owned object exactly once") {
    Tracker::reset_counts();
    {
        Owner p(new Tracker(1));
        p.reset(new Tracker(2));
        TINY_CHECK_EQ(Tracker::destroyed, 1);
        TINY_CHECK_EQ(Tracker::alive(), 1);
    }
    TINY_CHECK_EQ(Tracker::destroyed, 2);
}

TINY_TEST("reset(ptr): adopts the new pointer") {
    Owner p(new Tracker(1));
    Tracker* fresh = new Tracker(2);
    p.reset(fresh);
    TINY_CHECK_EQ(p.get(), fresh);
    TINY_CHECK_EQ(p->value, 2);
}

TINY_TEST("reset(): with no argument destroys the object and empties the pointer") {
    Tracker::reset_counts();
    Owner p(new Tracker(1));
    p.reset();
    TINY_CHECK_EQ(Tracker::destroyed, 1);
    TINY_CHECK_EQ(p.get(), nullptr);
    TINY_CHECK_EQ(static_cast<bool>(p), false);
}

TINY_TEST("reset(): on an empty unique_ptr destroys nothing") {
    Tracker::reset_counts();
    Owner p;
    p.reset();
    TINY_CHECK_EQ(Tracker::destroyed, 0);
    TINY_CHECK_EQ(p.get(), nullptr);
}

}  // namespace

int main() { return tinytest::run_all("unique_ptr - default deleter"); }

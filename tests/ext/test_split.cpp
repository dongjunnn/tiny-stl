// split(str, delim) should behave like Python's str.split(sep).

#include "tiny/ext/string_parser.h"

#include <stdexcept>
#include <string>
#include <vector>

#include "test_harness.h"

namespace {

using Parts = std::vector<std::string>;

TINY_TEST("split: basic single-character delimiter") {
    TINY_CHECK_EQ(split("a,b,c", ","), (Parts{"a", "b", "c"}));
}

TINY_TEST("split: delimiter not present returns the whole string") {
    TINY_CHECK_EQ(split("abc", ","), (Parts{"abc"}));
}

TINY_TEST("split: empty string returns one empty string") {
    TINY_CHECK_EQ(split("", ","), (Parts{""}));
}

TINY_TEST("split: consecutive delimiters produce empty strings") {
    TINY_CHECK_EQ(split("a,,b", ","), (Parts{"a", "", "b"}));
}

TINY_TEST("split: leading delimiter produces a leading empty string") {
    TINY_CHECK_EQ(split(",a,b", ","), (Parts{"", "a", "b"}));
}

TINY_TEST("split: trailing delimiter produces a trailing empty string") {
    TINY_CHECK_EQ(split("a,b,", ","), (Parts{"a", "b", ""}));
}

TINY_TEST("split: string that is only the delimiter") {
    TINY_CHECK_EQ(split(",", ","), (Parts{"", ""}));
}

TINY_TEST("split: only delimiters") {
    TINY_CHECK_EQ(split(",,,", ","), (Parts{"", "", "", ""}));
}

TINY_TEST("split: multi-character delimiter") {
    TINY_CHECK_EQ(split("a--b--c", "--"), (Parts{"a", "b", "c"}));
}

TINY_TEST("split: multi-character delimiter is matched as a whole") {
    TINY_CHECK_EQ(split("a-b--c", "--"), (Parts{"a-b", "c"}));
}

TINY_TEST("split: overlapping delimiter matches are consumed left to right") {
    // Python: "aaa".split("aa") == ["", "a"]
    TINY_CHECK_EQ(split("aaa", "aa"), (Parts{"", "a"}));
}

TINY_TEST("split: delimiter equal to the whole string") {
    TINY_CHECK_EQ(split("abc", "abc"), (Parts{"", ""}));
}

TINY_TEST("split: whitespace is kept, not trimmed") {
    TINY_CHECK_EQ(split(" a , b ", ","), (Parts{" a ", " b "}));
}

TINY_TEST("split: space delimiter does not collapse runs (unlike Python's split())") {
    TINY_CHECK_EQ(split("a  b", " "), (Parts{"a", "", "b"}));
}

TINY_TEST("split: empty delimiter throws std::invalid_argument") {
    bool threw = false;
    try {
        split("abc", "");
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    TINY_CHECK(threw);
}

}  // namespace

int main() { return tinytest::run_all("split"); }

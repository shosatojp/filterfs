#include "filterfs.hpp"
#include "segment.hpp"
#include <iostream>
#include <stdexcept>

int main() {
    // no wildcard
    ASSERT(segment("abc").match(4, "abcd") == false);
    ASSERT(segment("abc").match(3, "abc") == true);
    ASSERT(segment("abc").match(3, "abx") == false);
    ASSERT(segment("abc").match(2, "ab") == false);
    ASSERT(segment("").match(0, "") == true);

    // wildcard
    ASSERT(segment("a*c").match(3, "abc") == true);
    ASSERT(segment("a*").match(3, "abc") == true);
    ASSERT(segment("*c").match(3, "abc") == true);
    ASSERT(segment("*").match(3, "abc") == true);
    ASSERT(segment("a*c").match(4, "abcd") == false);
    ASSERT(segment("a*").match(4, "bcd") == false);
    ASSERT(segment("*c").match(4, "abcd") == false);
}

#include "filterfs.hpp"
#include "rule.hpp"
#include <iostream>
#include <vector>

int main() {
    // absolute
    ASSERT(rule("/a/b/c/", rule::RULE_TYPE_INCLUDE).match("/a/b/c/") == true);
    ASSERT(rule("/a/b/c", rule::RULE_TYPE_INCLUDE).match("/a/b/c/") == true);
    ASSERT(rule("/a/b/c/", rule::RULE_TYPE_INCLUDE).match("/a/b/c") == true);
    ASSERT(rule("/a/b/c", rule::RULE_TYPE_INCLUDE).match("/a/b/c") == true);

    ASSERT(rule("/a/b/c", rule::RULE_TYPE_INCLUDE).match("/a/b") == false);
    ASSERT(rule("/a/b", rule::RULE_TYPE_INCLUDE).match("/a/b/c") == false);
    ASSERT(rule("/a", rule::RULE_TYPE_INCLUDE).match("/b") == false);

    ASSERT(rule("/**", rule::RULE_TYPE_INCLUDE).match("/b") == true);
    ASSERT(rule("/a/**", rule::RULE_TYPE_INCLUDE).match("/a/b") == true);
    ASSERT(rule("/a/**", rule::RULE_TYPE_INCLUDE).match("/a/b/c") == true);
    ASSERT(rule("/b/**", rule::RULE_TYPE_INCLUDE).match("/a/b/c") == false);

    // relative
    ASSERT(rule("a", rule::RULE_TYPE_INCLUDE).match("/a/b/c") == true);
    ASSERT(rule("b", rule::RULE_TYPE_INCLUDE).match("/a/b/c") == true);
    ASSERT(rule("c", rule::RULE_TYPE_INCLUDE).match("/a/b/c") == true);
    ASSERT(rule("c*", rule::RULE_TYPE_INCLUDE).match("/a/b/c") == true);


    ASSERT(rule("/home/", rule::RULE_TYPE_INCLUDE).match("/home/test") == false);
    ASSERT(rule("/home/sho/", rule::RULE_TYPE_INCLUDE).match("/home/test") == false);
}

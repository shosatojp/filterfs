#include <stdexcept>

#define ASSERT(EXPR)                                         \
    if (!(EXPR)) {                                           \
        throw std::runtime_error("AssertionFailed: " #EXPR); \
    }

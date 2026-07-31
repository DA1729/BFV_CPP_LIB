#ifndef BFV_TESTS_CHECK_HPP
#define BFV_TESTS_CHECK_HPP

#include <iostream>
#include <string>

inline int failure_count = 0;

inline std::ostream& operator<<(std::ostream& out, __int128 value) {
    if (value == 0) return out << '0';
    const bool negative = value < 0;
    __uint128_t magnitude = negative ? -static_cast<__uint128_t>(value) : static_cast<__uint128_t>(value);

    std::string digits;
    while (magnitude != 0) {
        digits.push_back(static_cast<char>('0' + static_cast<int>(magnitude % 10)));
        magnitude /= 10;
    }
    if (negative) digits.push_back('-');
    return out << std::string(digits.rbegin(), digits.rend());
}

#define check(condition)                                                                    \
    do {                                                                                    \
        if (!(condition)) {                                                                 \
            std::cerr << "FAIL " << __FILE__ << ":" << __LINE__ << "  " #condition << "\n"; \
            ++failure_count;                                                                \
        }                                                                                   \
    } while (0)

#define check_eq(left, right)                                                                    \
    do {                                                                                         \
        const auto lhs = (left);                                                                 \
        const auto rhs = (right);                                                                \
        if (!(lhs == rhs)) {                                                                     \
            std::cerr << "FAIL " << __FILE__ << ":" << __LINE__ << "  " #left " == " #right      \
                      << " (" << lhs << " vs " << rhs << ")\n";                                  \
            ++failure_count;                                                                     \
        }                                                                                        \
    } while (0)

#define check_throws(expression)                                                                 \
    do {                                                                                         \
        bool threw = false;                                                                      \
        try {                                                                                    \
            (void)(expression);                                                                  \
        } catch (const std::exception&) {                                                        \
            threw = true;                                                                        \
        }                                                                                        \
        if (!threw) {                                                                            \
            std::cerr << "FAIL " << __FILE__ << ":" << __LINE__ << "  expected " #expression     \
                      << " to throw\n";                                                          \
            ++failure_count;                                                                     \
        }                                                                                        \
    } while (0)

inline int report(const std::string& name) {
    if (failure_count == 0) {
        std::cout << name << ": ok\n";
        return 0;
    }
    std::cout << name << ": " << failure_count << " failure(s)\n";
    return 1;
}

#endif

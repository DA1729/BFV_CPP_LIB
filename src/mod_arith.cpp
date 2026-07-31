#include "bfv/mod_arith.hpp"

#include <stdexcept>

namespace bfv {

u64 pow_mod(u64 base, u64 exp, u64 q) {
    if (q == 1) return 0;
    u64 result = 1;
    base %= q;
    while (exp > 0) {
        if (exp & 1) result = mul_mod(result, base, q);
        base = mul_mod(base, base, q);
        exp >>= 1;
    }
    return result;
}

std::tuple<i64, i64, i64> extended_gcd(i64 a, i64 b) {
    i64 old_r = a, r = b;
    i64 old_s = 1, s = 0;
    i64 old_t = 0, t = 1;
    while (r != 0) {
        const i64 quotient = old_r / r;
        i64 tmp = r;
        r = old_r - quotient * r;
        old_r = tmp;
        tmp = s;
        s = old_s - quotient * s;
        old_s = tmp;
        tmp = t;
        t = old_t - quotient * t;
        old_t = tmp;
    }
    return {old_r, old_s, old_t};
}

u64 inv_mod(u64 a, u64 q) {
    const auto [g, x, y] = extended_gcd(static_cast<i64>(a % q), static_cast<i64>(q));
    (void)y;
    if (g != 1) throw std::invalid_argument("inv_mod: value is not invertible modulo q");
    return reduce(x, q);
}

u64 gcd(u64 a, u64 b) {
    while (b != 0) {
        const u64 tmp = b;
        b = a % b;
        a = tmp;
    }
    return a;
}

u64 bit_reverse(u64 x, unsigned bits) {
    u64 result = 0;
    for (unsigned i = 0; i < bits; ++i) {
        result = (result << 1) | (x & 1);
        x >>= 1;
    }
    return result;
}

bool is_power_of_two(u64 x) {
    return x != 0 && (x & (x - 1)) == 0;
}

unsigned log2_floor(u64 x) {
    unsigned r = 0;
    while (x > 1) {
        x >>= 1;
        ++r;
    }
    return r;
}

} // namespace bfv

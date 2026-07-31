#ifndef BFV_MOD_ARITH_HPP
#define BFV_MOD_ARITH_HPP

#include <cstddef>
#include <cstdint>
#include <tuple>

namespace bfv {

using u64 = std::uint64_t;
using i64 = std::int64_t;
using u128 = __uint128_t;
using i128 = __int128_t;

// every modulus in the library must satisfy q < 2^62 so that a + b and
// (q + a - b) never overflow a u64 and t * x never overflows an i128.
constexpr u64 max_modulus = u64(1) << 62;

inline u64 add_mod(u64 a, u64 b, u64 q) {
    const u64 s = a + b;
    return s >= q ? s - q : s;
}

inline u64 sub_mod(u64 a, u64 b, u64 q) {
    return a >= b ? a - b : a + q - b;
}

inline u64 mul_mod(u64 a, u64 b, u64 q) {
    return static_cast<u64>(u128(a) * u128(b) % u128(q));
}

inline u64 neg_mod(u64 a, u64 q) {
    return a == 0 ? 0 : q - a;
}

// representative of a in (-q/2, q/2]
inline i64 center(u64 a, u64 q) {
    return a > q / 2 ? static_cast<i64>(a) - static_cast<i64>(q) : static_cast<i64>(a);
}

inline u64 reduce(i64 a, u64 q) {
    const i64 r = a % static_cast<i64>(q);
    return static_cast<u64>(r < 0 ? r + static_cast<i64>(q) : r);
}

inline u64 reduce(i128 a, u64 q) {
    const i128 r = a % static_cast<i128>(q);
    return static_cast<u64>(r < 0 ? r + static_cast<i128>(q) : r);
}

// round(a / d) for a signed numerator, ties away from zero
inline i128 round_div(i128 a, i128 d) {
    return a >= 0 ? (a + d / 2) / d : (a - d / 2) / d;
}

u64 pow_mod(u64 base, u64 exp, u64 q);

std::tuple<i64, i64, i64> extended_gcd(i64 a, i64 b);

// throws std::invalid_argument when a is not invertible
u64 inv_mod(u64 a, u64 q);

u64 gcd(u64 a, u64 b);

u64 bit_reverse(u64 x, unsigned bits);

bool is_power_of_two(u64 x);

unsigned log2_floor(u64 x);

} // namespace bfv

#endif

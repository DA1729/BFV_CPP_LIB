#include "bfv/mod_arith.hpp"
#include "bfv/params.hpp"
#include "bfv/prime.hpp"
#include "bfv/rng.hpp"
#include "check.hpp"

using namespace bfv;

int main() {
    rng source(20240731);

    const u64 q = 132120577;

    check_eq(pow_mod(2, 0, q), u64(1));
    check_eq(pow_mod(3, 1, q), u64(3));
    check_eq(pow_mod(2, 10, 1000), u64(24));

    for (int i = 0; i < 200; ++i) {
        const u64 a = 1 + source.uniform(q - 1);
        const u64 inverse = inv_mod(a, q);
        check_eq(mul_mod(a, inverse, q), u64(1));
    }

    check_eq(gcd(48, 18), u64(6));
    check_throws(inv_mod(4, 8));

    check_eq(center(1, 7), i64(1));
    check_eq(center(6, 7), i64(-1));
    check_eq(reduce(i64(-1), 7), u64(6));
    check_eq(reduce(i64(-8), 7), u64(6));

    check_eq(static_cast<i64>(round_div(i128(7), i128(2))), i64(4));
    check_eq(static_cast<i64>(round_div(i128(-7), i128(2))), i64(-4));
    check_eq(static_cast<i64>(round_div(i128(5), i128(3))), i64(2));
    check_eq(static_cast<i64>(round_div(i128(-5), i128(3))), i64(-2));

    check_eq(bit_reverse(1, 3), u64(4));
    check_eq(bit_reverse(3, 3), u64(6));
    check(is_power_of_two(1024));
    check(!is_power_of_two(1000));
    check_eq(log2_floor(1024), 10u);
    check_eq(log2_floor(1023), 9u);

    check(is_prime(q, 40, source));
    check(is_prime(2, 40, source));
    check(!is_prime(1, 40, source));
    check(!is_prime(q - 1, 40, source));
    check(is_prime(generate_prime(30, 40, source), 40, source));

    const u64 ntt_prime = find_ntt_prime(1024, 27, 40, source);
    check(is_prime(ntt_prime, 40, source));
    check_eq((ntt_prime - 1) % 2048, u64(0));

    const auto [found, root] = find_primitive_root(2048, ntt_prime, source);
    check(found);
    check(is_primitive_root(root, 2048, ntt_prime));
    check(!is_primitive_root(1, 2048, ntt_prime));

    return report("mod_arith");
}

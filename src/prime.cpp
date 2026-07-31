#include "bfv/prime.hpp"

#include <array>
#include <stdexcept>

namespace bfv {

namespace {

constexpr std::array<u64, 54> low_primes = {
    2,   3,   5,   7,   11,  13,  17,  19,  23,  29,  31,  37,  41,  43,
    47,  53,  59,  61,  67,  71,  73,  79,  83,  89,  97,  101, 103, 107,
    109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181,
    191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251};

} // namespace

bool miller_rabin(u64 n, int rounds, rng& source) {
    if (n < 5) return n == 2 || n == 3;
    if ((n & 1) == 0) return false;

    u64 d = n - 1;
    unsigned s = 0;
    while ((d & 1) == 0) {
        d >>= 1;
        ++s;
    }

    for (int i = 0; i < rounds; ++i) {
        const u64 a = 2 + source.uniform(n - 3);
        u64 x = pow_mod(a, d, n);
        if (x == 1 || x == n - 1) continue;

        bool witness = true;
        for (unsigned j = 0; j + 1 < s; ++j) {
            x = mul_mod(x, x, n);
            if (x == n - 1) {
                witness = false;
                break;
            }
        }
        if (witness) return false;
    }
    return true;
}

bool is_prime(u64 n, int rounds, rng& source) {
    if (n < 2) return false;
    for (u64 p : low_primes) {
        if (n == p) return true;
        if (n % p == 0) return false;
    }
    return miller_rabin(n, rounds, source);
}

u64 generate_prime(unsigned bits, int rounds, rng& source) {
    if (bits < 2 || bits > 62) throw std::invalid_argument("generate_prime: bits must be in [2, 62]");

    const u64 low = u64(1) << (bits - 1);
    const u64 high = (u64(1) << bits) - 1;

    for (int attempt = 0; attempt < 10000; ++attempt) {
        const u64 candidate = low + source.uniform(high - low + 1);
        if (is_prime(candidate | 1, rounds, source)) return candidate | 1;
    }
    throw std::runtime_error("generate_prime: no prime found within the attempt budget");
}

} // namespace bfv

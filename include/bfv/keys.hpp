#ifndef BFV_KEYS_HPP
#define BFV_KEYS_HPP

#include "bfv/context.hpp"
#include "bfv/poly.hpp"
#include "bfv/rng.hpp"

#include <utility>
#include <vector>

namespace bfv {

struct secret_key {
    poly s;
};

// (b, a) with b = -(a * s + e)
struct public_key {
    poly b;
    poly a;
};

// Digit decomposition variant: for each base-T digit i,
// data[i] = (T^i * s^2 - (a_i * s + e_i), a_i) over Z_q.
struct relin_key_v1 {
    u64 base = 0;
    std::vector<std::pair<poly, poly>> data;
};

// Modulus switching variant: (b, a) over Z_{p*q} with
// b = p * s^2 - (a * s + e). Stored as raw coefficient vectors because the
// modulus p * q has no transform tables.
struct relin_key_v2 {
    u64 p = 0;
    u64 modulus = 0;
    std::vector<u64> b;
    std::vector<u64> a;
};

// Holds references to the context and the generator, so both must outlive it.
class key_generator {
public:
    key_generator(const context& ctx, rng& source);

    secret_key generate_secret_key();
    public_key generate_public_key(const secret_key& sk);
    relin_key_v1 generate_relin_key_v1(const secret_key& sk);
    relin_key_v2 generate_relin_key_v2(const secret_key& sk);

private:
    const context& ctx_;
    rng& rng_;
};

} // namespace bfv

#endif

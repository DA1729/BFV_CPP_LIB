#include "bfv/keys.hpp"

#include <stdexcept>

namespace bfv {

key_generator::key_generator(const context& ctx, rng& source) : ctx_(ctx), rng_(source) {}

secret_key key_generator::generate_secret_key() {
    secret_key sk;
    sk.s = ctx_.ring().sample_ternary(rng_);
    return sk;
}

public_key key_generator::generate_public_key(const secret_key& sk) {
    const poly_ring& ring = ctx_.ring();

    const poly a = ring.sample_uniform(rng_);
    const poly e = ring.sample_gaussian(rng_, ctx_.parameters().sigma);

    public_key pk;
    pk.a = a;
    pk.b = ring.neg(ring.add(ring.mul(a, sk.s), e));
    return pk;
}

relin_key_v1 key_generator::generate_relin_key_v1(const secret_key& sk) {
    const poly_ring& ring = ctx_.ring();
    const params& p = ctx_.parameters();

    const poly s_squared = ring.mul(sk.s, sk.s);

    relin_key_v1 key;
    key.base = p.relin_base;
    key.data.reserve(p.relin_digits());

    u64 base_power = 1 % p.q;
    for (std::size_t i = 0; i < p.relin_digits(); ++i) {
        const poly a = ring.sample_uniform(rng_);
        const poly e = ring.sample_gaussian(rng_, p.sigma);

        const poly masked = ring.neg(ring.add(ring.mul(a, sk.s), e));
        const poly b = ring.add(ring.mul_scalar(s_squared, base_power), masked);

        key.data.emplace_back(b, a);
        base_power = mul_mod(base_power, p.relin_base, p.q);
    }
    return key;
}

relin_key_v2 key_generator::generate_relin_key_v2(const secret_key& sk) {
    const params& p = ctx_.parameters();
    if (p.relin_modulus == 0)
        throw std::invalid_argument("generate_relin_key_v2: params.relin_modulus is 0, so v2 is disabled");

    const std::size_t n = p.n;
    const u64 modulus = p.relin_modulus * p.q;

    std::vector<u64> s(n);
    for (std::size_t i = 0; i < n; ++i) s[i] = reduce(center(sk.s[i], p.q), modulus);

    std::vector<u64> a(n);
    std::vector<u64> e(n);
    for (std::size_t i = 0; i < n; ++i) {
        a[i] = rng_.uniform(modulus);
        e[i] = reduce(rng_.gaussian(p.sigma), modulus);
    }

    const std::vector<u64> s_squared = naive_negacyclic_mul(s, s, modulus);
    const std::vector<u64> a_s = naive_negacyclic_mul(a, s, modulus);

    std::vector<u64> b(n);
    for (std::size_t i = 0; i < n; ++i) {
        const u64 masked = neg_mod(add_mod(a_s[i], e[i], modulus), modulus);
        b[i] = add_mod(mul_mod(s_squared[i], p.relin_modulus % modulus, modulus), masked, modulus);
    }

    relin_key_v2 key;
    key.p = p.relin_modulus;
    key.modulus = modulus;
    key.b = std::move(b);
    key.a = std::move(a);
    return key;
}

} // namespace bfv

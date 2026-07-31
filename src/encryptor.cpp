#include "bfv/encryptor.hpp"

#include <stdexcept>

namespace bfv {

namespace {

// delta * m lifted from Z_t into Z_q
poly scaled_plaintext(const context& ctx, const plaintext& m) {
    if (m.size() != ctx.degree()) throw std::invalid_argument("encrypt: plaintext has the wrong length");

    poly result = ctx.ring().zero();
    for (std::size_t i = 0; i < m.size(); ++i) result[i] = mul_mod(m[i] % ctx.t(), ctx.delta(), ctx.q());
    return result;
}

} // namespace

encryptor::encryptor(const context& ctx, const public_key& pk, rng& source)
    : ctx_(ctx), rng_(source), symmetric_(false), pk_(pk) {}

encryptor::encryptor(const context& ctx, const secret_key& sk, rng& source)
    : ctx_(ctx), rng_(source), symmetric_(true), sk_(sk) {}

ciphertext encryptor::encrypt(const plaintext& m) {
    const poly_ring& ring = ctx_.ring();
    const double sigma = ctx_.parameters().sigma;
    const poly scaled = scaled_plaintext(ctx_, m);

    if (symmetric_) {
        const poly a = ring.sample_uniform(rng_);
        const poly e = ring.sample_gaussian(rng_, sigma);
        const poly c0 = ring.add(ring.neg(ring.add(ring.mul(a, sk_.s), e)), scaled);
        return ciphertext({c0, a});
    }

    const poly u = ring.sample_ternary(rng_);
    const poly e1 = ring.sample_gaussian(rng_, sigma);
    const poly e2 = ring.sample_gaussian(rng_, sigma);

    const poly c0 = ring.add(ring.add(ring.mul(pk_.b, u), e1), scaled);
    const poly c1 = ring.add(ring.mul(pk_.a, u), e2);
    return ciphertext({c0, c1});
}

} // namespace bfv

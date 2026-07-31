#include "bfv/decryptor.hpp"

#include <cmath>
#include <stdexcept>

namespace bfv {

decryptor::decryptor(const context& ctx, const secret_key& sk) : ctx_(ctx), sk_(sk) {}

poly decryptor::evaluate_at_secret(const ciphertext& ct) const {
    if (ct.size() < 2) throw std::invalid_argument("decrypt: a ciphertext needs at least two components");

    const poly_ring& ring = ctx_.ring();

    poly accumulator = ct[0];
    poly power = sk_.s;
    for (std::size_t i = 1; i < ct.size(); ++i) {
        accumulator = ring.add(accumulator, ring.mul(ct[i], power));
        if (i + 1 < ct.size()) power = ring.mul(power, sk_.s);
    }
    return accumulator;
}

plaintext decryptor::decrypt(const ciphertext& ct) const {
    const poly value = evaluate_at_secret(ct);
    const u64 q = ctx_.q();
    const u64 t = ctx_.t();

    plaintext m;
    m.coeffs.assign(value.size(), 0);
    for (std::size_t i = 0; i < value.size(); ++i) {
        const u128 scaled = (u128(t) * u128(value[i]) + u128(q / 2)) / u128(q);
        m[i] = static_cast<u64>(scaled % u128(t));
    }
    return m;
}

int decryptor::noise_budget(const ciphertext& ct) const {
    const poly value = evaluate_at_secret(ct);
    const u64 q = ctx_.q();
    const u64 t = ctx_.t();

    u64 largest = 0;
    for (std::size_t i = 0; i < value.size(); ++i) {
        const u64 scaled = mul_mod(value[i], t, q);
        const i64 centered = center(scaled, q);
        const u64 magnitude = centered < 0 ? u64(-centered) : u64(centered);
        if (magnitude > largest) largest = magnitude;
    }

    if (largest == 0) return static_cast<int>(log2_floor(q));

    const double budget = std::log2(double(q)) - std::log2(2.0 * double(largest));
    return budget <= 0.0 ? 0 : static_cast<int>(budget);
}

} // namespace bfv

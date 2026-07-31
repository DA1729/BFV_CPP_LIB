#include "bfv/evaluator.hpp"

#include <stdexcept>

namespace bfv {

namespace {

// The exact product of two ring elements needs n * (q/2)^2 to stay inside an
// i128, which bounds how large n and q may be together.
void check_multiply_range(std::size_t n, u64 q) {
    const unsigned bits = log2_floor(n) + 2 * (log2_floor(q) + 1);
    if (bits > 125)
        throw std::invalid_argument("multiply: n and q are too large for an exact 128 bit intermediate product");
}

poly lift_plaintext(const context& ctx, const plaintext& m) {
    if (m.size() != ctx.degree()) throw std::invalid_argument("evaluator: plaintext has the wrong length");

    poly result = ctx.ring().zero();
    for (std::size_t i = 0; i < m.size(); ++i) result[i] = reduce(center(m[i] % ctx.t(), ctx.t()), ctx.q());
    return result;
}

poly scaled_plaintext(const context& ctx, const plaintext& m) {
    poly result = lift_plaintext(ctx, m);
    for (std::size_t i = 0; i < result.size(); ++i) result[i] = mul_mod(result[i], ctx.delta(), ctx.q());
    return result;
}

void check_binary(const ciphertext& a, const ciphertext& b, const char* where) {
    if (a.size() != b.size()) throw std::invalid_argument(std::string(where) + ": ciphertext sizes differ");
    if (a.size() < 2) throw std::invalid_argument(std::string(where) + ": ciphertext is too small");
}

} // namespace

evaluator::evaluator(const context& ctx) : ctx_(ctx) {}

ciphertext evaluator::add(const ciphertext& a, const ciphertext& b) const {
    check_binary(a, b, "add");
    ciphertext result;
    result.parts.reserve(a.size());
    for (std::size_t i = 0; i < a.size(); ++i) result.parts.push_back(ctx_.ring().add(a[i], b[i]));
    return result;
}

ciphertext evaluator::sub(const ciphertext& a, const ciphertext& b) const {
    check_binary(a, b, "sub");
    ciphertext result;
    result.parts.reserve(a.size());
    for (std::size_t i = 0; i < a.size(); ++i) result.parts.push_back(ctx_.ring().sub(a[i], b[i]));
    return result;
}

ciphertext evaluator::negate(const ciphertext& a) const {
    ciphertext result;
    result.parts.reserve(a.size());
    for (const poly& part : a.parts) result.parts.push_back(ctx_.ring().neg(part));
    return result;
}

ciphertext evaluator::add_plain(const ciphertext& a, const plaintext& m) const {
    ciphertext result = a;
    result[0] = ctx_.ring().add(result[0], scaled_plaintext(ctx_, m));
    return result;
}

ciphertext evaluator::sub_plain(const ciphertext& a, const plaintext& m) const {
    ciphertext result = a;
    result[0] = ctx_.ring().sub(result[0], scaled_plaintext(ctx_, m));
    return result;
}

ciphertext evaluator::mul_plain(const ciphertext& a, const plaintext& m) const {
    const poly lifted = lift_plaintext(ctx_, m);
    ciphertext result;
    result.parts.reserve(a.size());
    for (const poly& part : a.parts) result.parts.push_back(ctx_.ring().mul(part, lifted));
    return result;
}

ciphertext evaluator::multiply(const ciphertext& a, const ciphertext& b) const {
    a.require_size(2, "multiply");
    b.require_size(2, "multiply");

    const poly_ring& ring = ctx_.ring();
    const std::size_t n = ctx_.degree();
    const u64 q = ctx_.q();
    const u64 t = ctx_.t();
    check_multiply_range(n, q);

    const std::vector<i64> a0 = ring.to_signed(a[0]);
    const std::vector<i64> a1 = ring.to_signed(a[1]);
    const std::vector<i64> b0 = ring.to_signed(b[0]);
    const std::vector<i64> b1 = ring.to_signed(b[1]);

    const std::vector<i128> r0 = exact_negacyclic_mul(a0, b0);
    const std::vector<i128> r01 = exact_negacyclic_mul(a0, b1);
    const std::vector<i128> r10 = exact_negacyclic_mul(a1, b0);
    const std::vector<i128> r2 = exact_negacyclic_mul(a1, b1);

    poly c0 = ring.zero();
    poly c1 = ring.zero();
    poly c2 = ring.zero();
    for (std::size_t i = 0; i < n; ++i) {
        c0[i] = reduce(round_div(i128(t) * r0[i], i128(q)), q);
        c1[i] = reduce(round_div(i128(t) * (r01[i] + r10[i]), i128(q)), q);
        c2[i] = reduce(round_div(i128(t) * r2[i], i128(q)), q);
    }
    return ciphertext({c0, c1, c2});
}

ciphertext evaluator::relinearize(const ciphertext& ct, const relin_key_v1& key) const {
    ct.require_size(3, "relinearize");

    const poly_ring& ring = ctx_.ring();
    const params& p = ctx_.parameters();
    if (key.base != p.relin_base) throw std::invalid_argument("relinearize: key base does not match the parameters");
    if (key.data.size() != p.relin_digits())
        throw std::invalid_argument("relinearize: key has the wrong number of digits");

    poly c0 = ct[0];
    poly c1 = ct[1];
    std::vector<u64> remaining = ct[2].coeffs;

    for (std::size_t i = 0; i < key.data.size(); ++i) {
        poly digit = ring.zero();
        for (std::size_t j = 0; j < remaining.size(); ++j) {
            digit[j] = remaining[j] % key.base;
            remaining[j] /= key.base;
        }
        c0 = ring.add(c0, ring.mul(key.data[i].first, digit));
        c1 = ring.add(c1, ring.mul(key.data[i].second, digit));
    }
    return ciphertext({c0, c1});
}

ciphertext evaluator::relinearize(const ciphertext& ct, const relin_key_v2& key) const {
    ct.require_size(3, "relinearize");

    const poly_ring& ring = ctx_.ring();
    const std::size_t n = ctx_.degree();
    const u64 q = ctx_.q();
    if (key.p == 0 || key.modulus != key.p * q)
        throw std::invalid_argument("relinearize: key modulus does not match the parameters");

    std::vector<u64> c2(n);
    for (std::size_t i = 0; i < n; ++i) c2[i] = ct[2][i] % key.modulus;

    const std::vector<u64> product_b = naive_negacyclic_mul(c2, key.b, key.modulus);
    const std::vector<u64> product_a = naive_negacyclic_mul(c2, key.a, key.modulus);

    poly extra_b = ring.zero();
    poly extra_a = ring.zero();
    for (std::size_t i = 0; i < n; ++i) {
        extra_b[i] = reduce(round_div(i128(center(product_b[i], key.modulus)), i128(key.p)), q);
        extra_a[i] = reduce(round_div(i128(center(product_a[i], key.modulus)), i128(key.p)), q);
    }

    return ciphertext({ring.add(ct[0], extra_b), ring.add(ct[1], extra_a)});
}

} // namespace bfv

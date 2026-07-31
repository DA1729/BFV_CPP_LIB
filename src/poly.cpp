#include "bfv/poly.hpp"

#include "bfv/ntt.hpp"

#include <algorithm>
#include <sstream>
#include <stdexcept>

namespace bfv {

std::vector<i128> exact_negacyclic_mul(const std::vector<i64>& a, const std::vector<i64>& b) {
    if (a.size() != b.size()) throw std::invalid_argument("exact_negacyclic_mul: operand sizes differ");

    const std::size_t n = a.size();
    std::vector<i128> full(2 * n, 0);
    for (std::size_t i = 0; i < n; ++i) {
        if (a[i] == 0) continue;
        const i128 ai = a[i];
        for (std::size_t j = 0; j < n; ++j) full[i + j] += ai * i128(b[j]);
    }

    std::vector<i128> result(n);
    for (std::size_t i = 0; i < n; ++i) result[i] = full[i] - full[i + n];
    return result;
}

std::vector<u64> naive_negacyclic_mul(const std::vector<u64>& a, const std::vector<u64>& b, u64 m) {
    if (a.size() != b.size()) throw std::invalid_argument("naive_negacyclic_mul: operand sizes differ");
    if (m < 2 || m >= max_modulus) throw std::invalid_argument("naive_negacyclic_mul: modulus out of range");

    const std::size_t n = a.size();
    std::vector<u64> result(n, 0);
    for (std::size_t i = 0; i < n; ++i) {
        const u64 ai = a[i] % m;
        if (ai == 0) continue;
        for (std::size_t j = 0; j < n; ++j) {
            const u64 product = mul_mod(ai, b[j] % m, m);
            const std::size_t k = i + j;
            if (k < n) {
                result[k] = add_mod(result[k], product, m);
            } else {
                result[k - n] = sub_mod(result[k - n], product, m);
            }
        }
    }
    return result;
}

poly_ring::poly_ring(std::size_t n, u64 q, ntt_tables tables) : n_(n), q_(q), tables_(std::move(tables)) {
    if (!is_power_of_two(n_)) throw std::invalid_argument("poly_ring: n must be a power of two");
    if (q_ < 2 || q_ >= max_modulus) throw std::invalid_argument("poly_ring: q must lie in [2, 2^62)");
    if (tables_.n != n_ || tables_.q != q_) throw std::invalid_argument("poly_ring: tables do not match n and q");
}

void poly_ring::check(const poly& a) const {
    if (a.size() != n_) throw std::invalid_argument("poly_ring: polynomial has the wrong degree");
}

void poly_ring::check_pair(const poly& a, const poly& b) const {
    check(a);
    check(b);
    if (a.in_ntt != b.in_ntt) throw std::invalid_argument("poly_ring: operands are in different domains");
}

poly poly_ring::zero() const {
    poly result;
    result.coeffs.assign(n_, 0);
    return result;
}

poly poly_ring::from_signed(const std::vector<i64>& values) const {
    if (values.size() != n_) throw std::invalid_argument("poly_ring::from_signed: wrong length");
    poly result = zero();
    for (std::size_t i = 0; i < n_; ++i) result[i] = reduce(values[i], q_);
    return result;
}

std::vector<i64> poly_ring::to_signed(const poly& a) const {
    check(a);
    std::vector<i64> result(n_);
    for (std::size_t i = 0; i < n_; ++i) result[i] = center(a[i], q_);
    return result;
}

poly poly_ring::add(const poly& a, const poly& b) const {
    check_pair(a, b);
    poly result = zero();
    result.in_ntt = a.in_ntt;
    for (std::size_t i = 0; i < n_; ++i) result[i] = add_mod(a[i], b[i], q_);
    return result;
}

poly poly_ring::sub(const poly& a, const poly& b) const {
    check_pair(a, b);
    poly result = zero();
    result.in_ntt = a.in_ntt;
    for (std::size_t i = 0; i < n_; ++i) result[i] = sub_mod(a[i], b[i], q_);
    return result;
}

poly poly_ring::neg(const poly& a) const {
    check(a);
    poly result = zero();
    result.in_ntt = a.in_ntt;
    for (std::size_t i = 0; i < n_; ++i) result[i] = neg_mod(a[i], q_);
    return result;
}

poly poly_ring::mul(const poly& a, const poly& b) const {
    check_pair(a, b);
    poly result = zero();
    result.in_ntt = a.in_ntt;

    if (a.in_ntt) {
        for (std::size_t i = 0; i < n_; ++i) result[i] = mul_mod(a[i], b[i], q_);
        return result;
    }

    std::vector<u64> left(n_), right(n_);
    for (std::size_t i = 0; i < n_; ++i) {
        left[i] = mul_mod(a[i], tables_.psi[i], q_);
        right[i] = mul_mod(b[i], tables_.psi[i], q_);
    }

    ntt_inplace(left, tables_.w, q_);
    ntt_inplace(right, tables_.w, q_);
    for (std::size_t i = 0; i < n_; ++i) left[i] = mul_mod(left[i], right[i], q_);
    intt_inplace(left, tables_.w_inv, q_);

    for (std::size_t i = 0; i < n_; ++i) result[i] = mul_mod(left[i], tables_.psi_inv[i], q_);
    return result;
}

poly poly_ring::mul_scalar(const poly& a, u64 scalar) const {
    check(a);
    poly result = zero();
    result.in_ntt = a.in_ntt;
    const u64 s = scalar % q_;
    for (std::size_t i = 0; i < n_; ++i) result[i] = mul_mod(a[i], s, q_);
    return result;
}

poly poly_ring::to_ntt(const poly& a) const {
    check(a);
    if (a.in_ntt) return a;
    poly result = a;
    ntt_inplace(result.coeffs, tables_.w, q_);
    result.in_ntt = true;
    return result;
}

poly poly_ring::from_ntt(const poly& a) const {
    check(a);
    if (!a.in_ntt) return a;
    poly result = a;
    intt_inplace(result.coeffs, tables_.w_inv, q_);
    result.in_ntt = false;
    return result;
}

poly poly_ring::sample_uniform(rng& source) const {
    poly result = zero();
    for (std::size_t i = 0; i < n_; ++i) result[i] = source.uniform(q_);
    return result;
}

poly poly_ring::sample_ternary(rng& source) const {
    poly result = zero();
    for (std::size_t i = 0; i < n_; ++i) result[i] = reduce(source.ternary(), q_);
    return result;
}

poly poly_ring::sample_gaussian(rng& source, double sigma) const {
    poly result = zero();
    for (std::size_t i = 0; i < n_; ++i) result[i] = reduce(source.gaussian(sigma), q_);
    return result;
}

bool poly_ring::equal(const poly& a, const poly& b) const {
    return a.in_ntt == b.in_ntt && a.coeffs == b.coeffs;
}

u64 poly_ring::max_abs_centered(const poly& a) const {
    check(a);
    u64 largest = 0;
    for (std::size_t i = 0; i < n_; ++i) {
        const i64 value = center(a[i], q_);
        const u64 magnitude = value < 0 ? u64(-value) : u64(value);
        if (magnitude > largest) largest = magnitude;
    }
    return largest;
}

std::string poly_ring::to_string(const poly& a, std::size_t terms) const {
    check(a);
    const std::size_t shown = std::min(terms, n_);
    std::ostringstream out;
    out << "[";
    for (std::size_t i = 0; i < shown; ++i) {
        if (i > 0) out << ", ";
        out << a[i];
    }
    if (shown < n_) out << ", ...";
    out << "]";
    if (a.in_ntt) out << " (ntt)";
    return out.str();
}

} // namespace bfv

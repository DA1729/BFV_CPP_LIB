#ifndef BFV_POLY_HPP
#define BFV_POLY_HPP

#include "bfv/params.hpp"
#include "bfv/rng.hpp"

#include <string>
#include <vector>

namespace bfv {

// An element of Z_q[x]/(x^n + 1). The owning poly_ring holds n, q and the
// transform tables, so a polynomial is only its coefficients plus a flag
// recording whether they are the coefficient or the evaluation representation.
struct poly {
    std::vector<u64> coeffs;
    bool in_ntt = false;

    std::size_t size() const { return coeffs.size(); }
    u64& operator[](std::size_t i) { return coeffs[i]; }
    const u64& operator[](std::size_t i) const { return coeffs[i]; }
};

using plaintext = poly;

// Exact negacyclic product over the integers, with no modular reduction.
// Inputs are centred representatives; the result is what the BFV
// multiplication needs before it rescales by t/q.
std::vector<i128> exact_negacyclic_mul(const std::vector<i64>& a, const std::vector<i64>& b);

// Schoolbook negacyclic product modulo an arbitrary m, used for the moduli
// that have no transform tables (for example p * q in relinearisation v2).
std::vector<u64> naive_negacyclic_mul(const std::vector<u64>& a, const std::vector<u64>& b, u64 m);

class poly_ring {
public:
    poly_ring(std::size_t n, u64 q, ntt_tables tables);

    std::size_t degree() const { return n_; }
    u64 modulus() const { return q_; }
    const ntt_tables& tables() const { return tables_; }

    poly zero() const;
    poly from_signed(const std::vector<i64>& values) const;
    std::vector<i64> to_signed(const poly& a) const;

    poly add(const poly& a, const poly& b) const;
    poly sub(const poly& a, const poly& b) const;
    poly neg(const poly& a) const;
    poly mul(const poly& a, const poly& b) const;
    poly mul_scalar(const poly& a, u64 scalar) const;

    poly to_ntt(const poly& a) const;
    poly from_ntt(const poly& a) const;

    poly sample_uniform(rng& source) const;
    poly sample_ternary(rng& source) const;
    poly sample_gaussian(rng& source, double sigma) const;

    bool equal(const poly& a, const poly& b) const;
    u64 max_abs_centered(const poly& a) const;
    std::string to_string(const poly& a, std::size_t terms = 8) const;

private:
    void check(const poly& a) const;
    void check_pair(const poly& a, const poly& b) const;

    std::size_t n_;
    u64 q_;
    ntt_tables tables_;
};

} // namespace bfv

#endif

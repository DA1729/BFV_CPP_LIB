#ifndef BFV_PARAMS_HPP
#define BFV_PARAMS_HPP

#include "bfv/mod_arith.hpp"
#include "bfv/rng.hpp"

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace bfv {

// Powers of the primitive n-th root w and of the primitive 2n-th root psi,
// used for the negacyclic convolution in Z_q[x]/(x^n + 1).
struct ntt_tables {
    std::size_t n = 0;
    u64 q = 0;
    u64 root = 0;
    std::vector<u64> w;
    std::vector<u64> w_inv;
    std::vector<u64> psi;
    std::vector<u64> psi_inv;

    // root must be a primitive 2n-th root of unity modulo q
    static ntt_tables build(std::size_t n, u64 q, u64 root);
};

struct params {
    std::size_t n = 1024;    // ring degree, a power of two
    u64 q = 132120577;       // ciphertext modulus, prime with 2n | q - 1
    u64 root = 73993;        // primitive 2n-th root of unity modulo q
    u64 t = 16;              // plaintext modulus
    double sigma = 3.2;      // error standard deviation
    u64 relin_base = 16;     // decomposition base T for relinearisation v1
    u64 relin_modulus = 0;   // extra modulus p for relinearisation v2, 0 disables it

    // throws std::invalid_argument describing the first violated constraint
    void validate() const;

    // number of base-T digits needed to decompose an element of Z_q
    std::size_t relin_digits() const;

    std::string to_string() const;
};

bool is_primitive_root(u64 candidate, std::size_t order, u64 q);

// searches for a primitive root of the given order modulo q
std::pair<bool, u64> find_primitive_root(std::size_t order, u64 q, rng& source);

// largest prime below 2^log_q that is congruent to 1 modulo 2n
u64 find_ntt_prime(std::size_t n, unsigned log_q, int rounds, rng& source);

// picks q and root for the requested degree and modulus size
params generate_params(std::size_t n, unsigned log_q, u64 t, rng& source);

namespace presets {

// Verified parameter sets. The relinearisation modulus is chosen so that
// p * q stays below 2^62.
params n1024_logq27();
params n2048_logq37();
params n4096_logq54();

} // namespace presets

} // namespace bfv

#endif

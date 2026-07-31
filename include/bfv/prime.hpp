#ifndef BFV_PRIME_HPP
#define BFV_PRIME_HPP

#include "bfv/mod_arith.hpp"
#include "bfv/rng.hpp"

namespace bfv {

bool miller_rabin(u64 n, int rounds, rng& source);

bool is_prime(u64 n, int rounds, rng& source);

// throws std::runtime_error if no prime is found within the attempt budget
u64 generate_prime(unsigned bits, int rounds, rng& source);

} // namespace bfv

#endif

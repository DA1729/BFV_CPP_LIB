#ifndef BFV_RNG_HPP
#define BFV_RNG_HPP

#include "bfv/mod_arith.hpp"

#include <random>

namespace bfv {

// Seedable source of randomness. Every routine that samples takes one of these
// by reference so that an experiment can be replayed exactly by reusing the
// seed reported by seed().
//
// This is a std::mt19937_64, not a cryptographically secure generator. The
// library is meant for research and experimentation, not for protecting real
// secrets.
class rng {
public:
    rng();
    explicit rng(u64 seed);

    u64 seed() const { return seed_; }

    u64 uniform(u64 bound);
    i64 uniform_range(i64 low, i64 high);
    i64 ternary();
    i64 gaussian(double sigma);

    std::mt19937_64& engine() { return engine_; }

private:
    u64 seed_;
    std::mt19937_64 engine_;
};

} // namespace bfv

#endif

#include "bfv/rng.hpp"

#include <cmath>

namespace bfv {

namespace {

u64 random_seed() {
    std::random_device device;
    return (u64(device()) << 32) | u64(device());
}

} // namespace

rng::rng() : rng(random_seed()) {}

rng::rng(u64 seed) : seed_(seed), engine_(seed) {}

u64 rng::uniform(u64 bound) {
    if (bound <= 1) return 0;
    std::uniform_int_distribution<u64> dist(0, bound - 1);
    return dist(engine_);
}

i64 rng::uniform_range(i64 low, i64 high) {
    std::uniform_int_distribution<i64> dist(low, high);
    return dist(engine_);
}

i64 rng::ternary() {
    return uniform_range(-1, 1);
}

i64 rng::gaussian(double sigma) {
    std::normal_distribution<double> dist(0.0, sigma);
    return static_cast<i64>(std::llround(dist(engine_)));
}

} // namespace bfv

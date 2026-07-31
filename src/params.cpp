#include "bfv/params.hpp"

#include "bfv/prime.hpp"

#include <sstream>
#include <stdexcept>

namespace bfv {

namespace {

constexpr int prime_test_rounds = 40;

std::vector<u64> power_table(u64 base, std::size_t count, u64 q) {
    std::vector<u64> table(count);
    u64 value = 1 % q;
    for (std::size_t i = 0; i < count; ++i) {
        table[i] = value;
        value = mul_mod(value, base, q);
    }
    return table;
}

} // namespace

ntt_tables ntt_tables::build(std::size_t n, u64 q, u64 root) {
    if (!is_power_of_two(n)) throw std::invalid_argument("ntt_tables: n must be a power of two");
    if (q < 2 || q >= max_modulus) throw std::invalid_argument("ntt_tables: q must lie in [2, 2^62)");
    if (!is_primitive_root(root, 2 * n, q))
        throw std::invalid_argument("ntt_tables: root is not a primitive 2n-th root of unity modulo q");

    ntt_tables tables;
    tables.n = n;
    tables.q = q;
    tables.root = root;

    const u64 root_inv = inv_mod(root, q);
    const u64 w = mul_mod(root, root, q);
    const u64 w_inv = mul_mod(root_inv, root_inv, q);

    tables.psi = power_table(root, n, q);
    tables.psi_inv = power_table(root_inv, n, q);
    tables.w = power_table(w, n, q);
    tables.w_inv = power_table(w_inv, n, q);
    return tables;
}

void params::validate() const {
    if (!is_power_of_two(n) || n < 2) throw std::invalid_argument("params: n must be a power of two and at least 2");
    if (q < 2 || q >= max_modulus) throw std::invalid_argument("params: q must lie in [2, 2^62)");
    if (t < 2) throw std::invalid_argument("params: t must be at least 2");
    if (t >= q) throw std::invalid_argument("params: t must be smaller than q");
    if ((q - 1) % (2 * n) != 0) throw std::invalid_argument("params: q must be congruent to 1 modulo 2n");
    if (!is_primitive_root(root, 2 * n, q))
        throw std::invalid_argument("params: root is not a primitive 2n-th root of unity modulo q");
    if (!(sigma > 0.0)) throw std::invalid_argument("params: sigma must be positive");
    if (relin_base < 2) throw std::invalid_argument("params: relin_base must be at least 2");
    if (relin_modulus != 0) {
        if (relin_modulus < 2) throw std::invalid_argument("params: relin_modulus must be 0 or at least 2");
        if (relin_modulus > (max_modulus - 1) / q)
            throw std::invalid_argument("params: relin_modulus * q must stay below 2^62");
    }
}

std::size_t params::relin_digits() const {
    std::size_t digits = 1;
    u64 remaining = q;
    while (remaining >= relin_base) {
        remaining /= relin_base;
        ++digits;
    }
    return digits;
}

std::string params::to_string() const {
    std::ostringstream out;
    out << "n              : " << n << "\n"
        << "q              : " << q << " (" << log2_floor(q) + 1 << " bits)\n"
        << "root           : " << root << "\n"
        << "t              : " << t << "\n"
        << "delta = q / t  : " << q / t << "\n"
        << "sigma          : " << sigma << "\n"
        << "relin_base     : " << relin_base << " (" << relin_digits() << " digits)\n"
        << "relin_modulus  : " << relin_modulus;
    return out.str();
}

bool is_primitive_root(u64 candidate, std::size_t order, u64 q) {
    if (candidate == 0 || order == 0) return false;
    if (pow_mod(candidate, order, q) != 1) return false;
    return pow_mod(candidate, order / 2, q) == q - 1;
}

std::pair<bool, u64> find_primitive_root(std::size_t order, u64 q, rng& source) {
    if (order == 0 || (q - 1) % order != 0) return {false, 0};

    const u64 exponent = (q - 1) / order;
    for (int attempt = 0; attempt < 1000; ++attempt) {
        const u64 a = 2 + source.uniform(q - 3);
        const u64 candidate = pow_mod(a, exponent, q);
        if (is_primitive_root(candidate, order, q)) return {true, candidate};
    }
    return {false, 0};
}

u64 find_ntt_prime(std::size_t n, unsigned log_q, int rounds, rng& source) {
    if (log_q < 2 || log_q > 62) throw std::invalid_argument("find_ntt_prime: log_q must lie in [2, 62]");

    const u64 step = 2 * static_cast<u64>(n);
    const u64 lower = u64(1) << (log_q - 1);
    if ((u64(1) << log_q) <= step) throw std::invalid_argument("find_ntt_prime: log_q is too small for this n");

    u64 candidate = (u64(1) << log_q) - step + 1;
    while (candidate > lower) {
        if (is_prime(candidate, rounds, source)) return candidate;
        candidate -= step;
    }
    throw std::runtime_error("find_ntt_prime: no suitable prime of the requested size");
}

params generate_params(std::size_t n, unsigned log_q, u64 t, rng& source) {
    if (!is_power_of_two(n)) throw std::invalid_argument("generate_params: n must be a power of two");

    params result;
    result.n = n;
    result.t = t;

    u64 candidate = (u64(1) << log_q) + 1;
    for (int attempt = 0; attempt < 64; ++attempt) {
        const u64 q = find_ntt_prime(n, log_q, prime_test_rounds, source);
        if (q >= candidate) break;
        candidate = q;

        const auto [found, root] = find_primitive_root(2 * n, q, source);
        if (!found) continue;

        result.q = q;
        result.root = root;
        result.validate();
        return result;
    }
    throw std::runtime_error("generate_params: no usable modulus and root pair found");
}

namespace presets {

namespace {

params make(std::size_t n, u64 q, u64 root, u64 t, u64 relin_modulus) {
    params result;
    result.n = n;
    result.q = q;
    result.root = root;
    result.t = t;
    result.relin_modulus = relin_modulus;
    result.validate();
    return result;
}

} // namespace

params n1024_logq27() {
    return make(1024, 132120577, 73993, 16, u64(1) << 34);
}

// p * q would have to stay below 2^62, which leaves no room for a useful
// relinearisation modulus at these sizes, so v2 is disabled and v1 is used.
params n2048_logq37() {
    return make(2048, 137438822401, 35158654494, 16, 0);
}

params n4096_logq54() {
    return make(4096, 18014398509309953, 2995990618507561, 16, 0);
}

} // namespace presets

} // namespace bfv

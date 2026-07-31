#include "bfv/ntt.hpp"

#include <stdexcept>

namespace bfv {

namespace {

void unscramble(std::vector<u64>& values, unsigned levels) {
    for (std::size_t i = 0; i < values.size(); ++i) {
        const std::size_t j = static_cast<std::size_t>(bit_reverse(i, levels));
        if (i < j) std::swap(values[i], values[j]);
    }
}

void butterfly_passes(std::vector<u64>& values, const std::vector<u64>& w_table, u64 q, unsigned levels) {
    for (unsigned level = 0; level < levels; ++level) {
        const std::size_t blocks = std::size_t(1) << level;
        const std::size_t step = std::size_t(1) << (levels - level - 1);

        for (std::size_t block = 0; block < blocks; ++block) {
            for (std::size_t k = 0; k < step; ++k) {
                const std::size_t lo = block * (step << 1) + k;
                const std::size_t hi = lo + step;

                const u64 w = w_table[(std::size_t(1) << level) * k];
                const u64 u = values[lo];
                const u64 v = values[hi];

                values[lo] = add_mod(u, v, q);
                values[hi] = mul_mod(sub_mod(u, v, q), w, q);
            }
        }
    }
}

unsigned check_size(const std::vector<u64>& values, const std::vector<u64>& table) {
    if (!is_power_of_two(values.size())) throw std::invalid_argument("ntt: length must be a power of two");
    if (table.size() < values.size()) throw std::invalid_argument("ntt: root table is too short");
    return log2_floor(values.size());
}

} // namespace

void ntt_inplace(std::vector<u64>& values, const std::vector<u64>& w_table, u64 q) {
    const unsigned levels = check_size(values, w_table);
    butterfly_passes(values, w_table, q, levels);
    unscramble(values, levels);
}

void intt_inplace(std::vector<u64>& values, const std::vector<u64>& w_inv_table, u64 q) {
    const unsigned levels = check_size(values, w_inv_table);
    butterfly_passes(values, w_inv_table, q, levels);
    unscramble(values, levels);

    const u64 n_inv = inv_mod(values.size() % q, q);
    for (u64& value : values) value = mul_mod(value, n_inv, q);
}

std::vector<u64> ntt(std::vector<u64> values, const std::vector<u64>& w_table, u64 q) {
    ntt_inplace(values, w_table, q);
    return values;
}

std::vector<u64> intt(std::vector<u64> values, const std::vector<u64>& w_inv_table, u64 q) {
    intt_inplace(values, w_inv_table, q);
    return values;
}

} // namespace bfv

#ifndef BFV_NTT_HPP
#define BFV_NTT_HPP

#include "bfv/mod_arith.hpp"

#include <vector>

namespace bfv {

// Cooley-Tukey style transform over Z_q with natural input order and natural
// output order. w_table[i] must hold w^i mod q for a primitive n-th root w.
void ntt_inplace(std::vector<u64>& values, const std::vector<u64>& w_table, u64 q);

// Inverse of ntt_inplace; w_inv_table[i] must hold w^-i mod q.
void intt_inplace(std::vector<u64>& values, const std::vector<u64>& w_inv_table, u64 q);

std::vector<u64> ntt(std::vector<u64> values, const std::vector<u64>& w_table, u64 q);

std::vector<u64> intt(std::vector<u64> values, const std::vector<u64>& w_inv_table, u64 q);

} // namespace bfv

#endif

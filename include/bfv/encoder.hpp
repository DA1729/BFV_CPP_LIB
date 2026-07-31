#ifndef BFV_ENCODER_HPP
#define BFV_ENCODER_HPP

#include "bfv/context.hpp"
#include "bfv/poly.hpp"

namespace bfv {

// Balanced base-2 encoding: an integer becomes the polynomial whose
// coefficients are its bits, with negative values represented by t - 1.
//
// Homomorphic products grow the coefficients of the encoding, not just the
// noise. Two k-bit operands produce coefficients of magnitude up to k, so a
// product only decodes correctly while k < t / 2. Pick t accordingly, or use a
// smaller input range.
class integer_encoder {
public:
    explicit integer_encoder(const context& ctx);

    plaintext encode(i64 value) const;
    i64 decode(const plaintext& m) const;

    // largest k such that every k-bit input still decodes after one product
    std::size_t safe_product_bits() const;

private:
    const context& ctx_;
};

} // namespace bfv

#endif

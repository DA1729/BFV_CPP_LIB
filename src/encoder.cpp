#include "bfv/encoder.hpp"

#include <cstdint>
#include <stdexcept>

namespace bfv {

integer_encoder::integer_encoder(const context& ctx) : ctx_(ctx) {}

plaintext integer_encoder::encode(i64 value) const {
    const u64 t = ctx_.t();
    const std::size_t n = ctx_.degree();

    plaintext m;
    m.coeffs.assign(n, 0);
    if (value == 0) return m;

    const bool negative = value < 0;
    u64 magnitude = negative ? u64(-(value + 1)) + 1 : u64(value);

    for (std::size_t i = 0; i < n && magnitude != 0; ++i) {
        const u64 bit = magnitude & 1;
        m[i] = negative ? neg_mod(bit, t) : bit;
        magnitude >>= 1;
    }
    if (magnitude != 0) throw std::invalid_argument("integer_encoder::encode: value does not fit in n coefficients");
    return m;
}

i64 integer_encoder::decode(const plaintext& m) const {
    const u64 t = ctx_.t();
    if (m.size() != ctx_.degree()) throw std::invalid_argument("integer_encoder::decode: wrong plaintext length");

    std::size_t highest = 0;
    bool any = false;
    for (std::size_t i = 0; i < m.size(); ++i) {
        if (m[i] % t != 0) {
            highest = i;
            any = true;
        }
    }
    if (!any) return 0;
    if (highest >= 62) throw std::invalid_argument("integer_encoder::decode: value does not fit in an i64");

    i128 result = 0;
    for (std::size_t i = highest + 1; i-- > 0;) result = result * 2 + center(m[i] % t, t);
    if (result > i128(INT64_MAX) || result < i128(INT64_MIN))
        throw std::invalid_argument("integer_encoder::decode: value does not fit in an i64");
    return static_cast<i64>(result);
}

std::size_t integer_encoder::safe_product_bits() const {
    return ctx_.t() < 4 ? 0 : static_cast<std::size_t>(ctx_.t() / 2 - 1);
}

} // namespace bfv

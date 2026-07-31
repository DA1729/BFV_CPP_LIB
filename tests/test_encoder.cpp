#include "bfv/bfv.hpp"
#include "check.hpp"

using namespace bfv;

int main() {
    rng source(7);
    const context ctx(presets::n1024_logq27());
    const integer_encoder encoder(ctx);

    check_eq(encoder.decode(encoder.encode(0)), i64(0));
    check_eq(encoder.decode(encoder.encode(1)), i64(1));
    check_eq(encoder.decode(encoder.encode(-1)), i64(-1));

    for (int trial = 0; trial < 500; ++trial) {
        const i64 value = source.uniform_range(-(i64(1) << 20), (i64(1) << 20) - 1);
        check_eq(encoder.decode(encoder.encode(value)), value);
    }

    // the encoding of a sum is the sum of the encodings, coefficientwise
    {
        const i64 x = 12345;
        const i64 y = -6789;
        const plaintext ex = encoder.encode(x);
        const plaintext ey = encoder.encode(y);

        plaintext sum;
        sum.coeffs.resize(ctx.degree());
        for (std::size_t i = 0; i < ctx.degree(); ++i) sum[i] = add_mod(ex[i], ey[i], ctx.t());
        check_eq(encoder.decode(sum), x + y);
    }

    check_eq(encoder.safe_product_bits(), std::size_t(7));

    return report("encoder");
}

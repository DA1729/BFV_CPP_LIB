#include "bfv/bfv.hpp"

#include <iostream>

using namespace bfv;

namespace {

void show(const char* label, const context& ctx, const ciphertext& ct, const decryptor& dec) {
    std::cout << "  " << label << " " << ctx.ring().to_string(ct[0], 4) << "  budget "
              << dec.noise_budget(ct) << " bits\n";
}

} // namespace

int main() {
    rng source;
    std::cout << "seed " << source.seed() << "\n\n";

    const context ctx(presets::n1024_logq27());
    std::cout << ctx.parameters().to_string() << "\n\n";

    key_generator generator(ctx, source);
    const secret_key sk = generator.generate_secret_key();
    const public_key pk = generator.generate_public_key(sk);
    const relin_key_v1 rlk = generator.generate_relin_key_v1(sk);

    const integer_encoder encoder(ctx);
    encryptor enc(ctx, pk, source);
    const decryptor dec(ctx, sk);
    const evaluator eval(ctx);

    const i64 x = source.uniform_range(-(i64(1) << 15), (i64(1) << 15) - 1);
    const i64 y = source.uniform_range(-(i64(1) << 15), (i64(1) << 15) - 1);

    const ciphertext cx = enc.encrypt(encoder.encode(x));
    const ciphertext cy = enc.encrypt(encoder.encode(y));

    std::cout << "x = " << x << ", y = " << y << "\n";
    show("enc(x)", ctx, cx, dec);
    show("enc(y)", ctx, cy, dec);
    std::cout << "\n";

    const ciphertext sum = eval.add(cx, cy);
    const ciphertext difference = eval.sub(cx, cy);
    show("x + y", ctx, sum, dec);
    std::cout << "    decodes to " << encoder.decode(dec.decrypt(sum)) << ", expected " << x + y << "\n";
    show("x - y", ctx, difference, dec);
    std::cout << "    decodes to " << encoder.decode(dec.decrypt(difference)) << ", expected " << x - y << "\n\n";

    // The base-2 encoder grows coefficients under multiplication, so a product
    // only decodes while the operands stay inside safe_product_bits().
    const i64 bound = i64(1) << encoder.safe_product_bits();
    const i64 u = source.uniform_range(-bound, bound - 1);
    const i64 v = source.uniform_range(-bound, bound - 1);

    const ciphertext cu = enc.encrypt(encoder.encode(u));
    const ciphertext cv = enc.encrypt(encoder.encode(v));
    const ciphertext product = eval.multiply(cu, cv);
    const ciphertext relinearised = eval.relinearize(product, rlk);

    std::cout << "u = " << u << ", v = " << v << "\n";
    show("u * v (size 3)", ctx, product, dec);
    std::cout << "    decodes to " << encoder.decode(dec.decrypt(product)) << ", expected " << u * v << "\n";
    show("u * v (relinearised)", ctx, relinearised, dec);
    std::cout << "    decodes to " << encoder.decode(dec.decrypt(relinearised)) << ", expected " << u * v << "\n";

    return 0;
}

#include "bfv/bfv.hpp"
#include "check.hpp"

using namespace bfv;

int main() {
    rng source(20250731);

    const context ctx(presets::n1024_logq27());
    const integer_encoder encoder(ctx);

    key_generator generator(ctx, source);
    const secret_key sk = generator.generate_secret_key();
    const public_key pk = generator.generate_public_key(sk);
    const relin_key_v1 rlk1 = generator.generate_relin_key_v1(sk);
    const relin_key_v2 rlk2 = generator.generate_relin_key_v2(sk);

    encryptor enc(ctx, pk, source);
    encryptor enc_symmetric(ctx, sk, source);
    const decryptor dec(ctx, sk);
    const evaluator eval(ctx);

    // the secret key must actually be a secret key
    check(!ctx.ring().equal(sk.s, ctx.ring().zero()));

    // the public key must satisfy b + a * s = -e, a small polynomial
    {
        const poly residue = ctx.ring().add(pk.b, ctx.ring().mul(pk.a, sk.s));
        check(ctx.ring().max_abs_centered(residue) < 100);
    }

    const i64 x = 31415;
    const i64 y = -2718;

    const ciphertext cx = enc.encrypt(encoder.encode(x));
    const ciphertext cy = enc.encrypt(encoder.encode(y));

    check_eq(encoder.decode(dec.decrypt(cx)), x);
    check_eq(encoder.decode(dec.decrypt(cy)), y);
    check(dec.noise_budget(cx) > 0);

    // symmetric encryption decrypts under the same key
    check_eq(encoder.decode(dec.decrypt(enc_symmetric.encrypt(encoder.encode(x)))), x);

    check_eq(encoder.decode(dec.decrypt(eval.add(cx, cy))), x + y);
    check_eq(encoder.decode(dec.decrypt(eval.sub(cx, cy))), x - y);
    check_eq(encoder.decode(dec.decrypt(eval.negate(cx))), -x);

    check_eq(encoder.decode(dec.decrypt(eval.add_plain(cx, encoder.encode(y)))), x + y);
    check_eq(encoder.decode(dec.decrypt(eval.sub_plain(cx, encoder.encode(y)))), x - y);

    // products of larger operands overflow the base-2 encoding, so stay inside
    // the range the encoder reports as safe
    const i64 small_x = 97;
    const i64 small_y = -53;
    const ciphertext csx = enc.encrypt(encoder.encode(small_x));
    const ciphertext csy = enc.encrypt(encoder.encode(small_y));

    check_eq(encoder.decode(dec.decrypt(eval.mul_plain(csx, encoder.encode(small_y)))), small_x * small_y);

    const ciphertext product = eval.multiply(csx, csy);
    check_eq(product.size(), std::size_t(3));
    check_eq(encoder.decode(dec.decrypt(product)), small_x * small_y);

    const ciphertext relinearised_v1 = eval.relinearize(product, rlk1);
    check_eq(relinearised_v1.size(), std::size_t(2));
    check_eq(encoder.decode(dec.decrypt(relinearised_v1)), small_x * small_y);

    const ciphertext relinearised_v2 = eval.relinearize(product, rlk2);
    check_eq(relinearised_v2.size(), std::size_t(2));
    check_eq(encoder.decode(dec.decrypt(relinearised_v2)), small_x * small_y);

    // v2 adds less noise than v1 at these parameters
    check(dec.noise_budget(relinearised_v2) >= dec.noise_budget(relinearised_v1));

    // the noise budget must shrink as work accumulates
    check(dec.noise_budget(product) < dec.noise_budget(csx));

    // seeding must make a run reproducible
    {
        rng first(99);
        rng second(99);
        key_generator gen_a(ctx, first);
        key_generator gen_b(ctx, second);
        check(ctx.ring().equal(gen_a.generate_secret_key().s, gen_b.generate_secret_key().s));
    }

    // a wrong key must not decrypt
    {
        const secret_key other = generator.generate_secret_key();
        const decryptor wrong(ctx, other);
        check(encoder.decode(wrong.decrypt(cx)) != x);
    }

    return report("scheme");
}

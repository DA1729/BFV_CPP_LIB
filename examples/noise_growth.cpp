// Measures how the invariant noise budget decays under homomorphic work,
// which is what decides how deep a circuit each parameter set can evaluate.
// A budget of zero means the ciphertext can no longer be trusted to decrypt.

#include "bfv/bfv.hpp"

#include <iomanip>
#include <iostream>

using namespace bfv;

namespace {

void survey(const char* name, const params& p) {
    rng source(20250731);
    const context ctx(p);

    key_generator generator(ctx, source);
    const secret_key sk = generator.generate_secret_key();
    const public_key pk = generator.generate_public_key(sk);
    const relin_key_v1 rlk1 = generator.generate_relin_key_v1(sk);

    const integer_encoder encoder(ctx);
    encryptor enc(ctx, pk, source);
    const decryptor dec(ctx, sk);
    const evaluator eval(ctx);

    const ciphertext one = enc.encrypt(encoder.encode(1));

    std::cout << name << "  n = " << p.n << ", log2 q = " << log2_floor(p.q) + 1 << ", t = " << p.t << "\n";
    std::cout << "  " << std::left << std::setw(26) << "fresh ciphertext" << dec.noise_budget(one) << "\n";

    ciphertext accumulated = one;
    for (int i = 0; i < 8; ++i) accumulated = eval.add(accumulated, one);
    std::cout << "  " << std::setw(26) << "after 8 additions" << dec.noise_budget(accumulated) << "\n";

    const ciphertext product = eval.multiply(one, one);
    std::cout << "  " << std::setw(26) << "one multiplication" << dec.noise_budget(product) << "\n";
    std::cout << "  " << std::setw(26) << "relinearised (v1)" << dec.noise_budget(eval.relinearize(product, rlk1))
              << "\n";
    if (p.relin_modulus != 0) {
        const relin_key_v2 rlk2 = generator.generate_relin_key_v2(sk);
        std::cout << "  " << std::setw(26) << "relinearised (v2)" << dec.noise_budget(eval.relinearize(product, rlk2))
                  << "\n";
    }

    const ciphertext factor = enc.encrypt(encoder.encode(2));
    ciphertext chain = factor;
    i64 expected = 2;
    int depth = 0;
    for (int level = 1; level <= 4; ++level) {
        chain = eval.relinearize(eval.multiply(chain, factor), rlk1);
        expected *= 2;

        bool correct = false;
        try {
            correct = dec.noise_budget(chain) > 0 && encoder.decode(dec.decrypt(chain)) == expected;
        } catch (const std::exception&) {
        }
        if (!correct) break;
        depth = level;
    }
    std::cout << "  " << std::setw(26) << "multiplicative depth" << depth << "\n\n";
}

} // namespace

int main() {
    std::cout << "invariant noise budget in bits\n\n";
    survey("n1024_logq27", presets::n1024_logq27());
    survey("n2048_logq37", presets::n2048_logq37());
    survey("n4096_logq54", presets::n4096_logq54());
    return 0;
}

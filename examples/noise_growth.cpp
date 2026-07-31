// Tracks how the invariant noise budget decays under repeated homomorphic
// work, which is the number that decides how deep a circuit may go.

#include "bfv/bfv.hpp"

#include <iomanip>
#include <iostream>

using namespace bfv;

int main() {
    rng source(20250731);

    const context ctx(presets::n1024_logq27());
    std::cout << ctx.parameters().to_string() << "\n\n";

    key_generator generator(ctx, source);
    const secret_key sk = generator.generate_secret_key();
    const public_key pk = generator.generate_public_key(sk);
    const relin_key_v1 rlk1 = generator.generate_relin_key_v1(sk);
    const relin_key_v2 rlk2 = generator.generate_relin_key_v2(sk);

    const integer_encoder encoder(ctx);
    encryptor enc(ctx, pk, source);
    const decryptor dec(ctx, sk);
    const evaluator eval(ctx);

    const plaintext one = encoder.encode(1);
    const ciphertext fresh = enc.encrypt(one);

    std::cout << std::left << std::setw(28) << "operation" << "budget (bits)\n";
    std::cout << std::setw(28) << "fresh ciphertext" << dec.noise_budget(fresh) << "\n";

    ciphertext accumulator = fresh;
    for (int i = 1; i <= 8; ++i) {
        accumulator = eval.add(accumulator, fresh);
        if (i % 4 == 0)
            std::cout << std::setw(28) << ("after " + std::to_string(i) + " additions")
                      << dec.noise_budget(accumulator) << "\n";
    }

    const ciphertext product = eval.multiply(fresh, fresh);
    std::cout << std::setw(28) << "one multiplication" << dec.noise_budget(product) << "\n";
    std::cout << std::setw(28) << "relinearised (v1)" << dec.noise_budget(eval.relinearize(product, rlk1)) << "\n";
    std::cout << std::setw(28) << "relinearised (v2)" << dec.noise_budget(eval.relinearize(product, rlk2)) << "\n";

    std::cout << "\nmultiplicative depth\n";
    ciphertext chain = fresh;
    for (int depth = 1; depth <= 4; ++depth) {
        chain = eval.relinearize(eval.multiply(chain, fresh), rlk2);
        const int budget = dec.noise_budget(chain);
        const bool correct = encoder.decode(dec.decrypt(chain)) == 1;
        std::cout << "  depth " << depth << ": budget " << budget << " bits, decrypts "
                  << (correct ? "correctly" : "incorrectly") << "\n";
        if (budget == 0) break;
    }

    return 0;
}

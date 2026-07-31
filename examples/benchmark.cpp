// Timings for the primitive operations, so a parameter choice can be judged
// against the cost it implies on the machine that will run the experiment.

#include "bfv/bfv.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>

using namespace bfv;

namespace {

template <typename callable>
double average_ms(callable work, int repetitions) {
    const auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < repetitions; ++i) work();
    const auto elapsed = std::chrono::steady_clock::now() - start;
    return std::chrono::duration<double, std::milli>(elapsed).count() / repetitions;
}

void measure(const char* name, const params& p) {
    rng source(1);
    const context ctx(p);

    key_generator generator(ctx, source);
    const secret_key sk = generator.generate_secret_key();
    const public_key pk = generator.generate_public_key(sk);
    const relin_key_v1 rlk1 = generator.generate_relin_key_v1(sk);

    const integer_encoder encoder(ctx);
    encryptor enc(ctx, pk, source);
    const decryptor dec(ctx, sk);
    const evaluator eval(ctx);

    const plaintext m = encoder.encode(3);
    const ciphertext ct = enc.encrypt(m);
    const ciphertext product = eval.multiply(ct, ct);

    std::cout << std::left << std::setw(15) << name << std::right << std::fixed << std::setprecision(3)
              << std::setw(10) << average_ms([&] { enc.encrypt(m); }, 20) << std::setw(10)
              << average_ms([&] { dec.decrypt(ct); }, 50) << std::setw(10)
              << average_ms([&] { eval.add(ct, ct); }, 500) << std::setw(10)
              << average_ms([&] { eval.mul_plain(ct, m); }, 50) << std::setw(10)
              << average_ms([&] { eval.multiply(ct, ct); }, 3) << std::setw(10)
              << average_ms([&] { eval.relinearize(product, rlk1); }, 5);

    if (p.relin_modulus != 0) {
        const relin_key_v2 rlk2 = generator.generate_relin_key_v2(sk);
        std::cout << std::setw(10) << average_ms([&] { eval.relinearize(product, rlk2); }, 3);
    } else {
        std::cout << std::setw(10) << "-";
    }
    std::cout << "\n";
}

} // namespace

int main() {
    std::cout << std::left << std::setw(15) << "milliseconds" << std::right << std::setw(10) << "encrypt"
              << std::setw(10) << "decrypt" << std::setw(10) << "add" << std::setw(10) << "mul_plain"
              << std::setw(10) << "multiply" << std::setw(10) << "relin_v1" << std::setw(10) << "relin_v2" << "\n";

    measure("n1024_logq27", presets::n1024_logq27());
    measure("n2048_logq37", presets::n2048_logq37());
    measure("n4096_logq54", presets::n4096_logq54());
    return 0;
}

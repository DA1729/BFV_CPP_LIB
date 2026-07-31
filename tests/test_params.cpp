#include "bfv/bfv.hpp"
#include "check.hpp"

using namespace bfv;

int main() {
    rng source(1234);

    for (const params& p : {presets::n1024_logq27(), presets::n2048_logq37(), presets::n4096_logq54()}) {
        p.validate();
        check(is_prime(p.q, 40, source));
        check(is_primitive_root(p.root, 2 * p.n, p.q));
        check(!p.to_string().empty());
    }

    check_eq(presets::n1024_logq27().relin_digits(), std::size_t(7));

    {
        const params generated = generate_params(512, 25, 256, source);
        check_eq(generated.n, std::size_t(512));
        check_eq(log2_floor(generated.q) + 1, 25u);
        check_eq(generated.t, u64(256));
        generated.validate();
        const context usable(generated);
        check_eq(usable.degree(), std::size_t(512));
    }

    // rejected parameter sets
    {
        params bad = presets::n1024_logq27();
        bad.n = 1000;
        check_throws(bad.validate());
    }
    {
        params bad = presets::n1024_logq27();
        bad.root = 2;
        check_throws(bad.validate());
    }
    {
        params bad = presets::n1024_logq27();
        bad.t = bad.q + 1;
        check_throws(bad.validate());
    }
    {
        params bad = presets::n1024_logq27();
        bad.sigma = 0.0;
        check_throws(bad.validate());
    }
    {
        params bad = presets::n1024_logq27();
        bad.relin_modulus = u64(1) << 40;
        check_throws(bad.validate());
    }

    // relinearisation v2 must refuse to produce a key when it is disabled
    {
        params disabled = presets::n1024_logq27();
        disabled.relin_modulus = 0;
        const context ctx(disabled);
        key_generator generator(ctx, source);
        const secret_key sk = generator.generate_secret_key();
        check_throws(generator.generate_relin_key_v2(sk));
    }

    return report("params");
}

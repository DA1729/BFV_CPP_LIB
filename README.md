# BFV C++ Library

A small, readable C++17 implementation of the [BFV](https://eprint.iacr.org/2012/144)
levelled homomorphic encryption scheme, built for research and experimentation
rather than deployment. Everything is in one namespace, has no dependencies
beyond the standard library, and is short enough to read end to end.

> **Not for protecting real secrets.** Randomness comes from `std::mt19937_64`,
> nothing is constant time, and no side-channel hardening has been attempted.
> Use it to prototype protocols, measure noise growth and teach the scheme.

## Building

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build
```

Options: `BFV_BUILD_TESTS` and `BFV_BUILD_EXAMPLES`, both `ON` by default.

Consuming it from another CMake project:

```cmake
add_subdirectory(bfv_cpp)
target_link_libraries(my_target PRIVATE bfv::bfv)
```

## Using it

```cpp
#include "bfv/bfv.hpp"

using namespace bfv;

rng source(12345);                              // reproducible; source.seed() reports it
const context ctx(presets::n2048_logq37());     // validates the parameters up front

key_generator generator(ctx, source);
const secret_key sk = generator.generate_secret_key();
const public_key pk = generator.generate_public_key(sk);
const relin_key_v1 rlk = generator.generate_relin_key_v1(sk);

const integer_encoder encoder(ctx);
encryptor enc(ctx, pk, source);
const decryptor dec(ctx, sk);
const evaluator eval(ctx);

const ciphertext a = enc.encrypt(encoder.encode(42));
const ciphertext b = enc.encrypt(encoder.encode(-7));

const ciphertext sum = eval.add(a, b);
const ciphertext product = eval.relinearize(eval.multiply(a, b), rlk);

encoder.decode(dec.decrypt(sum));       // 35
encoder.decode(dec.decrypt(product));   // -294
dec.noise_budget(product);              // bits of headroom left
```

Every object holding a `const context&` or an `rng&` stores it by reference, so
keep the context and the generator alive for as long as you use them.

## Layout

| Header | Contents |
| --- | --- |
| `bfv/mod_arith.hpp` | modular arithmetic on `u64` with `__int128` intermediates |
| `bfv/prime.hpp` | Miller-Rabin, prime generation |
| `bfv/rng.hpp` | seedable sampler for uniform, ternary and Gaussian values |
| `bfv/ntt.hpp` | forward and inverse number theoretic transform |
| `bfv/params.hpp` | `params`, `ntt_tables`, parameter search, presets |
| `bfv/poly.hpp` | `poly` and `poly_ring`, the ring `Z_q[x]/(x^n + 1)` |
| `bfv/context.hpp` | validated parameters plus derived data |
| `bfv/keys.hpp` | secret, public and relinearisation keys, `key_generator` |
| `bfv/ciphertext.hpp` | `ciphertext` |
| `bfv/encoder.hpp` | `integer_encoder` |
| `bfv/encryptor.hpp` | public key and secret key encryption |
| `bfv/decryptor.hpp` | decryption and the invariant noise budget |
| `bfv/evaluator.hpp` | add, sub, plaintext ops, multiply, relinearise |
| `bfv/bfv.hpp` | umbrella header |

A `poly` is only its coefficients and a domain flag. The transform tables live
in the owning `poly_ring`, so ring operations do not copy them.

## Parameters

`presets` supplies three verified sets; `generate_params(n, log_q, t, source)`
searches for others. `params::validate()` checks that `n` is a power of two,
that `q` is congruent to 1 modulo `2n`, that `root` really is a primitive
`2n`-th root of unity, and that `relin_modulus * q` stays below `2^62`.

Measured with `t = 16` by `examples/noise_growth`:

| preset | fresh budget | after one multiply | multiplicative depth |
| --- | --- | --- | --- |
| `n1024_logq27` | 12 bits | 0 bits | 0 |
| `n2048_logq37` | 22 bits | 8 bits | 1 |
| `n4096_logq54` | 39 bits | 23 bits | 2 |

A multiplication costs roughly `log2(t * n)` bits, so raising `t` or `n` buys
plaintext room at the cost of depth. `decryptor::noise_budget` follows the same
definition as Microsoft SEAL's invariant noise budget: zero means the
ciphertext can no longer be trusted to decrypt.

## Relinearisation

Both variants from the original write-up are implemented.

- **v1**, digit decomposition in base `params.relin_base`. Always available.
  Key size grows with the number of digits.
- **v2**, modulus switching through an auxiliary modulus `params.relin_modulus`.
  Enabled only when that field is non-zero, and it requires `p * q < 2^62`,
  which in practice restricts it to the smaller parameter sets. It adds less
  noise than v1 but runs in `O(n^2)` because `p * q` has no transform tables.

## Encoding

`integer_encoder` uses the balanced base-2 encoding: an integer becomes the
polynomial whose coefficients are its bits, with negatives represented by
`t - 1`. Products grow those coefficients, not just the noise, so a product of
two `k`-bit operands only decodes while `k < t / 2`. `safe_product_bits()`
reports the limit for the current `t`. Additions are unaffected.

## Performance

`examples/benchmark` reports timings for the current machine. On one core of a
desktop CPU, in milliseconds:

| preset | encrypt | decrypt | add | mul_plain | multiply | relin v1 | relin v2 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `n1024_logq27` | 0.60 | 0.15 | 0.002 | 0.27 | 3.98 | 3.05 | 17.0 |
| `n2048_logq37` | 1.35 | 0.42 | 0.004 | 0.74 | 15.6 | 9.82 | - |
| `n4096_logq54` | 2.89 | 1.07 | 0.008 | 1.79 | 63.4 | 31.8 | - |

Additions, plaintext multiplications and relinearisation v1 all go through the
NTT and cost `O(n log n)`. Ciphertext multiplication does not: BFV needs the
product over the integers before it rescales by `t / q`, and a single NTT
modulo `q` cannot supply that, so the product is formed schoolbook in 128 bits
and costs `O(n^2)`. That is the dominant cost above and the first thing worth
replacing.

## Limits

- `q < 2^62`, so that `a + b` fits a `u64` and `t * x` fits an `i128`.
- Ciphertext multiplication additionally needs `n * q^2 / 4 < 2^127`, checked at
  the call and reported as an exception.
- Single modulus only; there is no RNS decomposition of `q`.
- No batching. `t` need not be congruent to 1 modulo `2n`, and no SIMD slot
  packing is provided.

## Roadmap

- RNS multiplication over several NTT primes, to replace the `O(n^2)` product.
- Batch encoder for SIMD slots, requiring `t = 1 mod 2n`.
- Key switching and Galois automorphisms, for rotations.
- Serialisation of keys and ciphertexts.

## License

MIT; see `LICENSE`.

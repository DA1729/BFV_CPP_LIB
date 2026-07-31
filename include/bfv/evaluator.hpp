#ifndef BFV_EVALUATOR_HPP
#define BFV_EVALUATOR_HPP

#include "bfv/ciphertext.hpp"
#include "bfv/context.hpp"
#include "bfv/keys.hpp"

namespace bfv {

// Holds a reference to the context, which must outlive it.
class evaluator {
public:
    explicit evaluator(const context& ctx);

    ciphertext add(const ciphertext& a, const ciphertext& b) const;
    ciphertext sub(const ciphertext& a, const ciphertext& b) const;
    ciphertext negate(const ciphertext& a) const;

    ciphertext add_plain(const ciphertext& a, const plaintext& m) const;
    ciphertext sub_plain(const ciphertext& a, const plaintext& m) const;
    ciphertext mul_plain(const ciphertext& a, const plaintext& m) const;

    // Two size-2 ciphertexts produce a size-3 ciphertext. The intermediate
    // product is formed over the integers in 128 bits before it is rescaled by
    // t / q, so no reduction happens too early.
    ciphertext multiply(const ciphertext& a, const ciphertext& b) const;

    ciphertext relinearize(const ciphertext& ct, const relin_key_v1& key) const;
    ciphertext relinearize(const ciphertext& ct, const relin_key_v2& key) const;

private:
    const context& ctx_;
};

} // namespace bfv

#endif

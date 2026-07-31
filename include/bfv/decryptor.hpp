#ifndef BFV_DECRYPTOR_HPP
#define BFV_DECRYPTOR_HPP

#include "bfv/ciphertext.hpp"
#include "bfv/context.hpp"
#include "bfv/keys.hpp"

namespace bfv {

// Holds a reference to the context, which must outlive it.
class decryptor {
public:
    decryptor(const context& ctx, const secret_key& sk);

    // handles ciphertexts of any size; a product is size 3 until relinearised
    plaintext decrypt(const ciphertext& ct) const;

    // Bits of headroom left before decryption starts failing, in the sense of
    // Microsoft SEAL's invariant noise budget. Zero means the ciphertext can
    // no longer be trusted to decrypt.
    int noise_budget(const ciphertext& ct) const;

private:
    poly evaluate_at_secret(const ciphertext& ct) const;

    const context& ctx_;
    secret_key sk_;
};

} // namespace bfv

#endif

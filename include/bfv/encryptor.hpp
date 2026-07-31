#ifndef BFV_ENCRYPTOR_HPP
#define BFV_ENCRYPTOR_HPP

#include "bfv/ciphertext.hpp"
#include "bfv/context.hpp"
#include "bfv/keys.hpp"
#include "bfv/rng.hpp"

namespace bfv {

// Holds references to the context and the generator, so both must outlive it.
class encryptor {
public:
    encryptor(const context& ctx, const public_key& pk, rng& source);
    encryptor(const context& ctx, const secret_key& sk, rng& source);

    ciphertext encrypt(const plaintext& m);

private:
    const context& ctx_;
    rng& rng_;
    bool symmetric_;
    public_key pk_;
    secret_key sk_;
};

} // namespace bfv

#endif

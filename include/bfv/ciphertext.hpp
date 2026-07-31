#ifndef BFV_CIPHERTEXT_HPP
#define BFV_CIPHERTEXT_HPP

#include "bfv/poly.hpp"

#include <stdexcept>
#include <vector>

namespace bfv {

// A ciphertext is a list of ring elements (c0, c1, ...) that decrypts as
// sum_i c_i * s^i. Fresh ciphertexts have two components; a product has three
// until it is relinearised.
struct ciphertext {
    std::vector<poly> parts;

    ciphertext() = default;
    explicit ciphertext(std::vector<poly> parts) : parts(std::move(parts)) {}

    std::size_t size() const { return parts.size(); }

    poly& operator[](std::size_t i) { return parts.at(i); }
    const poly& operator[](std::size_t i) const { return parts.at(i); }

    void require_size(std::size_t expected, const char* where) const {
        if (parts.size() != expected) throw std::invalid_argument(std::string(where) + ": unexpected ciphertext size");
    }
};

} // namespace bfv

#endif

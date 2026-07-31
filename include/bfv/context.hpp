#ifndef BFV_CONTEXT_HPP
#define BFV_CONTEXT_HPP

#include "bfv/params.hpp"
#include "bfv/poly.hpp"

namespace bfv {

// Validated parameters plus the derived data every other component needs.
// Construct one of these first and hand it to the key generator, encryptor,
// decryptor and evaluator. It is immutable and cheap to share by reference.
class context {
public:
    explicit context(params parameters);

    const params& parameters() const { return params_; }
    const poly_ring& ring() const { return ring_; }

    std::size_t degree() const { return params_.n; }
    u64 q() const { return params_.q; }
    u64 t() const { return params_.t; }
    u64 delta() const { return params_.q / params_.t; }

private:
    params params_;
    poly_ring ring_;
};

} // namespace bfv

#endif

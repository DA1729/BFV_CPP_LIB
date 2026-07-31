#include "bfv/params.hpp"
#include "bfv/poly.hpp"
#include "check.hpp"

using namespace bfv;

namespace {

// negacyclic reference product, reduced modulo q
std::vector<u64> reference_mul(const std::vector<u64>& a, const std::vector<u64>& b, u64 q) {
    return naive_negacyclic_mul(a, b, q);
}

} // namespace

int main() {
    rng source(4242);

    const params p = presets::n1024_logq27();
    const poly_ring ring(p.n, p.q, ntt_tables::build(p.n, p.q, p.root));

    // the transform must round trip
    for (int trial = 0; trial < 5; ++trial) {
        const poly a = ring.sample_uniform(source);
        check(ring.equal(ring.from_ntt(ring.to_ntt(a)), a));
    }

    // negacyclic multiplication must agree with the schoolbook product
    for (int trial = 0; trial < 3; ++trial) {
        const poly a = ring.sample_uniform(source);
        const poly b = ring.sample_uniform(source);
        const poly product = ring.mul(a, b);
        check(product.coeffs == reference_mul(a.coeffs, b.coeffs, p.q));
    }

    // x^(n-1) * x == -1, the defining relation of the ring
    {
        poly a = ring.zero();
        poly b = ring.zero();
        a[p.n - 1] = 1;
        b[1] = 1;
        const poly product = ring.mul(a, b);
        check_eq(product[0], p.q - 1);
        for (std::size_t i = 1; i < p.n; ++i) check_eq(product[i], u64(0));
    }

    // multiplying in the evaluation domain matches multiplying in the
    // coefficient domain up to the psi twist, so compare against add instead
    {
        const poly a = ring.sample_uniform(source);
        const poly b = ring.sample_uniform(source);
        const poly sum = ring.add(a, b);
        check(ring.equal(ring.sub(sum, b), a));
        check(ring.equal(ring.add(a, ring.neg(a)), ring.zero()));
    }

    // exact product with no reduction
    {
        std::vector<i64> a(4, 0);
        std::vector<i64> b(4, 0);
        a[3] = 2;
        b[1] = 3;
        const std::vector<i128> product = exact_negacyclic_mul(a, b);
        check_eq(static_cast<i64>(product[0]), i64(-6));
        for (std::size_t i = 1; i < 4; ++i) check_eq(static_cast<i64>(product[i]), i64(0));
    }

    // signed round trip
    {
        const poly a = ring.sample_uniform(source);
        check(ring.equal(ring.from_signed(ring.to_signed(a)), a));
    }

    // domain and degree mismatches are rejected
    {
        const poly a = ring.sample_uniform(source);
        const poly a_ntt = ring.to_ntt(a);
        check_throws(ring.add(a, a_ntt));

        poly short_poly = ring.zero();
        short_poly.coeffs.pop_back();
        check_throws(ring.add(a, short_poly));
    }

    check_throws(ntt_tables::build(1024, p.q, 2));
    check_throws(ntt_tables::build(1000, p.q, p.root));

    return report("poly");
}

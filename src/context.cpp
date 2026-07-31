#include "bfv/context.hpp"

namespace bfv {

namespace {

params validated(params parameters) {
    parameters.validate();
    return parameters;
}

} // namespace

context::context(params parameters)
    : params_(validated(std::move(parameters))),
      ring_(params_.n, params_.q, ntt_tables::build(params_.n, params_.q, params_.root)) {}

} // namespace bfv

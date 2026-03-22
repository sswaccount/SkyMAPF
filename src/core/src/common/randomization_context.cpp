/**
 * @file randomization_context.cpp
 * @brief Implements unified randomization context helpers.
 */
#include "skymapf/common/randomization_context.hpp"

#include <chrono>
#include <sstream>

namespace skymapf::common {

namespace {

std::uint64_t make_entropy_seed() {
    std::random_device rd;
    const auto now_ticks = static_cast<std::uint64_t>(
        std::chrono::high_resolution_clock::now().time_since_epoch().count()
    );
    std::uint64_t value = now_ticks;
    value ^= (static_cast<std::uint64_t>(rd()) << 1);
    value ^= (static_cast<std::uint64_t>(rd()) << 33);
    if (value == 0) {
        value = 1;
    }
    return value;
}

}  // namespace

RandomizationContext RandomizationContext::make_default() {
    return make_random();
}

RandomizationContext RandomizationContext::make_random() {
    return RandomizationContext(make_entropy_seed(), RandomizationMode::Nondeterministic);
}

std::uint64_t RandomizationContext::derive_seed(
    RandomizationDomain domain,
    std::uint64_t salt
) const noexcept {
    std::uint64_t x = root_seed_;
    x ^= static_cast<std::uint64_t>(domain) * 0x9E3779B97F4A7C15ull;
    x ^= salt * 0xD6E8FEB86659FD93ull;
    x = splitmix64(x);
    return x == 0 ? 1 : x;
}

std::string RandomizationContext::derive_suffix(
    RandomizationDomain domain,
    std::uint64_t salt,
    std::size_t width
) const {
    return to_base36(derive_seed(domain, salt), width);
}

std::string RandomizationContext::trace_string() const {
    std::ostringstream os;
    os << "mode=" << (mode_ == RandomizationMode::Deterministic ? "deterministic" : "nondeterministic")
       << ", root_seed=" << root_seed_;
    return os.str();
}

std::uint64_t RandomizationContext::splitmix64(std::uint64_t x) noexcept {
    x += 0x9E3779B97F4A7C15ull;
    x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ull;
    x = (x ^ (x >> 27)) * 0x94D049BB133111EBull;
    return x ^ (x >> 31);
}

std::string RandomizationContext::to_base36(std::uint64_t value, std::size_t width) {
    static constexpr char kAlphabet[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    std::string out(width, '0');
    for (std::size_t i = 0; i < width; ++i) {
        out[width - 1 - i] = kAlphabet[value % 36ull];
        value /= 36ull;
    }
    return out;
}

}  // namespace skymapf::common

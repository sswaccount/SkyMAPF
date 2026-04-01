/**
* @file random_tool.cpp
* @brief Implements process-wide reproducible random tool.
*/

#include "skymapf/utils/random_tool.hpp"

#include <random>
#include <sstream>

namespace skymapf::utils {

RandomTool& RandomTool::instance() {
    static RandomTool tool;
    return tool;
}

void RandomTool::configure_deterministic(std::uint64_t root_seed) noexcept {
    root_seed_ = normalize_seed(root_seed);
    mode_ = RandomMode::Deterministic;
    reset();
}

void RandomTool::configure_random() {
    std::random_device rd;

    const std::uint64_t hi = static_cast<std::uint64_t>(rd());
    const std::uint64_t lo = static_cast<std::uint64_t>(rd());
    root_seed_ = normalize_seed((hi << 32U) ^ lo);

    mode_ = RandomMode::Nondeterministic;
    reset();
}

void RandomTool::reset() noexcept {
    rngs_.clear();
}

std::mt19937_64& RandomTool::rng(RandomDomain domain) noexcept {
    auto it = rngs_.find(domain);
    if (it != rngs_.end()) {
        return it->second;
    }

    const std::uint64_t seed = derive_seed(domain);
    auto [inserted_it, inserted] = rngs_.emplace(domain, std::mt19937_64(seed));
    (void)inserted;

    return inserted_it->second;
}

std::uint64_t RandomTool::next_id_value() noexcept {
    return normalize_seed(splitmix64(id_rng()()));
}

std::uint64_t RandomTool::next_name_value() noexcept {
    return splitmix64(name_rng()());
}

std::uint64_t RandomTool::next_gen_value() noexcept {
    return normalize_seed(splitmix64(generator_rng()()));
}

std::string RandomTool::trace_string() const {
    std::ostringstream oss;
    oss << "RandomTool(root_seed=" << root_seed_
        << ", mode="
        << (mode_ == RandomMode::Deterministic
                ? "deterministic"
                : "nondeterministic")
        << ")";
    return oss.str();
}

std::uint64_t RandomTool::derive_seed(RandomDomain domain) const noexcept {
    const std::uint64_t domain_value = static_cast<std::uint64_t>(domain);
    return splitmix64(root_seed_ ^ splitmix64(domain_value));
}

std::uint64_t RandomTool::normalize_seed(std::uint64_t seed) noexcept {
    return seed == 0 ? 1 : seed;
}

std::uint64_t RandomTool::splitmix64(std::uint64_t x) noexcept {
    x += 0x9E3779B97F4A7C15ULL;
    x = (x ^ (x >> 30U)) * 0xBF58476D1CE4E5B9ULL;
    x = (x ^ (x >> 27U)) * 0x94D049BB133111EBULL;
    x ^= (x >> 31U);
    return x;
}

}  // namespace skymapf::common
/**
 * @file randomization_context.hpp
 * @brief Defines unified randomization context for deterministic derivation.
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <random>
#include <string>

namespace skymapf::common {

/**
 * @brief Randomization mode for generation workflows.
 *
 * Deterministic mode means root seed is explicitly provided and reproducible.
 * Nondeterministic mode means root seed is generated from system entropy.
 */
enum class RandomizationMode : std::uint8_t {
    Deterministic,
    Nondeterministic
};

/**
 * @brief Stable derivation domains used to avoid scattered magic constants.
 */
enum class RandomizationDomain : std::uint64_t {
    IdAgent = 1,
    IdWorld = 2,
    IdTask = 3,
    IdInstance = 4,
    IdScenario = 5,
    NameAgent = 101,
    NameWorld = 102,
    NameTask = 103,
    NameInstance = 104,
    NameScenario = 105,
    GeneratorWorld = 201,
    GeneratorObstacle = 202,
    GeneratorTask = 203,
    GeneratorRoute = 204,
    GeneratorStartTime = 205,
    GeneratorScenario = 206,
    GeneratorInstance = 207
};

/**
 * @brief Unified randomization root and derivation helper.
 *
 * This class keeps one root seed and exposes deterministic domain-based
 * derivation APIs for ids, naming suffixes, and random engines.
 */
class RandomizationContext {
public:
    /**
     * @brief Creates a context with a random root seed.
     */
    static RandomizationContext make_default();

    /**
     * @brief Creates a context with a random root seed.
     */
    static RandomizationContext make_random();

    /**
     * @brief Creates a deterministic context from explicit seed.
     */
    static RandomizationContext make_deterministic(std::uint64_t root_seed) noexcept {
        return RandomizationContext(root_seed, RandomizationMode::Deterministic);
    }

    /**
     * @brief Builds a context from explicit state.
     */
    RandomizationContext(
        std::uint64_t root_seed = 1,
        RandomizationMode mode = RandomizationMode::Deterministic
    ) noexcept
        : root_seed_(root_seed == 0 ? 1 : root_seed), mode_(mode) {}

    /**
     * @brief Returns root seed.
     */
    std::uint64_t root_seed() const noexcept { return root_seed_; }

    /**
     * @brief Returns randomization mode.
     */
    RandomizationMode mode() const noexcept { return mode_; }

    /**
     * @brief Derives deterministic seed for one domain and salt.
     */
    std::uint64_t derive_seed(
        RandomizationDomain domain,
        std::uint64_t salt = 0
    ) const noexcept;

    /**
     * @brief Creates one random engine derived from domain and salt.
     */
    std::mt19937_64 make_rng(
        RandomizationDomain domain,
        std::uint64_t salt = 0
    ) const noexcept {
        return std::mt19937_64(derive_seed(domain, salt));
    }

    /**
     * @brief Derives fixed-width base36 suffix for naming.
     */
    std::string derive_suffix(
        RandomizationDomain domain,
        std::uint64_t salt = 0,
        std::size_t width = 6
    ) const;

    /**
     * @brief Returns a compact debug string for logging.
     */
    std::string trace_string() const;

private:
    static std::uint64_t splitmix64(std::uint64_t x) noexcept;
    static std::string to_base36(std::uint64_t value, std::size_t width);

    std::uint64_t root_seed_{1};
    RandomizationMode mode_{RandomizationMode::Deterministic};
};

}  // namespace skymapf::common

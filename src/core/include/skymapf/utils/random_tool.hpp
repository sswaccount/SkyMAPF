/**
* @file random_tool.hpp
* @brief Defines a process-wide random tool for reproducible generation.
*/
#pragma once

#include <cstdint>
#include <random>
#include <string>
#include <unordered_map>

namespace skymapf::utils {

enum class RandomMode : std::uint8_t {
    Deterministic,
    Nondeterministic,
};

enum class RandomDomain : std::uint64_t {
    Id = 1,
    Name = 2,
    Generator = 3,
};

class RandomTool {
public:
    static RandomTool& instance();

    void configure_deterministic(std::uint64_t root_seed) noexcept;
    void configure_random();
    void reset() noexcept;

    std::uint64_t root_seed() const noexcept { return root_seed_; }
    RandomMode mode() const noexcept { return mode_; }

    std::mt19937_64& rng(RandomDomain domain) noexcept;
    std::mt19937_64& id_rng() noexcept { return rng(RandomDomain::Id); }
    std::mt19937_64& name_rng() noexcept { return rng(RandomDomain::Name); }
    std::mt19937_64& generator_rng() noexcept { return rng(RandomDomain::Generator); }

    std::uint64_t next_id_value() noexcept;
    std::uint64_t next_name_value() noexcept;
    std::uint64_t next_gen_value() noexcept;

    std::string trace_string() const;

private:
    RandomTool() noexcept = default;

    static std::uint64_t splitmix64(std::uint64_t x) noexcept;
    static std::uint64_t normalize_seed(std::uint64_t seed) noexcept;

    std::uint64_t derive_seed(RandomDomain domain) const noexcept;

    std::uint64_t root_seed_{1};
    RandomMode mode_{RandomMode::Deterministic};
    std::unordered_map<RandomDomain, std::mt19937_64> rngs_;
};

}  // namespace skymapf::common
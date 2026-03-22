/**
 * @file default_naming.hpp
 * @brief Defines unified default naming helpers for domain models.
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "skymapf/common/randomization_context.hpp"
#include "skymapf/common/space.hpp"

namespace skymapf::utils {

/**
 * @brief Utility for generating normalized default display names.
 *
 * This helper centralizes the default naming policy for framework domain objects:
 * - Agent:    agent_[suffix]
 * - World:    world_[dim]_[size]_[suffix]
 * - Task:     task_[agent_sz]_[suffix]
 * - Instance: instance_[suffix]
 * - Scenario: scenario_[suffix]
 *
 * Suffix values are deterministic from `(seed, salt)` so generated names
 * are reproducible across runs under the same inputs.
 */
class DefaultNaming {
public:
    /**
     * @brief Generates a deterministic base36 suffix.
     *
     * @param seed Primary reproducibility seed.
     * @param salt Optional disambiguation value within the same seed context.
     * @return Fixed-width base36 suffix string.
     */
    static std::string suffix(std::uint64_t seed, std::uint64_t salt = 0) {
        return to_base36(hash64(seed, salt), 6);
    }
    static std::string suffix(
        const common::RandomizationContext& randomization,
        common::RandomizationDomain domain,
        std::uint64_t salt = 0
    ) {
        return randomization.derive_suffix(domain, salt, 6);
    }

    /**
     * @brief Generates a default agent display name.
     *
     * Format: `agent_[suffix]`.
     *
     * @param seed Primary reproducibility seed.
     * @param salt Optional disambiguation value.
     * @return Generated agent name.
     */
    static std::string agent_name(std::uint64_t seed, std::uint64_t salt = 0) {
        return "agent_" + suffix(seed, salt);
    }
    static std::string agent_name(
        const common::RandomizationContext& randomization,
        std::uint64_t salt = 0
    ) {
        return "agent_" + suffix(randomization, common::RandomizationDomain::NameAgent, salt);
    }

    /**
     * @brief Generates a default world display name.
     *
     * Format: `world_[dim]_[size]_[suffix]`.
     * Examples: `world_2d_32x32_ab12cd`, `world_3d_20x20x8_z9x8y7`.
     *
     * @param spec World space specification.
     * @param seed Primary reproducibility seed.
     * @param salt Optional disambiguation value.
     * @return Generated world name.
     */
    static std::string world_name(
        const common::SpaceSpec& spec,
        std::uint64_t seed,
        std::uint64_t salt = 0
    ) {
        return "world_" + world_shape_tag(spec) + "_" + suffix(seed, salt);
    }
    static std::string world_name(
        const common::SpaceSpec& spec,
        const common::RandomizationContext& randomization,
        std::uint64_t salt = 0
    ) {
        return "world_" + world_shape_tag(spec) + "_"
            + suffix(randomization, common::RandomizationDomain::NameWorld, salt);
    }

    /**
     * @brief Generates a default task display name.
     *
     * Format: `task_[agent_sz]_[suffix]`.
     *
     * @param agent_size Number of agents included in the task.
     * @param seed Primary reproducibility seed.
     * @param salt Optional disambiguation value.
     * @return Generated task name.
     */
    static std::string task_name(std::size_t agent_size, std::uint64_t seed, std::uint64_t salt = 0) {
        return "task_" + std::to_string(agent_size) + "_" + suffix(seed, salt);
    }
    static std::string task_name(
        std::size_t agent_size,
        const common::RandomizationContext& randomization,
        std::uint64_t salt = 0
    ) {
        return "task_" + std::to_string(agent_size) + "_"
            + suffix(randomization, common::RandomizationDomain::NameTask, salt);
    }

    /**
     * @brief Generates a default instance display name.
     *
     * Format: `instance_[suffix]`.
     *
     * @param seed Primary reproducibility seed.
     * @param salt Optional disambiguation value.
     * @return Generated instance name.
     */
    static std::string instance_name(std::uint64_t seed, std::uint64_t salt = 0) {
        return "instance_" + suffix(seed, salt);
    }
    static std::string instance_name(
        const common::RandomizationContext& randomization,
        std::uint64_t salt = 0
    ) {
        return "instance_" + suffix(randomization, common::RandomizationDomain::NameInstance, salt);
    }

    /**
     * @brief Generates a default scenario display name.
     *
     * Format: `scenario_[suffix]`.
     *
     * @param seed Primary reproducibility seed.
     * @param salt Optional disambiguation value.
     * @return Generated scenario name.
     */
    static std::string scenario_name(std::uint64_t seed, std::uint64_t salt = 0) {
        return "scenario_" + suffix(seed, salt);
    }
    static std::string scenario_name(
        const common::RandomizationContext& randomization,
        std::uint64_t salt = 0
    ) {
        return "scenario_" + suffix(randomization, common::RandomizationDomain::NameScenario, salt);
    }

private:
    /**
     * @brief Encodes space dimensions into a stable world-shape tag.
     *
     * 2D format: `2d_[cols]x[rows]`
     * 3D format: `3d_[cols]x[rows]x[layers]`
     *
     * @param spec World space specification.
     * @return Shape tag used by `world_name`.
     */
    static std::string world_shape_tag(const common::SpaceSpec& spec) {
        if (spec.is_3d()) {
            return "3d_" + std::to_string(spec.cols()) + "x" + std::to_string(spec.rows()) + "x"
                + std::to_string(spec.layers());
        }
        return "2d_" + std::to_string(spec.cols()) + "x" + std::to_string(spec.rows());
    }

    /**
     * @brief Computes a deterministic 64-bit mixed hash from seed/salt.
     *
     * This function is for name-suffix generation (non-cryptographic).
     *
     * @param seed Primary reproducibility seed.
     * @param salt Optional disambiguation value.
     * @return Mixed 64-bit value.
     */
    static std::uint64_t hash64(std::uint64_t seed, std::uint64_t salt) {
        std::uint64_t x = seed ^ (salt * 0x9E3779B97F4A7C15ull) ^ 0xA0761D6478BD642Full;
        x ^= (x >> 30);
        x *= 0xBF58476D1CE4E5B9ull;
        x ^= (x >> 27);
        x *= 0x94D049BB133111EBull;
        x ^= (x >> 31);
        return x;
    }

    /**
     * @brief Converts an integer to fixed-width base36.
     *
     * @param value Source integer value.
     * @param width Output width; high-order digits are truncated when needed.
     * @return Lowercase base36 string with left-zero padding.
     */
    static std::string to_base36(std::uint64_t value, std::size_t width) {
        static constexpr char kAlphabet[] = "0123456789abcdefghijklmnopqrstuvwxyz";
        std::string out(width, '0');
        for (std::size_t i = 0; i < width; ++i) {
            out[width - 1 - i] = kAlphabet[value % 36ull];
            value /= 36ull;
        }
        return out;
    }
};

}  // namespace skymapf::utils

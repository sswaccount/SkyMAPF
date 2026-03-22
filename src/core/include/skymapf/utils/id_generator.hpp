/**
 * @file id_generator.hpp
 * @brief Defines unified id generation helpers for core domain objects.
 */
#pragma once

#include <atomic>
#include <cstdint>

#include "skymapf/common/ids.hpp"
#include "skymapf/common/randomization_context.hpp"

namespace skymapf::utils {

/**
 * @brief Generates ids for core domain models.
 *
 * This utility centralizes id completion rules for:
 * - Agent
 * - World
 * - Task
 * - Instance
 * - Scenario
 *
 * It exposes two generation modes:
 * - `next_*_id()`: process-local monotonic ids, suitable for ad-hoc/manual construction.
 * - `deterministic_*_id(seed, salt)`: stable ids derived from seed/salt,
 *   suitable for reproducible generator workflows.
 *
 * Notes:
 * - All methods are thread-safe for concurrent calls.
 * - Returned id is guaranteed non-zero.
 */
class IdGenerator {
public:
    /// Returns next process-local agent id.
    static common::AgentId next_agent_id() noexcept { return next(counter_agent()); }
    /// Returns next process-local world id.
    static common::WorldId next_world_id() noexcept { return next(counter_world()); }
    /// Returns next process-local task id.
    static common::TaskId next_task_id() noexcept { return next(counter_task()); }
    /// Returns next process-local instance id.
    static common::InstanceId next_instance_id() noexcept { return next(counter_instance()); }
    /// Returns next process-local scenario id.
    static common::ScenarioId next_scenario_id() noexcept { return next(counter_scenario()); }

    /**
     * @brief Returns deterministic agent id from seed/salt.
     *
     * @param seed Primary reproducibility seed.
     * @param salt Optional disambiguation value within same seed context.
     */
    static common::AgentId deterministic_agent_id(std::uint64_t seed, std::uint64_t salt = 0) noexcept {
        return non_zero(splitmix64(seed ^ salt ^ 0xA31D'5EED'11ULL));
    }
    static common::AgentId deterministic_agent_id(
        const common::RandomizationContext& randomization,
        std::uint64_t salt = 0
    ) noexcept {
        return non_zero(randomization.derive_seed(common::RandomizationDomain::IdAgent, salt));
    }
    /**
     * @brief Returns deterministic world id from seed/salt.
     *
     * @param seed Primary reproducibility seed.
     * @param salt Optional disambiguation value within same seed context.
     */
    static common::WorldId deterministic_world_id(std::uint64_t seed, std::uint64_t salt = 0) noexcept {
        return non_zero(splitmix64(seed ^ salt ^ 0xB47A'01D2'22ULL));
    }
    static common::WorldId deterministic_world_id(
        const common::RandomizationContext& randomization,
        std::uint64_t salt = 0
    ) noexcept {
        return non_zero(randomization.derive_seed(common::RandomizationDomain::IdWorld, salt));
    }
    /**
     * @brief Returns deterministic task id from seed/salt.
     *
     * @param seed Primary reproducibility seed.
     * @param salt Optional disambiguation value within same seed context.
     */
    static common::TaskId deterministic_task_id(std::uint64_t seed, std::uint64_t salt = 0) noexcept {
        return non_zero(splitmix64(seed ^ salt ^ 0xC29F'8844'33ULL));
    }
    static common::TaskId deterministic_task_id(
        const common::RandomizationContext& randomization,
        std::uint64_t salt = 0
    ) noexcept {
        return non_zero(randomization.derive_seed(common::RandomizationDomain::IdTask, salt));
    }
    /**
     * @brief Returns deterministic instance id from seed/salt.
     *
     * @param seed Primary reproducibility seed.
     * @param salt Optional disambiguation value within same seed context.
     */
    static common::InstanceId deterministic_instance_id(
        std::uint64_t seed,
        std::uint64_t salt = 0
    ) noexcept {
        return non_zero(splitmix64(seed ^ salt ^ 0xD1E8'77A5'44ULL));
    }
    static common::InstanceId deterministic_instance_id(
        const common::RandomizationContext& randomization,
        std::uint64_t salt = 0
    ) noexcept {
        return non_zero(randomization.derive_seed(common::RandomizationDomain::IdInstance, salt));
    }
    /**
     * @brief Returns deterministic scenario id from seed/salt.
     *
     * @param seed Primary reproducibility seed.
     * @param salt Optional disambiguation value within same seed context.
     */
    static common::ScenarioId deterministic_scenario_id(
        std::uint64_t seed,
        std::uint64_t salt = 0
    ) noexcept {
        return non_zero(splitmix64(seed ^ salt ^ 0xE5CC'2209'55ULL));
    }
    static common::ScenarioId deterministic_scenario_id(
        const common::RandomizationContext& randomization,
        std::uint64_t salt = 0
    ) noexcept {
        return non_zero(randomization.derive_seed(common::RandomizationDomain::IdScenario, salt));
    }

private:
    /// Atomically fetches and increments one counter.
    static std::uint64_t next(std::atomic<std::uint64_t>& counter) noexcept {
        return counter.fetch_add(1, std::memory_order_relaxed);
    }

    /// Ensures generated id never returns zero.
    static std::uint64_t non_zero(std::uint64_t value) noexcept {
        return value == 0 ? 1 : value;
    }

    /// SplitMix64 mixing function for deterministic id derivation.
    static std::uint64_t splitmix64(std::uint64_t x) noexcept {
        x += 0x9E3779B97F4A7C15ull;
        x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ull;
        x = (x ^ (x >> 27)) * 0x94D049BB133111EBull;
        return x ^ (x >> 31);
    }

    /// Process-local counter storage for agent ids.
    static std::atomic<std::uint64_t>& counter_agent() noexcept {
        static std::atomic<std::uint64_t> counter{1};
        return counter;
    }
    /// Process-local counter storage for world ids.
    static std::atomic<std::uint64_t>& counter_world() noexcept {
        static std::atomic<std::uint64_t> counter{1};
        return counter;
    }
    /// Process-local counter storage for task ids.
    static std::atomic<std::uint64_t>& counter_task() noexcept {
        static std::atomic<std::uint64_t> counter{1};
        return counter;
    }
    /// Process-local counter storage for instance ids.
    static std::atomic<std::uint64_t>& counter_instance() noexcept {
        static std::atomic<std::uint64_t> counter{1};
        return counter;
    }
    /// Process-local counter storage for scenario ids.
    static std::atomic<std::uint64_t>& counter_scenario() noexcept {
        static std::atomic<std::uint64_t> counter{1};
        return counter;
    }
};

}  // namespace skymapf::utils

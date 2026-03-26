/**
 * @file i_solver.hpp
 * @brief Declares the base solver interface and shared solve data models.
 */
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "../common/ids.hpp"
#include "../instance/model.hpp"

namespace skymapf::solver {

/// Classifies solver implementation families.
enum class SolverFamily {
    Unknown,
    Classical,
    ModelBased,
    AI,
};

/// Standardized solve status outcomes.
enum class SolveStatus {
    Success,
    Infeasible,
    Timeout,
    Error,
};

/// Describes solver metadata exposed to clients.
struct SolverInfo {
    std::string name;
    SolverFamily family{SolverFamily::Unknown};
    std::string version;
    std::string description;
};

/// Common runtime options passed to solver executions.
struct SolveOptions {
    std::uint64_t time_limit_ms{0};
    std::uint64_t random_seed{0};
};

/// Multi-agent planning output represented as per-agent cell sequences.
struct Plan {
    std::vector<std::vector<common::CellIndex>> agent_paths;
};

/// Input bundle for one solver invocation.
struct SolveInstance {
    instance::InstanceModel instance;
};

/// Output bundle for one solver invocation.
struct SolveResult {
    SolveStatus status{SolveStatus::Error};
    Plan plan;
    std::uint64_t elapsed_ms{0};
    std::string message;
};

/**
 * @brief Abstract interface for all planner implementations.
 */
class ISolver {
public:
    virtual ~ISolver() = default;

    /// Returns static solver metadata.
    virtual SolverInfo info() const = 0;
    /**
     * @brief Solves one planning instance under given runtime options.
     *
     * @param instance Planning inputs.
     * @param options Runtime solve options.
     * @return Solve status, optional plan, and diagnostics.
     */
    virtual SolveResult solve(
        const SolveInstance& instance,
        const SolveOptions& options
    ) = 0;
};

}  // namespace skymapf::solver
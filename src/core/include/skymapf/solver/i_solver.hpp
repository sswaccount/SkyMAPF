#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "../agent/agent.hpp"
#include "../common/ids.hpp"
#include "../task/task.hpp"
#include "../world/world_model.hpp"

namespace skymapf::solver {

enum class SolverFamily {
    Unknown,
    Classical,
    ModelBased,
    AI,
};

enum class SolveStatus {
    Success,
    Infeasible,
    Timeout,
    Error,
};

struct SolverInfo {
    std::string name;
    SolverFamily family{SolverFamily::Unknown};
    std::string version;
    std::string description;
};

struct SolveOptions {
    std::uint64_t time_limit_ms{0};
    std::uint64_t random_seed{0};
};

struct Plan {
    std::vector<std::vector<common::CellIndex>> agent_paths;
};

struct SolveInstance {
    world::WorldModel world;
    std::vector<agent::AgentSpec> agents;
    std::vector<task::Task> tasks;
    common::TimeStep current_time{0};
};

struct SolveResult {
    SolveStatus status{SolveStatus::Error};
    Plan plan;
    std::uint64_t elapsed_ms{0};
    std::string message;
};

class ISolver {
public:
    virtual ~ISolver() = default;

    virtual SolverInfo info() const = 0;
    virtual SolveResult solve(
        const SolveInstance& instance,
        const SolveOptions& options
    ) = 0;
};

}  // namespace skymapf::solver
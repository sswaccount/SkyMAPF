/**
 * @file run_record.hpp
 * @brief Defines the canonical result of one benchmark run.
 */
#pragma once

#include <cstdint>

#include "skymapf/common/ids.hpp"
#include "skymapf/solution/validation.hpp"
#include "skymapf/solver/model/solver_info.hpp"
#include "skymapf/solver/solve_result.hpp"

namespace skymapf::benchmark {

/// Framework-owned record produced after solving, timing, and validation.
struct RunRecord {
    common::InstanceId instance_id{0};
    solver::SolverInfo solver_info;
    solver::SolveStatus solver_status{solver::SolveStatus::Error};
    solver::SolveResult result;
    bool validation_performed{false};
    solution::ValidationResult validation;
    std::uint64_t random_seed{0};
    std::uint64_t wall_time_ms{0};

    bool valid_success() const noexcept {
        return result.status == solver::SolveStatus::Success &&
            validation_performed && validation.valid();
    }
};

}  // namespace skymapf::benchmark

/**
 * @file validation.hpp
 * @brief Declares solution validation helpers.
 */
#pragma once

#include <string>
#include <vector>

#include "plan.hpp"
#include "skymapf/instance/model.hpp"

namespace skymapf::solution {

enum class ValidationErrorKind {
    MissingPath,
    EmptyPath,
    InvalidStart,
    InvalidGoal,
    InvalidCell,
    InvalidMove,
    VertexConflict,
    EdgeConflict,
};

struct ValidationError {
    ValidationErrorKind kind{ValidationErrorKind::InvalidCell};
    common::AgentId agent_id{0};
    common::AgentId other_agent_id{0};
    common::TimeStep time{0};
    common::CellIndex cell{0};
    common::CellIndex from{0};
    common::CellIndex to{0};
    std::string message;
};

struct ValidationResult {
    std::vector<ValidationError> errors;

    bool valid() const noexcept { return errors.empty(); }
};

class SolutionValidator {
public:
    static ValidationResult validate(
        const instance::InstanceModel& instance,
        const Plan& plan
    );
};

}  // namespace skymapf::solution

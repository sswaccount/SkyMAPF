#include "skymapf/solution/validation.hpp"

#include <algorithm>
#include <string>
#include <unordered_set>

namespace skymapf::solution {

namespace {

bool is_wait_or_neighbor(
    const world::WorldModel& world,
    common::CellIndex from,
    common::CellIndex to
) {
    if (from == to) {
        return true;
    }
    const auto neighbors = world.neighbors(from);
    return std::find(neighbors.begin(), neighbors.end(), to) != neighbors.end();
}

void add_error(
    ValidationResult& result,
    ValidationErrorKind kind,
    common::AgentId agent_id,
    common::TimeStep time,
    std::string message
) {
    ValidationError error;
    error.kind = kind;
    error.agent_id = agent_id;
    error.time = time;
    error.message = std::move(message);
    result.errors.push_back(std::move(error));
}

}  // namespace

ValidationResult SolutionValidator::validate(
    const instance::InstanceModel& instance,
    const Plan& plan
) {
    ValidationResult result;
    const auto& world = instance.world();

    std::unordered_set<common::AgentId> seen_agent_ids;
    for (const auto& path : plan.agent_paths) {
        if (!instance.task().has_agent(path.agent_id)) {
            add_error(
                result,
                ValidationErrorKind::UnexpectedPath,
                path.agent_id,
                path.start_time,
                "path belongs to an agent outside the instance"
            );
        }
        if (!seen_agent_ids.insert(path.agent_id).second) {
            add_error(
                result,
                ValidationErrorKind::DuplicatePath,
                path.agent_id,
                path.start_time,
                "duplicate path for agent"
            );
        }
    }

    for (const auto& entry : instance.task().agent_entries()) {
        const auto* path = plan.find_path(entry.agent_id);
        if (!path) {
            add_error(result, ValidationErrorKind::MissingPath, entry.agent_id, 0, "missing agent path");
            continue;
        }
        if (path->cells.empty()) {
            add_error(result, ValidationErrorKind::EmptyPath, entry.agent_id, path->start_time, "empty agent path");
            continue;
        }
        if (path->start_time != entry.start_time) {
            add_error(result, ValidationErrorKind::InvalidStart, entry.agent_id, path->start_time, "unexpected path start time");
        }
        if (entry.visit_sequence.has_minimum_points() && path->front() != entry.visit_sequence.start()) {
            add_error(result, ValidationErrorKind::InvalidStart, entry.agent_id, path->start_time, "path starts at wrong cell");
        }
        if (entry.visit_sequence.has_minimum_points() && path->back() != entry.visit_sequence.goal()) {
            add_error(result, ValidationErrorKind::InvalidGoal, entry.agent_id, path->end_time(), "path ends at wrong cell");
        }
        for (std::size_t i = 0; i < path->cells.size(); ++i) {
            const auto cell = path->cells[i];
            const auto time = static_cast<common::TimeStep>(path->start_time + i);
            if (!world.is_walkable(cell)) {
                ValidationError error;
                error.kind = ValidationErrorKind::InvalidCell;
                error.agent_id = path->agent_id;
                error.time = time;
                error.cell = cell;
                error.message = "path visits invalid or blocked cell";
                result.errors.push_back(std::move(error));
            }
            if (i > 0 && !is_wait_or_neighbor(world, path->cells[i - 1], cell)) {
                ValidationError error;
                error.kind = ValidationErrorKind::InvalidMove;
                error.agent_id = path->agent_id;
                error.time = time;
                error.from = path->cells[i - 1];
                error.to = cell;
                error.message = "path uses invalid move";
                result.errors.push_back(std::move(error));
            }
        }
    }

    for (std::size_t i = 0; i < plan.agent_paths.size(); ++i) {
        for (std::size_t j = i + 1; j < plan.agent_paths.size(); ++j) {
            const auto& a = plan.agent_paths[i];
            const auto& b = plan.agent_paths[j];
            const auto horizon = std::max(a.end_time(), b.end_time());
            for (common::TimeStep t = 0; t <= horizon; ++t) {
                const auto a_loc = a.location_at(t);
                const auto b_loc = b.location_at(t);
                if (a_loc == b_loc) {
                    ValidationError error;
                    error.kind = ValidationErrorKind::VertexConflict;
                    error.agent_id = a.agent_id;
                    error.other_agent_id = b.agent_id;
                    error.time = t;
                    error.cell = a_loc;
                    error.message = "vertex conflict";
                    result.errors.push_back(std::move(error));
                }
                if (t == 0) {
                    continue;
                }
                const auto a_prev = a.location_at(static_cast<common::TimeStep>(t - 1));
                const auto b_prev = b.location_at(static_cast<common::TimeStep>(t - 1));
                if (a_prev == b_loc && b_prev == a_loc) {
                    ValidationError error;
                    error.kind = ValidationErrorKind::EdgeConflict;
                    error.agent_id = a.agent_id;
                    error.other_agent_id = b.agent_id;
                    error.time = t;
                    error.from = a_prev;
                    error.to = a_loc;
                    error.message = "edge conflict";
                    result.errors.push_back(std::move(error));
                }
            }
        }
    }

    return result;
}

}  // namespace skymapf::solution

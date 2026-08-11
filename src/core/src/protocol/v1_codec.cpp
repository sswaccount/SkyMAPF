#include "skymapf/protocol/v1_codec.hpp"

#include <algorithm>
#include <exception>
#include <initializer_list>
#include <limits>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

#include "skymapf/common/index.hpp"
#include "skymapf/task/policy.hpp"
#include "skymapf/utils/checksum.hpp"

namespace skymapf::protocol {

namespace {

using ordered_json = nlohmann::ordered_json;

void set_error(std::string* error_message, std::string message) {
    if (error_message) {
        *error_message = std::move(message);
    }
}

void clear_error(std::string* error_message) {
    if (error_message) {
        error_message->clear();
    }
}

const char* objective_name(ObjectiveV1 objective) noexcept {
    return objective == ObjectiveV1::Makespan ? "makespan" : "sum_of_costs";
}

const char* status_name(solver::SolveStatus status) noexcept {
    switch (status) {
        case solver::SolveStatus::Success: return "success";
        case solver::SolveStatus::Infeasible: return "infeasible";
        case solver::SolveStatus::Timeout: return "timeout";
        case solver::SolveStatus::Unsupported: return "unsupported";
        case solver::SolveStatus::Error: return "error";
    }
    return "error";
}

const char* validation_code(solution::ValidationErrorKind kind) noexcept {
    switch (kind) {
        case solution::ValidationErrorKind::MissingPath: return "missing_path";
        case solution::ValidationErrorKind::UnexpectedPath: return "unexpected_path";
        case solution::ValidationErrorKind::DuplicatePath: return "duplicate_path";
        case solution::ValidationErrorKind::EmptyPath: return "empty_path";
        case solution::ValidationErrorKind::InvalidStart: return "invalid_start";
        case solution::ValidationErrorKind::InvalidGoal: return "invalid_goal";
        case solution::ValidationErrorKind::InvalidCell: return "invalid_cell";
        case solution::ValidationErrorKind::InvalidMove: return "invalid_move";
        case solution::ValidationErrorKind::VertexConflict: return "vertex_conflict";
        case solution::ValidationErrorKind::EdgeConflict: return "edge_conflict";
    }
    return "invalid_cell";
}

const char* final_status_name(const benchmark::RunRecord& record) noexcept {
    if (record.solver_status == solver::SolveStatus::Success) {
        if (record.result.status == solver::SolveStatus::Success &&
            record.validation_performed && record.validation.valid()) {
            return "valid_success";
        }
        return record.validation_performed && !record.validation.valid()
            ? "invalid_solution"
            : "error";
    }
    return status_name(record.solver_status);
}

bool has_only_keys(
    const nlohmann::json& value,
    std::initializer_list<std::string_view> allowed
) {
    if (!value.is_object()) {
        return false;
    }
    for (auto it = value.begin(); it != value.end(); ++it) {
        bool found = false;
        for (const auto key : allowed) {
            if (it.key() == key) {
                found = true;
                break;
            }
        }
        if (!found) {
            return false;
        }
    }
    return true;
}

std::optional<std::uint64_t> nonnegative_integer(const nlohmann::json& value) {
    if (value.is_number_unsigned()) {
        return value.get<std::uint64_t>();
    }
    if (value.is_number_integer()) {
        const auto signed_value = value.get<std::int64_t>();
        if (signed_value >= 0) {
            return static_cast<std::uint64_t>(signed_value);
        }
    }
    return std::nullopt;
}

std::optional<std::int32_t> coordinate_component(const nlohmann::json& value) {
    const auto parsed = nonnegative_integer(value);
    if (!parsed ||
        *parsed > static_cast<std::uint64_t>(std::numeric_limits<std::int32_t>::max())) {
        return std::nullopt;
    }
    return static_cast<std::int32_t>(*parsed);
}

std::optional<solver::SolveStatus> parse_status(const std::string& status) {
    if (status == "success") return solver::SolveStatus::Success;
    if (status == "infeasible") return solver::SolveStatus::Infeasible;
    if (status == "timeout") return solver::SolveStatus::Timeout;
    if (status == "unsupported") return solver::SolveStatus::Unsupported;
    if (status == "error") return solver::SolveStatus::Error;
    return std::nullopt;
}

ordered_json coordinate_json(common::CellIndex cell, common::GridShape2D shape) {
    const auto coord = common::to_coord_2d(cell, shape);
    return ordered_json::array({coord.x, coord.y});
}

bool has_standard_four_neighbor_connectivity(const world::WorldModel& world) {
    const auto shape = world.space_spec().shape_2d();
    for (common::CellIndex cell = 0; cell < world.cell_count(); ++cell) {
        if (!world.is_walkable(cell)) {
            continue;
        }
        const auto coord = common::to_coord_2d(cell, shape);
        std::vector<common::CellIndex> expected;
        const common::CellCoord2D candidates[] = {
            {coord.x - 1, coord.y},
            {coord.x + 1, coord.y},
            {coord.x, coord.y - 1},
            {coord.x, coord.y + 1},
        };
        for (const auto& candidate : candidates) {
            if (common::is_in_bounds(candidate, shape) && world.is_walkable(candidate)) {
                expected.push_back(common::to_index(candidate, shape));
            }
        }
        auto actual = world.neighbors(cell);
        std::sort(expected.begin(), expected.end());
        std::sort(actual.begin(), actual.end());
        if (actual != expected) {
            return false;
        }
    }
    return true;
}

}  // namespace

std::optional<ordered_json> ProtocolV1Codec::encode_problem(
    const instance::InstanceModel& instance,
    const std::string& problem_id,
    ObjectiveV1 objective,
    std::string* error_message
) {
    clear_error(error_message);
    if (problem_id.empty()) {
        set_error(error_message, "protocol problem_id must not be empty");
        return std::nullopt;
    }
    const auto& world = instance.world();
    if (!world.space_spec().is_2d()) {
        set_error(error_message, "protocol v1 supports only 2D worlds");
        return std::nullopt;
    }
    const auto shape = world.space_spec().shape_2d();
    if (!has_standard_four_neighbor_connectivity(world)) {
        set_error(
            error_message,
            "instance connectivity does not satisfy movingai_standard_v1"
        );
        return std::nullopt;
    }

    ordered_json blocked_cells = ordered_json::array();
    for (common::CellIndex cell = 0; cell < world.cell_count(); ++cell) {
        if (!world.is_walkable(cell)) {
            blocked_cells.push_back(coordinate_json(cell, shape));
        }
    }

    std::vector<const task::AgentTaskEntry*> sorted_entries;
    sorted_entries.reserve(instance.task().agent_entries().size());
    for (const auto& entry : instance.task().agent_entries()) {
        sorted_entries.push_back(&entry);
    }
    std::sort(
        sorted_entries.begin(),
        sorted_entries.end(),
        [](const auto* lhs, const auto* rhs) { return lhs->agent_id < rhs->agent_id; }
    );

    ordered_json agents = ordered_json::array();
    common::AgentId previous_agent_id = 0;
    for (const auto* entry_ptr : sorted_entries) {
        const auto& entry = *entry_ptr;
        if (entry.agent_id == 0 || entry.agent_id == previous_agent_id) {
            set_error(error_message, "protocol v1 requires unique positive agent ids");
            return std::nullopt;
        }
        previous_agent_id = entry.agent_id;
        if (entry.start_time != 0 ||
            entry.goal_behavior != task::GoalArrivalBehavior::StayAtGoal ||
            entry.visit_sequence.checkpoints.size() != 2) {
            set_error(
                error_message,
                "instance does not satisfy movingai_standard_v1 task semantics"
            );
            return std::nullopt;
        }
        const auto start = entry.visit_sequence.start();
        const auto goal = entry.visit_sequence.goal();
        if (!world.is_walkable(start) || !world.is_walkable(goal)) {
            set_error(error_message, "instance contains an invalid or blocked start or goal");
            return std::nullopt;
        }
        agents.push_back(ordered_json{
            {"id", entry.agent_id},
            {"start", coordinate_json(start, shape)},
            {"goal", coordinate_json(goal, shape)},
        });
    }
    if (agents.empty()) {
        set_error(error_message, "protocol v1 requires at least one agent");
        return std::nullopt;
    }

    return ordered_json{
        {"protocol_version", kProblemProtocolV1},
        {"problem_id", problem_id},
        {"problem_type", "standard_mapf"},
        {"semantics", kStandardMapfSemanticsV1},
        {"objective", objective_name(objective)},
        {"world", ordered_json{
            {"kind", "grid_2d"},
            {"width", shape.cols},
            {"height", shape.rows},
            {"blocked_cells", std::move(blocked_cells)},
        }},
        {"agents", std::move(agents)},
        {"metadata", ordered_json{
            {"instance_id", instance.id()},
            {"instance_name", instance.name()},
        }},
    };
}

std::optional<ordered_json> ProtocolV1Codec::encode_solve_request(
    const instance::InstanceModel& instance,
    const solver::SolveOptions& options,
    const std::string& problem_id,
    ObjectiveV1 objective,
    std::string* error_message
) {
    auto problem = encode_problem(instance, problem_id, objective, error_message);
    if (!problem) {
        return std::nullopt;
    }
    return ordered_json{
        {"protocol_version", kSolveRequestProtocolV1},
        {"problem", std::move(*problem)},
        {"options", ordered_json{
            {"time_limit_ms", options.time_limit_ms},
            {"random_seed", options.random_seed},
            {"node_limit", options.node_limit},
        }},
    };
}

std::optional<solver::SolveResult> ProtocolV1Codec::decode_raw_solve_result(
    const nlohmann::json& document,
    const instance::InstanceModel& instance,
    const std::string& expected_problem_id,
    std::string* error_message
) {
    clear_error(error_message);
    try {
        if (!has_only_keys(
                document,
                {"protocol_version", "problem_id", "solver", "status", "paths",
                 "statistics", "message"}
            )) {
            set_error(error_message, "raw result contains unknown fields or is not an object");
            return std::nullopt;
        }
        if (
            document.at("protocol_version").get<std::string>() !=
                kRawSolveResultProtocolV1) {
            set_error(error_message, "unexpected raw result protocol version");
            return std::nullopt;
        }
        if (document.at("problem_id").get<std::string>() != expected_problem_id) {
            set_error(error_message, "raw result problem_id does not match request");
            return std::nullopt;
        }
        const auto& solver_document = document.at("solver");
        if (!has_only_keys(
                solver_document,
                {"name", "version", "commit", "configuration"}
            ) ||
            solver_document.at("name").get<std::string>().empty() ||
            solver_document.at("version").get<std::string>().empty()) {
            set_error(error_message, "raw result contains invalid solver metadata");
            return std::nullopt;
        }
        const auto parsed_status = parse_status(document.at("status").get<std::string>());
        if (!parsed_status) {
            set_error(error_message, "raw result contains unknown status");
            return std::nullopt;
        }
        const auto& paths = document.at("paths");
        if (!paths.is_array()) {
            set_error(error_message, "raw result paths must be an array");
            return std::nullopt;
        }
        if ((*parsed_status == solver::SolveStatus::Success && paths.empty()) ||
            (*parsed_status != solver::SolveStatus::Success && !paths.empty())) {
            set_error(error_message, "raw result paths are inconsistent with status");
            return std::nullopt;
        }
        if (document.contains("message") && !document.at("message").is_string()) {
            set_error(error_message, "raw result message must be a string");
            return std::nullopt;
        }
        if (solver_document.contains("commit") &&
            !solver_document.at("commit").is_string()) {
            set_error(error_message, "raw result solver commit must be a string");
            return std::nullopt;
        }
        if (solver_document.contains("configuration") &&
            !solver_document.at("configuration").is_object()) {
            set_error(error_message, "raw result solver configuration must be an object");
            return std::nullopt;
        }

        solver::SolveResult result;
        result.status = *parsed_status;
        if (document.contains("message")) {
            result.message = document.at("message").get<std::string>();
        }
        const auto shape = instance.world().space_spec().shape_2d();
        std::unordered_set<common::AgentId> agent_ids;
        for (const auto& path_document : paths) {
            if (!has_only_keys(path_document, {"agent_id", "cells"})) {
                set_error(error_message, "raw result path contains unknown fields");
                return std::nullopt;
            }
            solution::AgentPath path;
            const auto parsed_agent_id = nonnegative_integer(
                path_document.at("agent_id")
            );
            if (!parsed_agent_id || *parsed_agent_id == 0) {
                set_error(error_message, "raw result agent_id must be a positive integer");
                return std::nullopt;
            }
            path.agent_id = static_cast<common::AgentId>(*parsed_agent_id);
            path.start_time = 0;
            if (!agent_ids.insert(path.agent_id).second) {
                set_error(error_message, "raw result contains duplicate agent paths");
                return std::nullopt;
            }
            const auto& cells = path_document.at("cells");
            if (!cells.is_array()) {
                set_error(error_message, "raw result path cells must be an array");
                return std::nullopt;
            }
            if (cells.empty()) {
                set_error(error_message, "raw result successful paths must not be empty");
                return std::nullopt;
            }
            for (const auto& cell_document : cells) {
                if (!cell_document.is_array() || cell_document.size() != 2) {
                    set_error(error_message, "raw result coordinate must contain x and y");
                    return std::nullopt;
                }
                const auto x = coordinate_component(cell_document.at(0));
                const auto y = coordinate_component(cell_document.at(1));
                if (!x || !y) {
                    set_error(
                        error_message,
                        "raw result coordinates must be nonnegative 32-bit integers"
                    );
                    return std::nullopt;
                }
                const common::CellCoord2D coord{*x, *y};
                if (!common::is_in_bounds(coord, shape)) {
                    set_error(error_message, "raw result path coordinate is out of bounds");
                    return std::nullopt;
                }
                path.cells.push_back(common::to_index(coord, shape));
            }
            result.plan.agent_paths.push_back(std::move(path));
        }

        if (document.contains("statistics")) {
            const auto& statistics = document.at("statistics");
            if (!statistics.is_object()) {
                set_error(error_message, "raw result statistics must be an object");
                return std::nullopt;
            }
            if (statistics.contains("expanded_nodes")) {
                const auto expanded = nonnegative_integer(
                    statistics.at("expanded_nodes")
                );
                if (!expanded) {
                    set_error(error_message, "expanded_nodes must be nonnegative");
                    return std::nullopt;
                }
                result.metrics.expanded_nodes = *expanded;
            }
            if (statistics.contains("generated_nodes")) {
                const auto generated = nonnegative_integer(
                    statistics.at("generated_nodes")
                );
                if (!generated) {
                    set_error(error_message, "generated_nodes must be nonnegative");
                    return std::nullopt;
                }
                result.metrics.generated_nodes = *generated;
            }
        }
        return result;
    } catch (const std::exception& ex) {
        set_error(error_message, std::string{"invalid raw result: "} + ex.what());
        return std::nullopt;
    }
}

std::optional<std::string> ProtocolV1Codec::compute_instance_checksum(
    const instance::InstanceModel& instance,
    std::string* error_message
) {
    auto problem = encode_problem(
        instance,
        "canonical-instance",
        ObjectiveV1::SumOfCosts,
        error_message
    );
    if (!problem) {
        return std::nullopt;
    }
    ordered_json semantic_instance{
        {"problem_type", problem->at("problem_type")},
        {"semantics", problem->at("semantics")},
        {"world", problem->at("world")},
        {"agents", problem->at("agents")},
    };
    return "fnv1a64:" + utils::fnv1a64_hex(semantic_instance.dump());
}

std::optional<ordered_json> ProtocolV1Codec::encode_run_record(
    const benchmark::RunRecord& record,
    const RunRecordV1Context& context,
    std::string* error_message
) {
    clear_error(error_message);
    if (context.run_id.empty() || context.problem_id.empty() ||
        context.instance_checksum.empty()) {
        set_error(
            error_message,
            "run record requires run_id, problem_id, and instance_checksum"
        );
        return std::nullopt;
    }
    if (record.solver_info.name.empty() || record.solver_info.version.empty()) {
        set_error(error_message, "run record requires solver name and version");
        return std::nullopt;
    }
    if (!context.environment.is_object()) {
        set_error(error_message, "run record environment must be an object");
        return std::nullopt;
    }
    if (!record.validation_performed && !record.validation.errors.empty()) {
        set_error(
            error_message,
            "run record cannot contain validation errors when validation was not performed"
        );
        return std::nullopt;
    }
    if (record.solver_status == solver::SolveStatus::Success &&
        record.result.status == solver::SolveStatus::Success &&
        !record.validation_performed) {
        set_error(error_message, "successful run record requires framework validation");
        return std::nullopt;
    }

    ordered_json validation_errors = ordered_json::array();
    for (const auto& error : record.validation.errors) {
        validation_errors.push_back(ordered_json{
            {"code", validation_code(error.kind)},
            {"agent_id", error.agent_id},
            {"other_agent_id", error.other_agent_id},
            {"time", error.time},
            {"cell", error.cell},
            {"from", error.from},
            {"to", error.to},
            {"message", error.message},
        });
    }

    ordered_json document{
        {"protocol_version", kRunRecordProtocolV1},
        {"run_id", context.run_id},
        {"problem_id", context.problem_id},
        {"instance_checksum", context.instance_checksum},
        {"solver_status", status_name(record.solver_status)},
        {"final_status", final_status_name(record)},
        {"validation", ordered_json{
            {"performed", record.validation_performed},
            {"valid", record.validation_performed && record.validation.valid()},
            {"errors", std::move(validation_errors)},
        }},
        {"metrics", ordered_json{
            {"sum_of_costs", record.result.metrics.sum_of_costs},
            {"makespan", record.result.metrics.makespan},
        }},
        {"diagnostics", ordered_json{
            {"expanded_nodes", record.result.metrics.expanded_nodes},
            {"generated_nodes", record.result.metrics.generated_nodes},
            {"high_level_expanded_nodes", record.result.metrics.high_level_expanded_nodes},
            {"high_level_generated_nodes", record.result.metrics.high_level_generated_nodes},
            {"low_level_expanded_nodes", record.result.metrics.low_level_expanded_nodes},
            {"low_level_generated_nodes", record.result.metrics.low_level_generated_nodes},
        }},
        {"resources", ordered_json{
            {"wall_time_ms", record.wall_time_ms},
            {"peak_memory_bytes", context.peak_memory_bytes},
        }},
        {"provenance", ordered_json{
            {"solver_name", record.solver_info.name},
            {"solver_version", record.solver_info.version},
            {"solver_commit", context.solver_commit},
            {"random_seed", record.random_seed},
            {"environment", context.environment},
        }},
    };
    if (!record.result.message.empty()) {
        document["message"] = record.result.message;
    }
    return document;
}

}  // namespace skymapf::protocol

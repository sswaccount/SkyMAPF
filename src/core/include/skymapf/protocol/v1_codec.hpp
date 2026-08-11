/**
 * @file v1_codec.hpp
 * @brief Encodes and decodes the language-neutral SkyMAPF protocol v1.
 */
#pragma once

#include <optional>
#include <string>

#include <nlohmann/json.hpp>

#include "skymapf/benchmark/run_record.hpp"
#include "skymapf/instance/model.hpp"
#include "skymapf/solver/solve_options.hpp"
#include "skymapf/solver/solve_result.hpp"

namespace skymapf::protocol {

inline constexpr const char* kProblemProtocolV1 = "skymapf.problem.v1";
inline constexpr const char* kSolveRequestProtocolV1 = "skymapf.solve-request.v1";
inline constexpr const char* kRawSolveResultProtocolV1 =
    "skymapf.raw-solve-result.v1";
inline constexpr const char* kRunRecordProtocolV1 = "skymapf.run-record.v1";
inline constexpr const char* kStandardMapfSemanticsV1 = "movingai_standard_v1";

enum class ObjectiveV1 {
    SumOfCosts,
    Makespan,
};

/// Runner-owned metadata required to publish one protocol run record.
struct RunRecordV1Context {
    std::string run_id;
    std::string problem_id;
    std::string instance_checksum;
    std::string solver_commit;
    std::uint64_t peak_memory_bytes{0};
    nlohmann::ordered_json environment = nlohmann::ordered_json::object();
};

class ProtocolV1Codec {
public:
    static std::optional<nlohmann::ordered_json> encode_problem(
        const instance::InstanceModel& instance,
        const std::string& problem_id,
        ObjectiveV1 objective = ObjectiveV1::SumOfCosts,
        std::string* error_message = nullptr
    );

    static std::optional<nlohmann::ordered_json> encode_solve_request(
        const instance::InstanceModel& instance,
        const solver::SolveOptions& options,
        const std::string& problem_id,
        ObjectiveV1 objective = ObjectiveV1::SumOfCosts,
        std::string* error_message = nullptr
    );

    static std::optional<solver::SolveResult> decode_raw_solve_result(
        const nlohmann::json& document,
        const instance::InstanceModel& instance,
        const std::string& expected_problem_id,
        std::string* error_message = nullptr
    );

    /**
     * @brief Computes the canonical v1 semantic-instance fingerprint.
     *
     * The fingerprint ignores display metadata, problem id, objective, and
     * agent insertion order. It currently uses the stable FNV-1a 64-bit helper
     * and carries an explicit algorithm prefix.
     */
    static std::optional<std::string> compute_instance_checksum(
        const instance::InstanceModel& instance,
        std::string* error_message = nullptr
    );

    static std::optional<nlohmann::ordered_json> encode_run_record(
        const benchmark::RunRecord& record,
        const RunRecordV1Context& context,
        std::string* error_message = nullptr
    );
};

}  // namespace skymapf::protocol

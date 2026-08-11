#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include "skymapf/common/space.hpp"
#include "skymapf/generator/world_generator.hpp"
#include "skymapf/instance/model.hpp"
#include "skymapf/protocol/v1_codec.hpp"
#include "skymapf/task/model.hpp"

namespace {

std::filesystem::path fixture_path(const char* category, const char* filename) {
    return std::filesystem::path(__FILE__).parent_path().parent_path() /
        "data" / "protocol" / "v1" / category / filename;
}

nlohmann::json read_fixture(const char* category, const char* filename) {
    std::ifstream input(fixture_path(category, filename));
    REQUIRE(input.good());
    nlohmann::json document;
    input >> document;
    return document;
}

skymapf::instance::InstanceModel make_protocol_instance() {
    auto world = skymapf::generator::WorldGenerator::generate(
        skymapf::generator::WorldGenerationOptions(
            skymapf::common::SpaceSpec::make_2d(2, 2),
            0.0
        )
    );
    skymapf::task::TaskModel task(1, "protocol");
    task.add_agent_entry(1, skymapf::task::SequenceModel{{0, 1}});
    task.add_agent_entry(2, skymapf::task::SequenceModel{{1, 0}});
    return skymapf::instance::InstanceModel::create(
        7,
        std::move(world),
        std::move(task),
        "protocol"
    );
}

}  // namespace

TEST_CASE("protocol v1 encodes a reproducible solve request", "[protocol]") {
    const auto instance = make_protocol_instance();
    std::string error;
    const auto request = skymapf::protocol::ProtocolV1Codec::encode_solve_request(
        instance,
        skymapf::solver::SolveOptions{250, 42, 1000},
        "case-7",
        skymapf::protocol::ObjectiveV1::SumOfCosts,
        &error
    );

    REQUIRE(request.has_value());
    REQUIRE(error.empty());
    REQUIRE((*request)["protocol_version"] == "skymapf.solve-request.v1");
    REQUIRE((*request)["problem"]["semantics"] == "movingai_standard_v1");
    REQUIRE((*request)["problem"]["world"]["width"] == 2);
    REQUIRE((*request)["problem"]["agents"].size() == 2);
    REQUIRE((*request)["options"]["time_limit_ms"] == 250);
    REQUIRE((*request)["options"]["random_seed"] == 42);
    REQUIRE((*request)["options"]["node_limit"] == 1000);
    const auto actual = nlohmann::json::parse(request->dump());
    REQUIRE(actual == read_fixture("valid", "solve-request.json"));
}

TEST_CASE("protocol v1 decodes coordinate paths and diagnostics", "[protocol]") {
    const auto instance = make_protocol_instance();
    const nlohmann::json document = {
        {"protocol_version", "skymapf.raw-solve-result.v1"},
        {"problem_id", "case-7"},
        {"solver", {{"name", "fixture"}, {"version", "1"}}},
        {"status", "success"},
        {"paths", nlohmann::json::array({
            {{"agent_id", 1}, {"cells", {{0, 0}, {0, 1}, {1, 1}, {1, 0}}}},
            {{"agent_id", 2}, {"cells", {{1, 0}, {0, 0}}}},
        })},
        {"statistics", {{"expanded_nodes", 7}, {"generated_nodes", 11}}},
    };
    std::string error;
    const auto result = skymapf::protocol::ProtocolV1Codec::decode_raw_solve_result(
        document,
        instance,
        "case-7",
        &error
    );

    REQUIRE(result.has_value());
    REQUIRE(error.empty());
    REQUIRE(result->status == skymapf::solver::SolveStatus::Success);
    REQUIRE(result->plan.agent_paths[0].cells == std::vector<skymapf::common::CellIndex>{0, 2, 3, 1});
    REQUIRE(result->metrics.expanded_nodes == 7);
    REQUIRE(result->metrics.generated_nodes == 11);
}

TEST_CASE("protocol v1 rejects results for another problem", "[protocol]") {
    const auto instance = make_protocol_instance();
    const nlohmann::json document = {
        {"protocol_version", "skymapf.raw-solve-result.v1"},
        {"problem_id", "other"},
        {"solver", {{"name", "fixture"}, {"version", "1"}}},
        {"status", "error"},
        {"paths", nlohmann::json::array()},
    };
    std::string error;
    const auto result = skymapf::protocol::ProtocolV1Codec::decode_raw_solve_result(
        document,
        instance,
        "case-7",
        &error
    );

    REQUIRE_FALSE(result.has_value());
    REQUIRE(error == "raw result problem_id does not match request");
}

TEST_CASE("protocol v1 accepts the published raw-result golden fixtures", "[protocol]") {
    const auto instance = make_protocol_instance();
    std::string error;

    const auto success = skymapf::protocol::ProtocolV1Codec::decode_raw_solve_result(
        read_fixture("valid", "raw-success.json"),
        instance,
        "case-7",
        &error
    );
    REQUIRE(success.has_value());
    REQUIRE(success->status == skymapf::solver::SolveStatus::Success);
    REQUIRE(success->metrics.expanded_nodes == 7);

    const auto timeout = skymapf::protocol::ProtocolV1Codec::decode_raw_solve_result(
        read_fixture("valid", "raw-timeout.json"),
        instance,
        "case-7",
        &error
    );
    REQUIRE(timeout.has_value());
    REQUIRE(timeout->status == skymapf::solver::SolveStatus::Timeout);
    REQUIRE(timeout->plan.empty());
}

TEST_CASE("protocol v1 rejects the published invalid golden fixtures", "[protocol]") {
    const auto instance = make_protocol_instance();
    const std::vector<const char*> fixture_names{
        "raw-extra-field.json",
        "raw-non-success-paths.json",
        "raw-empty-success-path.json",
    };
    for (const auto* fixture_name : fixture_names) {
        INFO(fixture_name);
        std::string error;
        const auto result = skymapf::protocol::ProtocolV1Codec::decode_raw_solve_result(
            read_fixture("invalid", fixture_name),
            instance,
            "case-7",
            &error
        );
        REQUIRE_FALSE(result.has_value());
        REQUIRE_FALSE(error.empty());
    }
}

TEST_CASE("protocol v1 checksum ignores agent insertion order", "[protocol]") {
    const auto instance = make_protocol_instance();
    auto reordered = instance;
    skymapf::task::TaskModel task(2, "reordered");
    task.add_agent_entry(2, skymapf::task::SequenceModel{{1, 0}});
    task.add_agent_entry(1, skymapf::task::SequenceModel{{0, 1}});
    reordered.set_task(std::move(task));
    std::string error;

    const auto checksum = skymapf::protocol::ProtocolV1Codec::compute_instance_checksum(
        instance,
        &error
    );
    const auto reordered_checksum =
        skymapf::protocol::ProtocolV1Codec::compute_instance_checksum(
            reordered,
            &error
        );

    REQUIRE(checksum == "fnv1a64:b137da8e33dfeb31");
    REQUIRE(reordered_checksum == checksum);
}

TEST_CASE("protocol v1 serializes a canonical framework-owned run record", "[protocol]") {
    skymapf::benchmark::RunRecord record;
    record.instance_id = 7;
    record.solver_info.name = "fixture";
    record.solver_info.version = "1.0";
    record.solver_status = skymapf::solver::SolveStatus::Success;
    record.result.status = skymapf::solver::SolveStatus::Success;
    record.validation_performed = true;
    record.result.metrics.sum_of_costs = 4;
    record.result.metrics.makespan = 3;
    record.result.metrics.expanded_nodes = 7;
    record.result.metrics.generated_nodes = 11;
    record.random_seed = 42;
    record.wall_time_ms = 12;

    skymapf::protocol::RunRecordV1Context context;
    context.run_id = "run-001";
    context.problem_id = "case-7";
    context.instance_checksum = "fnv1a64:b137da8e33dfeb31";
    context.solver_commit = "abc123";
    context.peak_memory_bytes = 1024;
    context.environment = {{"os", "test"}};
    std::string error;
    const auto document = skymapf::protocol::ProtocolV1Codec::encode_run_record(
        record,
        context,
        &error
    );

    REQUIRE(document.has_value());
    REQUIRE(error.empty());
    const auto actual = nlohmann::json::parse(document->dump());
    const auto expected = read_fixture("valid", "run-record-success.json");
    REQUIRE(actual == expected);
}

TEST_CASE("protocol v1 records rejected solver success as invalid solution", "[protocol]") {
    skymapf::benchmark::RunRecord record;
    record.solver_info.name = "invalid-fixture";
    record.solver_info.version = "1.0";
    record.solver_status = skymapf::solver::SolveStatus::Success;
    record.result.status = skymapf::solver::SolveStatus::Error;
    record.validation_performed = true;
    record.validation.errors.push_back({
        skymapf::solution::ValidationErrorKind::MissingPath,
        2,
        0,
        0,
        0,
        0,
        0,
        "missing agent path"
    });
    skymapf::protocol::RunRecordV1Context context;
    context.run_id = "run-invalid";
    context.problem_id = "case-7";
    context.instance_checksum = "fnv1a64:b137da8e33dfeb31";
    std::string error;

    const auto document = skymapf::protocol::ProtocolV1Codec::encode_run_record(
        record,
        context,
        &error
    );

    REQUIRE(document.has_value());
    REQUIRE((*document)["solver_status"] == "success");
    REQUIRE((*document)["final_status"] == "invalid_solution");
    REQUIRE((*document)["validation"]["performed"] == true);
    REQUIRE((*document)["validation"]["valid"] == false);
    REQUIRE((*document)["validation"]["errors"][0]["code"] == "missing_path");
}

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include <nlohmann/json.hpp>

int main(int argc, char** argv) {
    const std::string mode = argc > 1 ? argv[1] : "success";
    nlohmann::json request;
    try {
        std::cin >> request;
    } catch (const std::exception& ex) {
        std::cerr << "request parse failed: " << ex.what();
        return 2;
    }

    if (request.value("protocol_version", "") != "skymapf.solve-request.v1" ||
        request.at("problem").value("protocol_version", "") != "skymapf.problem.v1" ||
        !request.at("options").contains("time_limit_ms") ||
        !request.at("options").contains("random_seed") ||
        !request.at("options").contains("node_limit")) {
        std::cerr << "unexpected solve request";
        return 3;
    }

    if (mode == "sleep") {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        return 0;
    }
    if (mode == "invalid-json") {
        std::cout << "not-json";
        return 0;
    }
    if (mode == "failure") {
        std::cerr << "fixture failure";
        return 9;
    }

    const auto problem_id = request.at("problem").at("problem_id").get<std::string>();
    nlohmann::json result{
        {"protocol_version", "skymapf.raw-solve-result.v1"},
        {"problem_id", problem_id},
        {"solver", {
            {"name", "external_fixture"},
            {"version", "1.0"},
        }},
        {"status", "success"},
        {"paths", nlohmann::json::array({
            {
                {"agent_id", 1},
                {"cells", nlohmann::json::array({{0, 0}, {0, 1}, {1, 1}, {1, 0}})},
            },
            {
                {"agent_id", 2},
                {"cells", nlohmann::json::array({{1, 0}, {0, 0}})},
            },
        })},
        {"statistics", {
            {"expanded_nodes", 7},
            {"generated_nodes", 11},
        }},
    };
    if (mode == "wrong-id") {
        result["problem_id"] = "wrong";
    }
    std::cout << result.dump();
    return 0;
}

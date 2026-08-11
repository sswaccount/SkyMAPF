#include "skymapf/io/movingai_io.hpp"

#include <fstream>
#include <limits>
#include <sstream>
#include <string>
#include <utility>

#include "skymapf/common/coord.hpp"
#include "skymapf/common/index.hpp"
#include "skymapf/common/space.hpp"
#include "skymapf/generator/strategy/edge_generation_strategy.hpp"
#include "skymapf/task/model.hpp"

namespace skymapf::io {

namespace {

void set_error(std::string* error_message, std::string message) {
    if (error_message) {
        *error_message = std::move(message);
    }
}

void remove_carriage_return(std::string& line) {
    if (!line.empty() && line.back() == '\r') {
        line.pop_back();
    }
}

bool is_walkable_symbol(char symbol) noexcept {
    return symbol == '.' || symbol == 'G' || symbol == 'S';
}

bool is_blocked_symbol(char symbol) noexcept {
    return symbol == '@' || symbol == 'O' || symbol == 'T' || symbol == 'W';
}

}  // namespace

std::optional<world::WorldModel> MovingAIIO::read_map(
    const std::filesystem::path& map_path,
    common::WorldId world_id,
    std::string* error_message
) {
    std::ifstream input(map_path, std::ios::binary);
    if (!input) {
        set_error(error_message, "cannot open MovingAI map: " + map_path.string());
        return std::nullopt;
    }

    std::string map_type;
    std::int32_t width = 0;
    std::int32_t height = 0;
    bool found_map_marker = false;
    std::string line;
    while (std::getline(input, line)) {
        remove_carriage_return(line);
        if (line == "map") {
            found_map_marker = true;
            break;
        }
        std::istringstream header(line);
        std::string key;
        header >> key;
        if (key == "type") {
            header >> map_type;
        } else if (key == "width") {
            header >> width;
        } else if (key == "height") {
            header >> height;
        }
    }

    if (!found_map_marker || map_type.empty() || width <= 0 || height <= 0) {
        set_error(error_message, "invalid MovingAI map header: " + map_path.string());
        return std::nullopt;
    }
    if (map_type != "octile") {
        set_error(error_message, "unsupported MovingAI map type: " + map_type);
        return std::nullopt;
    }

    auto world = world::WorldModel(
        world_id,
        map_path.stem().string(),
        common::SpaceSpec::make_2d(width, height)
    );
    for (std::int32_t y = 0; y < height; ++y) {
        if (!std::getline(input, line)) {
            set_error(error_message, "MovingAI map has fewer rows than declared");
            return std::nullopt;
        }
        remove_carriage_return(line);
        if (line.size() != static_cast<std::size_t>(width)) {
            set_error(error_message, "MovingAI map row width does not match header");
            return std::nullopt;
        }
        for (std::int32_t x = 0; x < width; ++x) {
            const auto symbol = line[static_cast<std::size_t>(x)];
            if (!is_walkable_symbol(symbol) && !is_blocked_symbol(symbol)) {
                set_error(error_message, "MovingAI map contains an unknown terrain symbol");
                return std::nullopt;
            }
            world.set_walkable(common::CellCoord2D{x, y}, is_walkable_symbol(symbol));
        }
    }

    generator::DefaultEdgeGenerationStrategy{}.apply(world);
    world.set_info("Imported from MovingAI map format; source type=" + map_type);
    return world;
}

std::optional<std::vector<MovingAIScenarioEntry>> MovingAIIO::read_scenario(
    const std::filesystem::path& scenario_path,
    std::string* error_message
) {
    std::ifstream input(scenario_path, std::ios::binary);
    if (!input) {
        set_error(error_message, "cannot open MovingAI scenario: " + scenario_path.string());
        return std::nullopt;
    }

    std::string line;
    if (!std::getline(input, line)) {
        set_error(error_message, "MovingAI scenario is empty");
        return std::nullopt;
    }
    remove_carriage_return(line);
    std::istringstream version_line(line);
    std::string version_key;
    std::string version;
    version_line >> version_key >> version;
    if (version_key != "version" || version != "1") {
        set_error(error_message, "unsupported MovingAI scenario version");
        return std::nullopt;
    }

    std::vector<MovingAIScenarioEntry> entries;
    std::size_t line_number = 1;
    while (std::getline(input, line)) {
        ++line_number;
        remove_carriage_return(line);
        if (line.empty()) {
            continue;
        }
        MovingAIScenarioEntry entry;
        std::istringstream row(line);
        if (!(row >> entry.bucket
                  >> entry.map_name
                  >> entry.map_width
                  >> entry.map_height
                  >> entry.start_x
                  >> entry.start_y
                  >> entry.goal_x
                  >> entry.goal_y
                  >> entry.reference_distance)) {
            set_error(
                error_message,
                "invalid MovingAI scenario row at line " + std::to_string(line_number)
            );
            return std::nullopt;
        }
        std::string trailing;
        if (row >> trailing) {
            set_error(
                error_message,
                "unexpected fields in MovingAI scenario at line " +
                    std::to_string(line_number)
            );
            return std::nullopt;
        }
        entries.push_back(std::move(entry));
    }

    if (entries.empty()) {
        set_error(error_message, "MovingAI scenario contains no entries");
        return std::nullopt;
    }
    return entries;
}

std::optional<instance::InstanceModel> MovingAIIO::make_prefix_instance(
    const world::WorldModel& world,
    const std::vector<MovingAIScenarioEntry>& entries,
    std::size_t agent_count,
    common::TaskId task_id,
    common::InstanceId instance_id,
    std::string instance_name,
    std::string* error_message
) {
    if (agent_count == 0 || agent_count > entries.size()) {
        set_error(error_message, "MovingAI prefix agent count is outside scenario range");
        return std::nullopt;
    }
    if (agent_count > static_cast<std::size_t>(std::numeric_limits<common::AgentId>::max())) {
        set_error(error_message, "MovingAI prefix has too many agents");
        return std::nullopt;
    }

    task::TaskModel task(task_id, "movingai-prefix-" + std::to_string(agent_count));
    const auto shape = world.space_spec().shape_2d();
    for (std::size_t index = 0; index < agent_count; ++index) {
        const auto& entry = entries[index];
        if (entry.map_width != shape.cols || entry.map_height != shape.rows) {
            set_error(error_message, "MovingAI scenario dimensions do not match map");
            return std::nullopt;
        }
        const common::CellCoord2D start{entry.start_x, entry.start_y};
        const common::CellCoord2D goal{entry.goal_x, entry.goal_y};
        if (!world.is_walkable(start) || !world.is_walkable(goal)) {
            set_error(error_message, "MovingAI scenario start or goal is invalid or blocked");
            return std::nullopt;
        }
        const auto agent_id = static_cast<common::AgentId>(index + 1);
        task.add_agent_entry(
            agent_id,
            task::SequenceModel{{
                common::to_index(start, shape),
                common::to_index(goal, shape)
            }},
            0,
            task::GoalArrivalBehavior::StayAtGoal
        );
    }

    if (instance_name.empty()) {
        instance_name = "movingai-" + std::to_string(agent_count) + "-agents";
    }
    return instance::InstanceModel::create(
        instance_id,
        world,
        std::move(task),
        std::move(instance_name)
    );
}

std::optional<std::vector<instance::InstanceModel>> MovingAIIO::read_prefix_instances(
    const std::filesystem::path& map_path,
    const std::filesystem::path& scenario_path,
    const std::vector<std::size_t>& agent_counts,
    const MovingAIImportOptions& options,
    std::string* error_message
) {
    if (agent_counts.empty()) {
        set_error(error_message, "MovingAI import requires at least one agent count");
        return std::nullopt;
    }
    auto world = read_map(map_path, options.world_id, error_message);
    if (!world) {
        return std::nullopt;
    }
    auto entries = read_scenario(scenario_path, error_message);
    if (!entries) {
        return std::nullopt;
    }

    std::vector<instance::InstanceModel> instances;
    instances.reserve(agent_counts.size());
    for (std::size_t index = 0; index < agent_counts.size(); ++index) {
        const auto count = agent_counts[index];
        auto instance = make_prefix_instance(
            *world,
            *entries,
            count,
            static_cast<common::TaskId>(options.first_task_id + index),
            static_cast<common::InstanceId>(options.first_instance_id + index),
            scenario_path.stem().string() + "-" + std::to_string(count),
            error_message
        );
        if (!instance) {
            return std::nullopt;
        }
        instances.push_back(std::move(*instance));
    }
    return instances;
}

}  // namespace skymapf::io

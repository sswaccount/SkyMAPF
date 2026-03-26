/**
* @file default_naming.cpp
* @brief Implements default naming helpers for domain objects.
*/

#include "skymapf/utils/default_naming.hpp"
#include "skymapf/utils/random_tool.hpp"

#include <array>
#include <string>

namespace skymapf::utils {

namespace {
NamingConfig g_naming_config{};
}  // namespace

void DefaultNaming::configure(const NamingConfig& config) noexcept {
    g_naming_config = config;
}

const NamingConfig& DefaultNaming::config() noexcept {
    return g_naming_config;
}

std::string DefaultNaming::next_agent_name() {
    return "agent_" + next_suffix(config().agent_suffix_width);
}

std::string DefaultNaming::next_world_name(const common::SpaceSpec& spec) {
    return "world_" + world_shape_tag(spec) + "_"
        + next_suffix(config().world_suffix_width);
}

std::string DefaultNaming::next_task_name(std::size_t agent_size) {
    return "task_" + std::to_string(agent_size) + "_"
        + next_suffix(config().task_suffix_width);
}

std::string DefaultNaming::next_instance_name() {
    return "instance_" + next_suffix(config().instance_suffix_width);
}

std::string DefaultNaming::next_scenario_name() {
    return "scenario_" + next_suffix(config().scenario_suffix_width);
}

std::string DefaultNaming::next_suffix(std::size_t width) {
    const std::uint64_t raw = RandomTool::instance().next_name_value();
    return to_base36(raw, width);
}

std::string DefaultNaming::world_shape_tag(const common::SpaceSpec& spec) {
    if (spec.kind() == common::SpaceKind::Space3D) {
        return "3d_" + std::to_string(spec.cols())
            + "x" + std::to_string(spec.rows())
            + "x" + std::to_string(spec.layers());
    }

    return "2d_" + std::to_string(spec.cols())
        + "x" + std::to_string(spec.rows());
}

std::string DefaultNaming::to_base36(std::uint64_t value, std::size_t width) {
    static constexpr std::array<char, 36> kDigits = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
        'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
        'u', 'v', 'w', 'x', 'y', 'z'
    };

    if (width == 0) {
        return std::string();
    }

    std::string out(width, '0');
    for (std::size_t i = 0; i < width; ++i) {
        out[width - 1 - i] = kDigits[value % 36ULL];
        value /= 36ULL;
    }
    return out;
}

}  // namespace skymapf::common
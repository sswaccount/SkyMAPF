#pragma once

#include <optional>
#include <string>
#include <utility>

#include "skymapf/common/ids.hpp"
#include "skymapf/utils/default_naming.hpp"
#include "skymapf/utils/random_tool.hpp"

namespace skymapf::agent {

/**
 * @brief Input-only static specification for one agent.
 *
 * Missing id/name fields are completed during model materialization.
 */
struct AgentSpec {
    std::optional<common::AgentId> id;
    std::optional<std::string> name;
};

/**
 * @brief Static identity model for one agent.
 *
 * This model only answers "who the agent is".
 * Task demand, route requirement, and runtime execution states are
 * intentionally handled by task/runtime layers.
 */
class AgentModel {
public:
    AgentModel(common::AgentId id, std::string name)
        : id_(id), name_(std::move(name)) {}

    /**
     * @brief Materializes one static agent from spec.
     *
     * - Missing id -> generated via RandomTool::next_id_value().
     * - Missing/empty name -> generated via DefaultNaming::next_agent_name().
     */
    static AgentModel from_spec(const AgentSpec& spec) {
        const auto id = spec.id.value_or(utils::RandomTool::instance().next_id_value());
        auto name = spec.name.value_or(std::string{});
        if (name.empty()) {
            name = utils::DefaultNaming::next_agent_name();
        }
        return AgentModel(id, std::move(name));
    }

    common::AgentId id() const noexcept { return id_; }
    const std::string& name() const noexcept { return name_; }
    bool has_name() const noexcept { return !name_.empty(); }

    void set_name(std::string name) { name_ = std::move(name); }

private:
    common::AgentId id_;
    std::string name_;
};

}  // namespace skymapf::agent
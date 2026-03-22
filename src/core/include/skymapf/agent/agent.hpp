/**
 * @file agent.hpp
 * @brief Defines immutable agent-level planning semantics.
 */
#pragma once

#include <optional>
#include <string>
#include <utility>

#include "skymapf/common/ids.hpp"
#include "skymapf/utils/default_naming.hpp"
#include "skymapf/utils/id_generator.hpp"

namespace skymapf::agent {

/**
 * @brief Input-stage specification for constructing an agent model.
 *
 * Both id and name are optional at input stage and will be completed by builder.
 */
struct AgentSpec {
    std::optional<common::AgentId> id;
    std::optional<std::string> name;
    std::uint64_t naming_seed{0};
};

/**
 * @brief Static agent model with completed identity fields.
 */
class AgentModel {
public:
    AgentModel(
        common::AgentId id = utils::IdGenerator::next_agent_id(),
        std::string name = {}
    )
        : id_(id),
          name_(name.empty() ? utils::DefaultNaming::agent_name(id) : std::move(name)) {}

    const common::AgentId& id() const noexcept { return id_; }
    const std::string& name() const noexcept { return name_; }
    bool has_name() const noexcept { return !name_.empty(); }
    void set_name(std::string name) { name_ = std::move(name); }

private:
    common::AgentId id_{1};
    std::string name_;
};

/**
 * @brief Builder utility that completes id/name from AgentSpec.
 */
class AgentBuilder {
public:
    static AgentModel build(const AgentSpec& spec, std::uint64_t ordinal = 0) {
        const auto id = spec.id.value_or(utils::IdGenerator::next_agent_id());
        const auto name = spec.name.value_or(
            utils::DefaultNaming::agent_name(spec.naming_seed == 0 ? id : spec.naming_seed, ordinal)
        );
        return AgentModel(id, name);
    }
};

}  // namespace skymapf::agent

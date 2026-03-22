/**
 * @file agent.hpp
 * @brief Defines immutable agent-level planning semantics.
 */
#pragma once

#include <string>

#include "skymapf/common/ids.hpp"

namespace skymapf::agent {

/**
 * @brief Describes static attributes of an agent.
 *
 * This structure intentionally excludes time-dependent runtime state.
 */
struct AgentSpec {
    common::AgentId agent_id{0};
    std::string name{""};
};

}  // namespace skymapf::agent

/**
 * @file agent.hpp
 * @brief Defines immutable agent-level planning semantics.
 */
#pragma once

#include <string>

#include "skymapf/common/ids.hpp"

namespace skymapf::agent {

/// Defines occupancy behavior after an agent reaches its goal.
enum class GoalBehavior {
    // Agent disappears from occupancy constraints after reaching goal.
    DisappearAtGoal,
    // Agent keeps occupying goal cell after reaching goal.
    StayAtGoal,
};

/**
 * @brief Describes static attributes of an agent.
 *
 * This structure intentionally excludes time-dependent runtime state.
 */
struct AgentSpec {
    common::AgentId agent_id{0};
    GoalBehavior goal_behavior{GoalBehavior::StayAtGoal};
    std::string name;
};

}  // namespace skymapf::agent

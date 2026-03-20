#pragma once

#include <string>

#include "../common/ids.hpp"

namespace skymapf::agent {

enum class GoalBehavior {
    // Agent disappears from occupancy constraints after reaching goal.
    DisappearAtGoal,
    // Agent keeps occupying goal cell after reaching goal.
    StayAtGoal,
};

struct AgentSpec {
    common::AgentId agent_id{0};
    GoalBehavior goal_behavior{GoalBehavior::StayAtGoal};
    std::string name;
};

}  // namespace skymapf::agent

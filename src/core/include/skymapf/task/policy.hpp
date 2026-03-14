/**
 * @file policy.hpp
 * @brief Defines static execution policies attached to task assignments.
 */
#pragma once

namespace skymapf::task {

/**
 * @brief Defines occupancy behavior after one agent reaches its goal.
 *
 * This is static planning policy metadata and does not represent runtime state.
 */
enum class GoalArrivalBehavior {
    DisappearAtGoal,  ///< Agent is removed from occupancy constraints at goal.
    StayAtGoal,       ///< Agent keeps occupying the goal cell.
};

}  // namespace skymapf::task
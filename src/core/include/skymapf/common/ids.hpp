/**
 * @file ids.hpp
 * @brief Defines domain-specific identifier aliases used across SkyMAPF.
 */
#pragma once

#include <cstdint>

namespace skymapf::common {

/// Unique identifier for a world instance.
using WorldId = std::uint64_t;
/// Unique identifier for a task definition.
using TaskId = std::uint64_t;
/// Unique identifier for a scenario instance.
using ScenarioId = std::uint64_t;
/// Unique identifier for an agent entity.
using AgentId = std::uint64_t;
/// Legacy cell identifier alias.
using CellId = std::uint64_t;
/// Unique identifier for a trace or trajectory record.
using TraceId = std::uint64_t;
/// Linear index of a discrete cell in a world.
using CellIndex = std::uint64_t;
/// Discrete simulation time step.
using TimeStep = std::uint32_t;

}
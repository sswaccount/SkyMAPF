/**
 * @file validation.hpp
 * @brief Declares connectivity validation helpers for world models.
 */
#pragma once

namespace skymapf::world {

class WorldModel;

/// Final status of connectivity validation.
enum class ConnectivityValidationStatus {
    Success,  ///< Exactly one walkable connected component exists.
    Failed,   ///< Connectivity requirement is not satisfied.
};

/**
 * @brief Static validator for world connectivity.
 */
class ConnectivityValidator {
public:
    /**
     * @brief Validates whether the world has exactly one walkable component.
     *
     * @param world World to analyze.
     * @return Success when the world is single-component; Failed otherwise.
     */
    static ConnectivityValidationStatus validate(const WorldModel& world);
};

}  // namespace skymapf::world

/**
 * @file solver_info.hpp
 * @brief Defines solver metadata exposed by implementations.
 */
#pragma once

#include <string>

namespace skymapf::solver {

/// Classifies solver implementation families.
enum class SolverFamily {
    Unknown,
    Classical,
    SearchBased,
    CompilationBased,
    OptimizationBased,
    LearningGuided,
    AI,
};

/// Describes solver metadata exposed to clients.
struct SolverInfo {
    std::string name;
    SolverFamily family{SolverFamily::Unknown};
    std::string version;
    std::string description;
    std::string citation;
};

}  // namespace skymapf::solver

/**
 * @file registry.hpp
 * @brief Declares a simple in-process solver registry.
 */
#pragma once

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "i_solver.hpp"

namespace skymapf::solver {

class SolverRegistry {
public:
    using Creator = std::function<std::unique_ptr<ISolver>()>;

    bool register_solver(std::string name, Creator creator);
    bool is_registered(std::string_view name) const;
    std::unique_ptr<ISolver> create(std::string_view name) const;
    std::vector<std::string> registered_solver_names() const;

private:
    std::unordered_map<std::string, Creator> creators_;
};

}  // namespace skymapf::solver

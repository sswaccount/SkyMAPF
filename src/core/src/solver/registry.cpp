#include "skymapf/solver/registry.hpp"

#include <algorithm>

namespace skymapf::solver {

bool SolverRegistry::register_solver(std::string name, Creator creator) {
    if (name.empty() || !creator) {
        return false;
    }
    return creators_.emplace(std::move(name), std::move(creator)).second;
}

bool SolverRegistry::is_registered(std::string_view name) const {
    return creators_.find(std::string{name}) != creators_.end();
}

std::unique_ptr<ISolver> SolverRegistry::create(std::string_view name) const {
    const auto it = creators_.find(std::string{name});
    if (it == creators_.end()) {
        return nullptr;
    }
    return it->second();
}

std::vector<std::string> SolverRegistry::registered_solver_names() const {
    std::vector<std::string> names;
    names.reserve(creators_.size());
    for (const auto& [name, _] : creators_) {
        names.push_back(name);
    }
    std::sort(names.begin(), names.end());
    return names;
}

}  // namespace skymapf::solver

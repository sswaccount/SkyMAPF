/**
 * @file register_version.cpp
 * @brief Implements CLI registration for version display.
 */
#include "cli/commands/register.hpp"

#include <skymapf/version.hpp>

namespace skymapf::cli {

void register_version(CLI::App& app) {
    app.set_version_flag("-v,--version", skymapf::kVersionString);
}

}  // namespace skymapf::cli
/**
 * @file build_app.cpp
 * @brief Builds the root CLI11 app and registers built-in commands.
 */
#include "cli/app/build_app.hpp"

#include "cli/commands/register.hpp"

namespace skymapf::cli {

std::unique_ptr<CLI::App> build_app() {
    auto app = std::make_unique<CLI::App>("SkyMAPF command line interface");

    register_version(*app);

    return app;
}

}  // namespace skymapf::cli
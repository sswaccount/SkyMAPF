/**
 * @file build_app.hpp
 * @brief Declares the CLI application builder entrypoint.
 */
#pragma once

#include <memory>

#include "CLI/CLI.hpp"

namespace skymapf::cli {

/**
 * @brief Builds the root CLI application for SkyMAPF.
 *
 * The returned application owns all registered subcommands and options
 * for the command line interface.
 *
 * @return A fully initialized CLI11 application instance.
 */
std::unique_ptr<CLI::App> build_app();

}  // namespace skymapf::cli
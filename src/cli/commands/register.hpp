/**
 * @file register.hpp
 * @brief Declares command registration helpers for the CLI application.
 */
#pragma once

#include "CLI/CLI.hpp"

namespace skymapf::cli {

/**
 * @brief Registers the version flag on a CLI11 app.
 *
 * @param app Root CLI application object.
 */
void register_version(CLI::App& app);

}  
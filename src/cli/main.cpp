/**
 * @file main.cpp
 * @brief Program entrypoint for the SkyMAPF command line executable.
 */
#include "cli/app/build_app.hpp"

int main(int argc, char** argv) {
    auto app = skymapf::cli::build_app();
    CLI11_PARSE(*app, argc, argv);
    return 0;
}
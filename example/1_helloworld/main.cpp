/**
 * @file main.cpp
 * @brief Minimal example that prints the linked SkyMAPF version.
 */
#include <iostream>
#include <skymapf/version.hpp>

int main() {
    std::cout << "SkyMAPF version: " << SKYMAPF_VERSION << '\n';
    return 0;
}